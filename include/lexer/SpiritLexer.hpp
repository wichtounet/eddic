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
            /* keywords  */
            for_ = "for";
            while_ = "while";
            do_ = "do";
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

            /* Raw values  */
            identifier = "[a-zA-Z_][a-zA-Z0-9_]*";
            float_ = "[0-9]+\".\"[0-9]+";
            integer = "[0-9]+";
            litteral = "\\\"[^\\\"]*\\\"";

            /* Suffixes  */
            float_suffix = "f";

            /* Constructs  */
            left_parenth = '('; 
            right_parenth = ')'; 
            left_brace = '{'; 
            right_brace = '}'; 
            left_bracket = '['; 
            right_bracket = ']'; 

            stop = ';';
            comma = ',';

            /* Assignment operators */
            swap = "<=>";
            assign = '=';
           
            /* compound assignment operators */ 
            compound_add = "\\+=";
            compound_sub = "-=";
            compound_mul = "\\*=";
            compound_div = "\\/=";
            compound_mod = "%=";

            /* Math operators  */
            addition = '+';
            subtraction = '-';
            multiplication = '*';
            division = '/';
            modulo = '%';

            /* Suffix and prefix math operators  */
            increment = "\\+\\+";
            decrement = "--";

            /* Logical operators */
            and_ = "\\&\\&";
            or_ = "\\|\\|";

            /* Relational operators  */
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
            this->self += assign | swap;
            this->self += compound_add | compound_sub | compound_mul | compound_div | compound_mod;
            this->self += addition | subtraction | multiplication | division | modulo;
            this->self += increment | decrement;
            this->self += and_ | or_;
            this->self += for_ | do_ | while_ | true_ | false_ | if_ | else_ | from_ | to_ | in_ | foreach_ | return_ | const_ | include;
            this->self += equals | not_equals | greater_equals | less_equals | greater | less ;
            this->self += float_ | integer | identifier | litteral;
            this->self += float_suffix;

            //Ignore comments
            this->self += multiline_comment [lex::_pass = lex::pass_flags::pass_ignore]; 
            this->self += singleline_comment [lex::_pass = lex::pass_flags::pass_ignore]; 
        }
      
        typedef lex::token_def<lex::omit> ConsumedToken;
        typedef lex::token_def<std::string> StringToken;
        typedef lex::token_def<int> IntegerToken;
        typedef lex::token_def<char> CharToken;
        typedef lex::token_def<double> FloatToken;

        StringToken identifier, litteral;
        IntegerToken integer;
        FloatToken float_;
        
        CharToken addition, subtraction, multiplication, division, modulo;
        StringToken increment, decrement;
        StringToken compound_add, compound_sub, compound_mul, compound_div, compound_mod;
        StringToken equals, not_equals, greater, less, greater_equals, less_equals;
        StringToken and_, or_;
        StringToken float_suffix;

        ConsumedToken left_parenth, right_parenth, left_brace, right_brace, left_bracket, right_bracket;
        ConsumedToken stop, comma;
        ConsumedToken assign, swap;
        
        //Keywords
        ConsumedToken if_, else_, for_, while_, do_, from_, in_, to_, foreach_, return_;
        ConsumedToken true_, false_;
        ConsumedToken const_, include;

        //Ignored tokens
        ConsumedToken whitespaces, singleline_comment, multiline_comment;
};

typedef std::string::iterator base_iterator_type;
typedef boost::spirit::classic::position_iterator2<base_iterator_type> pos_iterator_type;
typedef boost::spirit::lex::lexertl::token<pos_iterator_type> Tok;
typedef lex::lexertl::actor_lexer<Tok> lexer_type;

//Typedef for the parsers
typedef lexer::lexer_type::iterator_type Iterator;
typedef lexer::SpiritLexer<lexer::lexer_type> Lexer;

} //end of lexer

} //end of eddic

#endif
