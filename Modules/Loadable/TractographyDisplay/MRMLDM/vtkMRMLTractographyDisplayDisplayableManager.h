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
class vtkMRMLFiberBundleNode;

// MRML DisplayableManager includes
#include <vtkMRMLAbstractThreeDViewDisplayableManager.h>

// STD includes
#include <vector>

/// \ingroup Slicer_QtModules_Tractography
class VTK_SLICER_TRACTOGRAPHYDISPLAY_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLTractographyDisplayDisplayableManager
  : public vtkMRMLAbstractThreeDViewDisplayableManager
{
public:
  static vtkMRMLTractographyDisplayDisplayableManager *New();
  vtkTypeMacro(vtkMRMLTractographyDisplayDisplayableManager, vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetMacro(EnableFiberEdit, int);
  vtkSetMacro(EnableFiberEdit, int);

protected:
  vtkMRMLTractographyDisplayDisplayableManager();
  ~vtkMRMLTractographyDisplayDisplayableManager();
  vtkMRMLTractographyDisplayDisplayableManager(const vtkMRMLTractographyDisplayDisplayableManager&);
  void operator=(const vtkMRMLTractographyDisplayDisplayableManager&);

  virtual int ActiveInteractionModes();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  virtual void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData);

  virtual void OnInteractorStyleEvent(int eventId);

  vtkMRMLFiberBundleNode* GetPickedFiber(vtkMRMLFiberBundleDisplayNode* displayNode,
                                                         vtkIdType pickedCell, vtkIdType &cellID);
  void DeleteSelectedFibers();
  void ClearSelectedFibers();
  void DeletePickedFibers(vtkMRMLFiberBundleNode *fiberBundleNode, std::vector<vtkIdType> &cellIDs);
  void SelectPickedFibers(vtkMRMLFiberBundleNode *fiberBundleNode, std::vector<vtkIdType> &cellIDs);

protected:

  int EnableFiberEdit;
  vtkMRMLFiberBundleNode* SelectedFiberBundleNode;
  std::map <vtkIdType, std::vector<double> > SelectedCells;
};

#endif
