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

#ifndef __vtkMRMLLayerDMPipelineScriptedCreator_h
#define __vtkMRMLLayerDMPipelineScriptedCreator_h

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLLayerDMPipelineCallbackCreator.h"

// VTK includes
#include <vtkPython.h>

/// Python lambda implementation of \sa vtkMRMLLayerDMPipelineCallbackCreator
/// Delegates callback to underlying Python callable object.
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMPipelineScriptedCreator : public vtkMRMLLayerDMPipelineCallbackCreator
{
public:
  static vtkMRMLLayerDMPipelineScriptedCreator* New();

  vtkTypeMacro(vtkMRMLLayerDMPipelineScriptedCreator, vtkMRMLLayerDMPipelineCallbackCreator);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  void SetPythonCallback(PyObject* object);

protected:
  vtkMRMLLayerDMPipelineScriptedCreator();
  ~vtkMRMLLayerDMPipelineScriptedCreator() override;

private:
  PyObject* Object;
};

#endif
