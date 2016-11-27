#include "cts/semana/SemanticAnalysis.hpp"

using namespace std;
using RelationAttribute = SQLParser::RelationAttribute;
using Constant = SQLParser::Constant;

//---------------------------------------------------------------------------
// Semantic Analysis.
// based on the solution of Florian Walch, 2013
//---------------------------------------------------------------------------

SemanticAnalysis::SemanticAnalysis(Database& db): db(db) { }

SemanticAnalysis::SemanticError::SemanticError(const string& msg): runtime_error(msg) {}

void SemanticAnalysis::analyze(SQLParser::Result& result){
	for (auto& r: result.relations){
		checkRelation(r);
	}

	/// TODO: handle the SELECT * case here
	
	for (auto& attr: result.projections){
		checkAttribute(attr);
	}

	for (auto& join: result.joinConditions){
		checkJoin(join);
	}

	for (auto& sel: result.selections){
		checkSelection(sel);
	}
}

void SemanticAnalysis::checkRelation(SQLParser::Relation& rel){
	string key = rel.isFullyQualified? rel.binding : rel.name;
	if (relations.find(key)!= relations.end()){
		throw SemanticError("Duplicate relation name in the FROM clause: "+ key);
	}
	try{
		db.getTable(rel.name);
	} catch(...){
		throw SemanticError("Table "+rel.name + " does not exist");
	}
	relations[key] = rel;
}

void SemanticAnalysis::checkJoin(std::pair<RelationAttribute, RelationAttribute>& join){
	Attribute left = checkAttribute(join.first);
	Attribute right = checkAttribute(join.second);
	if (left.getType() != right.getType()){
		throw SemanticError("Joining attributes of incompatible type: "+left.getName()+" and "+right.getName());
	}
}

void SemanticAnalysis::checkSelection(std::pair<RelationAttribute, Constant>& sel){
	Attribute left = checkAttribute(sel.first);

	if (left.getType() != sel.second.type){
		throw SemanticError("Attribute "+left.getName()+" and constant "+sel.second.value + " do not have the same type");
	}
}

Attribute SemanticAnalysis::checkAttribute(RelationAttribute& attr){
	Attribute resolvedAttr;
	if (attr.relation.size() == 0){
		// look for an attribute in all relations
		bool found = false;
		for (auto& rel: relations){
      	Table& table = db.getTable(rel.second.name);
      	for (size_t i = 0; i < table.getAttributeCount(); ++i){
      		resolvedAttr = std::move(table.getAttribute(i));
      		if (resolvedAttr.getName() == attr.name) {
          		if (found) {
            		throw SemanticError("Attribute " + attr.name + " is ambiguous");
          		}
          		found = true;
          		//break;
        		}
      	}
		}
		if (!found) {
      	throw SemanticError("Attribute " + attr.name+ " not found in any relation in FROM clause.");
    	}

	} else {
		// find the relation that is mentioned
		auto rel = relations.find(attr.relation);
		if (rel == relations.end()){
			throw SemanticError("Table "+ attr.relation +" does not exist");
		}
		// find the attribute in the relation
		Table& table = db.getTable(rel->second.name);
		int index = table.findAttribute(attr.name);
		if (index == -1){
			throw SemanticError("Attribute "+ attr.name+" does not exist in relation "+ attr.relation);
		}
		resolvedAttr = table.getAttribute(index);
	}

	return std::move(resolvedAttr);
}
