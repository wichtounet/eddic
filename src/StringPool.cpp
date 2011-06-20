//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "StringPool.h"

using std::string;

int StringPool::index(string value){
	if(pool.find(value) == pool.end()){
		pool[value] = ++currentString;
	}

	return pool[value];
}
		
void StringPool::write(ByteCodeFileWriter& writer){
	writer.writeInt(pool.size());

	std::map<std::string, int>::const_iterator it;
	for(it = pool.begin(); it != pool.end(); ++it){
		writer.writeInt(it->second);
		writer.writeLitteral(it->first);
	}	
}