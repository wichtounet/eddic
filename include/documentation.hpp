//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
