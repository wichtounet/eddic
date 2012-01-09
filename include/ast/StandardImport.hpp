//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_STANDARD_IMPORT_H
#define AST_STANDARD_IMPORT_H

#include <boost/fusion/include/adapt_struct.hpp>

namespace eddic {

namespace ast {

/*!
 * \class StandardImport
 * \brief The AST node for an import of the standard import. 
 */
struct StandardImport {
    int fake;
    std::string ignoreLess;
    std::string header;
    std::string ignoreGreater;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::StandardImport, 
    (std::string, ignoreLess)       //Uglyfix in order to get the real value of header (ignoring <)
    (std::string, header)
    (std::string, ignoreGreater)    //Uglyfix in order to get the real value of header (ignoring >)
)

#endif
