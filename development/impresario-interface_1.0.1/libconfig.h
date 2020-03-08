/****************************************************************************************************
**   Impresario Interface - Image Processing Engineering System applying Reusable Interactive Objects
**   This file is part of the Impresario Interface.
**
**   Copyright (C) 2015, 2020  Lars Libuda
**   All rights reserved.
**
**   Redistribution and use in source and binary forms, with or without
**   modification, are permitted provided that the following conditions are met:
**       * Redistributions of source code must retain the above copyright
**         notice, this list of conditions and the following disclaimer.
**       * Redistributions in binary form must reproduce the above copyright
**         notice, this list of conditions and the following disclaimer in the
**         documentation and/or other materials provided with the distribution.
**       * Neither the name of the copyright holder nor the
**         names of its contributors may be used to endorse or promote products
**         derived from this software without specific prior written permission.
**
**   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
**   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
**   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
**   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
**   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
**   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
*****************************************************************************************************/
#ifndef LIBCONFIG_H_
#define LIBCONFIG_H_
#include "libinterface.h"

#define LIB_CREATOR        L"<NAME OF CREATOR>"
#define LIB_NAME           L"<NAME OF LIBRARY>"
#define LIB_VERSION_MAJOR  1
#define LIB_VERSION_MINOR  0
#define LIB_VERSION_PATCH  0
#define LIB_DESCRIPTION    L"<html><body><p>YOUR DESCRIPTION</p></body></html>"

#include "<YOUR MACRO HEADER FILE 1>"
#include "<YOUR MACRO HEADER FILE 2>"

MACRO_REGISTRATION_BEGIN
  MACRO_ADD(<YOUR MACRO CLASS 1>)
  MACRO_ADD(<YOUR MACRO CLASS 2>)
MACRO_REGISTRATION_END

#endif // LIBCONFIG_H_
