/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware SAS

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Thibault Pelletier, Kitware SAS,
  and was partially funded by ANR grants ANR-22-CE45-0034 and ANR-18-RHUS-005.

==============================================================================*/

#ifndef __vtkMRMLLayerDMObjectEventObserverScripted_h
#define __vtkMRMLLayerDMObjectEventObserverScripted_h

// LayerDM includes
#include "vtkMRMLLayerDMObjectEventObserver.h"
#include "vtkSlicerLayerDMModuleMRMLExport.h"

// VTK includes
#include <vtkPython.h>
#include <vtkSmartPyObject.h>

class vtkCallbackCommand;

/// Python lambda implementation of \sa vtkMRMLLayerDMObjectEventObserver
/// Delegates full callback to underlying Python callable object.
class VTK_SLICER_LAYERDM_MODULE_MRML_EXPORT vtkMRMLLayerDMObjectEventObserverScripted : public vtkMRMLLayerDMObjectEventObserver
{
public:
  static vtkMRMLLayerDMObjectEventObserverScripted* New();
  vtkTypeMacro(vtkMRMLLayerDMObjectEventObserverScripted, vtkMRMLLayerDMObjectEventObserver);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// \brief Sets the Python callable object to be invoked on events.
  ///
  /// The provided Python object should be callable (e.g., a function, lambda,
  /// or an object with a __call__ method). This callable will be invoked
  /// whenever the observed event is triggered.
  ///
  /// The python callable will be invoked with the following args:
  ///   (vtkObject* node, unsigned long eventId, void* callData)
  ///
  /// If callData is not nullptr, the callData can be converted to the right
  /// Python type using the \see CastCallData method.
  void SetPythonCallback(PyObject* object);

  /// \brief Cast call data to appropriate Python type based on VTK type
  /// \param object Python object to cast
  /// \param vtkType VTK type identifier to cast to
  /// \return PyObject* Casted Python object
  static PyObject* CastCallData(PyObject* object, int vtkType);

protected:
  vtkMRMLLayerDMObjectEventObserverScripted();
  ~vtkMRMLLayerDMObjectEventObserverScripted() override;

private:
  PyObject* Object;
};

#endif
