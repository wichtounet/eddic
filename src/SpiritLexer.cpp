//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "SpiritLexer.hpp"

#include <boost/spirit/include/lex_lexertl.hpp>

#include <iostream>
#include <string>

using std::pair;
using std::string;
using std::ios_base;

using namespace eddic;

SpiritLexer::SpiritLexer() {}

std::string readI(const std::string& spec){
    std::ifstream in(spec.c_str());
    in.unsetf(std::ios::skipws);
    std::string storage;
    
    std::copy(
            std::istream_iterator<char>(in),
            std::istream_iterator<char>(),
            std::back_inserter(storage));

    return storage;
}

void SpiritLexer::lex(const string& file) {
    std::string contents = readI(file);

    base_iterator_type first = contents.begin();
    base_iterator_type last = contents.end();

    iter = lexer.begin(first, last);
    end = lexer.end();

    #ifdef DEBUG_LEXER

    std::cout << "Token Ids : " << std::endl;

    std::cout << "keyword_for = " << lexer.keyword_for.id() << std::endl;
    std::cout << "keyword_while = " << lexer.keyword_while.id() << std::endl;
    std::cout << "keyword_if = " << lexer.keyword_while.id() << std::endl;
    std::cout << "keyword_else = " << lexer.keyword_else.id() << std::endl;
    std::cout << "keyword_false = " << lexer.keyword_false.id() << std::endl;
    std::cout << "keyword_true = " << lexer.keyword_true.id() << std::endl;
    std::cout << "keyword_from = " << lexer.keyword_from.id() << std::endl;
    std::cout << "keyword_to = " << lexer.keyword_foreach.id() << std::endl;
    std::cout << "keyword_foreach = " << lexer.keyword_foreach.id() << std::endl;

    std::cout << "word = " << lexer.word.id() << std::endl;
    std::cout << "integer = " << lexer.integer.id() << std::endl;
    std::cout << "litteral = " << lexer.litteral.id() << std::endl;

    std::cout << "left_parenth = " << lexer.left_parenth.id() << std::endl;
    std::cout << "right_parenth = " << lexer.right_parenth.id() << std::endl;
    std::cout << "left_brace = " << lexer.left_brace.id() << std::endl;
    std::cout << "right_brace = " << lexer.right_brace.id() << std::endl;

    std::cout << "stop = " << lexer.stop.id() << std::endl;
    std::cout << "comma = " << lexer.comma.id() << std::endl;

    std::cout << "assign = " << lexer.assign.id() << std::endl;
    std::cout << "swap = " << lexer.swap.id() << std::endl;
    std::cout << "addition = " << lexer.addition.id() << std::endl;
    std::cout << "subtraction = " << lexer.subtraction.id() << std::endl;
    std::cout << "multiplication = " << lexer.multiplication.id() << std::endl;
    std::cout << "division = " << lexer.division.id() << std::endl;
    std::cout << "modulo = " << lexer.modulo.id() << std::endl;

    std::cout << "equals = " << lexer.equals.id() << std::endl;
    std::cout << "not_equals = " << lexer.not_equals.id() << std::endl;
    std::cout << "greater = " << lexer.greater.id() << std::endl;
    std::cout << "less = " << lexer.less.id() << std::endl;
    std::cout << "greater_equals = " << lexer.greater_equals.id() << std::endl;
    std::cout << "less_equals = " << lexer.less_equals.id() << std::endl;

    std::cout << "whitespaces = " << lexer.whitespaces.id() << std::endl;
    std::cout << "comments = " << lexer.comments.id() << std::endl;

    #endif
}

const Tok& SpiritLexer::getDefaultToken() const{
    return defaultToken;
}

bool SpiritLexer::next() {
    if (!buffer.empty()) {
        current = buffer.top();

        buffer.pop();

        return true;
    } else if (readNext()) {
        return true;
    }

    return false;
}

void SpiritLexer::pushBack(Tok token) {
    buffer.push(token);
}

bool SpiritLexer::readNext() {
    if(iter == end){
        return false;
    }

    if(!token_is_valid(*iter)){
        //TODO Throw error
        return false;
    }

    current = *iter;

    ++iter;

    return true;
}

Tok SpiritLexer::getCurrentToken() const {
    return current;
}

bool SpiritLexer::isWord() const {
    return current.id() == lexer.word.id();
}

bool SpiritLexer::isLitteral() const {
    return current.id() == lexer.litteral.id();
}

bool SpiritLexer::isAssign() const {
    return current.id() == lexer.assign.id();
}

bool SpiritLexer::isSwap() const {
    return current.id() == lexer.swap.id();
}

bool SpiritLexer::isLeftParenth() const {
    return current.id() == lexer.left_parenth.id();
}

bool SpiritLexer::isRightParenth() const {
    return current.id() == lexer.right_parenth.id();
}

bool SpiritLexer::isLeftBrace() const {
    return current.id() == lexer.left_brace.id();
}

bool SpiritLexer::isRightBrace() const {
    return current.id() == lexer.right_brace.id();
}

bool SpiritLexer::isStop() const {
    return current.id() == lexer.stop.id();
}

bool SpiritLexer::isInteger() const {
    return current.id() == lexer.integer.id();
}

bool SpiritLexer::isAddition() const {
    return current.id() == lexer.addition.id();
}

bool SpiritLexer::isSubtraction() const {
    return current.id() == lexer.subtraction.id();
}

bool SpiritLexer::isMultiplication() const {
    return current.id() == lexer.multiplication.id();
}

bool SpiritLexer::isModulo() const {
    return current.id() == lexer.modulo.id();
}

bool SpiritLexer::isDivision() const {
    return current.id() == lexer.division.id();
}

bool SpiritLexer::isEquals() const {
    return current.id() == lexer.equals.id();
}

bool SpiritLexer::isNotEquals() const {
    return current.id() == lexer.not_equals.id();
}

bool SpiritLexer::isGreater() const {
    return current.id() == lexer.greater.id();
}

bool SpiritLexer::isLess() const {
    return current.id() == lexer.less.id();
}

bool SpiritLexer::isGreaterOrEquals() const {
    return current.id() == lexer.greater_equals.id();
}

bool SpiritLexer::isLessOrEquals() const {
    return current.id() == lexer.less_equals.id();
}

bool SpiritLexer::isIf() const {
    return current.id() == lexer.keyword_if.id();
}

bool SpiritLexer::isElse() const {
    return current.id() == lexer.keyword_else.id();
}

bool SpiritLexer::isWhile() const {
    return current.id() == lexer.keyword_while.id();
}

bool SpiritLexer::isFor() const {
    return current.id() == lexer.keyword_for.id();
}

bool SpiritLexer::isForeach() const {
    return current.id() == lexer.keyword_foreach.id();
}

bool SpiritLexer::isFrom() const {
    return current.id() == lexer.keyword_from.id();
}

bool SpiritLexer::isTo() const {
    return current.id() == lexer.keyword_to.id();
}

bool SpiritLexer::isTrue() const {
    return current.id() == lexer.keyword_true.id();
}

bool SpiritLexer::isFalse() const {
    return current.id() == lexer.keyword_false.id();
}

bool SpiritLexer::isComma() const {
    return current.id() == lexer.comma.id();
}
