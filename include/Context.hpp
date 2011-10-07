//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONTEXT_H
#define CONTEXT_H

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <unordered_map>
#include <unordered_set>

#include "Types.hpp"

#include "AssemblyFileWriter.hpp"

namespace eddic {

class Value;
class Variable;

class Context {
    private:
        std::shared_ptr<Context> m_parent;

    protected:
        typedef std::unordered_map<int, std::shared_ptr<Variable>> StoredVariables;
        typedef std::unordered_map<std::string, int> VisibleVariables;

        StoredVariables m_stored;
        VisibleVariables m_visibles;

        static int currentVariable;

    public:
        Context(std::shared_ptr<Context> parent);

        virtual std::shared_ptr<Variable> addVariable(const std::string& a, Type type) = 0;
        virtual bool exists(const std::string& a) const;
        virtual std::shared_ptr<Variable> getVariable(const std::string& variable) const;
        virtual std::shared_ptr<Variable> getVariable(int index) const;

        virtual void write(AssemblyFileWriter& writer);
        virtual void release(AssemblyFileWriter& writer);
        std::shared_ptr<Context> parent() const ;

        void storeVariable(int index, std::shared_ptr<Variable> variable);
};

} //end of eddic

#endif
