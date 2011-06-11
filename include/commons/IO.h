#ifndef IO_H
#define IO_H

#include <iostream>
#include <iomanip>

template<typename T>
std::ostream& binary_write(std::ostream* stream, const T& value){
	return stream->write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template<typename T>
std::istream & binary_read(std::istream* stream, T& value)
{
    return stream->read(reinterpret_cast<char*>(&value), sizeof(T));
}

std::ostream& binary_write_string(std::ofstream* stream, std::string value);

#endif
