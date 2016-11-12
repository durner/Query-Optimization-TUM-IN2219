#include "Database.hpp"
#include <vector>
#include <string>

namespace parser {

struct Select {
  std::string attribute;
};

struct Relation {
  std::string tablename;
  std::string binding;
};

struct Selection {
  std::string binding;
  std::string attribute;
  std::string constant;
};

struct JoinSelection {
  std::string binding_left;
  std::string attribute_left;
  std::string binding_right;
  std::string attribute_right;
};


class QueryParser {
public:
  QueryParser(std::string database, std::string query);
  bool ParseSQLQuery();

private:
  bool ParseSelects();
  bool ParseRelations();
  bool ParseWhereClause();

  Database db_;
  std::string query_;
  std::vector<Select> selects_;
  std::vector<Relation> relations_;
  std::vector<Selection> selections_;
  std::vector<JoinSelection> join_selections_;
};

} // end ns
