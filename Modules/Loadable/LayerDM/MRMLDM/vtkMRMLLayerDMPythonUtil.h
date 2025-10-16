#pragma once
#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

// VTK includes
#include <vtkObject.h>
#include <vtkPython.h>
#include <vtkPythonUtil.h>
#include <vtkSmartPyObject.h>

// STD includes
#include <vector>

/// \brief Utility class for Python/C++ interoperability in VTK MRML Layer Displayable Manager
///
/// This class provides a collection of static utility methods for converting between
/// VTK C++ objects and Python objects, managing Python object lifecycles, and calling
/// Python methods from C++. It serves as a bridge layer for Python integration in the
/// Layer Displayable Manager module.
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMPythonUtil : public vtkObject
{
public:
  static vtkMRMLLayerDMPythonUtil* New();
  vtkTypeMacro(vtkMRMLLayerDMPythonUtil, vtkObject);

  /// \brief Convert a VTK object to a Python object
  /// \param obj VTK object base pointer to convert
  /// \return PyObject* Python object representation of the VTK object
  static PyObject* ToPyObject(vtkObjectBase* obj);

  /// \brief Convert an unsigned long value to a Python object
  /// \param value Unsigned long value to convert
  /// \return PyObject* Python object representation of the value
  static PyObject* ToPyObject(unsigned long value);

  /// \brief Convert a raw C pointer to a Python object
  /// \param ptr Raw pointer to convert
  /// \return PyObject* Python object wrapping the pointer
  static PyObject* RawPtrToPython(void* ptr);

  /// \brief Create a Python tuple from a vector of Python objects
  /// \param pyObjs Vector of Python objects to package as arguments
  /// \return vtkSmartPyObject Smart pointer to Python tuple containing the objects
  static vtkSmartPyObject ToPyArgs(const std::vector<PyObject*>& pyObjs);

  /// \brief Create a Python tuple from a single VTK object
  /// \param obj VTK object base pointer to package as arguments
  /// \return vtkSmartPyObject Smart pointer to Python tuple containing the object
  static vtkSmartPyObject ToPyArgs(vtkObjectBase* obj);

  /// \brief Create a Python tuple for VTK event callback arguments
  /// \param obj VTK object that triggered the event
  /// \param eventId Event identifier
  /// \param callData Additional event-specific data
  /// \return vtkSmartPyObject Smart pointer to Python tuple containing the event arguments
  static vtkSmartPyObject ToPyArgs(vtkObject* obj, unsigned long eventId, void* callData);

  /// \brief Cast call data to appropriate Python type based on VTK type
  /// \param object Python object to cast
  /// \param vtkType VTK type identifier to cast to
  /// \return PyObject* Casted Python object
  static PyObject* CastCallData(PyObject* object, int vtkType);

  /// \brief Call a named method on a Python object with arguments
  /// \param object Python object on which to call the method
  /// \param pyArgs Python tuple of arguments to pass to the method
  /// \param fName Name of the method to call
  /// \return PyObject* Return value from the Python method call
  static PyObject* CallPythonMethod(PyObject* object, const vtkSmartPyObject& pyArgs, const std::string& fName);

  /// \brief Call a Python callable object with arguments
  /// \param object Python callable object to invoke
  /// \param pyArgs Python tuple of arguments to pass
  /// \return PyObject* Return value from the Python callable
  static PyObject* CallPythonObject(PyObject* object, const vtkSmartPyObject& pyArgs);

  /// \brief Safely set a Python object pointer with proper reference counting
  /// \param destObject Pointer to destination PyObject pointer to update
  /// \param object Source Python object to assign
  static void SetPythonObject(PyObject** destObject, PyObject* object);

  /// \brief Safely delete a Python object with proper reference counting
  /// \param destObject Pointer to PyObject pointer to delete and set to nullptr
  static void DeletePythonObject(PyObject** destObject);

protected:
  vtkMRMLLayerDMPythonUtil();
  ~vtkMRMLLayerDMPythonUtil() override;
};
