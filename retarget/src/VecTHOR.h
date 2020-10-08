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

#ifndef VECTHOR_H
#define VECTHOR_H

// Project includes
#include <Config.h>
#include <TypeDefs.h>

// System includes
#include <memory>
#include <string>

namespace vecthor {

class Compressor;
using CompressorPtr = std::unique_ptr<Compressor>;

class Decompressor;
using DecompressorPtr = std::unique_ptr<Decompressor>;

class CompressedEmitter;
using CompressedEmitterPtr = std::unique_ptr<CompressedEmitter>;

class Validator;
using ValidatorPtr = std::unique_ptr<Validator>;

class VecTHOR {

public:
  Config &getConfig() { return m_config; }
  void init();
  void finalize();
  bool prepare();
  bool run();
  void reset();
  void validate();

private:
  Config m_config;

  std::string m_run_name;
  BitVec m_bit_vec;
  CompressorPtr m_compressor = nullptr;
  DecompressorPtr m_decompressor = nullptr;
  CompressedEmitterPtr m_emitter = nullptr;
  ValidatorPtr m_validator = nullptr;
};
} // namespace vecthor

int main(int argc, char **argv);

#endif
