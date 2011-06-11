#ifndef COMPILER_H
#define COMPILER_H

#include <iostream>
#include <iomanip>
#include <fstream>

#include "ByteCodeFileWriter.h"

using namespace std;

class Compiler {
  public:
    int compile (string file);
  private:
    int compile (ifstream* inStream, ByteCodeFileWriter* outStream);
};

#endif
