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

  This file was originally developed by Michael Jeulin-Lagarrigue, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// .NAME vtkSlicerReformatLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerReformatLogic_h
#define __vtkSlicerReformatLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerReformatModuleLogicExport.h"


/// \ingroup Slicer_QtModules_TransformsReformatWidget
class VTK_SLICER_REFORMAT_MODULE_LOGIC_EXPORT
vtkSlicerReformatLogic : public vtkSlicerModuleLogic
{
public:
  static vtkSlicerReformatLogic *New();
  typedef vtkSlicerReformatLogic Self;
  vtkTypeMacro(vtkSlicerReformatLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set the world coordinate origin position
  static void SetSliceOrigin(vtkMRMLSliceNode* node, double x, double y, double z);
  static void SetSliceOrigin(vtkMRMLSliceNode* node, double position[3]);

  /// Set the normal to the plane of the slice node.
  static void SetSliceNormal(vtkMRMLSliceNode* node, double x, double y, double z);
  static void SetSliceNormal(vtkMRMLSliceNode* node, double normal[3]);

  /// Compute and return the volume bounding box
  static void GetVolumeBounds(vtkMRMLSliceNode* node, double bounds[6]);

  /// Compute the center from a bounds
  static void GetCenterFromBounds(double bounds[6], double center[3]);

protected:
  vtkSlicerReformatLogic();
  ~vtkSlicerReformatLogic() override;

private:

  vtkSlicerReformatLogic(const vtkSlicerReformatLogic&) = delete;
  void operator=(const vtkSlicerReformatLogic&) = delete;
};

#endif

