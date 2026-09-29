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

#ifndef __vtkSlicerScriptedFileWriterBridge_h
#define __vtkSlicerScriptedFileWriterBridge_h

// Slicer includes
#include "vtkSlicerBaseLogicExport.h"

// MRML includes
#include <vtkMRMLFileWriter.h>

// VTK includes
#include <vtkPython.h>

// STD includes
#include <string>
#include <vector>

class vtkSmartPyObject;
class vtkWeakReference;

/// \brief Python bridge for vtkMRMLFileWriter.
///
/// File writers are implemented in Python by subclassing slicer.vtkSlicerScriptedFileWriter (defined in
/// slicer.ScriptedFileIO), which is a Python subclass of this class. This class calls the methods that are
/// overridden in Python and uses the vtkMRMLFileWriter implementation of the other methods.
///
/// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#readers-and-writers-implemented-in-python
///
/// \sa vtkSlicerScriptedFileReaderBridge
class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerScriptedFileWriterBridge : public vtkMRMLFileWriter
{
public:
  static vtkSlicerScriptedFileWriterBridge* New();
  vtkTypeMacro(vtkSlicerScriptedFileWriterBridge, vtkMRMLFileWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  bool CanWriteObject(vtkObject* object) override;
  double CanWriteObjectConfidence(vtkObject* object) override;
  std::vector<std::string> GetNameFiltersForObject(vtkObject* object) override;
  bool Write(vtkMRMLIOProperties* properties) override;
  void GetOptionsDescription(vtkMRMLIOOptionsDescription* description) override;

  /// Weak reference to this object. It is cleared when the object starts to be deleted (before DeleteEvent
  /// observers are called), therefore it can be used for referring to this object from Python without
  /// preventing the deletion of the object, even in DeleteEvent observers.
  vtkWeakReference* GetWeakReference();

protected:
  vtkSlicerScriptedFileWriterBridge();
  ~vtkSlicerScriptedFileWriterBridge() override;

  /// Clear the weak reference (see GetWeakReference) when the last reference is released,
  /// before DeleteEvent observers are invoked.
  void UnRegisterInternal(vtkObjectBase* o, vtkTypeBool check) override;

private:
  vtkSlicerScriptedFileWriterBridge(const vtkSlicerScriptedFileWriterBridge&) = delete;
  void operator=(const vtkSlicerScriptedFileWriterBridge&) = delete;

  /// Call the Python method (if the method is implemented in Python).
  /// Returns the result (new reference, unless decrementResult is true) or nullptr if the method is not
  /// implemented in Python or the call failed (the error is logged and cleared).
  /// If callFailed is specified then it is set to true if the Python method raised an exception.
  PyObject* CallPythonMethod(const vtkSmartPyObject& pyArgs, const std::string& fName, bool decrementResult, bool* callFailed = nullptr);

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
