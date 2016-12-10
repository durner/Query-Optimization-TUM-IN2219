#include "GOO.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <list>
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
