//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_QUADRUPLE_H
#define MTAC_QUADRUPLE_H

#include <memory>
#include <ostream>
#include <boost/optional.hpp>

#include "tac/Size.hpp"

#include "mtac/Operator.hpp"
#include "mtac/Argument.hpp"

namespace eddic {

class Function;
class Variable;

namespace mtac {

class basic_block;
typedef std::shared_ptr<basic_block> basic_block_p;

//TODO Quadruple should be made smaller to allow faster removal

struct Quadruple {
    private:
        std::size_t _uid;

    public:
        std::shared_ptr<Variable> result;
        boost::optional<mtac::Argument> arg1;
        boost::optional<mtac::Argument> arg2;
        mtac::Operator op;
        tac::Size size;

        std::shared_ptr<Variable> secondary; //For CALL

        eddic::Function* m_function = nullptr; //For PARAM

        std::string m_param; //For LABEL, GOTO, PARAM

        //Filled only in later phase replacing the label
        mtac::basic_block_p block;

        //Copy constructors
        Quadruple(const Quadruple& rhs);
        Quadruple& operator=(const Quadruple& rhs);
        
        //TODO Apparently, it is not really noexcept
        
        //Move constructors
        Quadruple(Quadruple&& rhs) noexcept;
        Quadruple& operator=(Quadruple&& rhs) noexcept;

        //Quadruples without assign to result and no param
        explicit Quadruple(mtac::Operator op, tac::Size = tac::Size::DEFAULT);

        //Quadruple for unary operators
        explicit Quadruple(std::shared_ptr<Variable> result, mtac::Argument arg1, mtac::Operator op, tac::Size = tac::Size::DEFAULT);

        //Quadruple for binary operators
        explicit Quadruple(std::shared_ptr<Variable> result, mtac::Argument arg1, mtac::Operator op, mtac::Argument arg2, tac::Size = tac::Size::DEFAULT);

        //Quadruples without assign to result
        explicit Quadruple(mtac::Operator op, mtac::Argument arg1, tac::Size = tac::Size::DEFAULT);

        //Quadruples without assign to result
        explicit Quadruple(mtac::Operator op, mtac::Argument arg1, mtac::Argument arg2, tac::Size = tac::Size::DEFAULT);

        //Quadruples manipulating labels (reversed param order to not be ambiguous because of std::string)
        explicit Quadruple(std::string param, mtac::Operator op, tac::Size = tac::Size::DEFAULT);

        //Quadruples for params
        explicit Quadruple(mtac::Operator op, mtac::Argument arg, std::shared_ptr<Variable> param, eddic::Function& function, tac::Size = tac::Size::DEFAULT);
        explicit Quadruple(mtac::Operator op, mtac::Argument arg, std::string param, eddic::Function& function, tac::Size = tac::Size::DEFAULT);

        explicit Quadruple(mtac::Operator op, mtac::Argument arg, std::string label, tac::Size = tac::Size::DEFAULT);

        //Quadruple for calls
        explicit Quadruple(mtac::Operator op, eddic::Function& function, std::shared_ptr<Variable> return1 = nullptr, std::shared_ptr<Variable> return2 = nullptr, tac::Size = tac::Size::DEFAULT);

        const std::string& label() const;
        const std::string& std_param() const;

        const std::shared_ptr<Variable>& param() const;
        const std::shared_ptr<Variable>& return1() const;
        const std::shared_ptr<Variable>& return2() const;

        eddic::Function& function();
        const eddic::Function& function() const;

        bool is_if();
        bool is_if_false();

        std::size_t uid() const {
            return _uid;
        }

        bool operator==(const mtac::Quadruple& quadruple) const;
        bool operator!=(const mtac::Quadruple& quadruple) const;
};

void transform_to_nop(mtac::Quadruple& quadruple);

std::ostream& operator<<(std::ostream& stream, const mtac::Quadruple& quadruple);

} //end of mtac

} //end of eddic

#endif
