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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLLightBoxRendererManagerProxy_h
#define __vtkMRMLLightBoxRendererManagerProxy_h

// MRMLDisplayableManager include
#include "vtkMRMLDisplayableManagerExport.h"
#include "vtkMRMLAbstractLogic.h"

class vtkRenderer;

/// \brief Proxy class to provide mechanisms for a displayable manager to
/// communicate with 3rd party renderer managers (like CTK).
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLLightBoxRendererManagerProxy
  : public vtkMRMLAbstractLogic
{
public:
  static vtkMRMLLightBoxRendererManagerProxy* New();
  vtkTypeMacro(vtkMRMLLightBoxRendererManagerProxy,
                       vtkMRMLAbstractLogic);

  /// Method to query the mapping from an id of a LightBox frame to
  /// the Renderer for that frame
  virtual vtkRenderer *GetRenderer(int vtkNotUsed(id)) { return nullptr; };

protected:
  vtkMRMLLightBoxRendererManagerProxy() ;
  ~vtkMRMLLightBoxRendererManagerProxy() override ;

private:
  vtkMRMLLightBoxRendererManagerProxy(const vtkMRMLLightBoxRendererManagerProxy&) = delete;
  void operator=(const vtkMRMLLightBoxRendererManagerProxy&) = delete;


};

#endif
