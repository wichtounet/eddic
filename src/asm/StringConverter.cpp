//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Utils.hpp"

#include "asm/StringConverter.hpp"

using namespace eddic;

std::string as::StringConverter::address_to_string(eddic::ltac::Address& address) const {
    if(address.absolute){
        if(address.displacement){
            return "[" + *address.absolute + " + " + ::toString(*address.displacement) + "]";
        }

        if(address.base_register){
            return "[" + *address.absolute + " + " + (*this)(*address.base_register) + "]";
        }

        return "[" + *address.absolute + "]";
    }
        
    if(address.base_register){
        if(address.scaled_register){
            if(address.scale){
                if(address.displacement){
                    return "[" + (*this)(*address.base_register) + " + " + (*this)(*address.scaled_register) + " * " + ::toString(*address.scale) + " + " + ::toString(*address.displacement) + "]";
                }
                
                return "[" + (*this)(*address.base_register) + " + " + (*this)(*address.scaled_register) + " * " + ::toString(*address.scale) + "]";
            }
                
            if(address.displacement){
                return "[" + (*this)(*address.base_register) + " + " + (*this)(*address.scaled_register) + " + " + ::toString(*address.displacement) + "]";
            }
            
            return "[" + (*this)(*address.base_register) + " + " + (*this)(*address.scaled_register) + "]";
        }
                
        if(address.displacement){
            return "[" + (*this)(*address.base_register) + " + " + ::toString(*address.displacement) + "]";
        }

        return "[" + (*this)(*address.base_register) + "]";
    }

    if(address.displacement){
        return "[" + ::toString(*address.displacement) + "]";
    }

    ASSERT_PATH_NOT_TAKEN("Invalid address type");
}
