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

#include <TDRGen.h>

// Project includes

// System includes
#include <chrono>
#include <ctime>
#include <iostream>

// Boost includes
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

//#define NON_DETERMINISTIC 1
namespace vecthor {

using namespace std;

const BitVec TDRGen::generateRTDR(unsigned int num_bytes, bool allow_x) {
  BitVec bit_vec(num_bytes);
  auto time_now = chrono::duration_cast<chrono::milliseconds>(
  chrono::system_clock::now().time_since_epoch());
  boost::random::mt19937 rng(time_now.count());
  boost::random::mt19937 rng_x(time_now.count());
 
  boost::random::uniform_int_distribution<> dist(0, 1);
  boost::random::uniform_int_distribution<> dist_x(0, 5); // TODO

  for (unsigned int i = 0; i < num_bytes; i++) {
    bit_vec[i] = dist(rng);
    if (allow_x) {
      auto x_prob = dist_x(rng_x);
      if (x_prob == 3) {
        bit_vec[i] = 2u;
      }
    }
  }
  return bit_vec;
}

unsigned int TDRGen::generateRBit(unsigned int &ctr, bool allow_x) {
  unsigned int ret_val = 0;
  if (ctr > 45) {
    ctr = 0;
  } else if (ctr >= 12 && ctr <= 28) {
    ret_val = 0;
  } else if (ctr >= 38 && ctr <= 45) {
    ret_val = 1;
  } else if (allow_x && ((ctr % 7) == 0)) {
    ret_val = 0;
  } else if ((ctr % 5) == 0) {
    ret_val = 1;
  } else if ((ctr % 3) == 0) {
    ret_val = 0;
  } else {
#ifdef NON_DETERMINISTIC
    auto time_now = chrono::duration_cast<chrono::milliseconds>(
    chrono::system_clock::now().time_since_epoch());
    boost::random::mt19937 rng(time_now.count());
    boost::random::mt19937 rng_x(time_now.count());
#else
    boost::random::mt19937 rng(10);
#endif
    boost::random::uniform_int_distribution<> dist(0, 1);
    return dist(rng);
  }
  ++ctr;
  return ret_val;
}
} // namespace vecthor
