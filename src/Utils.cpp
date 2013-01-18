//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <fstream>

#include "Utils.hpp"

bool eddic::has_extension(const std::string& file, const std::string& extension){
    return file.rfind("." + extension) != std::string::npos;
}

bool eddic::file_exists(const std::string& file){
   std::ifstream ifile(file.c_str());
   return ifile; 
}

std::string eddic::execCommand(const std::string& command) {
    std::stringstream output;

    char buffer[1024];

    FILE* stream = popen(command.c_str(), "r");

    while (fgets(buffer, 1024, stream) != NULL) {
        output << buffer;
    }

    pclose(stream);

    return output.str();
}

bool eddic::isPowerOfTwo (int x){
    return ((x > 0) && !(x & (x - 1)));
}

int eddic::powerOfTwo(int x){
    int i = 0; 
    while (x > 1){
        x /= 2;
        ++i;
    }

    return i;
}
