/*
Copyright (c) 2015-2020 Sebastian Huhn < huhn@informatik.uni-bremen.de >

This file is part of VecTHOR < https://github.com/sebastianhuhn/VecTHOR >

VecTHOR is under MIT License:
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Utils.h>

// System includes
#include <iostream>
#include <math.h>

namespace vecthor {

using namespace std;

std::string serializeBitVec(BitVecCItr start, BitVecCItr end) {
  // auto dist = std::distance( start, end );
  // char* ret_val = new char[dist + 1];
  std::string ret_val = "";
  for (unsigned int i = 0; start != end; start++, i++) {
    ret_val += (*start) ? '1' : '0';
  }
  // ret_val[dist] = '\0';
  return ret_val;
}

void writeBitVec(const BitVec &bit_vec, ostream *stream, bool header) {
  if (header) {
    *stream << "\t// Bit vector contains " << bit_vec.size()
            << " bytes:" << endl
            << "\t// ";
  }
  unsigned int ctr = 1;
  for (unsigned int i = 0; i < bit_vec.size(); i++) {
    *stream << (int)bit_vec[i];
    if (ctr % 64 == 0) {
      if (header) {
        *stream << "\t// ";
      }
    } else if (ctr % 8 == 0) {
    }
    ctr++;
  }
  *stream << "\n";
}

void writeBitVec(BitVecCItr start, BitVecCItr end, ostream *stream) {
  BitVec out_vec = BitVec(start, end);
  writeBitVec(out_vec, stream);
}

// Not used.
/*unsigned int convertBitVec( BitVecCItr start, BitVecCItr end )
{
    unsigned int ret = 0;
    for( unsigned int i = 0; start != end; ++i ) {
        --end;
        ret += pow( 2, i ) * ( *end );
    }
    return ret;
}
*/

unsigned int countBitVecX(BitVecCItr start, BitVecCItr end) {
  unsigned int ctr = 0;
  for (; start != end; ++start) {
    if (*start == 2) {
      ++ctr;
    }
  }
  return ctr;
}
} // namespace vecthor
