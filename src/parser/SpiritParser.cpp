//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iomanip>
#include <istream>
#include <sstream>
#include <iostream>
#include <string>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <boost/spirit/include/classic_position_iterator.hpp>

#include "parser/SpiritParser.hpp"
#include "lexer/SpiritLexer.hpp"

#include "ast/Program.hpp"

namespace qi = boost::spirit::qi;

using namespace eddic;

std::string readI(const std::string& file){
    std::ifstream in(file.c_str());
    in.unsetf(std::ios::skipws);
    std::string storage;
    
    std::copy(
            std::istream_iterator<char>(in),
            std::istream_iterator<char>(),
            std::back_inserter(storage));

    return storage;
}

template <typename Iterator, typename Lexer>
struct EddiGrammar : qi::grammar<Iterator, Program()> {
    EddiGrammar(const Lexer& lexer) : EddiGrammar::base_type(program, "EDDI Grammar") {
        /*
        //Define language rules
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

        primaryValue = constant | tok.word | (tok.left_parenth > value > tok.right_parenth);

        globalDeclaration %= tok.word >> identifier >> tok.assign >> constant;
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

        function %= 
            tok.word >> identifier >> tok.left_parenth  
            >> -( arg >> *( tok.comma >> arg))
            >> tok.right_parenth >> tok.left_brace
            >> *(instruction)
            >> tok.right_brace;
        program %= *( function | (globalDeclaration >> tok.stop)); */

        globalDeclaration %= 
            lexer.word 
            >> lexer.word 
            >> lexer.stop;

        function %= 
            lexer.word >> 
            lexer.word >> 
            lexer.left_parenth > 
            lexer.right_parenth > 
            lexer.left_brace >
            lexer.right_brace;

        program %= lexer.left_brace >> *(globalDeclaration | function) >> lexer.right_brace;

        //Name the rules
        globalDeclaration.name("EDDI global variable");
        function.name("EDDI function declaration");
        program.name("EDDI program");
   }

   qi::rule<Iterator, Program()> program;
   qi::rule<Iterator, GlobalVariableDeclaration()> globalDeclaration;
   qi::rule<Iterator, FunctionDeclaration()> function;
   /*qi::rule<Iterator> arg;

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

   //Utility rules to return values for tokens
   qi::rule<Iterator, std::string()> identifier;*/
};

#include <boost/exception/all.hpp>

bool SpiritParser::parse(const std::string& file, Program& program){
    std::string contents = readI(file);

    pos_iterator_type position_begin(contents.begin(), contents.end(), file);
    pos_iterator_type position_end;

    SimpleLexer<lexer_type> lexer;
    EddiGrammar<lexer_type::iterator_type, SimpleLexer<lexer_type>> grammar(lexer); 

    //In order to avoid a seg fault....
    Program p;
    
    try {
        bool r = lex::tokenize_and_parse(position_begin, position_end, lexer, grammar, program);

        if(r && position_begin == position_end) {
            std::cout << "Parsing passed" << std::endl;
            
            return true;
        } else {
            std::cout << "Parsing failed" << std::endl;
            const spirit::classic::file_position_base<std::string>& pos = position_begin.get_position();
            std::cout <<
                "Error at file " << pos.file << " line " << pos.line << " column " << pos.column << std::endl <<
                "'" << position_begin.get_currentline() << "'" << std::endl <<
                std::setw(pos.column) << " " << "^- here";
            
            return false;
        }
    } catch (const qi::expectation_failure<lexer_type::iterator_type>& exception) {
        std::cout << "Parsing failed" << std::endl;
      
        //TODO Improve to get information from exception 
        const spirit::classic::file_position_base<std::string>& pos = position_begin.get_position();
       
        std::cout <<
            "Error at file " << pos.file << " line " << pos.line << " column " << pos.column << " Expecting " << exception.what_ << std::endl <<
            "'" << position_begin.get_currentline() << "'" << std::endl <<
            std::setw(pos.column) << " " << "^- here" << std::endl;
        
        return false;
    }
}
