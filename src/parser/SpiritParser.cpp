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

template <typename Iterator, typename AttributeT>
struct Rule {
    typedef typename boost::spirit::qi::rule<Iterator, AttributeT> type;
};

template <typename Iterator, typename Lexer>
struct EddiGrammar : qi::grammar<Iterator, ASTProgram()> {
    EddiGrammar(const Lexer& lexer) : EddiGrammar::base_type(program, "EDDI Grammar") {
        value = additiveValue.alias();
        
        additiveValue %=
                multiplicativeValue
            >>  *(
                    (lexer.addition >> multiplicativeValue)
                |   (lexer.subtraction >> multiplicativeValue)
                );
       
        multiplicativeValue %=
                unaryValue
            >>  *(
                    (lexer.multiplication >> unaryValue)
                |   (lexer.division >> unaryValue)
                |   (lexer.modulo >> unaryValue)
                );
        
        //TODO Support + - primaryValue
        unaryValue = primaryValue.alias();
        
        primaryValue = 
                constant 
            |   variable 
            |   (lexer.left_parenth >> value >> lexer.right_parenth);

        integer %= 
                qi::eps 
            >>  lexer.integer;
       
        variable %= 
                qi::eps
            >>  lexer.word;
        
        litteral %= 
                qi::eps 
            >> lexer.litteral;

        constant %= 
                integer 
            |   litteral;

        true_ %= 
                qi::eps
            >>  lexer.true_;
        
        false_ %= 
                qi::eps
            >>  lexer.false_;

        binary_condition %=
                value
            >>  (
                    lexer.greater_equals
                |   lexer.greater
                |   lexer.less_equals
                |   lexer.less
                |   lexer.not_equals
                |   lexer.equals
                )
            >>  value;

        condition %= 
                true_ 
            |   false_ 
            |   binary_condition;
        
        else_if_ %= 
                lexer.else_ 
            >>  lexer.if_ 
            >>  lexer.left_parenth 
            >>  condition 
            >>  lexer.right_parenth 
            >>  lexer.left_brace
            >>  *(instruction)
            >>  lexer.right_brace;

        else_ %= 
                lexer.else_ 
            >>  lexer.left_brace
            >>  *(instruction)
            >>  lexer.right_brace;

        if_ %= 
                lexer.if_ 
            >>  lexer.left_parenth 
            >>  condition 
            >>  lexer.right_parenth 
            >>  lexer.left_brace 
            >>  *(instruction) 
            >>  lexer.right_brace
            >>  *(else_if_)
            >>  -(else_);
        
        for_ %= 
                lexer.for_ 
            >>  lexer.left_parenth 
            >>  -declaration 
            >>  lexer.stop 
            >>  -condition 
            >>  lexer.stop 
            >>  -repeatable_instruction 
            >>  lexer.right_parenth 
            >>  lexer.left_brace
            >>  (*instruction)
            >>  lexer.right_brace;
        
        foreach_ = 
                lexer.foreach_ 
            >>  lexer.left_parenth 
            >>  lexer.word 
            >>  lexer.word 
            >>  lexer.from_ 
            >>  lexer.integer 
            >>  lexer.to_ 
            >>  lexer.integer 
            >>  lexer.right_parenth 
            >>  lexer.left_brace 
            >>  *(instruction)
            >>  lexer.right_brace;
        
        while_ %=
                lexer.while_ 
            >>  lexer.left_parenth 
            >>  condition 
            >>  lexer.right_parenth 
            >>  lexer.left_brace 
            >>  *(instruction)
            >>  lexer.right_brace;

        declaration %= 
                lexer.word 
            >>  lexer.word 
            >>  lexer.assign 
            >>  value;
        
        assignment %= 
                lexer.word 
            >>  lexer.assign 
            >>  value;
        
        globalDeclaration %= 
                lexer.word 
            >>  lexer.word 
            >>  lexer.assign
            >>  constant
            >>  lexer.stop;

        functionCall %=
                lexer.word
            >>  lexer.left_parenth
            >>  -( value >> *( lexer.comma >> value))
            >>  lexer.right_parenth;
        
        swap %= 
                lexer.word 
            >>  lexer.swap 
            >>  lexer.word;
        
        instruction %= 
                (functionCall > lexer.stop)
            |   (swap > lexer.stop)
            |   (assignment > lexer.stop)
            |   (declaration > lexer.stop)
            |   while_
            |   for_
            |   foreach_
            |   if_;

        repeatable_instruction = assignment | declaration | swap;
        
        arg %= 
                lexer.word 
            >   lexer.word;
        
        function %= 
                lexer.word 
            >>  lexer.word
            >>  lexer.left_parenth
            >>  -( arg >> *( lexer.comma >> arg))
            >>  lexer.right_parenth
            >>  lexer.left_brace
            >>  *(instruction)
            >>  lexer.right_brace;

        program %=
                qi::eps 
            >>  *(globalDeclaration | function);

        //Name the rules
        globalDeclaration.name("EDDI global variable");
        function.name("EDDI function declaration");
        program.name("EDDI program");
   }

   //Rule<Iterator, Deferred<ASTProgram>()>::type program;

   qi::rule<Iterator, ASTProgram()> program;
   qi::rule<Iterator, GlobalVariableDeclaration()> globalDeclaration;
   qi::rule<Iterator, ASTFunctionDeclaration()> function;
   qi::rule<Iterator, FunctionParameter()> arg;
   
   qi::rule<Iterator, ASTInstruction()> instruction;
   qi::rule<Iterator, ASTInstruction()> repeatable_instruction;
   qi::rule<Iterator, ASTSwap()> swap;
   qi::rule<Iterator, ASTFunctionCall()> functionCall;
   qi::rule<Iterator, ASTDeclaration()> declaration;
   qi::rule<Iterator, ASTAssignment()> assignment;
   qi::rule<Iterator, ASTWhile()> while_;
   qi::rule<Iterator, ASTFor()> for_;
   qi::rule<Iterator, ASTForeach()> foreach_;
   qi::rule<Iterator, ASTIf()> if_;
   qi::rule<Iterator, ASTElse()> else_;
   qi::rule<Iterator, ASTElseIf()> else_if_;
   
   qi::rule<Iterator, ASTValue()> value;
   qi::rule<Iterator, ASTValue()> primaryValue;
   qi::rule<Iterator, ASTValue()> unaryValue;
   qi::rule<Iterator, ASTComposedValue()> additiveValue;
   qi::rule<Iterator, ASTComposedValue()> multiplicativeValue;
   qi::rule<Iterator, ASTValue()> constant;
   qi::rule<Iterator, ASTInteger()> integer;
   qi::rule<Iterator, ASTLitteral()> litteral;
   qi::rule<Iterator, ASTVariable()> variable;
   
   qi::rule<Iterator, ASTCondition()> condition;
   qi::rule<Iterator, ASTTrue()> true_;
   qi::rule<Iterator, ASTFalse()> false_;
   qi::rule<Iterator, ASTBinaryCondition()> binary_condition;
};

bool SpiritParser::parse(const std::string& file, ASTProgram& program){
    std::ifstream in(file.c_str());
    in.unsetf(std::ios::skipws);
    
    std::string contents;
    std::copy(
            std::istream_iterator<char>(in),
            std::istream_iterator<char>(),
            std::back_inserter(contents));

    pos_iterator_type position_begin(contents.begin(), contents.end(), file);
    pos_iterator_type position_end;

    SimpleLexer<lexer_type> lexer;
    EddiGrammar<lexer_type::iterator_type, SimpleLexer<lexer_type>> grammar(lexer); 
    
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
