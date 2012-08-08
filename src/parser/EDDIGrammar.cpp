//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "parser/EDDIGrammar.hpp"
#include "parser/Utils.hpp"

#include "lexer/adapttokens.hpp"
#include "lexer/position.hpp"

using namespace eddic;

parser::EddiGrammar::EddiGrammar(const lexer::Lexer& lexer, const lexer::pos_iterator_type& position_begin) : 
        EddiGrammar::base_type(program, "EDDI Grammar"), 
        value(lexer, position_begin), 
        type(lexer, position_begin),
        position_begin(position_begin){

    delete_ %=
            qi::position(position_begin)
        >>  lexer.delete_
        >>  lexer.identifier;
    
    default_case %=
            qi::eps
        >>  lexer.default_
        >>  lexer.double_dot
        >>  *(instruction);

    switch_case %=
            qi::position(position_begin)
        >>  lexer.case_
        >>  value
        >>  lexer.double_dot
        >>  (*instruction);

    switch_ %=
            qi::position(position_begin)
        >>  lexer.switch_
        >>  lexer.left_parenth
        >>  value
        >>  lexer.right_parenth
        >>  lexer.left_brace
        >>  *(switch_case)
        >>  -(default_case)
        >>  lexer.right_brace
            ;

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
            qi::position(position_begin)
        >>  lexer.foreach_ 
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
            qi::position(position_begin)
        >>  lexer.foreach_ 
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
    
    struct_declaration %= 
            qi::position(position_begin)
        >>  type 
        >>  lexer.identifier 
        >>  lexer.left_parenth
        >>  (value >> *(lexer.comma > value))
        >>  lexer.right_parenth;

    declaration %= 
            qi::position(position_begin)
        >>  type 
        >>  lexer.identifier 
        >>  -(lexer.assign >> value);
    
    arrayDeclaration %= 
            qi::position(position_begin)
        >>  type
        >>  lexer.identifier 
        >>  lexer.left_bracket
        >>  value
        >>  lexer.right_bracket;

    return_ %=
            qi::position(position_begin)
        >>  lexer.return_
        >>  value
        >>  lexer.stop;
    
    globalDeclaration %= 
            qi::position(position_begin)
        >>  type 
        >>  lexer.identifier 
        >>  -(lexer.assign >> value.constant)
        >>  lexer.stop;
    
    globalArrayDeclaration %= 
            qi::position(position_begin)
        >>  type 
        >>  lexer.identifier 
        >>  lexer.left_bracket
        >>  value
        >>  lexer.right_bracket
        >>  lexer.stop;

    swap %= 
            qi::position(position_begin)
        >>  lexer.identifier 
        >>  lexer.swap 
        >>  lexer.identifier;
    
    instruction %= 
            switch_
        |   (value.member_function_call > lexer.stop)
        |   (value.function_call > lexer.stop)
        |   (value.assignment > lexer.stop)
        |   (struct_declaration >> lexer.stop)
        |   (declaration >> lexer.stop)
        |   (value.suffix_operation > lexer.stop)
        |   (value.prefix_operation > lexer.stop)
        |   (arrayDeclaration >> lexer.stop)
        |   if_
        |   for_
        |   while_
        |   do_while_
        |   foreach_
        |   foreachin_
        |   return_
        |   (swap > lexer.stop)
        |   (delete_ > lexer.stop)
        ;

    repeatable_instruction = 
            value.assignment 
        |   swap 
        |   value.suffix_operation
        |   value.prefix_operation
        |   value.function_call;
    
    arg %= 
            type 
        >>  lexer.identifier;
    
    function %= 
            qi::position(position_begin)
        >>  type 
        >>  lexer.identifier
        >>  lexer.left_parenth
        >>  -( arg >> *( lexer.comma > arg))
        >>  lexer.right_parenth
        >>  lexer.left_brace
        >>  *(instruction)
        >>  lexer.right_brace;

    member_declaration %=
            qi::position(position_begin)
        >>  type
        >>  lexer.identifier
        >>  lexer.stop;
    
    constructor %= 
            qi::position(position_begin)
        >>  qi::omit[lexer.this_]
        >>  lexer.left_parenth
        >>  -( arg >> *( lexer.comma > arg))
        >>  lexer.right_parenth
        >>  lexer.left_brace
        >>  *(instruction)
        >>  lexer.right_brace;
    
    destructor %= 
            qi::position(position_begin)
        >>  lexer.tilde     
        >>  qi::omit[lexer.this_]
        >>  lexer.left_parenth
        >>  lexer.right_parenth
        >>  lexer.left_brace
        >>  *(instruction)
        >>  lexer.right_brace;

    struct_ %=
            qi::position(position_begin)
        >>  lexer.struct_
        >>  lexer.identifier
        >>  lexer.left_brace
        >>  *(member_declaration)
        >>  *(constructor)
        >>  *(destructor)
        >>  *(function)
        >>  lexer.right_brace;

    standardImport %= 
            lexer.include
        >>  lexer.less
        >>  lexer.identifier
        >>  lexer.greater;

    import %=
            lexer.include
        >>  lexer.string_litteral;

    program %=
            qi::eps 
        >>  qi::position(position_begin)
        >>  *(
                    function 
                |   globalDeclaration 
                |   globalArrayDeclaration 
                |   standardImport 
                |   import 
                |   struct_
            );

    /* Debugging rules */
    DEBUG_RULE(program);
    DEBUG_RULE(function);
    DEBUG_RULE(instruction);
}
