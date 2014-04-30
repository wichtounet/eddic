#ifndef PARSER_PRINTER_H
#define PARSER_PRINTER_H

namespace x3_ast {

struct printer: public boost::static_visitor<>  {
    std::size_t i = 0;

    std::string indent(){
        std::string v(i, ' ');
        return v;
    }

    void operator()(const source_file& source_file){
        std::cout << indent() << "source_file" << std::endl;

        i += 2;
        for(auto& block : source_file.blocks){
            boost::apply_visitor(*this, block);
        }
        i -= 2;
    }

    void operator()(const standard_import& import){
        std::cout << indent() << "standard_import: " << import.file << std::endl;
    }
    
    void operator()(const import& import){
        std::cout << indent() << "import: " << import.file << std::endl;
    }
    
    void operator()(const template_struct&){
        //TODO
    }
    
    void operator()(const template_function_declaration& function){
        std::cout << indent() << "template_function_declaration: " << function.name << std::endl;
        i += 2;
        std::cout << indent() << "template_types: ";
        for(auto& v : function.template_types) std::cout << v << ", ";
        std::cout << std::endl << indent() << "return_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, function.return_type);
        i -= 2;
        std::cout << indent() << "parameters: " << std::endl;
        i += 2;
        for(auto& parameter : function.parameters){
            std::cout << indent() << "name: " << parameter.parameter_name << std::endl;
            std::cout << indent() << "type: " << std::endl;
            i += 2;
            boost::apply_visitor(*this, parameter.parameter_type);
            i -= 2;
        }
        i -= 2;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : function.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const foreach& foreach){
        std::cout << indent() << "foreach: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, foreach.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << foreach.variable_name << std::endl;
        std::cout << indent() << "from: " << foreach.from << std::endl;
        std::cout << indent() << "to: " << foreach.to << std::endl;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : foreach.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }
    
    void operator()(const foreach_in& foreach){
        std::cout << indent() << "foreach_in: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, foreach.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << foreach.variable_name << std::endl;
        std::cout << indent() << "array_name: " << foreach.array_name << std::endl;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : foreach.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const while_& loop){
        std::cout << indent() << "while: " << std::endl;
        i += 2;
        std::cout << indent() << "condition: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, loop.condition);
        i -= 2;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : loop.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const do_while& loop){
        std::cout << indent() << "do while: " << std::endl;
        i += 2;
        std::cout << indent() << "condition: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, loop.condition);
        i -= 2;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : loop.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const if_& if_){
        std::cout << indent() << "if: " << std::endl;
        i += 2;
        std::cout << indent() << "condition: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, if_.condition);
        i -= 2;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : if_.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }
    
    void operator()(const return_& return_){
        std::cout << indent() << "return: " << std::endl;
        i += 2;
        std::cout << indent() << "value: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, return_.return_value);
        i -= 2;
        i -= 2;
    }
    
    void operator()(const delete_& delete_){
        std::cout << indent() << "delete: " << std::endl;
        i += 2;
        std::cout << indent() << "value: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, delete_.value);
        i -= 2;
        i -= 2;
    }
    
    void operator()(const variable_declaration& declaration){
        std::cout << indent() << "variable_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << declaration.variable_name << std::endl;
        if(declaration.value){
            std::cout << indent() << "value: " << std::endl;
            i += 2;
            boost::apply_visitor(*this, *declaration.value);
            i -= 2;
        }
        i -= 2;
    }
    
    void operator()(const struct_declaration& declaration){
        std::cout << indent() << "struct_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << declaration.variable_name << std::endl;
        std::cout << indent() << "values: " << std::endl;
        i += 2;
        for(auto& v : declaration.values){
            boost::apply_visitor(*this, v);
        }
        i -= 2;
        i -= 2;
    }
    
    void operator()(const global_variable_declaration& declaration){
        std::cout << indent() << "global_variable_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << declaration.variable_name << std::endl;
        if(declaration.value){
            std::cout << indent() << "value: " << std::endl;
            i += 2;
            boost::apply_visitor(*this, *declaration.value);
            i -= 2;
        }
        i -= 2;
    }
    
    void operator()(const global_array_declaration& declaration){
        std::cout << indent() << "global_array_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "array_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.array_type);
        i -= 2;
        std::cout << indent() << "array_name: " << declaration.array_name << std::endl;
        std::cout << indent() << "size: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.size);
        i -= 2;
        i -= 2;
    }
    
    void operator()(const array_declaration& declaration){
        std::cout << indent() << "array_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "array_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.array_type);
        i -= 2;
        std::cout << indent() << "array_name: " << declaration.array_name << std::endl;
        std::cout << indent() << "size: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.size);
        i -= 2;
        i -= 2;
    }

    void operator()(const simple_type& type){
        std::cout << indent() << "simple_type: " << type.base_type << std::endl;
    }
    
    void operator()(const array_type& type){
        std::cout << indent() << "array_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, type.base_type);
        i -= 2;
    }
    
    void operator()(const template_type& type){
        std::cout << indent() << "template_type: " << std::endl;
        i += 2;
        std::cout << indent() << "base_type: " << type.base_type << std::endl;
        std::cout << indent() << "template_types: " << std::endl;
        i += 2;
        for(auto& t : type.template_types){
            boost::apply_visitor(*this, t);
        }
        i -= 2;
        i -= 2;
    }
    
    void operator()(const pointer_type& type){
        std::cout << indent() << "pointer_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, type.base_type);
        i -= 2;
    }

    void operator()(const integer_literal& integer){
        std::cout << indent() << "integer_literal: " << integer.value << std::endl;
    }
    
    void operator()(const integer_suffix_literal& integer){
        std::cout << indent() << "integer_suffix_literal: " << integer.value << integer.suffix << std::endl;
    }

    void operator()(const float_literal& integer){
        std::cout << indent() << "float_literal: " << integer.value << std::endl;
    }
    
    void operator()(const string_literal& integer){
        std::cout << indent() << "string_literal: " << integer.value << std::endl;
    }
    
    void operator()(const char_literal& integer){
        std::cout << indent() << "char_literal: " << integer.value << std::endl;
    }
    
    void operator()(const variable_value& integer){
        std::cout << indent() << "variable_value: " << integer.variable_name << std::endl;
    }
};

}

#endif
