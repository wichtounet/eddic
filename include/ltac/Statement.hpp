//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_STATEMENT_H
#define LTAC_STATEMENT_H

#include "variant.hpp"
#include "variant_hash.hpp"

#include "ltac/Instruction.hpp"
#include "ltac/Jump.hpp"

namespace eddic {

namespace ltac {

typedef boost::variant<
        std::shared_ptr<ltac::Instruction>,        //Basic quadruples
        std::shared_ptr<ltac::Jump>,             //Jumps
        std::string                             //For labels
    > Statement;

std::ostream& operator<<(std::ostream& out, const Statement& statement);
std::ostream& operator<<(std::ostream& out, std::shared_ptr<Statement> statement);

} //end of ltac

} //end of eddic

namespace std {
    template<>
    class hash<eddic::ltac::Statement> {
    public:
        size_t operator()(const eddic::ltac::Statement& val) const {
            std::size_t seed = boost::apply_visitor(boost::detail::variant::variant_hasher(), val);
            boost::hash_combine(seed, val.which());
            return seed;
        }
    };
}

#endif
