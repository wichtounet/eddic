//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "Options.hpp"

using namespace eddic;

po::variables_map eddic::options;

po::options_description desc("Usage : edic [options]");

void eddic::parseOptions(int argc, const char* argv[]) {
    desc.add_options()
        ("help,h", "Generate this help message")
        ("assembly,S", "Generate only the assembly")
        ("version", "Print the version of eddic")
        ("output,o", po::value<std::string>()->default_value("a.out"), "Set the name of the executable")
        ("optimize-all", "Enable all optimizations")
        ("optimize-strings", "Enable the optimizations on strings")
        ("optimize-integers", "Enable the optimizations on integers")
        ("input", po::value<std::string>(), "Input file");

    po::positional_options_description p;
    p.add("input", -1);

    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), options);
    po::notify(options);
}

void eddic::printHelp(){
    std::cout << desc << std::endl;
}

void eddic::printVersion(){
    std::cout << "eddic version 0.5.0" << std::endl;
}

bool Options::booleanOptions[(int) BooleanOption::COUNT];
std::string Options::valueOptions[(int) ValueOption::COUNT];
        
void Options::setDefaults() {
    set(ValueOption::OUTPUT, "a.out");
}

void Options::set(BooleanOption option) {
    booleanOptions[(int) option] = true;
}

void Options::unset(BooleanOption option) {
    booleanOptions[(int) option] = false;
}

bool Options::isSet(BooleanOption option) {
    return booleanOptions[(int) option];
}

void Options::set(ValueOption option, const std::string& value) {
    valueOptions[(int) option] = value;
}

const std::string& Options::get(ValueOption option) {
    return valueOptions[(int) option];
}
