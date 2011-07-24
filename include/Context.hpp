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

#include "Types.hpp"

#include "AssemblyFileWriter.hpp"

namespace eddic {

class Variable {
    private:
        const std::string m_name;
        const Type m_type;
        const int m_index;
    public:
        Variable(const std::string& name, Type type, int index) : m_name(name), m_type(type), m_index(index) {}
        std::string name() const  {
            return m_name;
        }
        int index() const {
            return m_index;
        }
        Type type() const {
            return m_type;
        }
};

class Context {
    private:
        static std::vector<Context*> contexts;
        static unsigned int currentVariable;

        std::map<std::string, Variable*> variables;
        Context* m_parent;

    public:
        Context() : m_parent(NULL) {
            contexts.push_back(this);
        }
        Context(Context* parent) : m_parent(parent) {
            contexts.push_back(this);
        }
        ~Context();

        bool exists(const std::string& variable) const;
        unsigned int index(const std::string& variable) const;
        Variable* create(const std::string& variable, Type type);
        Variable* find(const std::string& variable);
        void write(AssemblyFileWriter& writer);

        Context* parent() {
            return m_parent;
        }

        static void writeAll(AssemblyFileWriter& writer);
        static void cleanup();
};

} //end of eddic

#endif
