#include "Algorithms.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <list>
#include <random>
#include <stack>
#include <tuple>
#include <unordered_map>


std::vector<QueryGraphNode> JoinTree::get_tree_relations(const JoinTree& tree) {
    std::vector<QueryGraphNode> nodes;
    std::stack<const JoinTree*> trees{{&tree}};
    while (!trees.empty()) {
        const JoinTree* tree = trees.top();
        trees.pop();
        if (tree->is_leaf) {
            nodes.emplace_back(*tree->leaf_node);
        } else {
            trees.push(tree->left_tree.get());
            trees.push(tree->right_tree.get());
        }
    }
    return nodes;
}


double JoinTree::cardinality(const QueryGraph& graph) const {
    if (is_leaf) {
        return leaf_node->cardinality_;
    } else {
        return cardinality(graph, *left_tree, *right_tree);
    }
}


double JoinTree::cardinality(
    const QueryGraph& graph, const JoinTree& left, const JoinTree& right
) {
    const auto relations_left = get_tree_relations(left);
    const auto relations_right = get_tree_relations(right);
    double cardinality = left.cardinality(graph) * right.cardinality(graph);
    for (const auto& node : relations_left) {
        const auto& edges = graph.at(node.relation_.binding).second;
        for (const auto& edge : edges) {
            if (
                    std::find(
                        relations_right.begin(),
                        relations_right.end(),
                        edge.connected_to_
                    ) != relations_right.end()
            ) {
                cardinality *= edge.selectivity_;
            }
        }
    }
    return cardinality;
}


double JoinTree::cost(const QueryGraph& graph) const {
    if (is_leaf) {
        return 0;
    } else {
        return cardinality(graph) + left_tree->cost(graph) + right_tree->cost(graph);
    }
}


double JoinTree::cost(
    const QueryGraph& graph, const JoinTree& left, const JoinTree& right
) {
    return cardinality(graph, left, right) + left.cost(graph) + right.cost(graph);
}


void JoinTree::print_tree_with_costs(const QueryGraph& graph) const {
    std::vector<
        std::tuple<const JoinTree*, ssize_t, ssize_t>
    > reverse_join_order;
    std::stack<std::pair<const JoinTree*, ssize_t>> trees{{std::make_pair(this, -1)}};
    std::unordered_map<ssize_t, const QueryGraphNode*> base_relations;
    while (!trees.empty()) {
        auto top = trees.top();
        const JoinTree* tree = top.first;
        ssize_t parent_index = top.second;
        trees.pop();
        ssize_t index = reverse_join_order.size();
        reverse_join_order.emplace_back(tree, -1, -1);
        if (parent_index != -1) {
            auto& parent_entry = reverse_join_order[parent_index];
            if (std::get<1>(parent_entry) == -1) {
                std::get<1>(parent_entry) = index;
            } else if (std::get<1>(parent_entry) < index) {
                std::get<2>(parent_entry) = std::get<1>(parent_entry);
                std::get<1>(parent_entry) = index;
            } else {
                std::get<2>(parent_entry) = index;
            }
        }
        if (tree->is_leaf) {
            base_relations.emplace(std::make_pair(index, tree->leaf_node));
        } else {
            trees.emplace(tree->left_tree.get(), index);
            trees.emplace(tree->right_tree.get(), index);
        }
    }

    std::unordered_map<ssize_t, size_t> join_numbers;
    size_t size = reverse_join_order.size();
    for (ssize_t i = size - 1; i >= 0; --i) {
        auto join = reverse_join_order[i];
        auto tree = std::get<0>(join);
        if (tree->is_leaf) {
            continue;
        }
        auto join_number = join_numbers.size();
        join_numbers.emplace(i, join_number);
        auto index_left = std::get<1>(join);
        auto index_right = std::get<2>(join);
        if (i == 0) {
            std::cout << "Result";
        } else {
            std::cout << "J" << join_number;
        }
        std::cout << " = ";
        if (base_relations.count(index_left) > 0) {
            auto r = base_relations[index_left]->relation_;
            std::cout << r.name << ' ' << r.binding;
        } else {
            std::cout << "J" << join_numbers[index_left];
        }
        std::cout << " |><| ";
        if (base_relations.count(index_right) > 0) {
            auto r = base_relations[index_right]->relation_;
            std::cout << r.name << ' ' << r.binding;
        } else {
            std::cout << "J" << join_numbers[index_right];
        }
        std::cout << "  Cost: " << tree->cost(graph) << std::endl;
    }
}


JoinTree run_goo(const QueryGraph& graph) {
    std::list<JoinTree> tree;
    // add all relations as separate joins
    for (const auto& entry : graph) {
        tree.emplace_back(entry.second.first);
    }
    while (tree.size() > 1) {
        double min_cost = std::numeric_limits<double>::infinity();
        auto min_left_it = tree.begin();
        auto min_right_it = tree.end();
        --min_right_it;

        for (auto left_it = tree.begin(); left_it != tree.end(); ++left_it) {
            auto right_it = left_it;
            ++right_it;
            for (; right_it != tree.end(); ++right_it) {
                double cost = JoinTree::cost(graph, *left_it, *right_it);
                if (cost < min_cost) {
                    min_cost = cost;
                    min_left_it = left_it;
                    min_right_it = right_it;
                }
            }
        }
        JoinTree new_tree{std::move(*min_left_it), std::move(*min_right_it)};
        tree.erase(min_left_it);
        tree.erase(min_right_it);
        tree.push_back(new_tree);
    }
    return tree.front();
}

bool subset(uint32_t r1, uint32_t r2) {
    uint32_t res = r1 & r2;
    if (!res)
        return false;
    else
        return true;
}

bool crossproduct(const std::unordered_map<uint32_t, QueryGraphNode>& map, uint32_t r1,
        uint32_t r2, const QueryGraph& graph) {
    uint32_t pos1 = 1, pos2 = 1;
    while (r1 != 0)
    {
        unsigned long long bit = r1 & 1;
        if( bit == 1 )
        {
            while (r2 != 0)
            {
                unsigned long long bit2 = r2 & 1;
                if( bit2 == 1 )
                {
                    const auto& edges = graph.at(map.find(pos1)->second.relation_.binding).second;
                    const auto& binding2 = map.find(pos2)->second.relation_.binding;
                    for (const auto& edge : edges) {
                        if (!binding2.compare(edge.connected_to_.relation_.binding)) {
                            return false;
                        }
                    }
                }
                ++pos2;
                r2 >>= 1;
            }
        }
        ++pos1;
        r1 >>= 1;
    }
    return true;
}

void printDPTable(const std::unordered_map<uint32_t, DPEntry>& table,
        const std::unordered_map<uint32_t, QueryGraphNode>& map) {
    for (const auto& entry : table) {
        uint32_t r = entry.first;
        uint32_t pos = 1;
        std::string relations = "{";
        while (r != 0)
        {
            unsigned long long bit = r & 1;
            if( bit == 1 )
            {
                const auto& binding = map.find(pos)->second.relation_.binding;
                relations += binding + ", ";
            }
            ++pos;
            r >>= 1;
        }
        relations = relations.substr(0, relations.size()-2);
        std::cout << "Relations: " << relations << "} | Cost: "
            << entry.second.cost << " | Card: " << entry.second.card << std::endl;
    }

}

void printDPEntry(uint32_t r, DPEntry entry,
    const std::unordered_map<uint32_t, QueryGraphNode>& map) {
    uint32_t pos = 1;
    std::string relations = "{";
    while (r != 0)
    {
        unsigned long long bit = r & 1;
        if( bit == 1 )
        {
            const auto& binding = map.find(pos)->second.relation_.binding;
            relations += binding + ", ";
        }
        ++pos;
        r >>= 1;
    }
    relations = relations.substr(0, relations.size()-2);
    std::cout << "Relations: " << relations << "} | Cost: "
        << entry.cost << " | Card: " << entry.card << std::endl;
}




JoinTree run_dp(const QueryGraph& graph) {
    uint32_t tree_size = 0;
    uint32_t tree_ctr = 1;
    std::unordered_map<uint32_t, QueryGraphNode> map;

    std::unordered_map<uint32_t, DPEntry> table;

    for (const auto& entry : graph) {
        tree_size++;
        map.insert(std::make_pair(tree_size, entry.second.first));

        DPEntry dp_entry { entry.second.first, 0, (double) entry.second.first.cardinality_ };
        table.insert(std::make_pair(tree_ctr, dp_entry));
        tree_ctr = 2 * tree_ctr;
    }

    uint32_t n = 1 << tree_size;

    for (uint32_t i = 1; i < n; ++i) {
        for (uint32_t relation = 1; relation <= i; ++relation) {
            if (!subset(relation, i))
                continue;

            for (uint32_t s1 = 1; s1 <= i; ++s1) {
                if (!subset(relation, s1) && !crossproduct(map, relation, s1, graph)) {
                    auto p1 = table.find(relation);
                    auto p2 = table.find(s1);

                    if (p1 == table.end() || p2 == table.end())
                        continue;

                    double cost = JoinTree::cost(graph, p1->second.plan, p2->second.plan);
                    double current_cost = table.find(s1 + relation) != table.end() ?
                        table.find(s1 + relation)->second.plan.cost(graph) : std::numeric_limits<uint32_t>::max();

                    if (current_cost < cost)
                        continue;

                    JoinTree new_tree {p1->second.plan, p2->second.plan};
                    DPEntry dp_entry { new_tree, cost, new_tree.cardinality(graph)};
                    table.insert(std::make_pair(relation + s1, dp_entry));

                }
            }
        }
#ifdef _ALGO_DEBUG_
        if (table.find(i) != table.end()) {
            printDPEntry(i, table.find(i)->second, map);
        }
#endif

    }

#ifndef _ALGO_DEBUG_
    printDPTable(table, map);
#endif

    std::cout << std::endl << std::endl;

    return table.find(n-1)->second.plan;
}


JoinTree run_quickpick(const QueryGraph& graph, const size_t num_trees) {
    JoinTree minimal_tree;
    double current_cost = std::numeric_limits<double>::infinity();
    std::mt19937 gen;
    for (size_t i = 0; i < num_trees; i++) {
        std::vector<std::string> relations;
        for (const auto& entry : graph) {
            relations.push_back(entry.first);
        }
        std::shuffle(relations.begin(), relations.end(), gen);
        std::unordered_map<std::string, std::shared_ptr<JoinTree>> trees;
        for (const auto& relation : relations) {
            const auto& entry = graph.at(relation);
            const auto& node = entry.first;
            const auto& edges = entry.second;
            auto edge_it = edges.begin();
            // try if node can be connected to existing tree
            for (; edge_it != edges.end(); ++edge_it) {
                auto tree_it = trees.find(edge_it->connected_to_.relation_.binding);
                if (tree_it != trees.end()) {
                    tree_it->second = std::make_shared<JoinTree>(
                        std::move(*tree_it->second), JoinTree{node}
                    );
                    trees[relation] = tree_it->second;
                    goto found_connecting_tree;
                }
            }
            trees[relation] = std::make_shared<JoinTree>(node);
found_connecting_tree:;
            auto& node_tree = trees.at(relation);
            // combine trees for all other found connections
            for (; edge_it != edges.end(); ++edge_it) {
                auto tree_it = trees.find(edge_it->connected_to_.relation_.binding);
                if (tree_it != trees.end()) {
                    node_tree = std::make_shared<JoinTree>(
                        std::move(*node_tree), std::move(*tree_it->second)
                    );
                    tree_it->second = node_tree;
                }
            }
        }
        // combine all remaining trees if the user gave us a cross product
        auto tree_it = trees.begin();
        auto tree = tree_it->second;
        for (; tree_it != trees.end(); ++tree_it) {
            if (tree_it->second != tree) {
                tree = std::make_shared<JoinTree>(
                    std::move(*tree), std::move(*tree_it->second)
                );
                tree_it->second = tree;
            }
        }

        const double cost = tree->cost(graph);
        if (cost < current_cost) {
            current_cost = cost;
            minimal_tree = std::move(*tree);
        }
    }
    return minimal_tree;
}
