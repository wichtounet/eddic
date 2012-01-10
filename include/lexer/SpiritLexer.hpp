//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SPIRIT_LEXER_H
#define SPIRIT_LEXER_H

#include <fstream>
#include <string>
#include <utility>
#include <stack>

#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_functor_parser.hpp>
#include <boost/spirit/include/classic_attribute.hpp>
#include <boost/spirit/include/classic_symbols.hpp>

namespace eddic {

namespace lexer {

namespace spirit = boost::spirit;
namespace lex = boost::spirit::lex;
    
typedef std::string::iterator base_iterator_type;
typedef boost::spirit::classic::position_iterator2<base_iterator_type> pos_iterator_type;
typedef boost::spirit::lex::lexertl::token<pos_iterator_type> Tok;
typedef lex::lexertl::actor_lexer<Tok> lexer_type;

/*!
 * \class SimpleLexer
 * \brief The EDDI lexer. 
 *
 * This class is used to do lexical analysis on an EDDI source file. This file is based on a Boost Spirit Lexer. It's 
 * used by the parser to parse a source file. 
 */
template<typename L>
class SpiritLexer : public lex::lexer<L> {
    public:
        SpiritLexer() {
            //Define keywords
            for_ = "for";
            while_ = "while";
            if_ = "if";
            else_ = "else";
            false_ = "false";
            true_ = "true";
            from_ = "from";
            to_ = "to";
            foreach_ = "foreach";
            in_ = "in";
            return_ = "return";
            const_ = "const";
            include = "include";

            word = "[a-zA-Z]+";
            integer = "[0-9]+";
            litteral = "\\\"[^\\\"]*\\\"";

            left_parenth = '('; 
            right_parenth = ')'; 
            left_brace = '{'; 
            right_brace = '}'; 
            left_bracket = '['; 
            right_bracket = ']'; 

            stop = ';';
            comma = ',';

            swap = "<=>";
            assign = '=';
            addition = '+';
            subtraction = '-';
            multiplication = '*';
            division = '/';
            modulo = '%';

            //Bool operators
            and_ = "\\&\\&";
            or_ = "\\|\\|";

            equals = "==";
            not_equals = "!=";
            greater = ">";
            less = "<";
            greater_equals = ">=";
            less_equals = "<=";

            whitespaces = "[ \\t\\n]+";
            multiline_comment = "\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/";
            singleline_comment = "\\/\\/[^\n]*";
            
            //Ignore whitespaces
            this->self += whitespaces [lex::_pass = lex::pass_flags::pass_ignore];

            this->self += left_parenth | right_parenth | left_brace | right_brace | left_bracket | right_bracket;
            this->self += comma | stop;
            this->self += assign | swap | addition | subtraction | multiplication | division | modulo;
            this->self += and_ | or_;
            this->self += for_ | while_ | true_ | false_ | if_ | else_ | from_ | to_ | in_ | foreach_ | return_ | const_ | include;
            this->self += equals | not_equals | greater_equals | less_equals | greater | less ;
            this->self += integer | word | litteral;

            //Ignore comments
            this->self += multiline_comment [lex::_pass = lex::pass_flags::pass_ignore]; 
            this->self += singleline_comment [lex::_pass = lex::pass_flags::pass_ignore]; 
        }
      
        typedef lex::token_def<lex::omit> ConsumedToken;
        typedef lex::token_def<std::string> StringToken;
        typedef lex::token_def<int> IntegerToken;
        typedef lex::token_def<char> CharToken;

        StringToken word, litteral;
        IntegerToken integer;
        
        CharToken addition, subtraction, multiplication, division, modulo;
        StringToken equals, not_equals, greater, less, greater_equals, less_equals;
        StringToken and_, or_;

        ConsumedToken left_parenth, right_parenth, left_brace, right_brace, left_bracket, right_bracket;
        ConsumedToken stop, comma;
        ConsumedToken assign, swap;
        
        //Keywords
        ConsumedToken if_, else_, for_, while_, from_, in_, to_, foreach_, return_;
        ConsumedToken true_, false_;
        ConsumedToken const_, include;

        //Ignored tokens
        ConsumedToken whitespaces, singleline_comment, multiline_comment;
};

//Typedef for the parsers
typedef lexer::lexer_type::iterator_type Iterator;
typedef lexer::SpiritLexer<lexer::lexer_type> Lexer;

} //end of lexer

} //end of eddic

#endif
