#include "mymacroclass.h"
// Include your header files here

MyMacroClass::MyMacroClass() : MacroBase() {
  // Set up the macro's name as it will appear in Impresario.
  // This must not necessarily be identical to the class name.
  setName(L"My macro name");
  // Set up the programmer's name. It is displayed in 
  // Impresario's macro data base and can be used for 
  // filtering and sorting.
  setCreator(L"Programmer's name");
  // Set up the the group this macro belongs to. It is displayed
  // in Impresario's macro data base and can be used for 
  // filtering and sorting.
  setGroup(L"My macro group");
  // Set up a description of the macro. It is displayed in 
  // Impresario.
  setDescription(L"My description");
  
  // Special case: If you provide your own QML widget for 
  // displaying and editing the macro's parameters, you can 
  // provide Impresario the widget's file name with the 
  // following call. Impresario expects this file to be found 
  // in its resource directory. Please refer to the Developer's
  // Guide for more information about using customized widgets.
  //setPropertyWidgetComponent(L"MyQMLWidget.qml");
  
  // Define your inputs with addInput. Example for an input 
  // of type "int":
  addInput<int>(L"Name of input as displayed in Impresario",
                L"Your description of this input");
  
  // Define your outputs with addOutput. Example for an output
  // of type "int":
  addOutput<int>(L"Name of output as displayed in Impresario",
                 L"Your description of this output");
  
  // Define your parameters with addParameter. Please refer to
  // the Developer's Guide for more information about how to 
  // define parameters.
  // Example for a parameter of type "int":
  addParameter<int>(L"Name of output as displayed in Impresario", 
                    L"Your description of this parameter", 3, 
                    L"IntSpinBox",
                    L"{ \"minValue\": 3, \"maxValue\": 15 }");
}

MyMacroClass::~MyMacroClass() {
  // Empty by default but can be used for releasing memory, etc.
}

MacroBase::Status MyMacroClass::onInit() {
  // Place your code here which will be run before the first 
  // call to onApply(). The following example checks whether 
  // the first input with index 0 is connected.
  // Access the input with index 0, this is the first input 
  // defined in the constructor:
  const int* input = accessInput<int>(0);
  // Example: check whether input is connected in Impresario:
  if (!input) {
    // Set error message to be displayed in Impresario's 
    // system message window:
    setErrorMsg(L"Input is not connected.");
    // Return Error to stop further processing in Impresario:
    return Error;
  }
  // Return Ok to continue processing in Impresario.
  return Ok;
}

MacroBase::Status MyMacroClass::onApply() {
  // Here you code your processing algorithm. The following
  // code example shows how to access the input, output, and 
  // parameter as created in the constructor.
  // Access input:
  const int* input = accessInput<int>(0);
  // Access output:
  int& output = accessOutput<int>(0);
  // Access parameter:
  const int& param = getParameterValue<int>(0);
  // Do our magic
  output = param * input;
  // Return Ok to tell Impresario to continue processing.
  return Ok;
  // In case your code produces an unrecoverable error, 
  // you can set an error message and return Error afterwards:
  //setErrorMsg(L"An error occurred during processing.");
  //return Error;
  // You can also return "Stop" in case you want Impresario 
  // to stop processing without further user interaction. 
  // This will stop the complete process graph without error 
  // notification.
  //return Stop;  
}

MacroBase::Status MyMacroClass::onExit() {
  // Place your code here which is run after the last
  // call to onApply(). This method is called in any case, 
  // independent of how processing was stopped.
  // Return Ok to indicate, your code was successfully 
  // executed or Error in conjunction with setErrorMsg 
  // to indicate an error.
  return Ok;
}

void MyMacroClass::onParametersChanged(ParameterSet& parameters) {
  // The code in here will be run before onInit(), onApply(),
  // and onExit(). In case parameters have been changed since
  // the last run, the ParameterSet contains the indices of 
  // all changed parameters. However, before the call to 
  // onInit() all parameters will be reported as changed to 
  // allow for proper initialization. You can use this method
  // to do one time calculations depending on changed
  // parameters. Example:
  // Check whether parameter with index 0 has been changed:
  if (parameters.find(0) != parameters.end()) {
    const int& param = getParameterValue<int>(0);
    // do something meaningful...
  }
}