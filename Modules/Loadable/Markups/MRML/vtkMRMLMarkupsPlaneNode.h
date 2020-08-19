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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __vtkMRMLMarkupsPlaneNode_h
#define __vtkMRMLMarkupsPlaneNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

// VTK includes
#include <vtkMatrix4x4.h>

/// \brief MRML node to represent a plane markup
/// Plane Markups nodes contain three control points.
/// Visualization parameters are set in the vtkMRMLMarkupsDisplayNode class.
///
/// Markups is intended to be used for manual marking/editing of point positions.
///
/// Coordinate systems used:
///   - Local: Local coordinates
///   - World: All parent transforms on node applied to local.
///   - Plane: Plane coordinate space (Origin of plane at 0,0,0, XYZ axis aligned to XYZ unit vectors).
///            Can have additional offset/rotation compared to local.
/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsPlaneNode : public vtkMRMLMarkupsNode
{
public:
  static vtkMRMLMarkupsPlaneNode *New();
  vtkTypeMacro(vtkMRMLMarkupsPlaneNode,vtkMRMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char* GetIcon() override {return ":/Icons/MarkupsPlaneMouseModePlace.png";}

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  enum
  {
    SizeModeAuto,
    SizeModeAbsolute,
    SizeMode_Last,
  };

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsPlane";}

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLMarkupsPlaneNode);

  /// Method for calculating the size of the plane along the direction vectors.
  /// With size mode auto, the size of the plane is automatically calculated so that it ecompasses all of the points.
  /// Size mode absolute will never be recalculated.
  /// Default is SizeModeAuto.
  vtkSetMacro(SizeMode, int);
  vtkGetMacro(SizeMode, int);
  const char* GetSizeModeAsString(int sizeMode);
  int GetSizeModeFromString(const char* sizeMode);

  /// The plane size multiplier used to calculate the size of the plane.
  /// This is only used when the size mode is auto.
  /// Default is 1.0.
  vtkGetMacro(AutoSizeScalingFactor, double);
  vtkSetMacro(AutoSizeScalingFactor, double);

  /// The bounds of the plane in the plane coordinate system.
  /// When the size mode is absolute, SetPlaneBounds can be used to specify the size of the plane.
  /// This is only used when the size mode is absolute.
  void GetPlaneBounds(double bounds[6]);
  vtkSetVector6Macro(PlaneBounds, double);

  /// The normal vector for the plane.
  /// Calculated as the vector perpendicular to the plane containing the 3 markup points, and transformed by the PlaneToPlaneOffsetMatrix.
  void GetNormal(double normal[3]);
  void SetNormal(const double normal[3]);
  void GetNormalWorld(double normal[3]);
  void SetNormalWorld(const double normal[3]);

  /// The origin of the plane.
  /// Calculated as the location of the 0th markup point, and translated by the PlaneToPlaneOffsetMatrix.
  void GetOrigin(double origin[3]);
  void SetOrigin(const double origin[3]);
  void GetOriginWorld(double origin[3]);
  void SetOriginWorld(const double origin[3]);

  /// The direction vectors defined by the markup points.
  /// Calculated as follows and then transformed by the offset matrix:
  /// X: Vector from 1st to 0th point.
  /// Y: Cross product of the Z vector and X vectors.
  /// Z: Cross product of the X vector and the vector from the 2nd to 0th point.
  void GetAxes(double x[3], double y[3], double z[3]);
  void SetAxes(const double x[3], const double y[3], const double z[3]);
  void GetAxesWorld(double x[3], double y[3], double z[3]);
  void SetAxesWorld(const double x[3], const double y[3], const double z[3]);

  // Mapping from XYZ plane coordinates to local coordinates
  virtual void GetPlaneToLocalMatrix(vtkMatrix4x4* planeToLocalMatrix);
  // Mapping from XYZ plane coordinates to world coordinates
  virtual void GetPlaneToWorldMatrix(vtkMatrix4x4* planeToWorldMatrix);

  /// 4x4 matrix detailing the offset (rotation/translation) of the plane from the plane defined by the markup points.
  /// Default is the identity matrix.
  virtual vtkMatrix4x4* GetPlaneToPlaneOffsetMatrix();

  /// Get the closest position on the plane in world coordinates.
  /// Returns the signed distance from the input point to the plane. Positive distance is in the direction of the plane normal,
  /// and negative distance is in the opposite direction.
  /// \param posWorld input position
  /// \param closestPosWorld: output found closest position
  /// \param infinitePlane if false, the closest position will be restricted to the plane bounds
  /// \return Signed distance from the point to the plane. Positive distance is in the direction of the plane normal
  double GetClosestPointOnPlaneWorld(const double posWorld[3], double closestPosWorld[3], bool infinitePlane = true);

protected:

  /// Calculates the x y and z axis of the plane from the 3 input points.
  void CalculateAxesFromPoints(const double point0[3], const double point1[3], const double point2[3], double x[3], double y[3], double z[3]);

  int SizeMode;
  double AutoSizeScalingFactor;
  double PlaneBounds[6];
  vtkSmartPointer<vtkMatrix4x4> PlaneToPlaneOffsetMatrix;

  /// Helper method for ensuring that the plane has enough points and that the points/vectors are not coincident.
  /// Used when calling SetNormal(), SetVectors() to ensure that the plane is valid before transforming to the new
  /// orientation.
  void CreatePlane();

  /// Calculates the handle to world matrix based on the current control points
  void UpdateInteractionHandleToWorldMatrix() override;

  vtkMRMLMarkupsPlaneNode();
  ~vtkMRMLMarkupsPlaneNode() override;
  vtkMRMLMarkupsPlaneNode(const vtkMRMLMarkupsPlaneNode&);
  void operator=(const vtkMRMLMarkupsPlaneNode&);
};

#endif
