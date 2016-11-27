#ifndef SemanticAnalysis_HPP
#define SemanticAnalysis_HPP
#include <string>
#include <vector>
#include <exception>
#include <utility>
#include "Attribute.hpp"
#include "Database.hpp"
#include "cts/parser/SQLParser.hpp"

class SemanticAnalysis {
private:
	Database& db;
	// relations in the query
	std::unordered_map<std::string, SQLParser::Relation> relations;
	// check relation in the FROM clause
	void checkRelation(SQLParser::Relation& rel);
	// resolve attribute
	Attribute checkAttribute(SQLParser::RelationAttribute& attr);
	// check selection
	void checkSelection(std::pair<SQLParser::RelationAttribute, SQLParser::Constant>& sel);
	// check join condition
	void checkJoin(std::pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>& join);
public:
	explicit SemanticAnalysis(Database& db);
	~SemanticAnalysis() = default;

	class SemanticError: public std::runtime_error{
	public:
		SemanticError(const std::string& msg);
	};

	// check that all mentioned attributes and relations exist and have compatible types
	void analyze(SQLParser::Result& result);

};

#endif