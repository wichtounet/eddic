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
struct EddiGrammar : qi::grammar<Iterator, ast::Program()> {
    EddiGrammar(const Lexer& lexer) : EddiGrammar::base_type(program, "EDDI Grammar") {
        value = additiveValue.alias();
        
        additiveValue %=
                multiplicativeValue
            >>  *(
                    (lexer.addition > multiplicativeValue)
                |   (lexer.subtraction > multiplicativeValue)
                );
       
        multiplicativeValue %=
                unaryValue
            >>  *(
                    (lexer.multiplication > unaryValue)
                |   (lexer.division > unaryValue)
                |   (lexer.modulo > unaryValue)
                );
        
        //TODO Support + - primaryValue
        unaryValue = primaryValue.alias();
        
        primaryValue = 
                constant 
            |   arrayValue
            |   variable 
            |   (lexer.left_parenth >> value > lexer.right_parenth);

        integer %= 
                qi::eps 
            >>  lexer.integer;
       
        variable %= 
                qi::eps
            >>  lexer.word;
       
        arrayValue %=
                lexer.word
            >>  lexer.left_bracket
            >>  value
            >>  lexer.right_bracket;
        
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
            >>   value;

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
            >   lexer.left_parenth 
            >   -declaration 
            >   lexer.stop 
            >   -condition 
            >   lexer.stop 
            >   -repeatable_instruction 
            >   lexer.right_parenth 
            >   lexer.left_brace
            >   (*instruction)
            >   lexer.right_brace;
        
        foreach_ = 
                lexer.foreach_ 
            >   lexer.left_parenth 
            >   lexer.word 
            >   lexer.word 
            >   lexer.from_ 
            >   lexer.integer 
            >   lexer.to_ 
            >   lexer.integer 
            >   lexer.right_parenth 
            >   lexer.left_brace 
            >   *(instruction)
            >   lexer.right_brace;
        
        while_ %=
                lexer.while_ 
            >   lexer.left_parenth 
            >   condition 
            >   lexer.right_parenth 
            >   lexer.left_brace 
            >   *(instruction)
            >   lexer.right_brace;

        declaration %= 
                lexer.word 
            >>  lexer.word 
            >>  -(lexer.assign >> value);
        
        arrayDeclaration %= 
                lexer.word 
            >>  lexer.word 
            >>  lexer.left_bracket
            >>  lexer.integer
            >>  lexer.right_bracket;
        
        assignment %= 
                lexer.word 
            >>  lexer.assign 
            >>  value;
        
        arrayAssignment %= 
                lexer.word 
            >>  lexer.left_bracket
            >>  value
            >>  lexer.right_bracket
            >>  lexer.assign 
            >>  value;
        
        globalDeclaration %= 
                lexer.word 
            >>  lexer.word 
            >>  -(lexer.assign >> constant)
            >>  lexer.stop;
        
        globalArrayDeclaration %= 
                lexer.word 
            >>  lexer.word 
            >>  lexer.left_bracket
            >>  lexer.integer
            >>  lexer.right_bracket
            >>  lexer.stop;

        functionCall %=
                lexer.word
            >>  lexer.left_parenth
            >>  -( value >> *( lexer.comma > value))
            >>  lexer.right_parenth;
        
        swap %= 
                lexer.word 
            >>  lexer.swap 
            >>  lexer.word;
        
        instruction %= 
                (functionCall > lexer.stop)
            |   (assignment > lexer.stop)
            |   (declaration > lexer.stop)
            |   (arrayDeclaration > lexer.stop)
            |   (arrayAssignment > lexer.stop)
            |   if_
            |   for_
            |   while_
            |   foreach_
            |   (swap > lexer.stop);

        repeatable_instruction = assignment | declaration | swap;
        
        arg %= 
                lexer.word 
            >>  lexer.word;
        
        function %= 
                lexer.word 
            >>  lexer.word
            >>  lexer.left_parenth
            >>  -( arg >> *( lexer.comma > arg))
            >>  lexer.right_parenth
            >>  lexer.left_brace
            >>  *(instruction)
            >>  lexer.right_brace;

        program %=
                qi::eps 
            >>  *(function | globalDeclaration | globalArrayDeclaration);

        //Name the rules
        globalDeclaration.name("EDDI global variable");
        function.name("EDDI function declaration");
        program.name("EDDI program");
    }

    qi::rule<Iterator, ast::Program()> program;
    qi::rule<Iterator, ast::GlobalVariableDeclaration()> globalDeclaration;
    qi::rule<Iterator, ast::GlobalArrayDeclaration()> globalArrayDeclaration;
    qi::rule<Iterator, ast::FunctionDeclaration()> function;
    qi::rule<Iterator, ast::FunctionParameter()> arg;

    qi::rule<Iterator, ast::Instruction()> instruction;
    qi::rule<Iterator, ast::Instruction()> repeatable_instruction;
    qi::rule<Iterator, ast::Swap()> swap;
    qi::rule<Iterator, ast::FunctionCall()> functionCall;
    qi::rule<Iterator, ast::VariableDeclaration()> declaration;
    qi::rule<Iterator, ast::ArrayDeclaration()> arrayDeclaration;
    qi::rule<Iterator, ast::Assignment()> assignment;
    qi::rule<Iterator, ast::ArrayAssignment()> arrayAssignment;
    qi::rule<Iterator, ast::While()> while_;
    qi::rule<Iterator, ast::For()> for_;
    qi::rule<Iterator, ast::Foreach()> foreach_;
    qi::rule<Iterator, ast::If()> if_;

    qi::rule<Iterator, ast::Else()> else_;
    qi::rule<Iterator, ast::ElseIf()> else_if_;

    qi::rule<Iterator, ast::Value()> value;
    qi::rule<Iterator, ast::Value()> primaryValue;
    qi::rule<Iterator, ast::Value()> unaryValue;
    qi::rule<Iterator, ast::ComposedValue()> additiveValue;
    qi::rule<Iterator, ast::ComposedValue()> multiplicativeValue;
    qi::rule<Iterator, ast::Value()> constant;
    qi::rule<Iterator, ast::Integer()> integer;
    qi::rule<Iterator, ast::Litteral()> litteral;
    qi::rule<Iterator, ast::VariableValue()> variable;
    qi::rule<Iterator, ast::ArrayValue()> arrayValue;

    qi::rule<Iterator, ast::Condition()> condition;
    qi::rule<Iterator, ast::True()> true_;
    qi::rule<Iterator, ast::False()> false_;
    qi::rule<Iterator, ast::BinaryCondition()> binary_condition;
};

bool SpiritParser::parse(const std::string& file, ast::Program& program){
    std::ifstream in(file.c_str());
    in.unsetf(std::ios::skipws);
   
    in.seekg(0, std::istream::end);
    std::size_t size(static_cast<size_t>(in.tellg()));

    in.seekg(0, std::istream::beg);

    std::string contents(size, 0);
    in.read(&contents[0], size);    

    pos_iterator_type position_begin(contents.begin(), contents.end(), file);
    pos_iterator_type position_end;

    SimpleLexer<lexer_type> lexer;
    EddiGrammar<lexer_type::iterator_type, SimpleLexer<lexer_type>> grammar(lexer); 
    
    try {
        bool r = lex::tokenize_and_parse(position_begin, position_end, lexer, grammar, program);

        if(r && position_begin == position_end) {
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
