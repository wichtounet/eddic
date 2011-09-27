//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PARSE_NODE_H
#define PARSE_NODE_H

#include <list>
#include <vector>

namespace eddic {

class ParseNode;
class Program;

typedef std::list<ParseNode*>::const_iterator NodeIterator;
typedef std::vector<ParseNode*>::const_iterator TrashIterator;

class Context;
class AssemblyFileWriter;
class StringPool;
class Token;

class ParseNode {
    private:
        std::list<ParseNode*> childs;
        std::vector<ParseNode*> trash;

        Context* m_context;
        const Token* m_token;

    protected:
        ParseNode* parent;

    public:
        ParseNode(Context* context) : m_context(context), m_token(NULL), parent(NULL) {}
        ParseNode(Context* context, const Token* token) : m_context(context), m_token(token), parent(NULL) {} 
        virtual ~ParseNode();

        virtual void write(AssemblyFileWriter& writer);
        virtual void checkFunctions(Program& program);
        virtual void checkVariables();
        virtual void checkStrings(StringPool& pool);
        virtual void optimize();

        void addFirst(ParseNode* node);
        void addLast(ParseNode* node);
        void replace(ParseNode* old, ParseNode* node);
        void remove(ParseNode* node);
        NodeIterator begin();
        NodeIterator end();

        Context* context() {
            return m_context;
        }

        const Token* token(){
            return m_token;
        }
};

} //end of eddic

#endif
