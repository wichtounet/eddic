//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPILER_EXCEPTION_H
#define COMPILER_EXCEPTION_H

#include <string>

namespace eddic {

class CompilerException: public std::exception {
  private:
    std::string message;
  public:
    CompilerException(const std::string& m) : message(m) {};
    ~CompilerException() throw() {};
    const char* what() throw();
};

} //end of eddic

#endif
