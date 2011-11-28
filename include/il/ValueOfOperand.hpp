//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VALUE_OF_OPERAND_H
#define VALUE_OF_OPERAND_H

#include "il/Operand.hpp"

namespace eddic {

class ValueOfOperand : public Operand {
    private:
        std::string location;
        int offset;

    public:
        ValueOfOperand(const std::string& location, int offset);

        std::string getValue() const ;
};

} //end of eddic

#endif
