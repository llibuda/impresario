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
#ifndef MACROEXTENDED_H_
#define MACROEXTENDED_H_

#include "macrobase.h"
#include <memory>

class MacroExtBase : public MacroBase {
public:
  MacroExtBase(const MacroExtBase&) = delete;
  MacroExtBase& operator=(const MacroExtBase&) = delete;
  MacroExtBase(MacroExtBase&&) = delete;
  MacroExtBase& operator=(MacroExtBase&&) = delete;

  // standard constructor
  MacroExtBase() : MacroBase{} {
  }

  // standard destructor
  ~MacroExtBase() override = default;

  // methods to access private attributes, neccessary for the main application
  MacroType getType() const override { return ExtendedMacro; }

  // methods to create and destroy custom widget
  virtual void* createWidget() = 0;
  virtual void destroyWidget() = 0;
};

template <typename T>
class ViewerBase : public MacroExtBase {
public:
  ViewerBase(const ViewerBase&) = delete;
  ViewerBase& operator=(const ViewerBase&) = delete;
  ViewerBase(ViewerBase&&) = delete;
  ViewerBase& operator=(ViewerBase&&) = delete;

  // standard constructor
    ViewerBase() : MacroExtBase{} {
  }

  // standard destructor
  ~ViewerBase() override = default;

  // methods to create and destroy custom widget
  void* createWidget() override {
    if (widgetPtr == nullptr) {
      widgetPtr = std::unique_ptr<T>{new T()};
    }
    return reinterpret_cast<void*>(widgetPtr.get());
  }

  void destroyWidget() override {
    widgetPtr = nullptr;
  }

  // methods to access private attributes, neccessary for the main application
  MacroType getType() const override { return Viewer; }

protected:
  T* accessWidget() const {
    return widgetPtr.get();
  }

private:
  std::unique_ptr<T> widgetPtr;
};


#endif // MACROEXTENDED_H_
