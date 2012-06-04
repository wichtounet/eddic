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

#include "lexer/SpiritLexer.hpp"

#include "parser/SpiritParser.hpp"
#include "parser/EDDIGrammar.hpp"

namespace qi = boost::spirit::qi;
namespace spirit = boost::spirit;

using namespace eddic;

bool parser::SpiritParser::parse(const std::string& file, ast::SourceFile& program){
    std::ifstream in(file.c_str());
    in.unsetf(std::ios::skipws);
   
    //Collect the size of the file
    in.seekg(0, std::istream::end);
    std::size_t size(static_cast<size_t>(in.tellg()));
    in.seekg(0, std::istream::beg);

    std::string contents(size, 0);
    in.read(&contents[0], size);    

    lexer::pos_iterator_type position_begin(contents.begin(), contents.end(), file);
    lexer::pos_iterator_type position_end;

    lexer::Lexer lexer;
    parser::EddiGrammar grammar(lexer, position_begin); 
    
    try {
        bool r = spirit::lex::tokenize_and_parse(position_begin, position_end, lexer, grammar, program);

        if(r && position_begin == position_end) {
            return true;
        } else {
            std::cout << "Parsing failed" << std::endl;
            const spirit::classic::file_position_base<std::string>& pos = position_begin.get_position();
            std::cout <<
                "Error at file " << pos.file << " line " << pos.line << " column " << pos.column << std::endl <<
                "'" << position_begin.get_currentline() << "'" << std::endl <<
                std::setw(pos.column - 1) << " " << "^- here" << std::endl;
            
            return false;
        }
    } catch (const qi::expectation_failure<lexer::lexer_type::iterator_type>& exception) {
        std::cout << "Parsing failed" << std::endl;
      
        //TODO Improve to get information from exception 
        const auto& pos = position_begin.get_position();
       
        std::cout <<
            "Error at file " << pos.file << " line " << pos.line << " column " << pos.column << " Expecting " << exception.what_ << std::endl <<
            "'" << position_begin.get_currentline() << "'" << std::endl <<
            std::setw(pos.column - 1) << " " << "^- here" << std::endl;
        
        return false;
    }
}
