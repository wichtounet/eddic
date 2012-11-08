//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#error Documentation only

/*!
 * \mainpage
 *
 * \section Introduction
 *  
 * This page describes the EDDI Compiler. 
 * 
 * \section Compilation phases
 * 
 * The EDDI compiler works in several phases. 
 * 
 * \li Parse the provided file using a recursive descent LL Parser into an Abstract Syntax Tree (AST)
 * \li Checks the AST
 * \li Optimize the AST
 * \li Compile the AST into a machine-independent instruction set
 * \li Compile the instruction set into machine assembly
 * \li If activated, assemble and link the assembly into an executable
 */
