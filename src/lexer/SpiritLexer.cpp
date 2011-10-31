//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "lexer/SpiritLexer.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

#include <boost/spirit/include/classic_position_iterator.hpp>

#include <iomanip>
#include <istream>
#include <sstream>
#include <iostream>
#include <string>

using std::pair;
using std::string;
using std::ios_base;

using namespace eddic;

namespace qi = boost::spirit::qi;

SpiritLexer::SpiritLexer() : defaultToken(def), first(true) {}

void SpiritLexer::lex(const string& file) {
    //Temporarily do nothing
}

const Tok& SpiritLexer::getDefaultToken() const{
    return defaultToken;
}

bool SpiritLexer::next() {
    if(first){
		first = false;
		return token_is_valid(*iter);
	}

    if(iter == end){
        return false;
    }
	
    ++iter;

    if(!token_is_valid(*iter)){
    	std::cout << "Lexer::invalid_token()" << std::endl;
        return false;
    }
    
    return true;
}

void SpiritLexer::pushBack(const Tok& token) {
	throw "shit";
}

bool SpiritLexer::readNext() {
	throw "shit";
}

const Tok& SpiritLexer::getCurrentToken() const {
    return *iter;
}

bool SpiritLexer::isWord() const {
    return iter->id() == lexer.word.id();
}

bool SpiritLexer::isLitteral() const {
    return iter->id() == lexer.litteral.id();
}

bool SpiritLexer::isAssign() const {
    return iter->id() == lexer.assign.id();
}

bool SpiritLexer::isSwap() const {
    return iter->id() == lexer.swap.id();
}

bool SpiritLexer::isLeftParenth() const {
    return iter->id() == lexer.left_parenth.id();
}

bool SpiritLexer::isRightParenth() const {
    return iter->id() == lexer.right_parenth.id();
}

bool SpiritLexer::isLeftBrace() const {
    return iter->id() == lexer.left_brace.id();
}

bool SpiritLexer::isRightBrace() const {
    return iter->id() == lexer.right_brace.id();
}

bool SpiritLexer::isStop() const {
    return iter->id() == lexer.stop.id();
}

bool SpiritLexer::isInteger() const {
    return iter->id() == lexer.integer.id();
}

bool SpiritLexer::isAddition() const {
    return iter->id() == lexer.addition.id();
}

bool SpiritLexer::isSubtraction() const {
    return iter->id() == lexer.subtraction.id();
}

bool SpiritLexer::isMultiplication() const {
    return iter->id() == lexer.multiplication.id();
}

bool SpiritLexer::isModulo() const {
    return iter->id() == lexer.modulo.id();
}

bool SpiritLexer::isDivision() const {
    return iter->id() == lexer.division.id();
}

bool SpiritLexer::isEquals() const {
    return iter->id() == lexer.equals.id();
}

bool SpiritLexer::isNotEquals() const {
    return iter->id() == lexer.not_equals.id();
}

bool SpiritLexer::isGreater() const {
    return iter->id() == lexer.greater.id();
}

bool SpiritLexer::isLess() const {
    return iter->id() == lexer.less.id();
}

bool SpiritLexer::isGreaterOrEquals() const {
    return iter->id() == lexer.greater_equals.id();
}

bool SpiritLexer::isLessOrEquals() const {
    return iter->id() == lexer.less_equals.id();
}

bool SpiritLexer::isIf() const {
    return iter->id() == lexer.if_.id();
}

bool SpiritLexer::isElse() const {
    return iter->id() == lexer.else_.id();
}

bool SpiritLexer::isWhile() const {
    return iter->id() == lexer.while_.id();
}

bool SpiritLexer::isFor() const {
    return iter->id() == lexer.for_.id();
}

bool SpiritLexer::isForeach() const {
    return iter->id() == lexer.foreach_.id();
}

bool SpiritLexer::isFrom() const {
    return iter->id() == lexer.from_.id();
}

bool SpiritLexer::isTo() const {
    return iter->id() == lexer.to_.id();
}

bool SpiritLexer::isTrue() const {
    return iter->id() == lexer.true_.id();
}

bool SpiritLexer::isFalse() const {
    return iter->id() == lexer.false_.id();
}

bool SpiritLexer::isComma() const {
    return iter->id() == lexer.comma.id();
}
