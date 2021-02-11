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
///   - Local: Coordinate system of the markup node.
///   - ROI: Coordinate system that the ROI is defined in. ROI is axis aligned with center of ROI at (0,0,0).
///   - World: Patient coordinate system (RAS)
///
/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsROINode : public vtkMRMLMarkupsNode
{
public:
  static vtkMRMLMarkupsROINode *New();
  vtkTypeMacro(vtkMRMLMarkupsROINode,vtkMRMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char* GetIcon() override {return ":/Icons/MarkupsCurveMouseModePlace.png";}

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsROI";}

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLMarkupsROINode);

  /// Length of the ROI sides
  vtkGetVector3Macro(Size, double);
  void SetSize(const double size[3]);
  void SetSize(double x, double y, double z);

  /// Center of the ROI
  void GetCenter(double center[3]);
  void GetCenterWorld(double center[3]);
  void SetCenterWorld(const double center[3]);
  void SetCenter(const double center[3]);

  /// The directional axis of the ROI that are defined by ROIToLocalMatrix.
  /// \sa GetROIToLocalMatrix()
  void GetXAxisWorld(double axis_World[3]);
  void GetYAxisWorld(double axis_World[3]);
  void GetZAxisWorld(double axis_World[3]);
  void GetAxisWorld(int axisIndex, double axis_World[3]);
  void GetXAxisLocal(double axis_Local[3]);
  void GetYAxisLocal(double axis_Local[3]);
  void GetZAxisLocal(double axis_Local[3]);
  void GetAxisLocal(int axisIndex, double axis_Local[3]);

  /// 4x4 matrix defining the ROI center and axis directions within the local coordinate system.
  vtkMatrix4x4* GetROIToLocalMatrix()
  {
    return this->ROIToLocalMatrix;
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

  // ROI type enum defines the calculation method that should be used to convert to and from control points.
  enum
    {
    ROITypeBox, // Requires two Control points that are removed after they have been placed.
    ROITypeBoundingBox, // ROI forms a bounding box around the control points.
    ROIType_Last
    };

  // Scale handle indexes
  enum
  {
    HandleLFace,
    HandleRFace,
    HandlePFace,
    HandleAFace,
    HandleIFace,
    HandleSFace,

    HandleLPICorner,
    HandleRPICorner,
    HandleLAICorner,
    HandleRAICorner,
    HandleLPSCorner,
    HandleRPSCorner,
    HandleLASCorner,
    HandleRASCorner,
    };

  /// Reimplemented to recalculate InteractionHandleToWorld matrix when parent transform is changed.
  void OnTransformNodeReferenceChanged(vtkMRMLTransformNode* transformNode) override;


  void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData) override;

  /// Update the InteractionHandleToWorldMatrix based on the ROIToLocal and LocalToWorld transforms.
  void UpdateInteractionHandleToWorldMatrix() override;

  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  /// Create default storage node or nullptr if does not have one
  void CreateDefaultDisplayNodes() override;

  ///
  /// Legacy vtkMRMLAnnotationROINode methods
  ///

  /// Legacy method from vtkMRMLAnnotationROINode
  /// Get/Set for ROI Position in RAS cooridnates
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
  /// Get/Set for radius of the ROI in RAS cooridnates
  /// Old API:
  /// void SetRadiusXYZ(double RadiusX, double RadiusY, double RadiusZ);
  /// void SetRadiusXYZ(double* RadiusXYZ);
  /// vtkGetVectorMacro(RadiusXYZ,double,3);
  /// \sa SetSize(), GetSize()
  void SetRadiusXYZ(double radiusXYZ[3]);
  void SetRadiusXYZ(double x, double y, double z);
  void GetRadiusXYZ(double radiusXYZ[3]);

  /// Legacy method from vtkMRMLAnnotationROINode
  void GetTransformedPlanes(vtkPlanes* planes);

  /// Legacy method from vtkMRMLAnnotationROINode
  /// Indicates if the ROI box is inside out
  vtkSetMacro(InsideOut, bool);
  vtkGetMacro(InsideOut, bool);
  vtkBooleanMacro(InsideOut, bool);

protected:

  bool InsideOut;
  int ROIType;

  double Size[3];
  vtkSmartPointer<vtkMatrix4x4> ROIToLocalMatrix;

  bool IsUpdatingControlPointsFromROI;
  bool IsUpdatingROIFromControlPoints;

  vtkMRMLMarkupsROINode();
  ~vtkMRMLMarkupsROINode() override;
  vtkMRMLMarkupsROINode(const vtkMRMLMarkupsROINode&);
  void operator=(const vtkMRMLMarkupsROINode&);
};

#endif
