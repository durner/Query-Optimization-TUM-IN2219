#include "QueryParser.hpp"
#include <algorithm>

namespace parser {

static inline std::vector<std::string> Split(const std::string& text,
                                             const std::string& sep) {
  std::vector<std::string> tokens;
  std::size_t start = 0, end = 0;
  while ((end = text.find(sep, start)) != std::string::npos) {
    tokens.push_back(text.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(text.substr(start));
  return tokens;
}

static inline void RemoveWhiteSpaces(std::string& string) {
  string.erase(remove_if(string.begin(), string.end(), isspace), string.end());
}

QueryParser::QueryParser(std::string database, std::string query) {
  query_ = query;
  db_.open(database);
}

bool QueryParser::ParseSQLQuery() {
  bool sementically_correct = true;
  std::string query_copy = query_;

  // removing unneccessary whitespace
  query_.clear();
  unique_copy(query_copy.begin(), query_copy.end(),
              std::back_insert_iterator<std::string>(query_),
              [](char a, char b) { return isspace(a) && isspace(b); });

  if (!ParseSelects())
    return false;
  if (!ParseRelations())
    return false;
  if (!ParseWhereClause())
    return false;

  try {
    auto selects = selects_;
    for (auto r : relations_) {
      if (!db_.hasTable(r.tablename))
        return false;

      Table t = db_.getTable(r.tablename);
      for (auto it = selects.begin(); it != selects.end();) {
        if (t.findAttribute(it->attribute) != -1) {
          selects.erase(it);
        } else
          it++;
      }
      for (auto s : selections_) {
        if (s.binding == r.binding) {
          if (t.findAttribute(s.attribute) == -1)
            return false;
        }
      }
      for (auto j : join_selections_) {
        if (j.binding_left == r.binding) {
          if (t.findAttribute(j.attribute_left) == -1)
            return false;
        }
        if (j.binding_right == r.binding) {
          if (t.findAttribute(j.attribute_right) == -1)
            return false;
        }
      }
    }
    if (selects.size() > 0 && selects.at(0).attribute != "*")
      return false;

    for (auto s : selections_) {
      bool found = false;
      for (auto r : relations_) {
        if (s.binding == r.binding) {
          found = true;
        }
      }
      if (!found)
        return false;
    }
    for (auto j : join_selections_) {
      bool found = false;
      for (auto r : relations_) {
        if (j.binding_right == r.binding) {
          found = true;
        }
      }
      if (!found)
        return false;

      found = false;
      for (auto r : relations_) {
        if (j.binding_left == r.binding) {
          found = true;
        }
      }
      if (!found)
        return false;
    }

  } catch (...) {
    std::cout << "General Error" << std::endl;
    return false;
  }

  return sementically_correct;
}

bool QueryParser::ParseWhereClause() {
  std::string::size_type pos = query_.find("where");
  if (pos == std::string::npos)
    return false;
  pos += 6;
  if (pos >= query_.size())
    return false;

  std::string where = query_.substr(pos, query_.size() - pos);
  std::vector<std::string> t_where = Split(where, "and");

  std::vector<std::string> t_where_equals, t_where_right, t_where_left;
  for (auto w : t_where) {
    t_where_equals = Split(w, "=");

    // if empty where or one side empty abort
    if (t_where_equals.size() != 2 || t_where_equals.at(0).size() == 0 ||
        t_where_equals.at(1).size() == 0)
      return false;

    // right side needs to be binding.attribute
    t_where_left = Split(t_where_equals.at(0), ".");
    if (t_where_left.size() != 2 || t_where_left.at(0).size() == 0 ||
        t_where_left.at(1).size() == 0)
      return false;
    RemoveWhiteSpaces(t_where_left.at(0));
    RemoveWhiteSpaces(t_where_left.at(1));

    // left side can be either binding.attribute or constant
    bool join_predicate;
    t_where_right = Split(t_where_equals.at(1), ".");
    // if we have a constant we are not allowed to have 1 dot. TODO floats,
    // strings, ...
    // also no keywords allowed in tablename, attribute, constant (not sure if
    // this is an important issue/limitation)
    if (t_where_right.size() != 2) {
      join_predicate = false;
    } else if (t_where_right.at(0).size() == 0 ||
               t_where_right.at(1).size() == 0)
      return false;
    else {
      join_predicate = true;
      RemoveWhiteSpaces(t_where_right.at(0));
      RemoveWhiteSpaces(t_where_right.at(1));
    }

    if (join_predicate) {
      join_selections_.push_back(
          JoinSelection{t_where_left.at(0), t_where_left.at(1),
                        t_where_right.at(0), t_where_right.at(1)});
    } else {
      selections_.push_back(Selection{t_where_left.at(0), t_where_left.at(1),
                                      t_where_equals.at(1)});
    }
  }
  return true;
}

bool QueryParser::ParseRelations() {
  std::string::size_type pos = query_.find("from");
  if (pos == std::string::npos)
    return false;
  pos += 5;

  // definition requires where clause
  std::string::size_type spos = query_.find("where", pos);
  if (spos == std::string::npos)
    return false;

  std::string relations = query_.substr(pos, spos - pos);
  std::vector<std::string> t_relations = Split(relations, ",");

  std::vector<std::string> t_relation_binding;
  for (auto r : t_relations) {
    t_relation_binding = Split(r, " ");

    // if empty relation or binding abort which also handles the empty case
    // can be more than two after split because of whitespace before where!
    if (t_relation_binding.size() < 2 || t_relation_binding.at(0).size() == 0 ||
        t_relation_binding.at(1).size() == 0)
      return false;

    relations_.push_back(
        Relation{t_relation_binding.at(0), t_relation_binding.at(1)});
  }
  return true;
}

bool QueryParser::ParseSelects() {
  std::string::size_type pos = query_.find("select");
  if (pos == std::string::npos)
    return false;
  pos += 7;

  // check if there is a star
  std::string::size_type spos = query_.find("*", pos);
  // if not look for the attributes
  if (spos != pos) {
    // definition requires from
    spos = query_.find("from", pos);
    if (spos == std::string::npos)
      return false;

    std::string attributes = query_.substr(pos, spos - pos);
    RemoveWhiteSpaces(attributes);

    // no attribute at all
    if (!attributes.size())
      return false;

    std::vector<std::string> t_attributes = Split(attributes, ",");
    for (auto s : t_attributes)
      selects_.push_back(Select{s});
  } else {
    spos = query_.find("from", pos);
    std::string attributes = query_.substr(pos + 1, spos - (pos + 1));
    RemoveWhiteSpaces(attributes);

    // more attributes after star
    if (attributes.size())
      return false;

    selects_.push_back(Select{"*"});
  }

  return true;
}

};  // ns
