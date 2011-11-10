//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTION_TABLE_H
#define FUNCTION_TABLE_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "Types.hpp"

namespace eddic {

struct ParameterType {
    std::string name;
    Type paramType;

    //To be compliant with mangling
    Type type(){
        return paramType;
    }
};

struct Function {
    std::string name;
    std::string mangledName;
    std::vector<std::shared_ptr<ParameterType>> parameters;
    int references;
};

class FunctionTable {
    private:
        std::unordered_map<std::string, std::shared_ptr<Function>> functions;

    public:
        void addFunction(std::shared_ptr<Function> function);
        bool exists(const std::string& function);

        void addReference(const std::string& function);
        int referenceCount(const std::string& function);
};

} //end of eddic

#endif
