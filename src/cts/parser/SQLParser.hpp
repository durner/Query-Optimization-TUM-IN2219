#ifndef SQLParser_HPP
#define SQLParser_HPP

#include <unordered_map>
#include <string>
#include <exception>
#include <vector>
#include <utility>
#include "Attribute.hpp"
#include "cts/parser/SQLLexer.hpp"

class SQLLexer;

class SQLParser{
private:
	SQLLexer& lexer;

	void parseSelect();

	void parseFrom();

	void parseWhere();

public:
	class ParserException: public std::runtime_error {
	public:
		ParserException(const std::string& msg);
	};

	explicit SQLParser(SQLLexer& lexer);
	~SQLParser();

	void parse();

	// relation in the FROM clause
	struct Relation{
		// table name
		std::string name; 
		// binding
		std::string binding;
		// both table name and binding are not empty
		bool isFullyQualified; 

		std::string getName(){
			return isFullyQualified? name+ " "+binding: name;
		}
	};

	// attribute of the relations mentioned in SELECT or WHERE
	struct RelationAttribute{
		// relation name 
		std::string relation;
		// name of the attribute
		std::string name;
		// both relation and attribute name are not empty
		bool isFullyQualified;

		std::string getName(){
			return isFullyQualified? relation + "." + name: name;
		}
	};

	// constant mentioned in the condition 
	struct Constant {
		Attribute::Type type;
		std::string value;
	};

	// result of the parser
	struct Result {
		std::vector<Relation> relations;
		std::vector<RelationAttribute> projections;
		std::vector<std::pair<RelationAttribute, Constant>> selections;
		std::vector<std::pair<RelationAttribute, RelationAttribute>> joinConditions;
	};

	Result getParserResult();

private:
	Result result;

	RelationAttribute	parseAttributeName();

	Relation parseRelation();
};

#endif