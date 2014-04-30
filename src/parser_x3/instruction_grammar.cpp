#include "parser_x3/instruction_grammar.hpp"
#include "parser_x3/type_grammar.hpp"
#include "parser_x3/value_grammar.hpp"
#include "parser_x3/utils.hpp"

namespace {
    
typedef x3::identity<struct instruction> instruction_id;
typedef x3::identity<struct foreach> foreach_id;
typedef x3::identity<struct foreach_in> foreach_in_id;
typedef x3::identity<struct while_> while_id;
typedef x3::identity<struct do_while> do_while_id;
typedef x3::identity<struct variable_declaration> variable_declaration_id;
typedef x3::identity<struct struct_declaration> struct_declaration_id;
typedef x3::identity<struct array_declaration> array_declaration_id;
typedef x3::identity<struct return_> return_id;
typedef x3::identity<struct delete_> delete_id;
typedef x3::identity<struct if_> if_id;
typedef x3::identity<struct else_if> else_if_id;
typedef x3::identity<struct else_> else_id;

x3::rule<instruction_id, x3_ast::instruction> const instruction("instruction");
x3::rule<foreach_id, x3_ast::foreach> const foreach("foreach");
x3::rule<foreach_in_id, x3_ast::foreach_in> const foreach_in("foreach_in");
x3::rule<while_id, x3_ast::while_> const while_("while");
x3::rule<do_while_id, x3_ast::do_while> const do_while("do_while");
x3::rule<variable_declaration_id, x3_ast::variable_declaration> const variable_declaration("variable_declaration");
x3::rule<struct_declaration_id, x3_ast::struct_declaration> const struct_declaration("struct_declaration");
x3::rule<array_declaration_id, x3_ast::array_declaration> const array_declaration("array_declaration");
x3::rule<return_id, x3_ast::return_> const return_("return");
x3::rule<delete_id, x3_ast::delete_> const delete_("delete");
x3::rule<if_id, x3_ast::if_> const if_("if");
x3::rule<else_if_id, x3_ast::else_if> const else_if("else_if");
x3::rule<else_id, x3_ast::else_> const else_("else");

ANNOTATE(foreach_id);
ANNOTATE(foreach_in_id);
ANNOTATE(while_id);
ANNOTATE(do_while_id);
ANNOTATE(variable_declaration_id);
ANNOTATE(struct_declaration_id);
ANNOTATE(array_declaration_id);
ANNOTATE(return_id);
ANNOTATE(delete_id);
   
    #include "parser_x3/skipper_inc.hpp"
    #include "parser_x3/identifier_inc.hpp"

    auto const instruction_def =
            foreach
        |   foreach_in
        //|   if_
        |   while_
        |   do_while
        |   (return_ > ';')
        |   (delete_ > ';')
        |   (struct_declaration > ';')
        |   (array_declaration > ';')
        |   (variable_declaration > ';');

    auto const foreach_def =
            x3::lit("foreach")
        >>  '('
        >>  x3_grammar::type_grammar
        >>  identifier
        >>  "from"
        >>  x3::int_
        >>  "to"
        >>  x3::int_
        >>  ')'
        >>  '{'
        >>  *instruction
        >>  '}';
    
    auto const foreach_in_def =
            x3::lit("foreach")
        >>  '('
        >>  x3_grammar::type_grammar
        >>  identifier
        >>  "in"
        >>  identifier
        >>  ')'
        >>  '{'
        >>  *instruction
        >>  '}';
    
    auto const while_def =
            x3::lit("while")
        >>  '('
        >>  x3_grammar::value_grammar
        >>  ')'
        >>  '{'
        >>  *instruction
        >>  '}';
    
    auto const do_while_def =
            x3::lit("do")
        >>  '{'
        >>  *instruction
        >>  '}'
        >>  "while"
        >>  '('
        >>  x3_grammar::value_grammar
        >>  ')'
        >>  ';';
    
    auto const variable_declaration_def =
            x3_grammar::type_grammar
        >>  identifier
        >>  -('=' >> x3_grammar::value_grammar);
    
    auto const struct_declaration_def =
            x3_grammar::type_grammar
        >>  identifier
        >>  '('
        >>  -(x3_grammar::value_grammar % ',')
        >>  ')';
    
    auto const array_declaration_def =
            x3_grammar::type_grammar
        >>  identifier
        >>  '['
        >>  x3_grammar::value_grammar
        >>  ']';

    auto const return_def =
            x3::lit("return")
        >>  x3::attr(1)
        >>  x3_grammar::value_grammar;

    auto const delete_def =
            x3::lit("delete")
        >>  x3::attr(1)
        >>  x3_grammar::value_grammar;

    auto const if_def =
            x3::lit("if")
        >>  '('
        >>  x3_grammar::value_grammar
        >>  ')'
        >>  '{'
        >>  *instruction
        >>  '}'
        >>  *else_if
        >>  -else_;

    auto const else_if_def =
            x3::lit("else")
        >>  x3::lit("if")
        >>  '('
        >>  x3_grammar::value_grammar
        >>  ')'
        >>  '{'
        >>  *instruction
        >>  '}';
    
    auto const else_def =
            x3::lit("else")
        >>  x3::attr(1)
        >>  '{'
        >>  *instruction
        >>  '}';
        
} //end of anonymous namespace

x3_grammar::instruction_parser_type x3_grammar::instruction_grammar_create(){
    return x3::skip(skipper)[x3::grammar(
        "eddi::instruction",
        instruction = instruction_def,
        foreach = foreach_def,
        foreach_in = foreach_in_def,
        while_ = while_def,
        do_while = do_while_def,
        variable_declaration = variable_declaration_def,
        struct_declaration = struct_declaration_def,
        array_declaration = array_declaration_def,
        return_ = return_def,
        delete_ = delete_def,
        if_ = if_def,
        else_if = else_if_def,
        else_ = else_def
        )];
}
