//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VARIABLES_H
#define VARIABLES_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>

class Variables {
  private:
    std::map<std::string, unsigned int> variables;
    unsigned int currentVariable;
  public:
    Variables();
    bool exists(std::string variable);
    unsigned int index(std::string variable);
    void createIfNotExists(std::string variable);
};

#endif
