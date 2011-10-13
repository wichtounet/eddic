//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PROGRAM_H
#define PROGRAM_H

#include <string>
#include <unordered_map>

#include "ParseNode.hpp"

namespace eddic {

class Function;

class Program : public ParseNode {
    private:
        std::unordered_map<std::string, std::shared_ptr<Function>> functions;

    public:
        Program(std::shared_ptr<Context> context, const Tok& tok);
        
        void write(AssemblyFileWriter& writer);
        void addFunction(std::shared_ptr<Function> function);
        bool exists(const std::string& function);
};

} //end of eddic

#endif
