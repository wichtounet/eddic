//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <memory>

namespace eddic {

class AssemblyFileWriter;
class FunctionTable;
class GlobalContext;
class StringPool;

namespace as {

class CodeGenerator {
    public:
        CodeGenerator(AssemblyFileWriter& writer);
        
        virtual void writeRuntimeSupport(FunctionTable& table) = 0;
        
        void addGlobalVariables(std::shared_ptr<GlobalContext> context, StringPool& pool);

    protected:
        AssemblyFileWriter& writer;
};

} //end of as

} //end of eddic

#endif
