//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "ast/TypeTransformer.hpp"

using namespace eddic;

std::shared_ptr<const eddic::Type> ast::TypeTransformer::operator()(ast::SimpleType& type) const {
    return new_type(context, type.type, type.const_);
}

std::shared_ptr<const eddic::Type> ast::TypeTransformer::operator()(ast::ArrayType& type) const {
    return new_array_type(visit(*this, type.type.get()));
}

std::shared_ptr<const eddic::Type> ast::TypeTransformer::operator()(ast::PointerType& type) const {
    return new_pointer_type(new_type(context, type.type));
}

std::shared_ptr<const eddic::Type> ast::TypeTransformer::operator()(ast::TemplateType& type) const {
    std::vector<std::shared_ptr<const eddic::Type>> template_types;

    for(auto& tmp_type : type.template_types){
       template_types.push_back(visit(*this, tmp_type));
    }

    return new_template_type(type.type, template_types);
}
