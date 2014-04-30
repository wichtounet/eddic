#ifndef PARSER_ITERATOR_H
#define PARSER_ITERATOR_H

#include <string>

#include <boost/spirit/include/classic_position_iterator.hpp>

typedef std::string::iterator base_iterator_type;
typedef boost::spirit::classic::position_iterator2<base_iterator_type> pos_iterator_type;
//typedef extended_iterator<base_iterator_type> pos_iterator_type;

#endif
