//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iomanip>
#include <istream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

//#include "boost_cfg.hpp"

//#define BOOST_SPIRIT_X3_DEBUG

#include <boost/mpl/vector.hpp>
#include <boost/mpl/count.hpp>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>

//#include "GlobalContext.hpp"

#include "parser_x3/SpiritParser.hpp"

namespace x3 = boost::spirit::x3;

using namespace eddic;

template<typename ForwardIteratorT>
class extended_iterator : public boost::spirit::classic::position_iterator2<ForwardIteratorT> {
    std::size_t current_file;

public:
    extended_iterator() : boost::spirit::classic::position_iterator2<ForwardIteratorT>() {
        //Nothing
    }

    template <typename FileNameT>
    extended_iterator(
        const ForwardIteratorT& begin, const ForwardIteratorT& end,
        FileNameT file, std::size_t current_file)
            : boost::spirit::classic::position_iterator2<ForwardIteratorT>(begin, end, file), current_file(current_file)
    {}

    std::size_t get_current_file() const {
        return current_file;
    }

    extended_iterator(const extended_iterator& iter) = default;
    extended_iterator& operator=(const extended_iterator& iter) = default;
};


namespace x3_ast {

struct position {
    std::size_t file = 0;               /*!< The source file */
    std::size_t line = 0;               /*!< The source line number */
    std::size_t column = 0;             /*!< The source column number */
};

//*****************************************

struct simple_type;
struct array_type;
struct pointer_type;
struct template_type;

typedef x3::variant<
        simple_type, 
        x3::forward_ast<array_type>, 
        x3::forward_ast<template_type>, 
        x3::forward_ast<pointer_type>
    > type;

struct simple_type {
    bool const_;
    std::string base_type;
};

struct array_type {
    type base_type;
};

struct pointer_type {
    type base_type;
};

struct template_type {
    std::string base_type;
    std::vector<type> template_types;
};

//*****************************************

struct function_declaration {
    position pos;
    type return_type;
    std::string name;
};

struct standard_import {
    position pos;
    int fake_;
    std::string file;
};

struct import {
    position pos;
    int fake_;
    std::string file;
};

typedef x3::variant<
        function_declaration,
        standard_import,
        import
    > block;

struct source_file {
    std::vector<block> blocks;
};

} //end of x3_ast namespace

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::source_file,
    (std::vector<x3_ast::block>, blocks)
)

    //***************

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::simple_type, 
    (bool, const_)
    (std::string, base_type)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::array_type, 
    (x3_ast::type, base_type)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::pointer_type, 
    (x3_ast::type, base_type)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::template_type, 
    (std::string, base_type)
    (std::vector<x3_ast::type>, template_types)
)

//***************

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::function_declaration,
    (x3_ast::type, return_type)
    //(std::string, return_type)
    (std::string, name)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::import,
    (int, fake_)
    (std::string, file)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::standard_import,
    (int, fake_)
    (std::string, file)
)

#define ANNOTATE(Type)\
template <typename iterator_type, typename Attr, typename Context>\
inline void on_success(Type, const iterator_type& first, const iterator_type&, Attr& attr, Context const&){\
    auto& pos = first.get_position();\
    attr.pos.file = first.get_current_file();\
    attr.pos.line = pos.line;\
    attr.pos.column = pos.column;\
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-shift-op-parentheses"

namespace x3_grammar {
    
    typedef x3::identity<struct source_file> source_file_id;

    typedef x3::identity<struct type> type_id;
    typedef x3::identity<struct simple_type> simple_type_id;
    typedef x3::identity<struct array_type> array_type_id;
    typedef x3::identity<struct pointer_type> pointer_type_id;
    typedef x3::identity<struct template_type> template_type_id;

    typedef x3::identity<struct function_declaration> function_declaration_id;
    typedef x3::identity<struct import> import_id;
    typedef x3::identity<struct standard_import> standard_import_id;

    x3::rule<source_file_id, x3_ast::source_file> const source_file("source_file");

    x3::rule<type_id, x3_ast::type> const type("type");
    x3::rule<simple_type_id, x3_ast::simple_type> const simple_type("simple_type");
    x3::rule<array_type_id, x3_ast::array_type> const array_type("array_type");
    x3::rule<pointer_type_id, x3_ast::pointer_type> const pointer_type("pointer_type");
    x3::rule<template_type_id, x3_ast::template_type> const template_type("template_type");

    x3::rule<function_declaration_id, x3_ast::function_declaration> const function_declaration("function_declaration");
    x3::rule<standard_import_id, x3_ast::standard_import> const standard_import("standard_import");
    x3::rule<import_id, x3_ast::import> const import("import");

    ANNOTATE(import_id);
    ANNOTATE(standard_import_id);
    ANNOTATE(function_declaration_id);
    
    auto const source_file_def = 
         *(
                standard_import
            |   import
            |   function_declaration
         );

    auto const standard_import_def = 
            x3::attr(1)
        >>  "import"
        >>  '<' 
        >   *x3::alpha
        >   '>';
    
    auto const import_def = 
            x3::attr(1)
        >>  "import"
        >>  '"' 
        >   *x3::alpha
        >   '"';
    
    auto const function_declaration_def = 
            type 
//*x3::alpha
        >>  *x3::alpha 
        >>  '(' 
        >   ')'
        >   '{' 
        >   '}';

    //*********************************************
   
    auto const_ = (
            (x3::lit("const") > x3::attr(true))
        |   x3::attr(false)
    );

    auto type_def =
            array_type
        |   pointer_type
        |   template_type
        |   simple_type;

    auto simple_type_def = 
            const_
        >>  *x3::alpha;

    auto template_type_def =
            *x3::alpha
        >>  '<'
        >>  type % ','
        >>  '>';
    
    auto array_type_def =
            (
                    template_type
                |   simple_type
            )
        >>  '[' 
        >>  ']';

    auto pointer_type_def =
           (
                    template_type
                |   simple_type
            )
        >>  '*';

    //*********************************************
    
    auto const parser = x3::grammar(
        "eddi", 
        source_file = source_file_def,

        type = type_def,
        array_type = array_type_def,
        pointer_type = pointer_type_def,
        template_type = template_type_def,
        simple_type = simple_type_def,

        function_declaration = function_declaration_def, 
        standard_import = standard_import_def,
        import = import_def);


    //********************************************

    auto const skipper = 
            x3::ascii::space
        |   ("/*" >> *(x3::char_ - "*/") >> "*/")
        |   ("//" >> *(x3::char_ - (x3::eol | x3::eoi)) >> (x3::eol | x3::eoi));


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

    //std::string& file_contents = context->get_file_content(current_file);
    std::string file_contents;
    file_contents.resize(size);
    in.read(&file_contents[0], size);

    auto& parser = x3_grammar::parser;

    x3_ast::source_file result;
//    boost::spirit::x3::ascii::space_type space;

    typedef std::string::iterator base_iterator_type;
    //typedef boost::spirit::classic::position_iterator2<base_iterator_type> pos_iterator_type;
    typedef extended_iterator<base_iterator_type> pos_iterator_type;

    pos_iterator_type it(file_contents.begin(), file_contents.end(), file, 1);
    pos_iterator_type end;

    try {
        bool r = x3::phrase_parse(it, end, parser, x3_grammar::skipper, result);

        if(r && it == end){
            std::cout << "Blocks: " << result.blocks.size() << std::endl;

            return true;
        } else {
            auto& pos = it.get_position();
            std::cout <<
                "parse error at file " << pos.file <<
                " line " << pos.line << " column " << pos.column << std::endl <<
                "'" << it.get_currentline() << "'" << std::endl <<
                std::setw(pos.column) << " " << "^- here" << std::endl;

            return false;
        }
    } catch(const boost::spirit::x3::expectation_failure<pos_iterator_type>& e){
        auto& pos = e.first.get_position();
        std::cout <<
                "parse error at file " << pos.file << " line " << pos.line << " column " << pos.column << std::endl
            <<  "expected: " << e.what_ << std::endl
            <<  "got: \"" << std::string(e.first, e.last) << '"' << std::endl
            <<  "'" << e.first.get_currentline() << "'" << std::endl
            <<  std::setw(pos.column) << " " << "^- here" << std::endl;

        //TODO The position seems really off

        return false;
    }
}
