//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PARSE_NODE_H
#define PARSE_NODE_H

#include <memory>
#include <list>
#include <vector>

namespace eddic {

class ParseNode;
class Program;

typedef std::list<std::shared_ptr<ParseNode>>::const_iterator NodeIterator;
typedef std::vector<std::shared_ptr<ParseNode>>::const_iterator TrashIterator;

class Context;
class AssemblyFileWriter;
class StringPool;
class Token;

class ParseNode {
    private:
        std::list<std::shared_ptr<ParseNode>> childs;
        std::vector<std::shared_ptr<ParseNode>> trash;

        std::shared_ptr<Context> m_context;
        const Token* m_token;

    protected://TODO Put weak
        std::shared_ptr<ParseNode> parent;

    public:
        ParseNode(std::shared_ptr<Context> context) : m_context(context) {}
        ParseNode(std::shared_ptr<Context> context, const Token* token) : m_context(context), m_token(token){} 

        virtual void write(AssemblyFileWriter& writer);
        virtual void checkFunctions(Program& program);
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();

        void addFirst(std::shared_ptr<ParseNode> node);
        void addLast(std::shared_ptr<ParseNode> node);
        void replace(std::shared_ptr<ParseNode> old, std::shared_ptr<ParseNode> node);
        void remove(std::shared_ptr<ParseNode> node);
        
        NodeIterator begin();
        NodeIterator end();

        std::shared_ptr<Context> context() {
            return m_context;
        }

        const Token* token(){
            return m_token;
        }
};

} //end of eddic

#endif
