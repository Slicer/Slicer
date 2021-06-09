/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was centerally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __vtkMRMLMarkupsROINode_h
#define __vtkMRMLMarkupsROINode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLModelNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>

// std includes
#include <vector>

class vtkImplicitFunction;
class vtkPlanes;

/// \brief MRML node to represent an ROI markup
///
/// Coordinate systems used:
///   - Object: Coordinate system that the ROI.
///     Origin of the coordinate system is in the center of the ROI box.
///     Axes of the coordinate system are aligned with the axes of the ROI box.
///   - Node: Coordinate system of the markup node. Coordinates of the control points are stored in this coordinate system.
///   - World: Patient coordinate system (RAS). Transform between Node and World
///     coordinate systems are defined by the parent transform of the node.
///
/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsROINode : public vtkMRMLMarkupsNode
{
public:
  static vtkMRMLMarkupsROINode *New();
  vtkTypeMacro(vtkMRMLMarkupsROINode,vtkMRMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char* GetIcon() override {return ":/Icons/MarkupsROI.png";}
  const char* GetAddIcon() override {return ":/Icons/MarkupsROIMouseModePlace.png";}
  const char* GetPlaceAddIcon() override {return ":/Icons/MarkupsROIMouseModePlaceAdd.png";}

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsROI";}

  /// Get markup name
  const char* GetMarkupType() override {return "ROI";};

  /// Get markup short name
  const char* GetDefaultNodeNamePrefix() override {return "R";};

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLMarkupsROINode);

  /// Apply the passed transformation to the ROI
  void ApplyTransform(vtkAbstractTransform* transform) override;

  /// Length of the ROI sides
  vtkGetVector3Macro(Size, double);
  void SetSize(const double size[3]);
  void SetSize(double x, double y, double z);
  void GetSizeWorld(double size_World[3]);
  void SetSizeWorld(const double size_World[3]);
  void SetSizeWorld(double x_World, double y_World, double z_World);

  /// Center of the ROI
  void GetCenter(double center[3]);
  void GetCenterWorld(double center[3]);
  void SetCenterWorld(const double center[3]);
  void SetCenterWorld(double x, double y, double z);
  void SetCenter(const double center[3]);
  void SetCenter(double x, double y, double z);

  /// The directional axis of the ROI that are defined by ObjectToNodeMatrix.
  /// \sa GetObjectToNodeMatrix()
  void GetXAxisWorld(double axis_World[3]);
  void GetYAxisWorld(double axis_World[3]);
  void GetZAxisWorld(double axis_World[3]);
  void GetAxisWorld(int axisIndex, double axis_World[3]);
  void GetXAxis(double axis_Node[3]);
  void GetYAxis(double axis_Node[3]);
  void GetZAxis(double axis_Node[3]);
  void GetAxis(int axisIndex, double axis_Node[3]);

  /// 4x4 matrix defining the object center and axis directions within the node coordinate system.
  vtkMatrix4x4* GetObjectToNodeMatrix()
    {
    return this->ObjectToNodeMatrix;
    };
  void SetAndObserveObjectToNodeMatrix(vtkMatrix4x4* objectToNodeMatrix);

  /// 4x4 matrix defining the object center and axis directions within the world coordinate system.
  /// Changes made to the matrix will not be applied.
  vtkMatrix4x4* GetObjectToWorldMatrix()
    {
    return this->ObjectToWorldMatrix;
    };

  /// Get the bounds of the ROI in the ROI coordinate system.
  void GetBoundsROI(double bounds[6]);

  /// ROIType represents the method that is used to calculate the size of the ROI.
  /// BOX ROI does not require control points to define a region, while the size of a BOUNDING_BOX ROI will be defined by the control points.
  vtkGetMacro(ROIType, int);
  void SetROIType(int roiType);
  static const char* GetROITypeAsString(int roiType);
  static int GetROITypeFromString(const char* roiType);

  /// Calculate the ROI dimensions from the control points
  virtual void UpdateROIFromControlPoints();
  virtual void UpdateBoxROIFromControlPoints();
  virtual void UpdateBoundingBoxROIFromControlPoints();

  /// Calculate the position of control points from the ROI
  virtual void UpdateControlPointsFromROI();
  virtual void UpdateControlPointsFromBoundingBoxROI();
  virtual void UpdateControlPointsFromBoxROI();

  // ROI type enum defines the calculation method that should be used to convert to and from control points.
  enum
    {
    ROITypeBox, // Requires two Control points that are removed after they have been placed.
    ROITypeBoundingBox, // ROI forms a bounding box around the control points.
    ROIType_Last
    };

  /// Reimplemented to recalculate InteractionHandleToWorld matrix when parent transform is changed.
  void OnTransformNodeReferenceChanged(vtkMRMLTransformNode* transformNode) override;


  void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData) override;

  /// Update the InteractionHandleToWorldMatrix based on the ObjectToNode and NodeToWorld transforms.
  void UpdateInteractionHandleToWorldMatrix() override;

  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  /// Create default storage node or nullptr if does not have one
  void CreateDefaultDisplayNodes() override;

  void GetRASBounds(double bounds[6]) override;
  void GetBounds(double bounds[6]) override;

  void GetPlanes(vtkPlanes* planes, bool insideOut=false);
  void GetPlanesWorld(vtkPlanes* planes, bool insideOut=false);

  bool IsPointInROI(double point_Node[3]);
  bool IsPointInROIWorld(double point_World[3]);

  ///
  /// Legacy vtkMRMLAnnotationROINode methods
  ///

  /// Legacy method from vtkMRMLAnnotationROINode
  /// Get/Set for ROI Position in RAS coordinates
  /// Note: The ROI Position is the center of the ROI
  /// Old API:
  /// void SetXYZ(double X, double Y, double Z);
  /// void SetXYZ(double* XYZ);
  /// vtkGetVectorMacro(XYZ,double,3);
  /// double* GetXYZ() {return this->GetControlPointCoordinates(0);}
  /// \sa SetCenter(), GetCenter()
  int SetXYZ(double center[3]);
  int SetXYZ(double x, double y, double z);
  bool GetXYZ(double center[3]);

  /// Legacy method from vtkMRMLAnnotationROINode
  /// Get/Set for radius of the ROI in RAS coordinates
  /// Old API:
  /// void SetRadiusXYZ(double RadiusX, double RadiusY, double RadiusZ);
  /// void SetRadiusXYZ(double* RadiusXYZ);
  /// vtkGetVectorMacro(RadiusXYZ,double,3);
  /// \sa SetSize(), GetSize()
  void SetRadiusXYZ(double radiusXYZ[3]);
  void SetRadiusXYZ(double x, double y, double z);
  void GetRadiusXYZ(double radiusXYZ[3]);

  /// Legacy method from vtkMRMLAnnotationROINode
  void GetTransformedPlanes(vtkPlanes* planes, bool insideOut=false);

  /// Helper method for generating an orthogonal right handed matrix from axes.
  /// Transform can optionally be specified to apply an additional transform on the vectors before generating the matrix.
  static void GenerateOrthogonalMatrix(vtkMatrix4x4* inputMatrix,
    vtkMatrix4x4* outputMatrix, vtkAbstractTransform* transform = nullptr, bool applyScaling = true);
  static void GenerateOrthogonalMatrix(double xAxis[3], double yAxis[3], double zAxis[3], double origin[3],
    vtkMatrix4x4* outputMatrix, vtkAbstractTransform* transform = nullptr, bool applyScaling = true);

protected:
  int ROIType{vtkMRMLMarkupsROINode::ROITypeBox};

  double Size[3]{ 0.0, 0.0, 0.0 };

  bool IsUpdatingControlPointsFromROI{false};
  bool IsUpdatingROIFromControlPoints{false};
  bool IsUpdatingInteractionHandleToWorldMatrix{false};

  vtkSmartPointer<vtkMatrix4x4> ObjectToNodeMatrix { nullptr };
  vtkSmartPointer<vtkMatrix4x4> ObjectToWorldMatrix { nullptr };

  /// Fills the specified vtkPoints with the points for all of the box ROI corners
  void GenerateBoxBounds(double bounds[6], double xAxis[3], double yAxis[3], double zAxis[3], double center[3], double size[3]);

  /// Calculates the transform from the Object (ROI) to World coordinates.
  void UpdateObjectToWorldMatrix();

  vtkMRMLMarkupsROINode();
  ~vtkMRMLMarkupsROINode() override;
  vtkMRMLMarkupsROINode(const vtkMRMLMarkupsROINode&);
  void operator=(const vtkMRMLMarkupsROINode&);
};

#endif
