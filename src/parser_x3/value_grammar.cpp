#include "parser_x3/value_grammar.hpp"
#include "parser_x3/utils.hpp"

namespace {

typedef x3::identity<struct integer_literal> integer_literal_id;
typedef x3::identity<struct integer_suffix_literal> integer_suffix_literal_id;
typedef x3::identity<struct float_literal> float_literal_id;
typedef x3::identity<struct string_literal> string_literal_id;
typedef x3::identity<struct char_literal> char_literal_id;
typedef x3::identity<struct variable_value> variable_value_id;
typedef x3::identity<struct value> value_id;

x3::rule<integer_literal_id, x3_ast::integer_literal> const integer_literal("integer_literal");
x3::rule<integer_suffix_literal_id, x3_ast::integer_suffix_literal> const integer_suffix_literal("integer_suffix_literal");
x3::rule<float_literal_id, x3_ast::float_literal> const float_literal("float_literal");
x3::rule<string_literal_id, x3_ast::string_literal> const string_literal("string_literal");
x3::rule<char_literal_id, x3_ast::char_literal> const char_literal("char_literal");
x3::rule<variable_value_id, x3_ast::variable_value> const variable_value("variable_value");
x3::rule<value_id, x3_ast::value> const value("value");
    
ANNOTATE(variable_value_id);
    
    #include "parser_x3/skipper_inc.hpp"
    #include "parser_x3/identifier_inc.hpp"

    x3::real_parser<double, x3::strict_real_policies<double>> strict_double;

    auto const integer_literal_def =
        x3::int_;
    
    auto const integer_suffix_literal_def =
        x3::lexeme[
                x3::int_ 
            >>  +x3::alpha
        ];

    auto const float_literal_def =
        strict_double;
    
    auto const char_literal_def =
            x3::lit('\'')
        >>  x3::char_
        >>  x3::lit('\'');
    
    auto const string_literal_def =
            x3::lit('"') 
        >>  x3::no_skip[*(x3::char_ - '"')] 
        >>  x3::lit('"');

    auto const variable_value_def =
        identifier;

    auto const value_def =
            variable_value
        |   integer_suffix_literal
        |   float_literal
        |   integer_literal
        |   string_literal
        |   char_literal;
        
   
} //end of anonymous namespace

x3_grammar::value_parser_type x3_grammar::value_grammar_create(){
    return x3::skip(skipper)[x3::grammar(
        "eddi::value",
        value = value_def,
        integer_literal = integer_literal_def,
        integer_suffix_literal = integer_suffix_literal_def,
        float_literal = float_literal_def,
        char_literal = char_literal_def,
        string_literal = string_literal_def,
        variable_value = variable_value_def
        )];
}
