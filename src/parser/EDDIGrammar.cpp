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
    
    suffix_op.add
        ("++", ast::Operator::INC)
        ("--", ast::Operator::DEC)
    
    prefix_op.add
        ("++", ast::Operator::INC)
        ("--", ast::Operator::DEC)
   
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
    
    foreachin_ = 
            lexer.foreach_ 
        >>  lexer.left_parenth 
        >>  lexer.word 
        >>  lexer.word 
        >>  lexer.in_ 
        >>  lexer.word 
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

    declaration %= 
            const_
        >>  lexer.word 
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

    prefixOperation %=
            qi::adapttokens[prefix_op]
        >>  lexer.word
        >>  lexer.stop;            

    suffixOperation %=
            lexer.word
        >>  qi::adapttokens[suffix_op]
        >>  lexer.stop;            

    return_ %=
            lexer.return_
        >>  value
        >>  lexer.stop;
    
    arrayAssignment %= 
            lexer.word 
        >>  lexer.left_bracket
        >>  value
        >>  lexer.right_bracket
        >>  lexer.assign 
        >>  value;
    
    globalDeclaration %= 
            const_
        >>  lexer.word 
        >>  lexer.word 
        >>  -(lexer.assign >> value.constant)
        >>  lexer.stop;
    
    globalArrayDeclaration %= 
            lexer.word 
        >>  lexer.word 
        >>  lexer.left_bracket
        >>  lexer.integer
        >>  lexer.right_bracket
        >>  lexer.stop;

    swap %= 
            lexer.word 
        >>  lexer.swap 
        >>  lexer.word;
    
    instruction %= 
            (value.functionCall > lexer.stop)
        |   (assignment > lexer.stop)
        |   (declaration >> lexer.stop)
        |   (arrayDeclaration >> lexer.stop)
        |   (arrayAssignment > lexer.stop)
        |   suffixOperation
        |   prefixOperation
        |   if_
        |   for_
        |   while_
        |   foreach_
        |   foreachin_
        |   return_
        |   (swap > lexer.stop);

    repeatable_instruction = assignment | declaration | swap;
    
    arg %= 
            type 
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

    standardImport %= 
            lexer.include
        >>  lexer.less
        >>  lexer.word
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
