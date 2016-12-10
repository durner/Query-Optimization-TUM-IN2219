#include <iostream>
#include <sstream>
#include "Database.hpp"
#include "QueryGraph.hpp"
#include "cts/semana/SemanticAnalysis.hpp"


std::string getQuery(std::ifstream& in) {
  std::ostringstream buf;
  std::string line;
  while (true) {
    std::getline(in, line);
    buf << line;
    if (!in.good()) {
      break;
    }
    buf << std::endl;
  }
  return buf.str();
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "usage: " << argv[0] << " <db> <query file> " << std::endl;
    return 1;
  }

  Database db;
  db.open(argv[1]);
  std::ifstream input(argv[2]);
  std::string query = getQuery(input);
  input.close();

  SQLLexer lexer(query);
  SQLParser parser(lexer);

  try {
    parser.parse();
  } catch (std::runtime_error& e) {
    std::cout << "exception: " << e.what() << std::endl;
    return 1;
  }

  SQLParser::Result res = parser.getParserResult();
  try {
    SemanticAnalysis semana(db);
    semana.analyze(res);
  } catch (SemanticAnalysis::SemanticError& ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
    return 1;
  }

  auto graph = make_query_graph(db, res);

  std::cout << "[";
  for (auto node : graph) {
    std::cout << "\t{binding: " << node.second.first.relation_.binding
              << ", card: " << node.second.first.cardinality_
              << ", selection_slectivity: "
              << node.second.first.selectivity_selections_
              << ", pushed_down_selections: [" << std::endl;

    for (auto pred : node.second.first.pushed_down_) {
      std::cout << "\t\t{attribute: " << pred.first.name
                << ", value: " << pred.second.value << "}," << std::endl;
    }
    std::cout << "\t], neighbors: [" << std::endl;

    for (auto edge : node.second.second) {
      std::cout << "\t\t{partner: " << edge.connected_to_.relation_.binding
                << ", selectivity: " << edge.selectivity_ << ", cross_card: "
                << edge.connected_to_.cardinality_ *
                       node.second.first.cardinality_
                << "}, " << std::endl;
    }
    std::cout << "\t]}," << std::endl;
  }
  std::cout << "]" << std::endl;
}
