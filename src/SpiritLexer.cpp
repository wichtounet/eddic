//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "SpiritLexer.hpp"

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
//namespace phoenix = boost::phoenix;

SpiritLexer::SpiritLexer() : defaultToken(def), first(true) {}

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

struct error_handler_ {
    template <typename, typename, typename>
    struct result { typedef void type; };

    template <typename Iterator>
    void operator()(qi::info const& what, Iterator err_pos, Iterator last) const
    {
        std::cout
            << "Error! Expecting "
            << what                         // what failed?
            << " here: \""
            << std::string(err_pos, last)   // iterators to error-pos, end
            << "\""
            << std::endl
            ;
   }
};

boost::phoenix::function<error_handler_> const error_handler = error_handler_();

template <typename Iterator>
struct EddiGrammar : qi::grammar<Iterator> {
   template <typename TokenDef>
   EddiGrammar(const TokenDef& tok) : EddiGrammar::base_type(program) {
       constant = tok.integer | tok.litteral;
       
       value = additiveValue.alias();
       
       additiveValue = 
            multiplicativeValue 
            >> *(   (tok.addition > multiplicativeValue)
                |   (tok.subtraction > multiplicativeValue)
                );
       
       multiplicativeValue = 
            unaryValue
            >> *(   (tok.multiplication > unaryValue)
                |   (tok.division > unaryValue)
                |   (tok.modulo > unaryValue)
                );

        //TODO Support + - primaryValue
       unaryValue = 
            primaryValue.alias();
            
       //TODO Add an error handler
       //TODO Take care that the parenth are not mixed with function calls
       primaryValue = constant | tok.word | (tok.left_parenth > value > tok.right_parenth);

       globalDeclaration = tok.word >> tok.word >> tok.assign >> constant;
       declaration = tok.word >> tok.word >> tok.assign >> value;
       assignment = tok.word >> tok.assign >> value;
       swap = tok.word >> tok.swap >> tok.word;

       binary_operator = tok.equals | tok.not_equals | tok.greater | tok.less | tok.greater_equals | tok.less_equals;
       condition = (value >> binary_operator >> value) | tok.true_ | tok.false_;

       while_ = 
           tok.while_ >> tok.left_parenth >> condition >> tok.right_parenth >> tok.left_brace 
           >> *(instruction)
           >> tok.right_brace; 

       for_ = 
           tok.for_ >> tok.left_parenth >> -declaration >> tok.stop >> -condition >> tok.stop >> -repeatable_instruction >> tok.right_parenth >> tok.left_brace
           >> (*instruction)
           >> tok.right_brace;

       foreach_ = 
           tok.foreach_ >> tok.left_parenth >> tok.word >> tok.word >> tok.from_ >> tok.integer >> tok.to_ >> tok.integer >> tok.right_parenth >> tok.left_brace 
           >> *(instruction)
           >> tok.right_brace;

       elseif_ = 
           tok.else_ >> tok.if_ >> tok.left_parenth >> condition >> tok.right_parenth >> tok.left_brace
           >> *(instruction)
           >> tok.right_brace;

       else_ = 
           tok.else_ >> tok.left_brace
           >> *(instruction)
           >> tok.right_brace;

       if_ = 
           tok.if_ >> tok.left_parenth >> condition >> tok.right_parenth >> tok.left_brace 
           >> *(instruction) 
           >> tok.right_brace
           >> *(elseif_)
           >> -(else_);

       arg = tok.word >> tok.word;

       functionCall = 
           tok.word >> tok.left_parenth 
           >> -( value >> *( tok.comma >> value))
           >> tok.right_parenth; 

       repeatable_instruction = assignment | declaration | swap;

       instruction = 
           ((assignment | declaration | functionCall | swap) >> tok.stop)
           | if_ | while_ | for_ | foreach_;

       function = 
           tok.word >> tok.word >> tok.left_parenth  
           >> -( arg >> *( tok.comma >> arg))
           >> tok.right_parenth >> tok.left_brace
           >> *(instruction)
           >> tok.right_brace;

       program = *( function | (globalDeclaration >> tok.stop));  

       qi::on_error<qi::fail>(program, error_handler(spirit::_4, spirit::_3, spirit::_2));
   }

   qi::rule<Iterator> program;
   qi::rule<Iterator> globalDeclaration;
   qi::rule<Iterator> function;
   qi::rule<Iterator> arg;

   qi::rule<Iterator> value;
   qi::rule<Iterator> additiveValue;
   qi::rule<Iterator> multiplicativeValue;
   qi::rule<Iterator> unaryValue;
   qi::rule<Iterator> primaryValue;
   qi::rule<Iterator> constant;

   qi::rule<Iterator> instruction;
   qi::rule<Iterator> repeatable_instruction;
   qi::rule<Iterator> declaration;
   qi::rule<Iterator> assignment;
   qi::rule<Iterator> swap;
   qi::rule<Iterator> functionCall;
   qi::rule<Iterator> condition;
   qi::rule<Iterator> binary_operator;

   qi::rule<Iterator> while_;
   qi::rule<Iterator> for_;
   qi::rule<Iterator> foreach_;
   qi::rule<Iterator> if_;
   qi::rule<Iterator> else_;
   qi::rule<Iterator> elseif_;
};

void SpiritLexer::lex(const string& file) {
    std::string contents = readI(file);

    pos_iterator_type position_begin(contents.begin(), contents.end(), file);
    pos_iterator_type position_end;

    SimpleLexer<lexer_type> lexer;
    EddiGrammar<lexer_type::iterator_type> parser (lexer); 

    bool r = lex::tokenize_and_parse(position_begin, position_end, lexer, parser);

    if(r && position_begin == position_end) {
        std::cout << "Parsing passed" << std::endl;
    } else {
        const boost::spirit::classic::file_position_base<std::string>& pos = position_begin.get_position();
        std::stringstream msg;
        msg <<
            "parse error at file " << pos.file <<
            " line " << pos.line << " column " << pos.column << std::endl <<
            "'" << position_begin.get_currentline() << "'" << std::endl <<
            std::setw(pos.column) << " " << "^- here";
        throw std::runtime_error(msg.str());
    }

    /*std::cout << "Token Ids : " << std::endl;

      std::cout << "for = " << lexer.for.id() << std::endl;
      std::cout << "while = " << lexer.while.id() << std::endl;
      std::cout << "if = " << lexer.while.id() << std::endl;
      std::cout << "else = " << lexer.else.id() << std::endl;
      std::cout << "false = " << lexer.false.id() << std::endl;
      std::cout << "true = " << lexer.true.id() << std::endl;
      std::cout << "from = " << lexer.from.id() << std::endl;
      std::cout << "to = " << lexer.foreach.id() << std::endl;
      std::cout << "foreach = " << lexer.foreach.id() << std::endl;

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

      std::cout << "Lexer test" << std::endl;

      while(iter != end){
      if(!token_is_valid(*iter)){
      std::cout << "Invalid" << std::endl;
      break;
      } else {
    //std::cout << "valid (" << iter->id() << ") = [" << iter->value() << "]" << std::endl;
    ++iter;
    }
    }

    if(iter == end){
    std::cout << "Lexical analysis passed" << std::endl;
    } else {
    std::cout << "Lexical analysis failed" << std::endl;
    }*/
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
