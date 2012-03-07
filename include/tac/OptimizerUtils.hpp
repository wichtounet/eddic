//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_COMPILER_H
#define TAC_COMPILER_H

namespace eddic {

namespace tac {

//Use for two pass optimization
enum class Pass : unsigned int {
    DATA_MINING,
    OPTIMIZE
};

template<typename T>
inline void replaceRight(T& visitor, std::shared_ptr<tac::Quadruple>& quadruple, tac::Argument arg){
    visitor.optimized = true;

    quadruple->op = tac::Operator::ASSIGN;
    quadruple->arg1 = arg;
    quadruple->arg2.reset();
}

template<typename T>
inline void replaceRight(T& visitor, std::shared_ptr<tac::Quadruple>& quadruple, tac::Argument arg, tac::Operator op){
    visitor.optimized = true;

    quadruple->op = op;
    quadruple->arg1 = arg;
    quadruple->arg2.reset();
}

template<typename T>
inline void replaceRight(T& visitor, std::shared_ptr<tac::Quadruple>& quadruple, tac::Argument arg1, tac::Operator op, tac::Argument arg2){
    visitor.optimized = true;

    quadruple->op = op;
    quadruple->arg1 = arg1;
    quadruple->arg2 = arg2;
}

//For optimizations done with offsets and variables

struct Offset {
    std::shared_ptr<Variable> variable;
    int offset;

    bool operator==(const Offset& b) const {
        return variable == b.variable && offset == b.offset;
    }
};

template <class T>
inline void hash_combine(std::size_t& seed, const T& v){
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

struct OffsetHash : std::unary_function<Offset, std::size_t> {
    std::size_t operator()(const Offset& p) const {
        std::size_t seed = 0;
        hash_combine(seed, p.variable);
        hash_combine(seed, p.offset);
        return seed;
    }
};

} //end of tac

} //end of eddic

#endif
