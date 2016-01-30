//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_CSE_H
#define MTAC_CSE_H

#include <memory>
#include <unordered_map>

#include <boost/functional/hash.hpp>

#include "mtac/Quadruple.hpp"
#include "mtac/EscapeAnalysis.hpp"
#include "mtac/Operator.hpp"
#include "mtac/Utils.hpp"

namespace eddic {

class Variable;
class Type;

namespace mtac {

struct expression {
    std::size_t uid;
    mtac::Argument arg1;
    mtac::Argument arg2;
    mtac::Operator op;
    std::shared_ptr<Variable> tmp;
    std::shared_ptr<const Type> type;

    expression(std::size_t uid, mtac::Argument arg1, mtac::Argument arg2, mtac::Operator op, std::shared_ptr<Variable> tmp, std::shared_ptr<const Type> type) 
            : uid(uid), arg1(arg1), arg2(arg2), op(op), tmp(tmp), type(type) {
        //Nothing
    }

    bool operator==(const mtac::expression& rhs) const {
        return arg1 == rhs.arg1 && arg2 == rhs.arg2 && op == rhs.op;
    }
    
    bool operator!=(const mtac::expression& rhs) const {
        return !(*this == rhs);
    }
    
    bool operator<(const mtac::expression& rhs) const {
        if(arg1 == rhs.arg1){
            if(arg2 == rhs.arg2){
                return op < rhs.op;
            }

            return arg2 < rhs.arg2;
        }

        return arg1 < rhs.arg1;
    }
};

bool is_interesting(mtac::Quadruple& quadruple);
bool is_expression(mtac::Operator op);
bool is_commutative(mtac::Operator op);

mtac::Operator assign_op(mtac::Operator op);

bool are_equivalent(mtac::Quadruple& quadruple, const expression& exp);
bool is_killing(mtac::Quadruple& quadruple, const mtac::expression& expression);

bool is_valid(mtac::Quadruple& quadruple, const mtac::escaped_variables& escaped);

template<typename Container>
void kill_expressions(mtac::Quadruple& quadruple, Container& expressions){
    auto op = quadruple.op;
    if(mtac::erase_result(op) || op == mtac::Operator::DOT_ASSIGN || op == mtac::Operator::DOT_FASSIGN || op == mtac::Operator::DOT_PASSIGN){
        auto eit = expressions.begin();

        while(eit != expressions.end()){
            auto& expression = *eit;

            if(mtac::is_killing(quadruple, expression)){
                eit = expressions.erase(eit);
                continue;
            }

            ++eit;
        }
    }
}

std::ostream& operator<<(std::ostream& stream, const expression& expression);

} //end of mtac

} //end of eddic

namespace std {
    template<>
    struct hash<eddic::mtac::expression> {
        size_t operator()(const eddic::mtac::expression& val) const {
            std::size_t seed = 13;

            boost::hash_combine(seed, val.arg1);
            boost::hash_combine(seed, val.arg2);
            boost::hash_combine(seed, static_cast<unsigned int>(val.op));

            return seed;
        }
    };
}

#endif
