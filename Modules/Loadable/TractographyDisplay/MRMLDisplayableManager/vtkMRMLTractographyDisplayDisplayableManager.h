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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLTractographyDisplayDisplayableManager_h
#define __vtkMRMLTractographyDisplayDisplayableManager_h

// Tractography includes
#include "vtkSlicerTractographyDisplayModuleMRMLDisplayableManagerExport.h"

class vtkMRMLFiberBundleDisplayNode;

// MRML DisplayableManager includes
#include <vtkMRMLAbstractThreeDViewDisplayableManager.h>

// VTK includes

/// \ingroup Slicer_QtModules_Tractography
class VTK_SLICER_TRACTOGRAPHYDISPLAY_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLTractographyDisplayDisplayableManager
  : public vtkMRMLAbstractThreeDViewDisplayableManager
{
public:
  static vtkMRMLTractographyDisplayDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLTractographyDisplayDisplayableManager, vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);


protected:
  vtkMRMLTractographyDisplayDisplayableManager();
  ~vtkMRMLTractographyDisplayDisplayableManager();
  vtkMRMLTractographyDisplayDisplayableManager(const vtkMRMLTractographyDisplayDisplayableManager&);
  void operator=(const vtkMRMLTractographyDisplayDisplayableManager&);

  virtual int ActiveInteractionModes();

  virtual void OnInteractorStyleEvent(int eventId);

  void DeletePickedFiber(vtkMRMLFiberBundleDisplayNode* dnode, vtkIdType pickedCell);

protected:

};

#endif
