//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef SPIRIT_PARSER_ERROR_HANDLER_H
#define SPIRIT_PARSER_ERROR_HANDLER_H

#include <string>
#include <memory>
#include <iostream> //Temporary

#include "boost_cfg.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "parser_x3/error_reporting.hpp"

namespace boost { namespace spirit { namespace x3 {

struct file_position_tagged : position_tagged {
    int id_file = -1;

    bool operator==(const x3::file_position_tagged& rhs) const {
        return id_first == rhs.id_first && id_last == rhs.id_last && id_file == rhs.id_file;
    }

    bool operator!=(const x3::file_position_tagged& rhs) const {
        return id_first != rhs.id_first || id_last != rhs.id_last || id_file != rhs.id_file;
    }
};

}}}

/* Error handling */

namespace x3_grammar {
    template <typename Iterator>
    using error_handler = boost::spirit::x3::error_handler<Iterator>;

    typedef std::string::const_iterator iterator_type;
    typedef boost::spirit::x3::phrase_parse_context<boost::spirit::x3::ascii::space_type>::type phrase_context_type;
    typedef error_handler<iterator_type> error_handler_type;

    struct global_error_handler {
        void register_handler(iterator_type it, iterator_type end, std::string file){
            error_handlers.emplace_back(it, end, std::cerr, std::move(file));
        }

        error_handler_type& current(){
            return error_handlers.back();
        }

        template<typename AST, std::enable_if_t<std::is_base_of<boost::spirit::x3::file_position_tagged, AST>::value, int> = 42>
        void tag(AST& t, iterator_type it, iterator_type end){
            t.id_file = error_handlers.size() - 1;
            error_handlers[t.id_file].tag(t, it, end);
        }

        template<typename AST, std::enable_if_t<!std::is_base_of<boost::spirit::x3::file_position_tagged, AST>::value, int> = 42>
        void tag(AST&, iterator_type, iterator_type){
            //Nothing to do here
        }

        void operator()(iterator_type err_pos, const std::string& message);
        void operator()(iterator_type err_pos, iterator_type err_last, const std::string& message);
        void operator()(const boost::spirit::x3::file_position_tagged& t, const std::string& message);

        std::string to_string(const boost::spirit::x3::file_position_tagged& t, const std::string& message = "");

        void semantical_exception(const std::string& message, const boost::spirit::x3::file_position_tagged& t);

    private:
        std::vector<error_handler_type> error_handlers;
    };

    // tag used to get our error handler from the context
    using error_handler_tag = boost::spirit::x3::error_handler_tag;

    struct error_handler_base {
        template <typename Iterator, typename Exception, typename Context>
        boost::spirit::x3::error_handler_result on_error(Iterator& /*first*/, Iterator const& /*last*/, Exception const& x, Context const& context){
            std::string message = "Error! Expecting: " + x.which() + " here:";
            auto& error_handler = boost::spirit::x3::get<error_handler_tag>(context).get();
            error_handler(x.where(), message);
            return boost::spirit::x3::error_handler_result::fail;
        }
    };

    // Used to annotate
    struct annotation_base {
        template <typename T, typename Iterator, typename Context>
        inline void on_success(Iterator const& first, Iterator const& last, T& ast, Context const& context){
            auto& error_handler = boost::spirit::x3::get<error_handler_tag>(context).get();
            error_handler.tag(ast, first, last);
        }
    };

} //end of namespace x3_grammar

#endif
