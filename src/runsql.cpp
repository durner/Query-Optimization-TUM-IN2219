#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <utility>
#include "Database.hpp"
#include "cts/parser/SQLLexer.hpp"
#include "cts/parser/SQLParser.hpp"
#include "cts/semana/SemanticAnalysis.hpp"
#include "operator/CrossProduct.hpp"
#include "operator/HashJoin.hpp"
#include "operator/Selection.hpp"
#include "operator/Tablescan.hpp"


std::string getQuery(std::ifstream& in){
    std::ostringstream buf;
    std::string line;
    while (true){
        std::getline(in, line);
        buf << line;
        if (!in.good()){
            break;
        }
        buf << std::endl;
    }
    return buf.str();
}


Register make_constant_register(Attribute::Type type, const std::string& value) {
    Register reg;
    switch (type) {
        case Attribute::Type::Int:
            reg.setInt(std::stoi(value));
            break;
        case Attribute::Type::Double:
            reg.setDouble(std::stod(value));
            break;
        case Attribute::Type::Bool:
            reg.setBool(value == "true");
            break;
        case Attribute::Type::String:
            reg.setString(value);
            break;
    }
    return reg;
}


int main(int argc, char* argv[]){
    if (argc < 3){
        std::cerr << "usage: "<<argv[0]<<" <db> <query file> "<< std::endl;
        return 1;
    }

    Database db;
    db.open(argv[1]);
    std::ifstream input(argv[2]);
    std::string query = getQuery(input);
    input.close();

    SQLLexer lexer(query);
    SQLParser parser(lexer);

    try{
        parser.parse();
    }
    catch (std::runtime_error& e){
        std::cout << "exception: "<<e.what() << std::endl;
        return 1;
    }

    SQLParser::Result res = parser.getParserResult();
    try {
        SemanticAnalysis semana(db);
        semana.analyze(res);
    }
    catch (SemanticAnalysis::SemanticError& ex){
        std::cerr << "exception: "<<ex.what() << std::endl;
        return 1;
    }

    std::unordered_map<std::string, SQLParser::Relation> bindings;
    for (const auto& relation : res.relations) {
        bindings.insert(std::make_pair(relation.binding, relation));
    }

    std::unordered_map<
        std::string, std::unordered_map<std::string, const Register*>
    > binding_attr_regs;
    std::unordered_map<std::string, std::unique_ptr<Operator>> binding_scans;

    // prepare scans
    for (const auto& binding : bindings) {
        auto& table = db.getTable(binding.second.name);
        binding_scans.insert(std::make_pair(binding.first, std::make_unique<Tablescan>(table)));
    }

    // load attributes for selections
    for (const auto& selection : res.selections) {
        auto& attr_regs = binding_attr_regs[selection.first.relation];
        if (attr_regs.count(selection.first.name) == 0) {
            auto& scan = binding_scans.at(selection.first.relation);
            attr_regs.insert(std::make_pair(selection.first.name, dynamic_cast<Tablescan&>(*scan).getOutput(selection.first.name)));
        }
    }
    // load attributes for join selections
    for (const auto& join : res.joinConditions) {
        for (const auto& attr : {join.first, join.second}) {
            auto& attr_regs = binding_attr_regs[attr.relation];
            if (attr_regs.count(attr.name) == 0) {
                auto& scan = binding_scans.at(attr.relation);
                attr_regs.insert(std::make_pair(attr.name, dynamic_cast<Tablescan&>(*scan).getOutput(attr.name)));
            }
        }
    }
    // load additional attributes for projections
    for (const auto& projection : res.projections) {
        auto& attr_regs = binding_attr_regs[projection.relation];
        if (attr_regs.count(projection.name) == 0) {
            auto& scan = binding_scans.at(projection.relation);
            attr_regs.insert(std::make_pair(projection.name, dynamic_cast<Tablescan&>(*scan).getOutput(projection.name)));
        }
    }

    // make selections
    std::vector<std::unique_ptr<Register>> const_regs;
    for (const auto& selection : res.selections) {
        auto attr_reg = binding_attr_regs.at(selection.first.relation).at(selection.first.name);
        const_regs.push_back(std::make_unique<Register>(make_constant_register(selection.second.type, selection.second.value)));
        auto& scan = binding_scans.at(selection.first.relation);
        auto sel = std::make_unique<Selection>(std::move(scan), attr_reg, const_regs.back().get());
        scan = std::move(sel);
    }

    // make joins
    std::unique_ptr<Operator> tree = std::move(binding_scans.at(res.relations[0].binding));
    std::unordered_set<std::string> bindings_in_tree;
    bindings_in_tree.insert(res.relations[0].binding);
    decltype(res.joinConditions) additional_join_selections;
    while (!res.joinConditions.empty()) {
        // find suitable join condition
        for (auto iter = res.joinConditions.begin(); iter != res.joinConditions.end();) {
            auto& join_cond = *iter;
            bool first_in_tree = bindings_in_tree.count(join_cond.first.relation) > 0;
            bool second_in_tree = bindings_in_tree.count(join_cond.second.relation) > 0;
            if (first_in_tree || second_in_tree) {
                if (first_in_tree && second_in_tree) {
                    // cycle in joins, run this as selection later
                    additional_join_selections.push_back(join_cond);
                    iter = res.joinConditions.erase(iter);
                    continue;
                }
                auto join_cond_left = first_in_tree ? join_cond.first : join_cond.second;
                auto join_cond_right = first_in_tree ? join_cond.second : join_cond.first;
                auto& right = binding_scans.at(join_cond_right.relation);
                auto reg_left = binding_attr_regs.at(join_cond_left.relation).at(join_cond_left.name);
                auto reg_right = binding_attr_regs.at(join_cond_right.relation).at(join_cond_right.name);
                auto join = std::make_unique<HashJoin>(std::move(tree), std::move(right), reg_left, reg_right);
                tree = std::move(join);
                bindings_in_tree.insert(join_cond_right.relation);
                res.joinConditions.erase(iter);
                goto found_condition;
            }
            ++iter;
        }
        // can't find "good" join condition, run a cross product
        for (const auto& relation : res.relations) {
            if (bindings_in_tree.count(relation.binding) == 0) {
                auto& right = binding_scans.at(relation.binding);
                auto cross = std::make_unique<CrossProduct>(std::move(tree), std::move(right));
                tree = std::move(cross);
                bindings_in_tree.insert(relation.binding);
                break;
            }
        }
        found_condition:;
    }
    // add cross products for all remaining relations
    for (const auto& relation : res.relations) {
        if (bindings_in_tree.count(relation.binding) == 0) {
            auto& right = binding_scans.at(relation.binding);
            auto cross = std::make_unique<CrossProduct>(std::move(tree), std::move(right));
            tree = std::move(cross);
            bindings_in_tree.insert(relation.binding);
        }
    }
    // add cyclic join conditions as selections
    for (const auto& cond : additional_join_selections) {
        auto reg_left = binding_attr_regs.at(cond.first.relation).at(cond.first.name);
        auto reg_right = binding_attr_regs.at(cond.second.relation).at(cond.second.name);
        auto selection = std::make_unique<Selection>(std::move(tree), reg_left, reg_right);
        tree = std::move(selection);
    }

    tree->open();
    while (tree->next()) {
        for (const auto& attr : res.projections) {
            auto reg = binding_attr_regs.at(attr.relation).at(attr.name);
            switch (reg->getState()) {
                case Register::State::Int:
                    std::cout << reg->getInt();
                    break;
                case Register::State::Double:
                    std::cout << reg->getDouble();
                    break;
                case Register::State::Bool:
                    std::cout << reg->getBool();
                    break;
                case Register::State::String:
                    std::cout << reg->getString();
                    break;
                case Register::State::Unbound:
                    std::cout << "NULL";
                    break;
            }
            std::cout << '\t';
        }
        std::cout << std::endl;
    }
    tree->close();

    return 0;
}
