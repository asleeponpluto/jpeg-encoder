// //////////////////////////////////////////////////////////
// originally toojpeg.h
// written by Stephan Brumme, 2018-2019
// see https://create.stephan-brumme.com/toojpeg/
//
// This is a compact baseline JPEG/JFIF writer, written in C++ (but looks like C for the most part).
// Its interface has only one function: writeJpeg() - and that's it !
//
// modified by Joseph Somerdin, 2022

#pragma once

#include "Encoder.h"

namespace TooJpeg
{
    // wf           - output file stream (to write byte by byte)
    // blocks       - vector of blocks that include l, cb, and cr
    // width,height - image size
    // comment      - optional JPEG comment (0/NULL if no comment), must not contain ASCII code 0xFF
    bool writeJpeg(std::ofstream& wf, const std::vector<Encoder::Block>& blocks, unsigned short width, unsigned short height, const char* comment = nullptr);
} // namespace TooJpeg
