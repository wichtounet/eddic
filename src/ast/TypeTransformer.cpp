//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "ast/TypeTransformer.hpp"

using namespace eddic;

std::shared_ptr<const eddic::Type> ast::TypeTransformer::operator()(const ast::SimpleType& type) const {
    return new_type(context, type.type, type.const_);
}

std::shared_ptr<const eddic::Type> ast::TypeTransformer::operator()(const ast::ArrayType& type) const {
    return new_array_type(visit(*this, type.type.get()));
}

std::shared_ptr<const eddic::Type> ast::TypeTransformer::operator()(const ast::PointerType& type) const {
    return new_pointer_type(visit(*this, type.type.get()));
}

std::shared_ptr<const eddic::Type> ast::TypeTransformer::operator()(const ast::TemplateType& type) const {
    std::vector<std::shared_ptr<const eddic::Type>> template_types;

    for(auto& tmp_type : type.template_types){
       template_types.push_back(visit(*this, tmp_type));
    }

    return new_template_type(context, type.type, template_types);
}
