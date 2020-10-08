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

#pragma once

#ifndef UTILS_H
#define UTILS_H

// Project includes
#include <TypeDefs.h>

// System includes
#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace vecthor {

// unsigned int convertBitVec( BitVecCItr start, BitVecCItr end );
unsigned int countBitVecX(BitVecCItr start, BitVecCItr end);
std::string serializeBitVec(BitVecCItr start, BitVecCItr end);
void writeBitVec(const BitVec &bit_vec, std::ostream *stream = &std::cout,
                 bool header = true);
void writeBitVec(BitVecCItr start, BitVecCItr end,
                 std::ostream *stream = &std::cout);
} // namespace vecthor

#endif // UTILS_H
