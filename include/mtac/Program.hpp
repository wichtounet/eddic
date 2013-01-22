//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_PROGRAM_H
#define MTAC_PROGRAM_H

#include <memory>
#include <iostream>

#include "mtac/Function.hpp"
#include "mtac/call_graph.hpp"

namespace eddic {

class Function;
struct GlobalContext;

namespace mtac {

enum class Mode : unsigned int {
    MTAC,
    LTAC
};

/*!
 * \struct Program
 * \brief Represent an EDDI program in its intermediate representation. 
 *
 * This structure to represent both MTAC and LTAC programs. It contains a list
 * of functions that form the whole program. 
 *
 * \see eddic::mtac::Function
 */
struct Program {
    std::shared_ptr<GlobalContext> context;
    std::vector<Function> functions;
    Mode mode = Mode::MTAC;

    call_graph call_graph;

    Program();

    //Program cannot be copied
    Program(const Program& rhs) = delete;
    Program& operator=(const Program& rhs) = delete;

    Function& mtac_function(const eddic::Function& function);

    std::vector<Function>::iterator begin(){
        return functions.begin();
    }

    std::vector<Function>::iterator end(){
        return functions.end();
    }
};

std::ostream& operator<<(std::ostream& stream, mtac::Program& program);

} //end of mtac

} //end of eddic

#endif
