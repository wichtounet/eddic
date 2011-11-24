//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MANGLING_H
#define MANGLING_H

#include <vector>
#include <string>

#include "ast/Program.hpp"

#include <boost/variant/apply_visitor.hpp>
#include "GetTypeVisitor.hpp"

#include "Types.hpp"
#include "FunctionTable.hpp"

namespace eddic {

std::string mangle(Type type);

std::string mangle(const std::string& functionName, const std::vector<ParameterType>& types);
std::string mangle(const std::string& functionName, const std::vector<ast::Value>& values);

} //end of eddic

#endif
