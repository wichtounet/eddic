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

#include "boost_cfg.hpp"
#include <boost/spirit/home/x3.hpp>

#include "GlobalContext.hpp"

#include "parser_x3/SpiritParser.hpp"

namespace x3 = boost::spirit::x3;

using namespace eddic;

bool parser_x3::SpiritParser::parse(const std::string& file, ast::SourceFile& , std::shared_ptr<GlobalContext> context){
    timing_timer timer(context->timing(), "parsing_x3");

    std::ifstream in(file.c_str(), std::ios::binary);
    in.unsetf(std::ios::skipws);

    //Collect the size of the file
    in.seekg(0, std::istream::end);
    std::size_t size(static_cast<size_t>(in.tellg()));
    in.seekg(0, std::istream::beg);

    int current_file = context->new_file(file);

    std::string& file_contents = context->get_file_content(current_file);
    file_contents.resize(size);
    in.read(&file_contents[0], size);

    //TODO Real stuff here
    
    return false;
}
