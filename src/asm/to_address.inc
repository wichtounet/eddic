//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

std::string to_string(eddic::ltac::Address& address){
    if(address.absolute){
        if(address.displacement){
            return "[" + *address.absolute + " + " + ::toString(*address.displacement) + "]";
        }

        if(address.base_register){
            return "[" + *address.absolute + " + " + to_string(*address.base_register) + "]";
        }

        return "[" + *address.absolute + "]";
    }
        
    if(address.base_register){
        if(address.scaled_register){
            if(address.scale){
                if(address.displacement){
                    return "[" + to_string(*address.base_register) + " + " + to_string(*address.scaled_register) + " * " + ::toString(*address.scale) + " + " + ::toString(*address.displacement) + "]";
                }
                
                return "[" + to_string(*address.base_register) + " + " + to_string(*address.scaled_register) + " * " + ::toString(*address.scale) + "]";
            }
            
            return "[" + to_string(*address.base_register) + " + " + to_string(*address.scaled_register) + "]";
        }

        return "[" + to_string(*address.base_register) + "]";
    }

    if(address.displacement){
        return "[" + ::toString(*address.displacement) + "]";
    }

    ASSERT_PATH_NOT_TAKEN("Invalid address type");
}
