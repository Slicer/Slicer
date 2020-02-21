
// Slicer includes
#include "qSlicerCorePythonManager.h"
#include "qSlicerScriptedUtils_p.h"

// PythonQt includes
#include <PythonQt.h>

//-----------------------------------------------------------------------------
bool setModuleAttribute(int line,
                        const QString& moduleName,
                        const QString& attributeName,
                        PyObject* expectedAttributeValue,
                        bool expectedResult)
{
  bool currentResult = qSlicerScriptedUtils::setModuleAttribute(
          moduleName, attributeName, expectedAttributeValue);
  if (currentResult != expectedResult)
    {
    std::cerr << "Line " << line << " - Problem with setModuleAttribute()\n"
              << "  moduleName: " << qPrintable(moduleName) << "\n"
              << "  attributeName: " << qPrintable(attributeName) << "\n"
              << "  currentResult: " << currentResult << "\n"
              << "  expectedResult: " << expectedResult << std::endl;
    return false;
    }

  if (!expectedResult)
    {
    return true;
    }

  PyObject* module = PyImport_AddModule(moduleName.isEmpty() ? "__main__" : moduleName.toUtf8());
  if (!module)
    {
    PythonQt::self()->handleError();
    return false;
    }

  PythonQtObjectPtr currentAttributeValue;
  currentAttributeValue.setNewRef(
        PyObject_GetAttrString(module, attributeName.toUtf8()));
  if (currentAttributeValue != expectedAttributeValue)
    {
    std::cerr << "Line " << line << " - Problem with setModuleAttribute()\n"
              << "  moduleName: " << qPrintable(moduleName) << "\n"
              << "  attributeName: " << qPrintable(attributeName) << "\n"
              << "  currentAttributeValue: " << currentAttributeValue << "\n"
              << "  expectedAttributeValue: " << expectedAttributeValue << std::endl;
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
int qSlicerScriptedUtilsTest1(int, char * [] )
{
  qSlicerCorePythonManager pythonManager;
  pythonManager.initialize();
  pythonManager.setSystemExitExceptionHandlerEnabled(false);

  PyImport_AddModule("moduleA");
  PyImport_AddModule("moduleA.moduleB");

  PythonQtObjectPtr attrMain;
  attrMain.setNewRef(PyInt_FromLong(1));

  PythonQtObjectPtr attrMain2;
  attrMain2.setNewRef(PyInt_FromLong(2));

  PythonQtObjectPtr attrA;
  attrA.setNewRef(PyInt_FromLong(10));

  PythonQtObjectPtr attrB;
  attrB.setNewRef(PyInt_FromLong(20));

  if (!setModuleAttribute(__LINE__, "moduleX", "attrX", attrA, false))
    {
    return EXIT_FAILURE;
    }
  if(!setModuleAttribute(__LINE__, "moduleA", "attrA", nullptr, false))
    {
    return EXIT_FAILURE;
    }
  if(!setModuleAttribute(__LINE__, "moduleA", "attrA", attrA, true))
    {
    return EXIT_FAILURE;
    }
  if(!setModuleAttribute(__LINE__, "moduleA.moduleB", "attrB", attrB, true))
    {
    return EXIT_FAILURE;
    }
  if(!setModuleAttribute(__LINE__, "__main__", "attrMain", attrMain, true))
    {
    return EXIT_FAILURE;
    }
  if(!setModuleAttribute(__LINE__, "", "attrMain2", attrMain2, true))
    {
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
