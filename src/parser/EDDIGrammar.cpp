//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "parser/EDDIGrammar.hpp"
#include "lexer/adapttokens.hpp"

using namespace eddic;

parser::EddiGrammar::EddiGrammar(const lexer::Lexer& lexer) : 
        EddiGrammar::base_type(program, "EDDI Grammar"), 
        value(lexer), 
        type(lexer){
    
    compound_op.add
        ("+=", ast::Operator::ADD)
        ("-=", ast::Operator::SUB)
        ("/=", ast::Operator::DIV)
        ("*=", ast::Operator::MUL)
        ("%=", ast::Operator::MOD)
        ;
   
    const_ %=
            (lexer.const_ > boost::spirit::attr(true))
        |   boost::spirit::attr(false);
    
    else_if_ %= 
            lexer.else_ 
        >>  lexer.if_ 
        >>  lexer.left_parenth 
        >>  value 
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
        >>  value 
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
        >   -value 
        >   lexer.stop 
        >   -repeatable_instruction 
        >   lexer.right_parenth 
        >   lexer.left_brace
        >   (*instruction)
        >   lexer.right_brace;
    
    foreach_ = 
            lexer.foreach_ 
        >>  lexer.left_parenth 
        >>  lexer.identifier 
        >>  lexer.identifier 
        >>  lexer.from_ 
        >>  lexer.integer 
        >>  lexer.to_ 
        >>  lexer.integer 
        >>  lexer.right_parenth 
        >>  lexer.left_brace 
        >>  *(instruction)
        >>  lexer.right_brace;
    
    foreachin_ = 
            lexer.foreach_ 
        >>  lexer.left_parenth 
        >>  lexer.identifier 
        >>  lexer.identifier 
        >>  lexer.in_ 
        >>  lexer.identifier 
        >>  lexer.right_parenth 
        >>  lexer.left_brace 
        >>  *(instruction)
        >>  lexer.right_brace;
    
    while_ %=
            lexer.while_ 
        >   lexer.left_parenth 
        >   value 
        >   lexer.right_parenth 
        >   lexer.left_brace 
        >   *(instruction)
        >   lexer.right_brace;
    
    do_while_ %=
            lexer.do_ 
        >   lexer.left_brace 
        >   *(instruction)
        >   lexer.right_brace
        >   lexer.while_  
        >   lexer.left_parenth 
        >   value 
        >   lexer.right_parenth
        >   lexer.stop;

    declaration %= 
            const_
        >>  lexer.identifier 
        >>  lexer.identifier 
        >>  -(lexer.assign >> value);
    
    arrayDeclaration %= 
            lexer.identifier 
        >>  lexer.identifier 
        >>  lexer.left_bracket
        >>  lexer.integer
        >>  lexer.right_bracket;
    
    assignment %= 
            lexer.identifier 
        >>  lexer.assign 
        >>  value;

    compound_assignment %=
            lexer.identifier
        >>  qi::adapttokens[compound_op]
        >>  value;

    return_ %=
            lexer.return_
        >>  value
        >>  lexer.stop;
    
    arrayAssignment %= 
            lexer.identifier 
        >>  lexer.left_bracket
        >>  value
        >>  lexer.right_bracket
        >>  lexer.assign 
        >>  value;
    
    globalDeclaration %= 
            const_
        >>  lexer.identifier 
        >>  lexer.identifier 
        >>  -(lexer.assign >> value.constant)
        >>  lexer.stop;
    
    globalArrayDeclaration %= 
            lexer.identifier 
        >>  lexer.identifier 
        >>  lexer.left_bracket
        >>  lexer.integer
        >>  lexer.right_bracket
        >>  lexer.stop;

    swap %= 
            lexer.identifier 
        >>  lexer.swap 
        >>  lexer.identifier;
    
    instruction %= 
            (value.functionCall > lexer.stop)
        |   (assignment > lexer.stop)
        |   (compound_assignment > lexer.stop)
        |   (declaration >> lexer.stop)
        |   (value.suffix_operation > lexer.stop)
        |   (value.prefix_operation > lexer.stop)
        |   (arrayDeclaration >> lexer.stop)
        |   (arrayAssignment > lexer.stop)
        |   if_
        |   for_
        |   while_
        |   do_while_
        |   foreach_
        |   foreachin_
        |   return_
        |   (swap > lexer.stop);

    //TODO Check that to see if it's complete and correct
    repeatable_instruction = 
            assignment 
        |   swap 
        |   compound_assignment
        |   value.suffix_operation
        |   value.prefix_operation
        |   value.functionCall
        |   arrayAssignment;
    
    arg %= 
            type 
        >>  lexer.identifier;
    
    function %= 
            lexer.identifier 
        >>  lexer.identifier
        >>  lexer.left_parenth
        >>  -( arg >> *( lexer.comma > arg))
        >>  lexer.right_parenth
        >>  lexer.left_brace
        >>  *(instruction)
        >>  lexer.right_brace;

    standardImport %= 
            lexer.include
        >>  lexer.less
        >>  lexer.identifier
        >>  lexer.greater;

    import %=
            lexer.include
        >>  lexer.litteral;

    program %=
            qi::eps 
        >>  *(function | globalDeclaration | globalArrayDeclaration | standardImport | import);

    //Name the rules
    globalDeclaration.name("EDDI global variable");
    function.name("EDDI function declaration");
    program.name("EDDI program");
}
