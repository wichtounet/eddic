//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

namespace eddic {

class AssemblyFileWriter;

/*!
 * \class Instruction
 * \brief Represent an machine-independant instruction
 */
struct Instruction {
    /*!
     * \brief Write the instruction as assembly in the given writer. 
     * \param writer The writer to use. 
     */
    virtual void write(AssemblyFileWriter& writer) const = 0;
};

} //end of eddic

#endif
