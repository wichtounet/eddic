//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_QUADRUPLE_H
#define MTAC_QUADRUPLE_H

#include <memory>
#include <ostream>
#include <boost/optional.hpp>

#include "mtac/Operator.hpp"
#include "mtac/Argument.hpp"

namespace eddic {

class Function;
class Variable;

namespace mtac {

class basic_block;

enum class Size : char {
    DEFAULT,
    BYTE,
    WORD,
    DOUBLE_WORD,
    QUAD_WORD
};

struct Quadruple {
    private:
        std::size_t _uid;

    public:
        std::shared_ptr<Variable> result;
        boost::optional<mtac::Argument> arg1;
        boost::optional<mtac::Argument> arg2;
        mtac::Operator op;
        mtac::Size size = mtac::Size::DEFAULT;
        bool address = false; //TODO Use PPARAM instead of PARAM + address = true
        unsigned int depth = 0;

        std::shared_ptr<Variable> secondary; //For CALL

        eddic::Function* m_function; //For PARAM

        std::string m_param; //For LABEL, GOTO, PARAM

        //Filled only in later phase replacing the label
        std::shared_ptr<mtac::basic_block> block;

        //Copy constructors
        Quadruple(const Quadruple& rhs);
        Quadruple& operator=(const Quadruple& rhs);
        
        //Move constructors
        Quadruple(Quadruple&& rhs) noexcept;
        Quadruple& operator=(Quadruple&& rhs) noexcept;

        //Default constructor
        Quadruple();

        //Quadruples without assign to result and no param
        Quadruple(mtac::Operator op);

        //Quadruple for unary operators
        Quadruple(std::shared_ptr<Variable> result, mtac::Argument arg1, mtac::Operator op);

        //Quadruple for binary operators
        Quadruple(std::shared_ptr<Variable> result, mtac::Argument arg1, mtac::Operator op, mtac::Argument arg2);

        //Quadruples without assign to result
        Quadruple(mtac::Operator op, mtac::Argument arg1);

        //Quadruples without assign to result
        Quadruple(mtac::Operator op, mtac::Argument arg1, mtac::Argument arg2);

        //Quadruples manipulating labels (reversed param order to not be ambiguous because of std::string)
        Quadruple(const std::string& param, mtac::Operator op);

        //Quadruples for params
        Quadruple(mtac::Operator op, mtac::Argument arg, std::shared_ptr<Variable> param, eddic::Function& function);
        Quadruple(mtac::Operator op, mtac::Argument arg, const std::string& param, eddic::Function& function);

        Quadruple(mtac::Operator op, mtac::Argument arg, const std::string& label);

        //Quadruple for calls
        Quadruple(mtac::Operator op, eddic::Function& function, std::shared_ptr<Variable> return1 = nullptr, std::shared_ptr<Variable> return2 = nullptr);

        const std::string& label() const;
        const std::string& std_param() const;

        const std::shared_ptr<Variable>& param() const;
        const std::shared_ptr<Variable>& return1() const;
        const std::shared_ptr<Variable>& return2() const;

        eddic::Function& function();
        const eddic::Function& function() const;

        bool is_if();
        bool is_if_false();

        std::size_t uid() const;

        bool operator==(const mtac::Quadruple& quadruple) const;
        bool operator!=(const mtac::Quadruple& quadruple) const;
};

std::ostream& operator<<(std::ostream& stream, const mtac::Quadruple& quadruple);

} //end of mtac

} //end of eddic

#endif
