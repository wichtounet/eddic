//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/**
 * eddic Parser using x3
 *
 * Unfortunately some hacks are still necessary to make the grammar works
 *
 * 1. Some structures have a fake_ field. This field is only here to make single-element structures works (confirmed with boost 1.59)
 * 2. start_instruction rule is only here to make type deduction works
 * 3. Still needs eps parser (confirmed with boost 1.59)
 * 4. Ideally the old AST should use x3::variant and forward_ast, but it does not seem to be possible without changing a whole lot of code :(
 */

#include <iomanip>
#include <istream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

//#include "boost_cfg.hpp"

//#define BOOST_SPIRIT_X3_DEBUG
#define BOOST_SPIRIT_X3_NO_RTTI

#include "ast/SourceFile.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>

#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

//#include "GlobalContext.hpp"

#include "parser_x3/SpiritParser.hpp"

namespace x3 = boost::spirit::x3;

using namespace eddic;

namespace x3_ast {

struct new_array;
struct new_;
struct builtin_operator;
struct function_call;

struct expression;
struct prefix_operation;
struct cast;
struct ternary;
struct assignment;

typedef x3::variant<
            ast::Integer,
            ast::IntegerSuffix,
            ast::Float,
            ast::Literal,
            ast::CharLiteral,
            ast::VariableValue,
            ast::Boolean,
            ast::Null,
            x3::forward_ast<new_array>,
            x3::forward_ast<new_>,
            builtin_operator,
            function_call,
            x3::forward_ast<prefix_operation>,
            x3::forward_ast<cast>,
            x3::forward_ast<ternary>,
            x3::forward_ast<expression>,
            x3::forward_ast<assignment>
        > value_t;

struct function_call : x3::position_tagged {
    std::string function_name;
    std::vector<ast::Type> template_types;
    std::vector<value_t> values;
};

struct builtin_operator {
    ast::BuiltinType type;
    std::vector<value_t> values;
};

struct new_array : x3::position_tagged {
    ast::Type type;
    value_t size;
};

struct new_ : x3::position_tagged {
    ast::Type type;
    std::vector<value_t> values;
};

using operation = boost::tuple<ast::Operator, value_t>;

struct expression : x3::position_tagged {
    value_t first;
    std::vector<operation> operations;
};

struct prefix_operation : x3::position_tagged {
    value_t left_value;
    ast::Operator op;
};

struct cast : x3::position_tagged {
    ast::Type type;
    value_t value;
};

struct ternary : x3::position_tagged {
    value_t condition;
    value_t true_value;
    value_t false_value;
};

struct assignment : x3::position_tagged {
    value_t left_value;
    value_t value;
    ast::Operator op = ast::Operator::ASSIGN; //If not specified, it is not a compound operator
};

//*****************************************

struct for_;
struct foreach;
struct while_;
struct do_while;
struct foreach_in;
struct if_;
struct switch_;

struct variable_declaration : x3::position_tagged {
    ast::Type variable_type;
    std::string variable_name;
    boost::optional<x3_ast::value_t> value;
};

struct struct_declaration : x3::position_tagged {
    ast::Type variable_type;
    std::string variable_name;
    std::vector<value_t> values;
};

struct array_declaration {
    ast::Type array_type;
    std::string array_name;
    value_t size;
};

struct delete_ : x3::position_tagged {
    value_t value;
	x3::unused_type fake_;
};

struct return_ : x3::position_tagged {
    value_t return_value;
	x3::unused_type fake_;
};

typedef x3::variant<
        expression,
        prefix_operation,
        return_,
        delete_,
        variable_declaration,
        x3::forward_ast<foreach>,
        x3::forward_ast<foreach_in>,
        x3::forward_ast<for_>,
        x3::forward_ast<if_>,
        x3::forward_ast<while_>,
        x3::forward_ast<do_while>,
        x3::forward_ast<switch_>,
        struct_declaration,
        array_declaration,
        function_call,
        assignment
    > instruction;

struct default_case_t : x3::position_tagged {
    std::vector<instruction> instructions;
	x3::unused_type fake_;
};

struct switch_case : x3::position_tagged {
    value_t value;
    std::vector<instruction> instructions;
};

struct switch_ : x3::position_tagged {
    value_t value;
    std::vector<switch_case> cases;
    boost::optional<default_case_t> default_case;
};

struct while_ : x3::position_tagged {
    value_t condition;
    std::vector<instruction> instructions;
};

struct do_while : x3::position_tagged {
    value_t condition;
    std::vector<instruction> instructions;
};

struct for_ : x3::position_tagged {
    boost::optional<instruction> start;
    boost::optional<value_t> condition;
    boost::optional<instruction> repeat;
    std::vector<instruction> instructions;
};

struct foreach_in : x3::position_tagged {
    ast::Type variable_type;
    std::string variable_name;
    std::string array_name;
    std::vector<instruction> instructions;
};

struct foreach : x3::position_tagged {
    ast::Type variable_type;
    std::string variable_name;
    int from;
    int to;
    std::vector<instruction> instructions;
};

struct else_if {
    value_t condition;
    std::vector<instruction> instructions;
};

struct else_ {
    std::vector<instruction> instructions;
	x3::unused_type fake_;
};

struct if_ {
    value_t condition;
    std::vector<instruction> instructions;
    std::vector<else_if> else_ifs;
    boost::optional<x3_ast::else_> else_;
};

//*****************************************

struct function_parameter {
    ast::Type  parameter_type;
    std::string parameter_name;
};

struct template_function_declaration : x3::position_tagged {
    std::vector<std::string> template_types;
    ast::Type return_type;
    std::string name;
    std::vector<function_parameter> parameters;
    std::vector<instruction> instructions;
};

struct global_variable_declaration : x3::position_tagged {
    ast::Type variable_type;
    std::string variable_name;
    boost::optional<x3_ast::value_t> value;
};

struct global_array_declaration : x3::position_tagged {
    ast::Type array_type;
    std::string array_name;
    value_t size;
};

struct standard_import : x3::position_tagged {
    std::string file;
};

struct import : x3::position_tagged {
    std::string file;
};

struct member_declaration : x3::position_tagged {
    ast::Type type;
    std::string name;
};

struct constructor : x3::position_tagged {
    std::vector<function_parameter> parameters;
    std::vector<instruction> instructions;
};

struct destructor {
    std::vector<function_parameter> parameters;
    std::vector<instruction> instructions;
	x3::unused_type fake_;
};

typedef x3::variant<
        member_declaration,
        array_declaration,
        template_function_declaration,
        constructor,
        destructor
    > struct_block;

struct template_struct : x3::position_tagged {
    std::vector<std::string> template_types;
    std::string name;
    boost::optional<ast::Type> parent_type;
    std::vector<struct_block> blocks;
};

typedef x3::variant<
        standard_import,
        import,
        template_struct,
        template_function_declaration,
        global_array_declaration,
        global_variable_declaration
    > block;

struct source_file {
    std::vector<block> blocks;
};

template<typename EHT>
struct printer: public boost::static_visitor<>  {
    std::size_t i = 0;
    const EHT& error_handler;

    printer(const EHT& error_handler) : error_handler(error_handler) {}

    std::string indent(){
        std::string v(i, ' ');
        return v;
    }

    void operator()(const source_file& source_file){
        std::cout << indent() << "source_file" << std::endl;

        i += 2;
        for(auto& block : source_file.blocks){
            boost::apply_visitor(*this, block);
        }
        i -= 2;
    }

    void operator()(const standard_import& import){
        std::cout << indent() << "standard_import: " << import.file << std::endl;
    }

    void operator()(const import& import){
        std::cout << indent() << "import: " << import.file << std::endl;
    }

    void operator()(const template_struct&){
        //TODO
    }

    void operator()(const template_function_declaration& function){
        std::cout << indent() << "template_function_declaration: " << function.name << std::endl;
        i += 2;
        std::cout << indent() << "template_types: ";
        for(auto& v : function.template_types) std::cout << v << ", ";
        std::cout << std::endl << indent() << "return_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, function.return_type);
        i -= 2;
        std::cout << indent() << "parameters: " << std::endl;
        i += 2;
        for(auto& parameter : function.parameters){
            std::cout << indent() << "name: " << parameter.parameter_name << std::endl;
            std::cout << indent() << "type: " << std::endl;
            i += 2;
            boost::apply_visitor(*this, parameter.parameter_type);
            i -= 2;
        }
        i -= 2;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : function.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const for_& for_){
        //TODO
    }

    void operator()(const foreach& foreach){
        std::cout << indent() << "foreach: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, foreach.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << foreach.variable_name << std::endl;
        std::cout << indent() << "from: " << foreach.from << std::endl;
        std::cout << indent() << "to: " << foreach.to << std::endl;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : foreach.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const foreach_in& foreach){
        std::cout << indent() << "foreach_in: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, foreach.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << foreach.variable_name << std::endl;
        std::cout << indent() << "array_name: " << foreach.array_name << std::endl;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : foreach.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const while_& loop){
        std::cout << indent() << "while: " << std::endl;
        i += 2;
        std::cout << indent() << "condition: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, loop.condition);
        i -= 2;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : loop.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const do_while& loop){
        std::cout << indent() << "do while: " << std::endl;
        i += 2;
        std::cout << indent() << "condition: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, loop.condition);
        i -= 2;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : loop.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const if_& if_){
        std::cout << indent() << "if: " << std::endl;
        i += 2;
        std::cout << indent() << "condition: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, if_.condition);
        i -= 2;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : if_.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const switch_& switch_){
        std::cout << indent() << "switch: " << std::endl;
        //TODO
    }

    void operator()(const return_& return_){
        std::cout << indent() << "return: " << std::endl;
        i += 2;
        std::cout << indent() << "value: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, return_.return_value);
        i -= 2;
        i -= 2;
    }

    void operator()(const delete_& delete_){
        std::cout << indent() << "delete: " << std::endl;
        i += 2;
        std::cout << indent() << "value: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, delete_.value);
        i -= 2;
        i -= 2;
    }

    void operator()(const variable_declaration& declaration){
        std::cout << indent() << "variable_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << declaration.variable_name << std::endl;
        if(declaration.value){
            std::cout << indent() << "value: " << std::endl;
            i += 2;
            boost::apply_visitor(*this, *declaration.value);
            i -= 2;
        }
        i -= 2;
        error_handler(declaration, "Variable declaration (handler)");
    }

    void operator()(const struct_declaration& declaration){
        std::cout << indent() << "struct_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << declaration.variable_name << std::endl;
        std::cout << indent() << "values: " << std::endl;
        i += 2;
        for(auto& v : declaration.values){
            boost::apply_visitor(*this, v);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const global_variable_declaration& declaration){
        std::cout << indent() << "global_variable_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << declaration.variable_name << std::endl;
        if(declaration.value){
            std::cout << indent() << "value: " << std::endl;
            i += 2;
            boost::apply_visitor(*this, *declaration.value);
            i -= 2;
        }
        i -= 2;
    }

    void operator()(const global_array_declaration& declaration){
        std::cout << indent() << "global_array_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "array_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.array_type);
        i -= 2;
        std::cout << indent() << "array_name: " << declaration.array_name << std::endl;
        std::cout << indent() << "size: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.size);
        i -= 2;
        i -= 2;
    }

    void operator()(const array_declaration& declaration){
        std::cout << indent() << "array_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "array_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.array_type);
        i -= 2;
        std::cout << indent() << "array_name: " << declaration.array_name << std::endl;
        std::cout << indent() << "size: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.size);
        i -= 2;
        i -= 2;
    }

    void operator()(const ast::SimpleType& type){
        std::cout << indent() << "simple_type: " << type.type << std::endl;
    }

    void operator()(const ast::ArrayType& type){
        std::cout << indent() << "array_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, type.type);
        i -= 2;
    }

    void operator()(const ast::TemplateType& type){
        std::cout << indent() << "template_type: " << std::endl;
        i += 2;
        std::cout << indent() << "base_type: " << type.type << std::endl;
        std::cout << indent() << "template_types: " << std::endl;
        i += 2;
        for(auto& t : type.template_types){
            boost::apply_visitor(*this, t);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const ast::PointerType& type){
        std::cout << indent() << "pointer_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, type.type);
        i -= 2;
    }

    void operator()(const new_array& value){
        std::cout << indent() << "new_array: " << std::endl;
        std::cout << indent() << "type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, value.type);
        i += 2;
        std::cout << indent() << "value_size: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, value.size);
        i += 2;
    }

    void operator()(const new_& value){
        std::cout << indent() << "new_: " << std::endl;
        std::cout << indent() << "type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, value.type);
        i += 2;
        std::cout << indent() << "values: " << std::endl;
        i += 2;
        for(auto& v : value.values){
            boost::apply_visitor(*this, v);
        }
        i += 2;
    }

    void operator()(const expression& value){
        //TODO
    }

    void operator()(const cast& value){
        //TODO
    }

    void operator()(const prefix_operation& value){
        //TODO
    }

    void operator()(const ternary& value){
        //TODO
    }

    void operator()(const assignment& value){
        //TODO
    }

    void operator()(const ast::Integer& integer){
        std::cout << indent() << "integer_literal: " << integer.value << std::endl;
    }

    void operator()(const ast::IntegerSuffix& integer){
        std::cout << indent() << "integer_suffix_literal: " << integer.value << integer.suffix << std::endl;
    }

    void operator()(const ast::Float& integer){
        std::cout << indent() << "float_literal: " << integer.value << std::endl;
    }

    void operator()(const ast::Literal& integer){
        std::cout << indent() << "string_literal: " << integer.value << std::endl;
    }

    void operator()(const ast::CharLiteral& integer){
        std::cout << indent() << "char_literal: " << integer.value << std::endl;
    }

    void operator()(const ast::VariableValue& integer){
        std::cout << indent() << "variable_value: " << integer.variableName << std::endl;
    }

    void operator()(const ast::Null&){
        std::cout << indent() << "null: " << std::endl;
    }

    void operator()(const ast::Boolean&){
        std::cout << indent() << "boolean: " << std::endl;
    }

    void operator()(const builtin_operator& value){
        std::cout << indent() << "builtin_operator: " << std::endl;
        std::cout << indent() << "op: " << static_cast<unsigned int>(value.type) << std::endl;
        std::cout << indent() << "values: " << std::endl;
        i += 2;
        for(auto& v : value.values){
            boost::apply_visitor(*this, v);
        }
        i += 2;
    }

    void operator()(const function_call& value){
        std::cout << indent() << "function_call: " << std::endl;
        std::cout << indent() << "function_name: " << value.function_name << std::endl;
        std::cout << indent() << "template_types: " << std::endl;
        i += 2;
        for(auto& v : value.template_types){
            boost::apply_visitor(*this, v);
        }
        i += 2;
        std::cout << indent() << "values: " << std::endl;
        i += 2;
        for(auto& v : value.values){
            boost::apply_visitor(*this, v);
        }
        i += 2;
    }
};

} //end of x3_ast namespace

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::source_file,
    (std::vector<x3_ast::block>, blocks)
)

//***************

BOOST_FUSION_ADAPT_STRUCT(
    ast::Boolean,
    (bool, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::new_array,
    (ast::Type, type)
    (x3_ast::value_t, size)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::new_,
    (ast::Type, type)
    (std::vector<x3_ast::value_t>, values)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::builtin_operator,
    (ast::BuiltinType, type)
    (std::vector<x3_ast::value_t>, values)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::function_call,
    (std::string, function_name)
    (std::vector<ast::Type>, template_types)
    (std::vector<x3_ast::value_t>, values)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::expression,
    (x3_ast::value_t, first)
    (std::vector<x3_ast::operation>, operations)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::prefix_operation,
    (ast::Operator, op)
    (x3_ast::value_t, left_value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::cast,
    (ast::Type, type)
    (x3_ast::value_t, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::ternary,
    (x3_ast::value_t, condition)
    (x3_ast::value_t, true_value)
    (x3_ast::value_t, false_value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::assignment,
    (x3_ast::value_t, left_value)
    (ast::Operator, op)
    (x3_ast::value_t, value)
)

//***************

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::for_,
    (boost::optional<x3_ast::instruction>, start)
    (boost::optional<x3_ast::value_t>, condition)
    (boost::optional<x3_ast::instruction>, repeat)
    (std::vector<x3_ast::instruction>, instructions)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::foreach_in,
    (ast::Type, variable_type)
    (std::string, variable_name)
    (std::string, array_name)
    (std::vector<x3_ast::instruction>, instructions)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::foreach,
    (ast::Type, variable_type)
    (std::string, variable_name)
    (int, from)
    (int, to)
    (std::vector<x3_ast::instruction>, instructions)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::while_,
    (x3_ast::value_t, condition)
    (std::vector<x3_ast::instruction>, instructions)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::do_while,
    (std::vector<x3_ast::instruction>, instructions)
    (x3_ast::value_t, condition)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::variable_declaration,
    (ast::Type, variable_type)
    (std::string, variable_name)
    (boost::optional<x3_ast::value_t>, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::struct_declaration,
    (ast::Type, variable_type)
    (std::string, variable_name)
    (std::vector<x3_ast::value_t>, values)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::array_declaration,
    (ast::Type, array_type)
    (std::string, array_name)
    (x3_ast::value_t, size)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::return_,
    (x3_ast::value_t, return_value)
    (x3::unused_type, fake_)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::delete_,
    (x3_ast::value_t, value)
    (x3::unused_type, fake_)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::else_if,
    (x3_ast::value_t, condition)
    (std::vector<x3_ast::instruction>, instructions)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::else_,
    (std::vector<x3_ast::instruction>, instructions)
    (x3::unused_type, fake_)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::if_,
    (x3_ast::value_t, condition)
    (std::vector<x3_ast::instruction>, instructions)
    (std::vector<x3_ast::else_if>, else_ifs)
    (boost::optional<x3_ast::else_>, else_)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::default_case_t,
    (std::vector<x3_ast::instruction>, instructions)
    (x3::unused_type, fake_)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::switch_,
    (x3_ast::value_t, value)
    (std::vector<x3_ast::switch_case>, cases)
    (boost::optional<x3_ast::default_case_t>, default_case)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::switch_case,
    (x3_ast::value_t, value)
    (std::vector<x3_ast::instruction>, instructions)
)

//***************

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::template_function_declaration,
    (std::vector<std::string>, template_types)
    (ast::Type, return_type)
    (std::string, name)
    (std::vector<x3_ast::function_parameter>, parameters)
    (std::vector<x3_ast::instruction>, instructions)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::function_parameter,
    (ast::Type, parameter_type)
    (std::string, parameter_name)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::global_variable_declaration,
    (ast::Type, variable_type)
    (std::string, variable_name)
    (boost::optional<x3_ast::value_t>, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::global_array_declaration,
    (ast::Type, array_type)
    (std::string, array_name)
    (x3_ast::value_t, size)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::import,
    (std::string, file)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::standard_import,
    (std::string, file)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::member_declaration,
    (ast::Type, type)
    (std::string, name)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::destructor,
    (std::vector<x3_ast::instruction>, instructions)
    (x3::unused_type, fake_)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::constructor,
    (std::vector<x3_ast::function_parameter>, parameters)
    (std::vector<x3_ast::instruction>, instructions)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::template_struct,
    (std::vector<std::string>, template_types)
    (std::string, name)
    (boost::optional<ast::Type>, parent_type)
    (std::vector<x3_ast::struct_block>, blocks)
)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-shift-op-parentheses"
#pragma clang diagnostic ignored "-Wunused-parameter"

/* Error handling */

namespace x3_grammar {
    template <typename Iterator>
    using error_handler = x3::error_handler<Iterator>;

    // tag used to get our error handler from the context
    struct error_handler_tag;

    struct error_handler_base {
        template <typename Iterator, typename Exception, typename Context>
        x3::error_handler_result on_error(Iterator& first, Iterator const& last, Exception const& x, Context const& context){
            std::string message = "Error! Expecting: " + x.which() + " here:";
            auto& error_handler = x3::get<error_handler_tag>(context).get();
            error_handler(x.where(), message);
            return x3::error_handler_result::fail;
        }
    };

    struct annotation_base {
        template <typename T, typename Iterator, typename Context>
        inline void on_success(Iterator const& first, Iterator const& last, T& ast, Context const& context){
            auto& error_handler = x3::get<error_handler_tag>(context).get();
            error_handler.tag(ast, first, last);
        }
    };

    //typedef std::string::iterator base_iterator_type;
    typedef std::string::const_iterator iterator_type;
    typedef x3::phrase_parse_context<x3::ascii::space_type>::type phrase_context_type;
    typedef error_handler<iterator_type> error_handler_type;

    typedef x3::with_context<
        error_handler_tag
        , std::reference_wrapper<error_handler_type> const
        , phrase_context_type>::type
        context_type;

    x3::symbols<ast::Operator> multiplicative_op;
    x3::symbols<ast::Operator> additive_op;
    x3::symbols<ast::Operator> relational_op;
    x3::symbols<ast::Operator> logical_and_op;
    x3::symbols<ast::Operator> logical_or_op;
    x3::symbols<ast::Operator> postfix_op;
    x3::symbols<ast::Operator> prefix_op;
    x3::symbols<ast::Operator> assign_op;
    x3::symbols<ast::Operator> unary_op;
    x3::symbols<ast::BuiltinType> builtin_op;

    /* Match operators into symbols */

    void add_keywords(){
        static bool once = false;
        if(once){
            return;
        }
        once = true;
        unary_op.add
            ("+", ast::Operator::ADD)
            ("-", ast::Operator::SUB)
            ("!", ast::Operator::NOT)
            ("*", ast::Operator::STAR)
            ("&", ast::Operator::ADDRESS)
            ;

        additive_op.add
            ("+", ast::Operator::ADD)
            ("-", ast::Operator::SUB)
            ;

        multiplicative_op.add
            ("/", ast::Operator::DIV)
            ("*", ast::Operator::MUL)
            ("%", ast::Operator::MOD)
            ;

        relational_op.add
            (">=", ast::Operator::GREATER_EQUALS)
            (">", ast::Operator::GREATER)
            ("<=", ast::Operator::LESS_EQUALS)
            ("<", ast::Operator::LESS)
            ("!=", ast::Operator::NOT_EQUALS)
            ("==", ast::Operator::EQUALS)
            ;

        logical_and_op.add
            ("&&", ast::Operator::AND)
            ;

        logical_or_op.add
            ("||", ast::Operator::OR)
            ;

        postfix_op.add
            ("++", ast::Operator::INC)
            ("--", ast::Operator::DEC)
            ;

        prefix_op.add
            ("++", ast::Operator::INC)
            ("--", ast::Operator::DEC)
            ;

        builtin_op.add
            ("size", ast::BuiltinType::SIZE)
            ("length", ast::BuiltinType::LENGTH)
            ;

        assign_op.add
            ("=",  ast::Operator::ASSIGN)
            ("+=", ast::Operator::ADD)
            ("-=", ast::Operator::SUB)
            ("/=", ast::Operator::DIV)
            ("*=", ast::Operator::MUL)
            ("%=", ast::Operator::MOD)
            ("<=>", ast::Operator::SWAP)
            ;
    }

    struct source_file_class;

    struct type_class {};
    struct simple_type_class {};
    struct array_type_class {};
    struct pointer_type_class {};
    struct template_type_class {};

    struct integer_literal_class;
    struct integer_suffix_literal_class;
    struct float_literal_class;
    struct string_literal_class;
    struct char_literal_class;
    struct value_class;
    struct primary_value_class;
    struct variable_value_class;
    struct new_array_class;
    struct new_class;
    struct boolean_class;
    struct null_class;
    struct builtin_operator_class;
    struct function_call_class;
    struct postfix_expression_class;
    struct prefix_expression_class;
    struct unary_expression_class;
    struct unary_operation_class;
    struct cast_class;
    struct cast_expression_class;
    struct conditional_expression_class;
    struct additive_expression_class;
    struct multiplicative_expression_class;
    struct relational_expression_class;
    struct logical_or_expression_class;
    struct logical_and_expression_class;
    struct ternary_class;
    struct assignment_class;
    struct assignment_expression_class;

    struct instruction_class;
    struct start_instruction_class;
    struct repeatable_instruction_class;
    struct for_class;
    struct foreach_class;
    struct foreach_in_class;
    struct while_class;
    struct do_while_class;
    struct struct_declaration_class;
    struct array_declaration_class;
    struct return_class;
    struct delete_class;
    struct if_class;
    struct else_if_class {};
    struct else_class {};
    struct switch_class;
    struct default_case_class;
    struct switch_case_class;

    struct variable_declaration_class;

    struct function_parameter_class;
    struct template_function_declaration_class;
    struct global_variable_declaration_class;
    struct global_array_declaration_class;
    struct import_class;
    struct standard_import_class;
    struct member_declaration_class;
    struct constructor_class;
    struct destructor_class;
    struct template_struct_class;

    x3::rule<source_file_class, x3_ast::source_file> const source_file("source_file");

    x3::rule<type_class, ast::Type> const type("type");
    x3::rule<simple_type_class, ast::SimpleType> const simple_type("simple_type");
    x3::rule<array_type_class, ast::ArrayType> const array_type("array_type");
    x3::rule<pointer_type_class, ast::PointerType> const pointer_type("pointer_type");
    x3::rule<template_type_class, ast::TemplateType> const template_type("template_type");

    x3::rule<integer_literal_class, ast::Integer> const integer_literal("integer_literal");
    x3::rule<integer_suffix_literal_class, ast::IntegerSuffix> const integer_suffix_literal("integer_suffix_literal");
    x3::rule<float_literal_class, ast::Float> const float_literal("float_literal");
    x3::rule<string_literal_class, ast::Literal> const string_literal("string_literal");
    x3::rule<char_literal_class, ast::CharLiteral> const char_literal("char_literal");
    x3::rule<new_array_class, x3_ast::new_array> const new_array("new_array");
    x3::rule<new_class, x3_ast::new_> const new_("new_");
    x3::rule<boolean_class, ast::Boolean> const boolean("boolean");
    x3::rule<null_class, ast::Null> const null("null");
    x3::rule<variable_value_class, ast::VariableValue> const variable_value("variable_value");
    x3::rule<builtin_operator_class, x3_ast::builtin_operator> const builtin_operator("builtin_operator");
    x3::rule<function_call_class, x3_ast::function_call> const function_call("function_call");
    x3::rule<postfix_expression_class, x3_ast::expression> const postfix_expression("postfix_expression");
    x3::rule<prefix_expression_class, x3_ast::prefix_operation> const prefix_expression("prefix_expression");
    x3::rule<unary_expression_class, x3_ast::value_t> const unary_expression("unary_expression");
    x3::rule<unary_operation_class, x3_ast::prefix_operation> const unary_operation("unary_operation");
    x3::rule<additive_expression_class, x3_ast::expression> const additive_expression("additive_expression");
    x3::rule<multiplicative_expression_class, x3_ast::expression> const multiplicative_expression("multiplicative_expression");
    x3::rule<relational_expression_class, x3_ast::expression> const relational_expression("relational_expression");
    x3::rule<logical_or_expression_class, x3_ast::expression> const logical_or_expression("logical_or_expression");
    x3::rule<logical_and_expression_class, x3_ast::expression> const logical_and_expression("logical_and_expression");
    x3::rule<cast_class, x3_ast::cast> const cast("cast");
    x3::rule<assignment_class, x3_ast::assignment> const assignment("assignment");
    x3::rule<assignment_expression_class, x3_ast::value_t> const assignment_expression("assignment_expression");
    x3::rule<ternary_class, x3_ast::ternary> const ternary("ternary");
    x3::rule<cast_expression_class, x3_ast::value_t> const cast_expression("cast_expression");
    x3::rule<conditional_expression_class, x3_ast::value_t> const conditional_expression("conditional_expression");
    x3::rule<value_class, x3_ast::value_t> const value("value");
    x3::rule<primary_value_class, x3_ast::value_t> const primary_value("primary_value");

    x3::rule<instruction_class, x3_ast::instruction> const instruction("instruction");
    x3::rule<start_instruction_class, x3_ast::instruction> const start_instruction("start_instruction");
    x3::rule<repeatable_instruction_class, x3_ast::instruction> const repeatable_instruction("repeatable_instruction");
    x3::rule<for_class, x3_ast::for_> const for_("for");
    x3::rule<foreach_class, x3_ast::foreach> const foreach("foreach");
    x3::rule<foreach_in_class, x3_ast::foreach_in> const foreach_in("foreach_in");
    x3::rule<while_class, x3_ast::while_> const while_("while");
    x3::rule<do_while_class, x3_ast::do_while> const do_while("do_while");
    x3::rule<variable_declaration_class, x3_ast::variable_declaration> const variable_declaration("variable_declaration");
    x3::rule<struct_declaration_class, x3_ast::struct_declaration> const struct_declaration("struct_declaration");
    x3::rule<array_declaration_class, x3_ast::array_declaration> const array_declaration("array_declaration");
    x3::rule<return_class, x3_ast::return_> const return_("return");
    x3::rule<delete_class, x3_ast::delete_> const delete_("delete");
    x3::rule<if_class, x3_ast::if_> const if_("if");
    x3::rule<else_if_class, x3_ast::else_if> const else_if("else_if");
    x3::rule<else_class, x3_ast::else_> const else_("else");
    x3::rule<switch_class, x3_ast::switch_> const switch_("switch");
    x3::rule<switch_case_class, x3_ast::switch_case> const switch_case("switch_case");
    x3::rule<default_case_class, x3_ast::default_case_t> const default_case("default_case");

    x3::rule<function_parameter_class, x3_ast::function_parameter> const function_parameter("function_parameter");
    x3::rule<template_function_declaration_class, x3_ast::template_function_declaration> const template_function_declaration("template_function_declaration");
    x3::rule<global_variable_declaration_class, x3_ast::global_variable_declaration> const global_variable_declaration("global_variable_declaration");
    x3::rule<global_array_declaration_class, x3_ast::global_array_declaration> const global_array_declaration("global_array_declaration");
    x3::rule<standard_import_class, x3_ast::standard_import> const standard_import("standard_import");
    x3::rule<import_class, x3_ast::import> const import("import");
    x3::rule<member_declaration_class, x3_ast::member_declaration> const member_declaration("member_declaration");
    x3::rule<constructor_class, x3_ast::constructor> const constructor("constructor");
    x3::rule<destructor_class, x3_ast::destructor> const destructor("destructor");
    x3::rule<template_struct_class, x3_ast::template_struct> const template_struct("template_struct");

    struct source_file_class : error_handler_base {};
    struct variable_value_class : annotation_base {};

    struct instruction_class {};
    struct start_instruction_class {};
    struct repeatable_instruction_class {};
    struct for_class : annotation_base {};
    struct foreach_class : annotation_base {};
    struct foreach_in_class : annotation_base {};
    struct while_class : annotation_base {};
    struct do_while_class : annotation_base {};
    struct struct_declaration_class : annotation_base {};
    struct array_declaration_class : annotation_base {};
    struct return_class : annotation_base {};
    struct delete_class : annotation_base {};
    struct if_class : annotation_base {};
    struct switch_class : annotation_base {};
    struct switch_case_class : annotation_base {};
    struct default_case_class : annotation_base {};
    struct variable_declaration_class : annotation_base {};
    struct new_array_class : annotation_base {};
    struct new_class : annotation_base {};
    struct null_class {};
    struct builtin_operator_class : annotation_base {};
    struct function_call_class : annotation_base {};
    struct postfix_expression_class : annotation_base {};
    struct prefix_expression_class : annotation_base {};
    struct unary_expression_class : annotation_base {};
    struct unary_operation_class : annotation_base {};
    struct cast_class : annotation_base {};
    struct ternary_class : annotation_base {};
    struct cast_expression_class : annotation_base {};
    struct assignment_class : annotation_base {};
    struct assignment_expression_class {};
    struct additive_expression_class : annotation_base {};
    struct multiplicative_expression_class : annotation_base {};
    struct relational_expression_class : annotation_base {};
    struct logical_or_expression_class : annotation_base {};
    struct logical_and_expression_class : annotation_base {};
    struct conditional_expression_class {};

    struct function_parameter_class : annotation_base {};
    struct template_function_declaration_class : annotation_base {};
    struct global_variable_declaration_class : annotation_base {};
    struct global_array_declaration_class : annotation_base {};
    struct import_class : annotation_base {};
    struct standard_import_class : annotation_base {};
    struct member_declaration_class : annotation_base {};
    struct cosntructor_class : annotation_base {};
    struct destructor_class : annotation_base {};
    struct template_struct_class : annotation_base {};

    /* Utilities */

    auto const skipper =
            x3::ascii::space
        |   "/*" >> *(x3::char_ - "*/") >> "*/"
        |   "//" >> *(x3::char_ - x3::eol);

    auto const const_ =
            (x3::lit("const") > x3::attr(true))
        |   x3::attr(false);

    x3::real_parser<double, x3::strict_real_policies<double>> strict_double;

    auto const identifier_def =
                x3::lexeme[(x3::char_('_') >> *(x3::alnum | x3::char_('_')))]
            |   x3::lexeme[(x3::alpha >> *(x3::alnum | x3::char_('_')))]
            ;

    class identifier_class;
    x3::rule<identifier_class, std::string> const identifier("identifier");

    BOOST_SPIRIT_DEFINE(identifier);


    /* Types */

    auto const type_def =
            array_type
        |   pointer_type
        |   template_type
        |   simple_type;

    auto const simple_type_def =
            const_
        >>  identifier;

    auto const template_type_def =
            identifier
        >>  '<'
        >>  type % ','
        >>  '>';

    auto const array_type_def =
            (
                    template_type
                |   simple_type
            )
        >>  '['
        >>  ']';

    auto const pointer_type_def =
           (
                    template_type
                |   simple_type
            )
        >>  '*';

    BOOST_SPIRIT_DEFINE(
        type,
        simple_type,
        template_type,
        array_type,
        pointer_type
    );

    /* Values */

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
        >   '\'';

    auto const string_literal_def =
            x3::lit('"')
        >>  x3::no_skip[*(x3::char_ - '"')]
        >   '"';

    auto const variable_value_def =
        identifier;

    auto const new_array_def =
            x3::lit("new")
        >>  type
        >>  '['
        >   value
        >   ']';

    auto const new__def =
            x3::lit("new")
        >>  type
        >>  '('
        >   -(value % ',')
        >   ')';

	auto const boolean_def = x3::bool_;

    auto const null_def = x3::eps >> x3::lit("null");

    auto const builtin_operator_def =
            builtin_op
        >>  '('
        >>  value % ','
        >>  ')';

    auto const function_call_def =
            identifier
        >>  -(
                    '<'
                >>  type % ','
                >>  '>'
            )
        >>  '('
        >>  -(value % ',')
        >>  ')';

    auto const primary_value_def =
            new_array
        |   new_
        |   boolean
        |   null
        |   builtin_operator
        |   function_call
        |   variable_value
        |   integer_suffix_literal
        |   float_literal
        |   integer_literal
        |   string_literal
        |   char_literal
        |   '(' >> value > ')'
        ;

    auto const postfix_expression_def =
            primary_value
        >>  +(
                         '['
                     >>  x3::attr(ast::Operator::BRACKET)
                     >>  value
                     >>  ']'
                |
                         '.'
                     >>  x3::attr(ast::Operator::CALL)
                     >>  function_call
                |
                         '.'
                     >>  x3::attr(ast::Operator::DOT)
                     >>  variable_value
                |
                         postfix_op
                     >   x3::attr(ast::VariableValue())
            );

    auto const prefix_expression_def =
            prefix_op
        >>  unary_expression;

    auto const unary_operation_def =
            unary_op
        >>  cast_expression;

    auto const unary_expression_def =
            postfix_expression
        |   prefix_expression
        |   unary_operation
        |   primary_value;

    auto const cast_def =
            '('
        >>  type
        >>  ')'
        >>  cast_expression;

    auto const cast_expression_def =
            cast
        |   unary_expression;

    auto const multiplicative_expression_def =
            cast_expression
        >>  *(multiplicative_op >> cast_expression); //TODO Not sure why > cannot be used here

    auto const additive_expression_def =
            multiplicative_expression
        >>  *(additive_op > multiplicative_expression);

    auto const relational_expression_def =
            additive_expression
        >>  *(relational_op > additive_expression);

    auto const logical_and_expression_def =
            relational_expression
        >>  *(logical_and_op > relational_expression);

    auto const logical_or_expression_def =
            logical_and_expression
        >>  *(logical_or_op > logical_and_expression);

    auto const ternary_def =
            logical_or_expression
        >>  '?'
        >   conditional_expression
        >   ':'
        >   conditional_expression;

    auto const conditional_expression_def =
            ternary
         |  logical_or_expression;

    auto const assignment_def =
            unary_expression
        >>  assign_op
        >>  assignment_expression;

    auto assignment_expression_def =
            assignment
        |   conditional_expression;

    //TODO Perhaps using a reference could save some time
    auto value_def = assignment_expression;

    BOOST_SPIRIT_DEFINE(
        value,
        primary_value,
        integer_literal,
        integer_suffix_literal,
        float_literal,
        char_literal,
        string_literal,
        variable_value,
        new_array,
        new_,
        boolean,
        null,
        builtin_operator,
        function_call,
        postfix_expression,
        prefix_expression,
        unary_operation,
        unary_expression,
        cast,
        additive_expression,
        multiplicative_expression,
        cast_expression,
        relational_expression,
        logical_and_expression,
        logical_or_expression,
        ternary,
        conditional_expression,
        assignment,
        assignment_expression
    );

    /* Instructions */

    auto const instruction_def =
            (assignment > ';')
        |   (postfix_expression > ';')
        |   (prefix_expression > ';')
        |   switch_
        |   if_
        |   foreach
        |   foreach_in
        |   for_
        |   while_
        |   do_while
        |   (return_ > ';')
        |   (delete_ > ';')
        |   (function_call > ';')
        |   (struct_declaration > ';')
        |   (array_declaration > ';')
        |   (variable_declaration > ';');

    auto start_instruction_def = variable_declaration;

    auto repeatable_instruction_def =
            assignment
        |   postfix_expression
        |   prefix_expression //TODO CHECk that
        |   function_call;

    auto const for__def =
            x3::lit("for")
        >   '('
        >   -start_instruction
        >   ';'
        >   -value
        >   ';'
        >   -repeatable_instruction
        >   ')'
        >   '{'
        >   *instruction
        >   '}';

    auto const foreach_def =
            x3::lit("foreach")
        >>  '('
        >>  type
        >>  identifier
        >>  "from"
        >>  x3::int_
        >>  "to"
        >>  x3::int_
        >   ')'
        >   '{'
        >   *instruction
        >   '}';

    auto const foreach_in_def =
            x3::lit("foreach")
        >>  '('
        >>  type
        >>  identifier
        >>  "in"
        >>  identifier
        >   ')'
        >   '{'
        >   *instruction
        >   '}';

    auto const while__def =
            x3::lit("while")
        >   '('
        >   value
        >   ')'
        >   '{'
        >   *instruction
        >   '}'
        ;

    auto const do_while_def =
            x3::lit("do")
        >  '{'
        >  *instruction
        >  '}'
        >  "while"
        >  '('
        >  value
        >  ')'
        >  ';'
        ;

    auto const variable_declaration_def =
            type
        >>  identifier
        >>  -('=' > value);

    auto const struct_declaration_def =
            type
        >>  identifier
        >>  '('
        >>  -(value % ',')
        >>  ')';

    auto const array_declaration_def =
            type
        >>  identifier
        >>  '['
        >>  value
        >>  ']';

    auto const return__def =
            x3::lit("return")
        >   value
        >   x3::attr(42);

    auto const delete__def =
            x3::lit("delete")
        >   value
        >   x3::attr(42);

    auto const if__def =
            x3::lit("if")
        >>  '('
        >>  value
        >>  ')'
        >>  '{'
        >>  *instruction
        >>  '}'
        >>  *else_if
        >>  -else_;

    auto const else_if_def =
            x3::lit("else")
        >>  x3::lit("if")
        >   '('
        >   value
        >   ')'
        >   '{'
        >   *instruction
        >   '}';

    auto const else__def =
            x3::lit("else")
        >   '{'
        >   *instruction
        >   '}'
        >   x3::attr(42);

    auto const default_case_def =
            x3::lit("default")
        >   ':'
        >   *instruction
        >   x3::attr(42);

    auto const switch_case_def =
            x3::lit("case")
        >   value
        >   ':'
        >   *instruction;

    auto const switch__def =
           x3::lit("switch")
        >  '('
        >  value
        >  ')'
        >  '{'
        >  *switch_case
        >  -default_case
        >  '}'
            ;

    BOOST_SPIRIT_DEFINE(
        instruction,
        start_instruction,
        repeatable_instruction,
        for_,
        foreach,
        foreach_in,
        while_,
        do_while,
        variable_declaration,
        struct_declaration,
        array_declaration,
        return_,
        delete_,
        if_,
        else_if,
        else_,
        switch_,
        switch_case,
        default_case
    );

    /* Base */

    auto const source_file_def =
         *(
                standard_import
            |   import
            |   template_struct
            |   template_function_declaration
            |   (global_array_declaration > ';')
            |   (global_variable_declaration > ';')
         );

    auto const standard_import_def =
            x3::lit("include")
        >>  '<'
        >   identifier
        >   '>';

    auto const import_def =
            x3::lit("include")
        >>  '"'
        >   x3::no_skip[*(x3::char_ - '"')]
        >   '"';

    auto const template_function_declaration_def =
            -(
                    x3::lit("template")
                >>  '<'
                >>  (x3::lit("type") >> identifier) % ','
                >>  '>'
            )
        >>  type
        >>  identifier
        >>  '('
        >>  -(function_parameter % ',')
        >   ')'
        >   '{'
        >   *instruction
        >   '}';

    auto const global_variable_declaration_def =
            type
        >>  identifier
        >>  -('=' >> value);

    auto const global_array_declaration_def =
            type
        >>  identifier
        >>  '['
        >>  value
        >>  ']';

    auto const function_parameter_def =
            type
        >>  identifier;

    auto const member_declaration_def =
            type
        >>  identifier
        >>  ';';

    auto const constructor_def =
            x3::lit("this")
        >   '('
        >   -(function_parameter % ',')
        >   ')'
        >   '{'
        >   *instruction
        >   '}';

    auto const destructor_def =
            '~'
        >   x3::lit("this")
        >   '('
        >   ')'
        >   '{'
        >   *instruction
        >   '}'
        >   x3::attr(1);

    auto template_struct_def =
            -(
                    x3::lit("template")
                >>  '<'
                >>  (x3::lit("type") >> identifier) % ','
                >>  '>'
            )
        >>  x3::lit("struct")
        >>  identifier
        >>  -(
                    "extends"
                >   type
             )
        >>  '{'
        >>  *(
                    member_declaration
                |   (array_declaration > ';')
                |   constructor
                |   destructor
                |   template_function_declaration
             )
        >>  '}';

    /* Grammar */

    BOOST_SPIRIT_DEFINE(
        source_file,
        function_parameter,
        template_function_declaration,
        global_variable_declaration,
        global_array_declaration,
        standard_import,
        import,
        member_declaration,
        constructor,
        destructor,
        template_struct
    );

} // end of grammar namespace

#pragma clang diagnostic pop

bool parser_x3::SpiritParser::parse(const std::string& file/*, ast::SourceFile& , std::shared_ptr<GlobalContext> context*/){
    //timing_timer timer(context->timing(), "parsing_x3");

    std::ifstream in(file.c_str(), std::ios::binary);
    in.unsetf(std::ios::skipws);

    //Collect the size of the file
    in.seekg(0, std::istream::end);
    std::size_t size(static_cast<size_t>(in.tellg()));
    in.seekg(0, std::istream::beg);

    //int current_file = context->new_file(file);

    x3_grammar::add_keywords();

    //std::string& file_contents = context->get_file_content(current_file);
    std::string file_contents;
    file_contents.resize(size);
    in.read(&file_contents[0], size);

    x3_ast::source_file result;

    x3_grammar::iterator_type it(file_contents.begin());
    x3_grammar::iterator_type end(file_contents.end());

    x3_grammar::error_handler_type error_handler(it, end, std::cerr, file);

    auto const parser = x3::with<x3_grammar::error_handler_tag>(std::ref(error_handler))[x3_grammar::source_file];
    auto& skipper = x3_grammar::skipper;

    bool r = x3::phrase_parse(it, end, parser, skipper, result);

    if(r && it == end){
        //x3_ast::printer<x3_grammar::error_handler_type> printer(error_handler);
        //printer(result);

        return true;
    } else {
        std::cout << "Invalid source file" << std::endl;
        return false;
    }
}
