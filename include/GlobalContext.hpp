//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef GLOBAL_CONTEXT_H
#define GLOBAL_CONTEXT_H

#include <map>

#include "Context.hpp"
#include "Function.hpp"
#include "Struct.hpp"
#include "Platform.hpp"
#include "statistics.hpp"
#include "timing.hpp"

namespace eddic {

/*!
 * \struct GlobalContext
 * \brief The global symbol table for the whole source. 
 *
 * There is always only one instance of this class in the application. This symbol table is responsible
 * of storing all the global variables. It is also responsible for storing the global functions and structures. 
 */
struct GlobalContext final : public Context {
    public: 
        typedef std::unordered_map<std::string, Function> FunctionMap;
        typedef std::unordered_map<std::string, std::shared_ptr<Struct>> StructMap;
    
    public:
        GlobalContext(Platform platform);
        
        Variables getVariables();
        
        std::shared_ptr<Variable> addVariable(const std::string& a, std::shared_ptr<const Type> type);
        std::shared_ptr<Variable> addVariable(const std::string& a, std::shared_ptr<const Type> type, ast::Value& value);
        
        std::shared_ptr<Variable> generate_variable(const std::string& prefix, std::shared_ptr<const Type> type) override;
        
        /*!
         * Add function to the symbol table. The function will be constructed in place. 
         * \param ret The return type of the function. 
         * \param name The name of the function
         * \param mangled_name The mangled name of the function
         * \return A reference to the newly created Function. 
         */
        Function& add_function(std::shared_ptr<const Type> ret, const std::string& name, const std::string& mangled_name);
        
        /*!
         * Returns the function with the given name. 
         * \param function The function to search for. 
         * \return A pointer to the function with the given name. 
         */
        Function& getFunction(const std::string& function);
        
        /*!
         * Indicates if a function with the given name exists. 
         * \param function The function to search for. 
         * \return true if a function with the given name exists, otherwise false. 
         */
        bool exists(const std::string& function) const;

        /*!
         * Add the given structure to the symbol table. 
         * \param struct_ The structure to add to the symbol table. 
         */
        void add_struct(std::shared_ptr<Struct> struct_);
        
        /*!
         * Returns the structure with the given name. 
         * \param struct_ The structure to search for. 
         * \return A pointer to the structure with the given name. 
         */
        std::shared_ptr<Struct> get_struct(const std::string& struct_) const;
        
        /*!
         * Returns the structure of the given type. 
         * \param type The structure type to search for. 
         * \return A pointer to the structure of the given type. 
         */
        std::shared_ptr<Struct> get_struct(std::shared_ptr<const Type> type) const;
        
        /*!
         * Indicates if a structure with the given name exists. 
         * \param struct_ The structure to search for. 
         * \return true if a structure with the given name exists, otherwise false. 
         */
        bool struct_exists(const std::string& struct_) const ;
        
        /*!
         * Indicates if a structure for the given type exists. 
         * \param type The structure type to search for. 
         * \return true if a structure for the given type exists, otherwise false. 
         */
        bool struct_exists(std::shared_ptr<const Type> type) const ;
        
        std::shared_ptr<const Type> member_type(std::shared_ptr<const Struct> struct_, int offset) const;
        int member_offset(std::shared_ptr<const Struct> struct_, const std::string& member) const;

        int self_size_of_struct(std::shared_ptr<const Struct> struct_) const;
        int total_size_of_struct(std::shared_ptr<const Struct> struct_) const;
        bool is_recursively_nested(std::shared_ptr<const Struct> struct_) const;

        std::size_t new_file(const std::string& file_name);
        std::string& get_file_content(std::size_t file);
        const std::string& get_file_name(std::size_t file);

        const FunctionMap& functions() const;

        Platform target_platform() const;

        statistics& stats();
        timing_system& timing();
    
    private:
        FunctionMap m_functions;
        StructMap m_structs;
        statistics m_statistics;
        timing_system m_timing;
        Platform platform;

        std::vector<std::string> file_names;
        std::vector<std::string> file_contents;

        void addPrintFunction(const std::string& function, std::shared_ptr<const Type> parameterType);
        void defineStandardFunctions();
        
        bool is_recursively_nested(std::shared_ptr<const Struct> struct_, unsigned int left) const;
};

} //end of eddic

#endif
