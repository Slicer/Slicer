/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Python includes (must be first)
#include "vtkSlicerScriptedFileIOUtilities_p.h"

#include "vtkSlicerScriptedFileWriterBridge.h"

// MRML includes
#include <vtkMRMLIOOptionsDescription.h>
#include <vtkMRMLIOProperties.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkWeakReference.h>

namespace utils = vtkSlicerScriptedFileIOUtilities;

//----------------------------------------------------------------------------
class vtkSlicerScriptedFileWriterBridge::vtkInternal
{
public:
  /// Weak reference to this object (see GetWeakReference)
  vtkNew<vtkWeakReference> WeakReference;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerScriptedFileWriterBridge);

//----------------------------------------------------------------------------
vtkSlicerScriptedFileWriterBridge::vtkSlicerScriptedFileWriterBridge()
{
  this->Internal = new vtkInternal;
  this->Internal->WeakReference->Set(this);
}

//----------------------------------------------------------------------------
vtkSlicerScriptedFileWriterBridge::~vtkSlicerScriptedFileWriterBridge()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkSlicerScriptedFileWriterBridge::UnRegisterInternal(vtkObjectBase* o, vtkTypeBool check)
{
  if (this->GetReferenceCount() == 1)
  {
    // The last reference is released, the object will be deleted
    this->Internal->WeakReference->Set(nullptr);
  }
  this->Superclass::UnRegisterInternal(o, check);
}

//----------------------------------------------------------------------------
void vtkSlicerScriptedFileWriterBridge::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  std::string pythonClassName = "(none)";
  if (Py_IsInitialized())
  {
    vtkPythonScopeGilEnsurer gilEnsurer;
    vtkSmartPyObject object;
    object.TakeReference(utils::GetPythonObject(this));
    if (object)
    {
      pythonClassName = Py_TYPE(object.GetPointer())->tp_name;
    }
  }
  os << indent << "Python class: " << pythonClassName << "\n";
}

//----------------------------------------------------------------------------
vtkWeakReference* vtkSlicerScriptedFileWriterBridge::GetWeakReference()
{
  return this->Internal->WeakReference;
}

//----------------------------------------------------------------------------
bool vtkSlicerScriptedFileWriterBridge::CanWriteObject(vtkObject* object)
{
  if (!utils::IsValidPythonContext())
  {
    return this->Superclass::CanWriteObject(object);
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  bool callFailed = false;
  if (PyObject* result = this->CallPythonMethod(utils::ToPyArgs(object), __func__, false, &callFailed))
  {
    bool canWrite = false;
    const bool valid = utils::ToBool(result, canWrite);
    Py_DECREF(result);
    if (valid)
    {
      return canWrite;
    }
    vtkErrorMacro(<< __func__ << ": expected a bool return value from the Python implementation");
    return false;
  }
  if (callFailed)
  {
    // The Python method raised an exception (the error is logged). The object is not claimed, so that a failing
    // writer does not take precedence over other writers.
    return false;
  }
  return this->Superclass::CanWriteObject(object);
}

//----------------------------------------------------------------------------
double vtkSlicerScriptedFileWriterBridge::CanWriteObjectConfidence(vtkObject* object)
{
  if (!utils::IsValidPythonContext())
  {
    return this->Superclass::CanWriteObjectConfidence(object);
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  bool callFailed = false;
  if (PyObject* result = this->CallPythonMethod(utils::ToPyArgs(object), __func__, false, &callFailed))
  {
    double confidence = 0.0;
    const bool valid = utils::ToDouble(result, confidence);
    Py_DECREF(result);
    if (valid)
    {
      return confidence;
    }
    vtkErrorMacro(<< __func__ << ": expected a float return value from the Python implementation");
    return 0.0;
  }
  if (callFailed)
  {
    // The Python method raised an exception (the error is logged). The object is not claimed, so that a failing
    // writer does not take precedence over other writers.
    return 0.0;
  }
  return this->Superclass::CanWriteObjectConfidence(object);
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerScriptedFileWriterBridge::GetNameFiltersForObject(vtkObject* object)
{
  if (!utils::IsValidPythonContext())
  {
    return this->Superclass::GetNameFiltersForObject(object);
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (PyObject* result = this->CallPythonMethod(utils::ToPyArgs(object), __func__, false))
  {
    std::vector<std::string> nameFilters;
    const bool valid = utils::ToStringList(result, nameFilters);
    Py_DECREF(result);
    if (valid)
    {
      return nameFilters;
    }
    vtkErrorMacro(<< __func__ << ": expected a list[str] return value from the Python implementation");
  }
  return this->Superclass::GetNameFiltersForObject(object);
}

//----------------------------------------------------------------------------
bool vtkSlicerScriptedFileWriterBridge::Write(vtkMRMLIOProperties* properties)
{
  if (!utils::IsValidPythonContext())
  {
    return this->Superclass::Write(properties);
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (PyObject* result = this->CallPythonMethod(utils::ToPyArgs(properties), __func__, false))
  {
    bool success = false;
    const bool valid = utils::ToBool(result, success);
    Py_DECREF(result);
    if (valid)
    {
      return success;
    }
    vtkErrorMacro(<< __func__ << ": expected a bool return value from the Python implementation");
    return false;
  }
  return this->Superclass::Write(properties);
}

//----------------------------------------------------------------------------
void vtkSlicerScriptedFileWriterBridge::GetOptionsDescription(vtkMRMLIOOptionsDescription* description)
{
  if (!utils::IsValidPythonContext())
  {
    this->Superclass::GetOptionsDescription(description);
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (PyObject* result = this->CallPythonMethod(utils::ToPyArgs(description), __func__, false))
  {
    Py_DECREF(result);
    return;
  }
  this->Superclass::GetOptionsDescription(description);
}

//----------------------------------------------------------------------------
PyObject* vtkSlicerScriptedFileWriterBridge::CallPythonMethod(const vtkSmartPyObject& pyArgs, const std::string& fName, bool decrementResult, bool* callFailed)
{
  if (callFailed)
  {
    *callFailed = false;
  }
  vtkPythonScopeGilEnsurer gilEnsurer;
  vtkSmartPyObject object;
  object.TakeReference(utils::GetPythonObject(this));
  if (!object || !utils::IsPythonOverride(object, fName))
  {
    // Not implemented in Python (for example, this class is used directly, without a Python subclass)
    return nullptr;
  }
  PyObject* result = utils::CallPythonMethod(object, pyArgs, fName);
  if (!result)
  {
    utils::PrintErrorTraceback(this, "Failed to call : " + fName + " : of object : " + utils::GetObjectStr(object) + ":");
    // There is no Python caller to report the error to (the writer is typically called from C++), and a pending
    // error would make IsValidPythonContext() return false, which would prevent calling any other scripted reader or writer.
    PyErr_Clear();
    if (callFailed)
    {
      *callFailed = true;
    }
    return nullptr;
  }
  if (decrementResult)
  {
    Py_DECREF(result);
    return nullptr;
  }
  return result;
}
