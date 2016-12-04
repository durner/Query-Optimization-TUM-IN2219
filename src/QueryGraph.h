#include <vector>
#include <string>
#include <unordered_map>
#include "cts/parser/SQLParser.hpp"

struct QueryGraphNode {
    SQLParser::Relation relation_;
    uint64_t cardinality_;
    double selectivity_selections_;
    std::vector<std::pair<SQLParser::RelationAttribute, SQLParser::Constant>> pushed_down_;

    bool operator==(const QueryGraphNode& a) const
    {
        return relation_.binding.compare(a.relation_.binding) == 0;
    }
};

struct QueryGraphEdge {
    QueryGraphNode& connected_to_;
    double selectivity_;
};


namespace std {
  template <> struct hash<QueryGraphNode>
  {
    size_t operator()(const QueryGraphNode & x) const
    {
      return hash<std::string>()(x.relation_.binding);
    }
  };
}
