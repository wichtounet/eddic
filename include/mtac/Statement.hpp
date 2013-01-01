//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_STATEMENT_H
#define MTAC_STATEMENT_H

#include "variant.hpp"
#include "variant_hash.hpp"

#include "mtac/Quadruple.hpp"
#include "mtac/Param.hpp"
#include "mtac/IfFalse.hpp"
#include "mtac/If.hpp"
#include "mtac/Goto.hpp"
#include "mtac/Call.hpp"

namespace eddic {

namespace mtac {

typedef boost::variant<
        std::shared_ptr<mtac::Quadruple>,        //Basic quadruples
        std::shared_ptr<mtac::Param>,            //Parameters
        std::shared_ptr<mtac::IfFalse>,          //Jumping quadruples
        std::shared_ptr<mtac::If>,               //Jumping quadruples
        std::shared_ptr<mtac::Goto>,             //Non-conditional jump
        std::shared_ptr<mtac::Call>,             //Call a function
        std::string                              //For labels
    > Statement;

std::ostream& operator<<(std::ostream& out, const Statement& statement);
std::ostream& operator<<(std::ostream& out, std::shared_ptr<Statement> statement);

} //end of mtac

} //end of eddic

namespace std {
    template<>
    class hash<eddic::mtac::Statement> {
    public:
        size_t operator()(const eddic::mtac::Statement& val) const {
            std::size_t seed = boost::apply_visitor(boost::detail::variant::variant_hasher(), val);
            boost::hash_combine(seed, val.which());
            return seed;
        }
    };
}

#endif
