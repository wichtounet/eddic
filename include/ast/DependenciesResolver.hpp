//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DEPENDENCIES_RESOLVER_H
#define DEPENDENCIES_RESOLVER_H

#include "ast/source_def.hpp"

namespace eddic {

namespace parser {
    class SpiritParser;
}

namespace ast {

/*!
 * \struct DependenciesResolver
 * \brief Utility class to resolve all the imports in a program.   
 */
struct DependenciesResolver {
    DependenciesResolver(parser::SpiritParser& parser);

    void resolve(ast::SourceFile& program) const;

    private:
        parser::SpiritParser& parser;
};

} //end of ast

} //end of eddic

#endif
