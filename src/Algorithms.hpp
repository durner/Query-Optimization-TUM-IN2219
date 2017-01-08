#ifndef _GOO_hpp
#define _GOO_hpp

#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include "QueryGraph.hpp"

#define _ALGO_DEBUG_


class JoinTree {
private:
    bool is_leaf;

    const QueryGraphNode* leaf_node;
    std::unique_ptr<JoinTree> left_tree;
    std::unique_ptr<JoinTree> right_tree;

    static std::vector<QueryGraphNode> get_tree_relations(const JoinTree& tree);

public:
    JoinTree(const JoinTree& other)
    : is_leaf(other.is_leaf), leaf_node{nullptr}, left_tree{}, right_tree{} {
        if (is_leaf) {
            leaf_node = other.leaf_node;
        } else {
            left_tree = std::make_unique<JoinTree>(*other.left_tree);
            right_tree = std::make_unique<JoinTree>(*other.right_tree);
        }
    }

    JoinTree(JoinTree&& other)
    : is_leaf(other.is_leaf), leaf_node{other.leaf_node},
      left_tree{std::move(other.left_tree)},
      right_tree{std::move(other.right_tree)} {}

    JoinTree(const QueryGraphNode& node)
    : is_leaf{true}, leaf_node{&node}, left_tree{}, right_tree{} {}

    JoinTree(JoinTree&& left, JoinTree&& right)
    : is_leaf{false}, leaf_node{nullptr},
      left_tree{std::make_unique<JoinTree>(left)},
      right_tree{std::make_unique<JoinTree>(right)} {}

    JoinTree(JoinTree& left, JoinTree& right)
    : is_leaf{false}, leaf_node{nullptr},
      left_tree{std::make_unique<JoinTree>(left)},
      right_tree{std::make_unique<JoinTree>(right)} {}



    double cardinality(const QueryGraph& graph) const;
    static double cardinality(
        const QueryGraph& graph, const JoinTree& left, const JoinTree& right
    );

    double cost(const QueryGraph& graph) const;
    static double cost(
        const QueryGraph& graph, const JoinTree& left, const JoinTree& right
    );

    void print_tree_with_costs(const QueryGraph& graph) const;
};

struct DPEntry {
    JoinTree plan;
    double cost;
    double card;
};


JoinTree run_goo(const QueryGraph& graph);

JoinTree run_dp(const QueryGraph& graph);

void printDPEntry(uint32_t r, DPEntry entry,
    const std::unordered_map<uint32_t, QueryGraphNode>& map);

void printDPTable(const std::unordered_map<uint32_t, DPEntry>& table,
        const std::unordered_map<uint32_t, QueryGraphNode>& map);

bool crossproduct(const std::unordered_map<uint32_t, QueryGraphNode>& map, uint32_t r1,
        uint32_t r2, const QueryGraph& graph);

bool subset(uint32_t r1, uint32_t r2);

#endif
