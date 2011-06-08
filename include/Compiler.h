#ifndef COMPILER_H
#define COMPILER_H

#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

class Compiler {
  public:
    int compile (string file);
  private:
    int compile (ifstream* inStream, ofstream* outStream);
};

#endif
