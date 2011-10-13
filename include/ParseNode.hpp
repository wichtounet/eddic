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

#include "SpiritLexer.hpp"

namespace eddic {

class ParseNode;
class Program;

typedef std::list<std::shared_ptr<ParseNode>>::const_iterator NodeIterator;

class Context;
class AssemblyFileWriter;
class StringPool;

class ParseNode : public std::enable_shared_from_this<ParseNode> {
    private:
        std::list<std::shared_ptr<ParseNode>> childs;

        std::shared_ptr<Context> m_context;
        const Tok& m_token;

    protected:
        std::weak_ptr<ParseNode> parent;

    public:
        ParseNode(std::shared_ptr<Context> context, const Tok& token);

        virtual void write(AssemblyFileWriter& writer);
        virtual void checkFunctions(Program& program);
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();

        NodeIterator begin();
        NodeIterator end();

        std::shared_ptr<Context> context();
        const Tok& token();
    
        void addFirst(std::shared_ptr<ParseNode> node);
        void addLast(std::shared_ptr<ParseNode> node);
        void replace(std::shared_ptr<ParseNode> old, std::shared_ptr<ParseNode> node);
        void remove(std::shared_ptr<ParseNode> node);
};

} //end of eddic

#endif
