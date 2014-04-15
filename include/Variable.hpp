//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef VARIABLE_H
#define VARIABLE_H

#include <utility>
#include <string>
#include <memory>
#include <ostream>

#include "variant.hpp"
#include "Position.hpp"
#include "assert.hpp"

#include "ast/Position.hpp"

namespace eddic {

class Type;
class Variable;

typedef boost::variant<int, double, bool, std::pair<std::string, int>> Val;
typedef boost::variant<int, std::shared_ptr<Variable>> Offset;

/*!
 * \class Variable
 * \brief An entry of the symbol table
 */
class Variable {
    private:
        std::size_t m_references = 0;

        const std::string m_name;
        std::shared_ptr<const Type> m_type;

        Position m_position;
        ast::Position m_source_position;
        Val v_value;

        //For temporary references
        std::shared_ptr<Variable> m_reference = nullptr;
        Offset m_offset;

    public:
        Variable(std::string name, std::shared_ptr<const Type> type, Position position);
        Variable(std::string name, std::shared_ptr<const Type> type, Position position, Val value);
        Variable(std::string name, std::shared_ptr<const Type> type, std::shared_ptr<Variable> reference, Offset offset);

        //TODO Find out why cannot be noexcept
        Variable(Variable&& rhs) = default;
        Variable& operator=(Variable&& rhs) = default;

        std::size_t references() const;
        void add_reference();

        std::string name() const ;
        std::shared_ptr<const Type> type() const ;
        Position position() const ;

        Val val() const ;
        
        const ast::Position& source_position() const ;
        void set_source_position(const ast::Position& position);

        void setPosition(Position position);

        bool is_reference() const;
        std::shared_ptr<Variable> reference() const;
        Offset reference_offset() const;
};

std::ostream& operator<<(std::ostream& stream, const Variable& variable);
std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<Variable>& variable);

} //end of eddic

#endif
