#include "cts/parser/SQLParser.hpp"
#include "Attribute.hpp"

using namespace std;

SQLParser::ParserException::ParserException(const std::string& msg): runtime_error(msg){
}

SQLParser::SQLParser(SQLLexer& lexer): lexer(lexer){
}

SQLParser::~SQLParser(){
}

void SQLParser::parseSelect(){
	SQLLexer::Token token = lexer.getNext();
	if (token!=SQLLexer::Identifier || !lexer.isKeyword("select")){
		throw ParserException("Query should start with SELECT");
	}

	bool emptySelect = true;

	while (true){
		token = lexer.getNext();
		if (token == SQLLexer::Identifier){
			if (lexer.isKeyword("from")){
				lexer.unget(token);
				if (emptySelect)
					throw ParserException("Projection should not be empty");
				break;
			}
			lexer.unget(token);
			RelationAttribute  attr = parseAttributeName();
			result.projections.push_back(attr);
		} else if (token == SQLLexer::Star){
			// we do nothing here
		} else if (token == SQLLexer::Comma){
			if (emptySelect){
				throw ParserException("Syntax error: Comma after empty expression");
			}
		} else {
			if (emptySelect){
				throw ParserException("Projection should not be empty");
			}
			lexer.unget(token);
			break;
		}
		emptySelect = false;
	}
}

void SQLParser::parseFrom(){
	SQLLexer::Token token = lexer.getNext();
	if (token != SQLLexer::Identifier || !lexer.isKeyword("from")){
		throw ParserException("Missing FROM clause");
	}

	bool isRelationParsed = false;
	while (true){
		token = lexer.getNext();
		if (lexer.isKeyword("where")){
			lexer.unget(token);
			break;
		}
		if (token == SQLLexer::Identifier){
			lexer.unget(token);
			Relation rel = parseRelation();
			result.relations.push_back(rel);
			isRelationParsed = true;
		} else if (token == SQLLexer::Comma){
			if (!isRelationParsed){
				throw ParserException("Unexpected Comma in the FROM clause");
			}
			isRelationParsed = false;
		} else {
			throw ParserException("Unexpected token in the FROM clause");
		}
	}
}

SQLParser::Relation SQLParser::parseRelation(){
	SQLLexer::Token token = lexer.getNext();
	if (token != SQLLexer::Identifier){
		throw ParserException("Expected a name of a relation");
	}
	string name = lexer.getTokenValue();
	Relation rel;
	rel.name = name;
	token = lexer.getNext();
	if (token != SQLLexer::Identifier || lexer.isKeyword("where")){
		lexer.unget(token);
		rel.isFullyQualified = false;
	} else {
		rel.binding = lexer.getTokenValue();
		rel.isFullyQualified = true;
	}
	return std::move(rel);
}

SQLParser::RelationAttribute SQLParser::parseAttributeName(){
	SQLLexer::Token token = lexer.getNext();
	if (token != SQLLexer::Identifier){
		throw ParserException("Expected attribute name");
	}

	string name = lexer.getTokenValue();

	token = lexer.getNext();
	if (token == SQLLexer::Dot){
		// relation.attribute
		token = lexer.getNext();
		if (token != SQLLexer::Identifier){
			throw ParserException("Expected attribute name after .");
		}
		string attribute = lexer.getTokenValue();
		RelationAttribute attr{name, attribute, true};
		return std::move(attr);
	} else {
		lexer.unget(token);
		RelationAttribute attr{string(), name, false};
		return std::move(attr);
	}
}

/// Warning: only handles expressions of a form attr1=attr2, or attr = constant
void SQLParser::parseWhere(){
	SQLLexer::Token token = lexer.getNext();
	
	if (token == SQLLexer::Eof)
		return;

	if (token != SQLLexer::Identifier || !lexer.isKeyword("where")){
		throw ParserException("Missing WHERE clause");
	}

	bool isLeftSideReady = false;
	bool isExpressionReady = false;
	bool isJoin = false;
	RelationAttribute attrLeft, attrRight;
	Constant c;

	while (true){
		token = lexer.getNext();
		if (token == SQLLexer::Identifier && lexer.isKeyword("and")){
			if (!isExpressionReady){
				throw ParserException("Unexpected AND");
			}
			isExpressionReady = false;
			isLeftSideReady = false;
		} else if (token == SQLLexer::Identifier){
			lexer.unget(token);
			if (isLeftSideReady){
				attrRight = parseAttributeName();
				isExpressionReady = true;
				isJoin = true;
			} else if (!isExpressionReady){
				attrLeft = parseAttributeName();
				isLeftSideReady = true;
			} else {
				throw ParserException("Error in WHERE clause");
			}
		} else if (token == SQLLexer::Equal) {
			if (!isLeftSideReady || isExpressionReady){
				throw ParserException("Error in WHERE clause");
			}
			continue;
		} else if (token == SQLLexer::String || token == SQLLexer::Integer){
			if (!isLeftSideReady || isExpressionReady){
				throw ParserException("Unexpected String constant in WHERE clause");
			}
			c.type = (token == SQLLexer::String)? Attribute::Type::String : Attribute::Type::Int;
			c.value = lexer.getTokenValue();
			isExpressionReady = true;
			isJoin = false;
		} else if (token == SQLLexer::Eof){
			break;
		} else {
			throw ParserException("Unexpected token: "+lexer.getTokenValue());
		}

		if (isExpressionReady){
			if (isJoin){
				result.joinConditions.push_back({attrLeft, attrRight});
			} else{
				result.selections.push_back({attrLeft, c});
			}
		}
	}

}

void SQLParser::parse(){
	
	parseSelect();
	parseFrom();
	parseWhere();

	if (lexer.getNext()!=SQLLexer::Eof){
		throw ParserException("Syntax error in the query");
	}
}

SQLParser::Result SQLParser::getParserResult(){
	return result;
}


