//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONTEXT_H
#define CONTEXT_H

#include <string>
#include <memory>

#include <unordered_map>

#include "Types.hpp"

#include "ast/Value.hpp"

namespace eddic {

class Variable;
class IntermediateProgram;

class Context {
    private:
        std::shared_ptr<Context> m_parent;

    protected:
        typedef std::unordered_map<std::string, std::shared_ptr<Variable>> Variables;

        Variables variables;

    public:
        explicit Context(std::shared_ptr<Context> parent);
        Context(const Context& rhs) = delete;

        virtual std::shared_ptr<Variable> addVariable(const std::string& a, Type type) = 0;
        virtual std::shared_ptr<Variable> addVariable(const std::string& a, Type type, ast::Value& value);
        virtual void removeVariable(const std::string& variable);

        bool exists(const std::string& a) const;
        std::shared_ptr<Variable> getVariable(const std::string& variable) const;

        Variables::const_iterator begin() const;
        Variables::const_iterator end() const;

        std::shared_ptr<Context> parent() const;
};

} //end of eddic

#endif
