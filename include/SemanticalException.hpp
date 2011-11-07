//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef SEMANTICAL_EXCEPTION_H
#define SEMANTICAL_EXCEPTION_H

#include <string>

namespace eddic {

class SemanticalException: public std::exception {
    protected:
        std::string m_message;

    public:
        SemanticalException(const std::string& message);
        ~SemanticalException() throw();

        virtual const char* what() const throw();
};

} //end of eddic

#endif
