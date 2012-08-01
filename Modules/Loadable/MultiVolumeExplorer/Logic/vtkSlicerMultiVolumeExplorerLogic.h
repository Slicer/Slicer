/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkSlicerMultiVolumeExplorerLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerMultiVolumeExplorerLogic_h
#define __vtkSlicerMultiVolumeExplorerLogic_h

// Slicer includes
#include <vtkSlicerModuleLogic.h>

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerMultiVolumeExplorerModuleLogicExport.h"

class vtkDoubleArray;
class vtkStringArray;
class vtkMRMLScalarVolumeNode;
class vtkMRMLVolumeArchetypeStorageNode;
class vtkMRMLScalarVolumeDisplayNode;
class vtkMRMLMultiVolumeNode;
class vtkSlicerVolumesLogic;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_MULTIVOLUMEEXPLORER_MODULE_LOGIC_EXPORT vtkSlicerMultiVolumeExplorerLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerMultiVolumeExplorerLogic *New();
  vtkTypeMacro(vtkSlicerMultiVolumeExplorerLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Initialize listening to MRML events
  void InitializeEventListeners();
  int ProcessDICOMSeries(std::string inputDir, std::string outputDir,
                         std::string dcmTag, vtkDoubleArray*);

  int InitializeMultivolumeNode(vtkStringArray*, vtkMRMLMultiVolumeNode*);

  /// Register the factory that the MultiVolume needs to manage nrrd
  /// file with the specified volumes logic
  void RegisterArchetypeVolumeNodeSetFactory(vtkSlicerVolumesLogic* volumesLogic);

protected:
  vtkSlicerMultiVolumeExplorerLogic();
  virtual ~vtkSlicerMultiVolumeExplorerLogic();

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();

  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
  void StoreVolumeNode(const std::vector<std::string>& filenames,
                       const std::string& seriesFileName);
private:

  vtkSlicerMultiVolumeExplorerLogic(const vtkSlicerMultiVolumeExplorerLogic&); // Not implemented
  void operator=(const vtkSlicerMultiVolumeExplorerLogic&);               // Not implemented

};

#endif
