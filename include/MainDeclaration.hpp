//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MAIN_DECLARATION_H
#define MAIN_DECLARATION_H

#include <memory>

#include "ParseNode.hpp"

namespace eddic {

class MainDeclaration : public ParseNode {
   public:
        MainDeclaration(std::shared_ptr<Context> context, const Tok& token);

        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
