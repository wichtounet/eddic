#include "boost_cfg.hpp"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/lex_generate_static_lexertl.hpp>

#include "lexer/SpiritLexer.hpp"

using namespace eddic;

int main(int, char*[]){
    // create the lexer object instance needed to invoke the generator
    lexer::DynamicLexer spirit_lexer; // the token definition

    // open the output file, where the generated tokenizer function will be
    // written to
    std::ofstream out("include/lexer/static_lexer.hpp");

    // invoke the generator, passing the token definition, the output stream
    // and the name suffix of the tables and functions to be generated
    //
    // The suffix "wc" used below results in a type lexertl::static_::lexer_wc
    // to be generated, which needs to be passed as a template parameter to the
    // lexertl::static_lexer template (see word_count_static.cpp).
    return boost::spirit::lex::lexertl::generate_static_dfa(spirit_lexer, out, "sl") ? 0 : -1;
}
