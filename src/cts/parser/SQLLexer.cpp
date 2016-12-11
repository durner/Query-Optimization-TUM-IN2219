#include <string>
#include <iostream>
#include <cctype>
#include "cts/parser/SQLLexer.hpp"

using namespace std;

SQLLexer::SQLLexer(const string& input): 
		input(input), pos(this->input.begin()), tokenStart(pos), 
		tokenEnd(pos), hasTokenEnd(false), putBack(None) {
}

SQLLexer::~SQLLexer(){
}

SQLLexer::Token SQLLexer::getNext(){
	if (putBack != None){
		Token result = putBack;
		putBack = None;
		return result;
	}
	
	hasTokenEnd = false;

	while (pos != input.end()){
		if (isspace(*pos)){
			++pos;
			continue;
		}
		tokenStart = pos;
		switch(*pos++){
			case '.': return Dot;
			case '+': return Plus;
         case '-': return Minus;
         case '*': return Star;
         case '=': return Equal;
         case ',': return Comma;
         case '<':
         	if ((pos==input.end())||((*pos)!='>'))
               return Error;
            ++pos;
            return NotEqual;
         case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            while (pos!=input.end()) {
               char c=*pos;
               if (isdigit(c)) {
                 ++pos;
               } else break;
            }
            return Integer;
         case '\'':
         	// String constant. No linebreaks inside
            tokenStart=pos;
            while (pos!=input.end()) {
               if ((*pos)=='\'')
                  break;
               ++pos;
            }
            tokenEnd = pos; hasTokenEnd = true;
            if (pos!=input.end()) ++pos;
            return String;
			case '\"':
			   // String constant. No linebreaks inside
            tokenStart=pos;
            while (pos!=input.end()) {
               if ((*pos)=='\"')
                  break;
               ++pos;
            }
            tokenEnd = pos; hasTokenEnd = true;
            if (pos!=input.end()) ++pos;
            return String;

         // Identifier
         default:
            --pos;
            while (pos!=input.end()) {
               char c=*pos;
               if (isalnum(c) || c == '_') {
                  ++pos;
               } else break;
            }
            if (pos==tokenStart)
               return Error;
            return Identifier;

		}
		cout << *pos << endl;
	}
	return Eof;
}

string SQLLexer::getTokenValue() const{
	if (hasTokenEnd)
		return string(tokenStart, tokenEnd);
	return string(tokenStart, pos);
}

bool SQLLexer::isKeyword(const char* keyword) const{
	std::string::const_iterator iter=tokenStart,limit=hasTokenEnd?tokenEnd:pos;

	while (iter != limit){
		char c1 = tolower(*iter);
		char c2 = tolower(*keyword);
		if (c1 != c2)
			return false;
		if (!*keyword)
			return false;
		++iter; ++keyword;
	}
	return !(*keyword);
}

void SQLLexer::unget(Token value){
	putBack = value;
}

bool SQLLexer::hasNext(){
	Token peek=getNext();
   unget(peek);
   return (peek != Eof && peek != None && peek != Error);
}

string::const_iterator SQLLexer::getReader() const {
	if (putBack != None)
		return tokenStart;
	return pos;
}
