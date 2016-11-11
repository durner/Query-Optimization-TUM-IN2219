#include "Database.hpp"
#include <list>
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
private:
  void ParseSQLQuery();
  void ParseSelects();
  void ParseRelations();
  void ParseSelection();
  void ParseJoinSelection();

  Database db_;
  std::string query;
  std::list<Select> selects_;
  std::list<Relation> relations_;
  std::list<Selection> selections_;
  std::list<JoinSelection> join_selections_;
};

} // end ns
