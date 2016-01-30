//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#error Documentation only

/*!
 * \namespace eddic
 * \brief Root namespace for the eddic compiler. 
 */
namespace eddic {

/*!
 * \namespace eddic::ast
 * \brief Contains all the Abstract Syntax Tree nodes.
 */
namespace ast {} //end of ast

/*!
 * \namespace eddic::tac
 * \brief Contains all the Three Address Code instructions. 
 */
namespace tac {} //end of tac

/*!
 * \namespace eddic::mtac
 * \brief Contains the Medium-Level Three Address Code Intermediate representation. 
 */
namespace mtac {} //end of mtac

/*!
 * \namespace eddic::ltac
 * \brief Contains the Low-Level Three Address Code Intermediate representation. 
 */
namespace ltac {} //end of ltac

/*!
 * \namespace eddic::lexer
 * \brief Contains the lexical analyzer
 */
namespace lexer {} //end of lexer

/*!
 * \namespace eddic::parser
 * \brief Contains the parser. 
 */
namespace parser {} //end of parser

/*!
 * \namespace eddic::as
 * \brief Contains the code generators. 
 */
namespace as {} //end of as

} //end of eddic
