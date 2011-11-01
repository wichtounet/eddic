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

#include "lexer/SpiritToken.hpp"

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_functor_parser.hpp>
#include <boost/spirit/include/classic_attribute.hpp>
#include <boost/spirit/include/classic_symbols.hpp>

namespace eddic {

namespace spirit = boost::spirit;
namespace lex = boost::spirit::lex;

typedef lex::lexertl::actor_lexer<Tok> lexer_type;

template<typename L>
class SimpleLexer : public lex::lexer<L> {
    private:

    public:
        SimpleLexer() {
            for_ = "for";
            while_ = "while";
            if_ = "if";
            else_ = "else";
            false_ = "false";
            true_ = "true";
            from_ = "from";
            to_ = "to";
            foreach_ = "foreach";

            word = "[a-zA-Z]+";
            integer = "[0-9]+";
            litteral = "\\\"[^\\\"]*\\\"";

            left_parenth = '('; 
            right_parenth = ')'; 
            left_brace = '{'; 
            right_brace = '}'; 

            stop = ';';
            comma = ',';

            swap = "<=>";
            assign = '=';
            addition = '+';
            subtraction = '-';
            multiplication = '*';
            division = '/';
            modulo = '%';

            equals = "==";
            not_equals = "!=";
            greater = '>';
            less = '<';
            greater_equals = ">=";
            less_equals = "<=";

            whitespaces = "[ \\t\\n]+";
            multiline_comment = "\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/";
            singleline_comment = "\\/\\/[^\n]*";

            //Add keywords
            this->self += for_ | while_ | true_ | false_ | if_ | else_ | from_ | to_ | foreach_;
            this->self += integer | word | litteral;

            this->self += equals | not_equals | greater_equals | less_equals | greater | less ;
            this->self += left_parenth | right_parenth | left_brace | right_brace;
            this->self += comma | stop;
            this->self += assign | swap | addition | subtraction | multiplication | division | modulo;

            //Ignore whitespaces and comments
            this->self += whitespaces [lex::_pass = lex::pass_flags::pass_ignore];
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

        ConsumedToken left_parenth, right_parenth, left_brace, right_brace;
        
        ConsumedToken stop, comma;
        
        ConsumedToken assign, swap;
        ConsumedToken equals, not_equals, greater, less, greater_equals, less_equals;
        
        //Keywords
        ConsumedToken if_, else_, for_, while_, from_, to_, foreach_;
        ConsumedToken true_, false_;

        //Ignored tokens
        ConsumedToken whitespaces, singleline_comment, multiline_comment;
};

class SpiritLexer {
    private:
        std::ifstream stream;
        
        SimpleLexer<lexer_type> lexer;
        
        lexer_type::iterator_type iter;
        lexer_type::iterator_type end;

		Tok def;

        //Tok& current;
        Tok& defaultToken;

		bool first;

        //std::stack<std::reference_wrapper<const Tok>> buffer;

        bool readNext();
	
    public:
        SpiritLexer();

        void lex(const std::string& file);
        bool next();
        void pushBack(const Tok& token);

        const Tok& getCurrentToken() const;
        const Tok& getDefaultToken() const;

        bool isWord() const;
        bool isAssign() const;
        bool isSwap() const;
        bool isLitteral() const;
        bool isLeftParenth() const;
        bool isRightParenth() const;
        bool isLeftBrace() const;
        bool isRightBrace() const;
        bool isStop() const;
        bool isInteger() const;
        bool isAddition() const;
        bool isSubtraction() const;
        bool isMultiplication() const;
        bool isDivision() const;
        bool isModulo() const;
        bool isEquals() const;
        bool isNotEquals() const;
        bool isGreater() const;
        bool isLess() const;
        bool isGreaterOrEquals() const;
        bool isLessOrEquals() const;
        bool isIf() const;
        bool isElse() const;
        bool isWhile() const;
        bool isFor() const;
        bool isForeach() const;
        bool isFrom() const;
        bool isTo() const;
        bool isTrue() const;
        bool isFalse() const;
        bool isComma() const;
    
};

} //end of eddic

#endif
