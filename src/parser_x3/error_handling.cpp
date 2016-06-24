#include "parser_x3/error_handling.hpp"
#include "SemanticalException.hpp"

void x3_grammar::global_error_handler::operator()(const boost::spirit::x3::file_position_tagged& t, const std::string& message){
    if(t.id_file == -1){
        std::cerr << "Unnotated AST node: " << message << std::endl;
    } else {
        error_handlers[t.id_file](t, message);
    }
}

std::string x3_grammar::global_error_handler::to_string(const boost::spirit::x3::file_position_tagged& t, const std::string& message){
    if(t.id_file == -1){
        return std::string("Unnotated AST node: ") + message;
    } else {
        return error_handlers[t.id_file].to_string(t, message);
    }
}

void x3_grammar::global_error_handler::operator()(iterator_type err_pos, const std::string& message){
    current()(err_pos, message);
}

void x3_grammar::global_error_handler::operator()(iterator_type err_pos, iterator_type err_last, const std::string& message){
    current()(err_pos, err_last, message);
}

void x3_grammar::global_error_handler::semantical_exception(const std::string& message, const boost::spirit::x3::file_position_tagged& t){
    throw eddic::SemanticalException(to_string(t, message));
}
