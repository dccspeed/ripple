// MIT License
//
// Copyright (c) 2017-2019 Stefano Leucci and Marco Bressan
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef MOTIVO_INCLUDE_NAUTY_H
#define MOTIVO_INCLUDE_NAUTY_H

#if defined(HAVE_TLS) || defined(MAXN)
    #error defines clash
#endif

#include <nauty/nauty.h>

#if(!defined(HAVE_TLS) || HAVE_TLS==0)
    #error Nauty does not have multithreading support
#endif

#if(MAXN != 0)
    #error Nauty is compiled with static allocation
#endif

static constexpr int MOTIVO_NAUTY_WORDSIZE = WORDSIZE;
static constexpr int MOTIVO_NAUTY_TRUE = TRUE;

typedef graph nauty_graph;
typedef set nauty_set;

#endif //MOTIVO_INCLUDE_NAUTY_H
