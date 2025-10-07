/*==============================================================================

Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLScriptedLightBoxRendererManagerProxy_h
#define __vtkMRMLScriptedLightBoxRendererManagerProxy_h

// MRMLDisplayableManager include
#include "vtkMRMLDisplayableManagerExport.h"
#include "vtkMRMLLightBoxRendererManagerProxy.h"

// VTK includes
#include <vtkPython.h>

/// \brief Scripted implementation for vtkMRMLLightBoxRendererManagerProxy
/// Delegates renderer access to underlying Python object if any.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLScriptedLightBoxRendererManagerProxy : public vtkMRMLLightBoxRendererManagerProxy
{
public:
  static vtkMRMLScriptedLightBoxRendererManagerProxy* New();
  vtkTypeMacro(vtkMRMLScriptedLightBoxRendererManagerProxy, vtkMRMLLightBoxRendererManagerProxy);

  vtkRenderer* GetRenderer(int lightboxId) override;
  void SetPythonObject(PyObject* object);

protected:
  vtkMRMLScriptedLightBoxRendererManagerProxy();
  ~vtkMRMLScriptedLightBoxRendererManagerProxy() override;

private:
  PyObject* m_object;
};

#endif
