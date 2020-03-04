#ifndef MYMACROCLASS_H_
#define MYMACROCLASS_H_

#include "macrobase.h"

class MyMacroClass : public MacroBase {
public:
  // Standard constructor: the macro's interface to Impresario
  // is defined here.
  MyMacroClass(void);
  
  // Standard destructor
  virtual ~MyMacroClass(void);

  // Override clone method to provide correct class instance. 
  // This method must be overridden in any case, otherwise 
  // the macro cannot be used in Impresario. 
  // The base class implementation just returns 0. The following
  // line shows the minimal implementation.
  virtual MacroBase* clone() const { 
    return new MyMacroClass(); 
  }

protected:
  // Override this method if you need special one time 
  // preparation for processing in the onApply() method. 
  // The base class implementation just returns Ok.
  // This method is called by Impresario as the first event 
  // before processing starts. 
  virtual Status onInit();
  
  // Override this method with your processing code. It is 
  // called by Impresario for every new set of input data during 
  // processing. The base class implementation just returns Ok.
  virtual Status onApply();

  // Override this method in case you need cleanup after 
  // processing has been stopped by the user or it has been 
  // terminated by Impresario due to errors reported by
  // macro instances.
  // The base class implementation just returns Ok.
  virtual Status onExit();
  
  // Override this method to be notified about every parameter 
  // change. This method is called by Impresario before each 
  // call to onInit(), onApply(), and onExit(). Before the call 
  // to onInit() all parameters are reported as changed to allow 
  // for proper initialization. In further calls only parameters 
  // are reported which have been changed since the last time. 
  // ParameterSet is an std::set<unsigned int> containing the 
  // zero-based indices of all parameters which have been 
  // changed since the last call. 
  // The base class implementation does nothing. 
  virtual void onParametersChanged(ParameterSet& parameters);

private:
  // Put your attributes and further methods here
  
};

#endif // MYMACROCLASS_H_