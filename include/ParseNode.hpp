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

#include "CompilerException.hpp"

class ParseNode;

typedef std::list<ParseNode*>::const_iterator NodeIterator;

class ByteCodeFileWriter;
class StringPool;
class Variables;

class ParseNode {
	private:
		std::list<ParseNode*> childs;
		std::vector<ParseNode*> trash;	
	protected:
		ParseNode* parent;
	public:
		ParseNode() : parent(NULL){};
		virtual ~ParseNode();
		virtual void write(ByteCodeFileWriter& writer);
		virtual void checkVariables(Variables& variables) throw (CompilerException);
		virtual void checkStrings(StringPool& pool);
		virtual void optimize();
		void addFirst(ParseNode* node);
		void addLast(ParseNode* node);
		void replace(ParseNode* old, ParseNode* node);
		void remove(ParseNode* node);
		NodeIterator begin();
		NodeIterator end();
};

#endif
