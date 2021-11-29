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

#ifndef __vtkMRMLMarkupsClosedCurveNode_h
#define __vtkMRMLMarkupsClosedCurveNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsCurveNode.h"

/// \brief MRML node to represent a closed curve markup
/// Closed Curve Markups nodes contain N control points.
/// Visualization parameters are set in the vtkMRMLMarkupsDisplayNode class.
///
/// Markups is intended to be used for manual marking/editing of point positions.
///
/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsClosedCurveNode : public vtkMRMLMarkupsCurveNode
{
public:
  static vtkMRMLMarkupsClosedCurveNode *New();
  vtkTypeMacro(vtkMRMLMarkupsClosedCurveNode, vtkMRMLMarkupsCurveNode);

  const char* GetIcon() override {return ":/Icons/MarkupsClosedCurve.png";}
  const char* GetAddIcon() override {return ":/Icons/MarkupsClosedCurveMouseModePlace.png";}
  const char* GetPlaceAddIcon() override {return ":/Icons/MarkupsClosedCurveMouseModePlaceAdd.png";}

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsClosedCurve";}

  /// Get markup type internal name
  const char* GetMarkupType() override {return "ClosedCurve";};

  // Get markup type GUI display name
  const char* GetTypeDisplayName() override {return "Closed Curve";};

  /// Get markup short name
  const char* GetDefaultNodeNamePrefix() override {return "CC";};

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentDefaultMacro(vtkMRMLMarkupsClosedCurveNode);


  //@{
  /// For internal use only.
  /// Moved here temporarily until pluggable markups will be implemented.
  /// Then the methods will be moved back to markups logic.
  static double GetClosedCurveSurfaceArea(vtkMRMLMarkupsClosedCurveNode* curveNode, vtkPolyData* surface = nullptr, bool projectWarp = true);
  static bool FitSurfaceProjectWarp(vtkPoints* curvePoints, vtkPolyData* surface, double radiusScalingFactor = 1.0, vtkIdType numberOfInternalGridPoints=225);
  static bool FitSurfaceDiskWarp(vtkPoints* curvePoints, vtkPolyData* surface, double radiusScalingFactor = 1.0);
  static bool IsPolygonClockwise(vtkPoints* points, vtkIdList* pointIds=nullptr);
  //@}

protected:
  vtkMRMLMarkupsClosedCurveNode();
  ~vtkMRMLMarkupsClosedCurveNode() override;
  vtkMRMLMarkupsClosedCurveNode(const vtkMRMLMarkupsClosedCurveNode&);
  void operator=(const vtkMRMLMarkupsClosedCurveNode&);

};

#endif
