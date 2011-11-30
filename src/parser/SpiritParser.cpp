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

#include <boost/spirit/include/classic_position_iterator.hpp>

#include "lexer/SpiritLexer.hpp"

#include "parser/SpiritParser.hpp"
#include "parser/EDDIGrammar.hpp"

namespace qi = boost::spirit::qi;

using namespace eddic;

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
    EddiGrammar grammar(lexer); 
    
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
                std::setw(pos.column) << " " << "^- here" << std::endl;
            
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
