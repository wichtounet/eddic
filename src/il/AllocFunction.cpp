//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "il/AllocFunction.hpp"
#include "AssemblyFileWriter.hpp"

using namespace eddic;

void AllocFunction::write(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "eddi_alloc:" << std::endl
        << "pushl %ebp" << std::endl
        << "movl %esp, %ebp" << std::endl

        << "movl 8(%ebp), %ecx" << std::endl
        << "movl VIeddi_remaining, %ebx" << std::endl

        << "cmpl %ebx, %ecx" << std::endl
        << "jle alloc_normal" << std::endl

        //Get the current address
        << "movl $45, %eax" << std::endl        //45 = sys_brk
        << "xorl %ebx, %ebx" << std::endl       //get end
        << "int  $0x80" << std::endl
       
        //%eax is the current address 
        << "movl %eax, %esi" << std::endl

        //Alloc new block of 16384K from the current address
        << "movl %eax, %ebx" << std::endl
        << "addl $16384, %ebx" << std::endl
        << "movl $45, %eax" << std::endl        //45 = sys_brk
        << "int  $0x80" << std::endl

        //zero'd the new block
        << "movl %eax, %edi" << std::endl       //edi = start of block

        << "subl $4, %edi" << std::endl         //EDI points to the last DWORD available to us
        << "movl $4096, %ecx" << std::endl         //this many DWORDs were allocated
        << "xorl %eax, %eax"  << std::endl         //   ; will write with zeroes
        << "std"  << std::endl         //        ; walk backwards
        << "rep stosl"  << std::endl         //      ; write all over the reserved area
        << "cld"  << std::endl         //        ; bring back the DF flag to normal state

        << "movl %esi, %eax" << std::endl

        //We now have 16K of available memory starting at %esi
        << "movl $16384, VIeddi_remaining" << std::endl
        << "movl %esi, VIeddi_current" << std::endl

        << "alloc_normal:" << std::endl

        //old = current
        << "movl VIeddi_current, %eax" << std::endl
        
        //current += size
        << "movl VIeddi_current, %ebx" << std::endl
        << "addl %ecx, %ebx" << std::endl
        << "movl %ebx, VIeddi_current" << std::endl
        
        //remaining -= size
        << "movl VIeddi_remaining, %ebx" << std::endl
        << "subl %ecx, %ebx" << std::endl
        << "movl %ebx, VIeddi_remaining" << std::endl
       
        << "alloc_end:" << std::endl

        << "leave" << std::endl
        << "ret" << std::endl;
}
