//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SPIRIT_PARSER_H
#define SPIRIT_PARSER_H

#include <string>

#include "ast/SourceFile.hpp"

namespace eddic {

namespace parser {

/*!
 * \struct SpiritParser
 * \brief The EDDI source file parser.
 *
 * This class takes an EDDI source file as input and produces an Abstract Syntax Tree as its output. The output syntax is created
 * using a Boost Spirit parser and a set of grammars describing the EDDI language.
 */
struct SpiritParser {
    /*!
     * \brief Parse the given source file and fills the given Abstract Syntax Tree. 
     * \param file The path to the file to parse. 
     * \param program The Abstract Syntax Tree root to fill. 
     * \return true if the file was valid, false otherwise
     */
    bool parse(const std::string& file, ast::SourceFile& program);
};

}

} //end of eddic

#endif
