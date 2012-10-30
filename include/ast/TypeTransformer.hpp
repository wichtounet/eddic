//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPE_TRANSFORMER_H
#define TYPE_TRANSFORMER_H

#include <memory>

#include "variant.hpp"

#include "ast/VariableType.hpp"

namespace eddic {

class GlobalContext;
class Type;

namespace ast {

/*!
 * \class TypeTransformer
 * \brief AST visitor to transform an AST type into a type descriptor.   
 */
class TypeTransformer : public boost::static_visitor<std::shared_ptr<const eddic::Type>> {
    public:
        TypeTransformer(std::shared_ptr<GlobalContext> context) : context(context) {}

        std::shared_ptr<const eddic::Type> operator()(const ast::SimpleType& type) const;
        std::shared_ptr<const eddic::Type> operator()(const ast::ArrayType& type) const;
        std::shared_ptr<const eddic::Type> operator()(const ast::PointerType& type) const;
        std::shared_ptr<const eddic::Type> operator()(const ast::TemplateType& type) const;

    private:
        std::shared_ptr<GlobalContext> context;
};

} //end of ast

} //end of eddic

#endif
