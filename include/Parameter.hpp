//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PARAMETER_H
#define PARAMETER_H

#include <string>

#include "Types.hpp"

namespace eddic {

class Parameter {
    private:
        std::string m_name;
        Type m_type;
        int m_offset;

    public:
        Parameter(const std::string& name, Type type, int offset);

        Type type();
        int offset();
};

} //end of eddic

#endif
