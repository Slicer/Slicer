/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLSliceNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLVolumeNode.h"

// VTK includes
#include <vtkAddonMathUtilities.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkVector.h>

// VNL includes
#include <vnl/vnl_double_3.h>

// STL includes
#include <algorithm>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSliceNode);

//----------------------------------------------------------------------------
// Constructor
vtkMRMLSliceNode::vtkMRMLSliceNode()
{
  // set by user
  this->SliceToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  this->SliceToRAS->Identity();

  this->JumpMode = OffsetJumpSlice;

  this->OrientationReference = nullptr;
  this->DefaultOrientation = nullptr;

  // calculated by UpdateMatrices()
  this->XYToSlice = vtkSmartPointer<vtkMatrix4x4>::New();
  this->XYToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  this->UVWToSlice = vtkSmartPointer<vtkMatrix4x4>::New();
  this->UVWToRAS = vtkSmartPointer<vtkMatrix4x4>::New();

  // set the default field of view to a convenient size for looking
  // at slices through human heads (a 1 pixel thick slab 25x25 cm)
  // TODO: how best to represent this as a slab rather than infinitesimal slice?
  this->FieldOfView[0] = 250.0;
  this->FieldOfView[1] = 250.0;
  this->FieldOfView[2] = 1.0;

  this->Dimensions[0] = 256;
  this->Dimensions[1] = 256;
  this->Dimensions[2] = 1;

  this->UVWDimensions[0] = 256;
  this->UVWDimensions[1] = 256;
  this->UVWDimensions[2] = 1;

  this->UVWMaximumDimensions[0] = 1024;
  this->UVWMaximumDimensions[1] = 1024;
  this->UVWMaximumDimensions[2] = 1024;

  this->UVWExtents[0] = 0;
  this->UVWExtents[1] = 0;
  this->UVWExtents[2] = 0;

  this->SliceResolutionMode = vtkMRMLSliceNode::SliceResolutionMatch2DView;

  this->SlabReconstructionEnabled = false;
  this->SlabReconstructionType = VTK_IMAGE_SLAB_MAX;
  this->SlabReconstructionThickness = 1.;
  this->SlabReconstructionOversamplingFactor = 2.0;

  this->XYZOrigin[0] = 0;
  this->XYZOrigin[1] = 0;
  this->XYZOrigin[2] = 0;

  this->UVWOrigin[0] = 0;
  this->UVWOrigin[1] = 0;
  this->UVWOrigin[2] = 0;

  this->SliceVisible = 0;
  this->WidgetVisible = 0;
  this->WidgetOutlineVisible = 1;
  this->WidgetNormalLockedToCamera = 0;
  this->UseLabelOutline = 0;

  this->LayoutGridColumns = 1;
  this->LayoutGridRows = 1;

  this->PrescribedSliceSpacing[0] = this->PrescribedSliceSpacing[1] = this->PrescribedSliceSpacing[2] = 1;
  this->SliceSpacingMode = AutomaticSliceSpacingMode;

  this->ActiveSlice = 0;

  this->Interacting = 0;
  this->InteractionFlags = 0;
  this->InteractionFlagsModifier = (unsigned int)-1;

  this->IsUpdatingMatrices = 0;

  this->LayoutColor[0] = vtkMRMLAbstractViewNode::GetGrayColor()[0];
  this->LayoutColor[1] = vtkMRMLAbstractViewNode::GetGrayColor()[1];
  this->LayoutColor[2] = vtkMRMLAbstractViewNode::GetGrayColor()[2];

  this->SetOrientationReference(vtkMRMLSliceNode::GetReformatOrientationName());
  this->SetLayoutLabel("");

  this->OrientationMarkerEnabled = true;
  this->RulerEnabled = true;
}

//----------------------------------------------------------------------------
vtkMRMLSliceNode::~vtkMRMLSliceNode()
{
  this->SetOrientationReference(nullptr);
  this->SetDefaultOrientation(nullptr);
}

//-----------------------------------------------------------
void vtkMRMLSliceNode::SetInteracting(int interacting)
{
  // Don't call Modified()
  this->Interacting = interacting;
}

//-----------------------------------------------------------
void vtkMRMLSliceNode::SetInteractionFlags(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlags = flags;
}

//-----------------------------------------------------------
void vtkMRMLSliceNode::SetInteractionFlagsModifier(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlagsModifier = flags;
}

//-----------------------------------------------------------
void vtkMRMLSliceNode::ResetInteractionFlagsModifier()
{
  // Don't call Modified()
  this->InteractionFlagsModifier = (unsigned int)-1;
}

//---------------------------------------------------------------------------
int vtkMRMLSliceNode::GetNumberOfThreeDViewIDs() const
{
  return static_cast<int>(this->ThreeDViewIDs.size());
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::AddThreeDViewID(const char* viewNodeID)
{
  if (!viewNodeID)
  {
    return;
  }

  if (this->IsThreeDViewIDPresent(viewNodeID))
  {
    return; // already exists, do nothing
  }

  this->ThreeDViewIDs.emplace_back(viewNodeID);
  if (this->Scene)
  {
    this->Scene->AddReferencedNodeID(viewNodeID, this);
  }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::RemoveThreeDViewID(char* viewNodeID)
{
  if (viewNodeID == nullptr)
  {
    return;
  }
  std::vector<std::string> viewNodeIDs;
  for (unsigned int i = 0; i < this->ThreeDViewIDs.size(); i++)
  {
    if (std::string(viewNodeID) != this->ThreeDViewIDs[i])
    {
      viewNodeIDs.push_back(this->ThreeDViewIDs[i]);
    }
  }
  if (viewNodeIDs.size() != this->ThreeDViewIDs.size())
  {
    this->Scene->RemoveReferencedNodeID(viewNodeID, this);
    this->ThreeDViewIDs = viewNodeIDs;
    this->Modified();
  }
  else
  {
    vtkErrorMacro("vtkMRMLDisplayNode::RemoveThreeDViewID() id " << viewNodeID << " not found");
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::RemoveAllThreeDViewIDs()
{
  for (unsigned int i = 0; i < this->ThreeDViewIDs.size(); i++)
  {
    this->Scene->RemoveReferencedNodeID(ThreeDViewIDs[i].c_str(), this);
  }
  this->ThreeDViewIDs.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
const char* vtkMRMLSliceNode::GetNthThreeDViewID(unsigned int index)
{
  if (index >= ThreeDViewIDs.size())
  {
    vtkErrorMacro("vtkMRMLDisplayNode::GetNthThreeDViewID() index " << index << " outside the range 0-"
                                                                    << this->ThreeDViewIDs.size() - 1);
    return nullptr;
  }
  return ThreeDViewIDs[index].c_str();
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::IsThreeDViewIDPresent(const char* viewNodeID) const
{
  if (viewNodeID == nullptr)
  {
    return false;
  }
  std::string value(viewNodeID);
  std::vector<std::string>::const_iterator it =
    std::find(this->ThreeDViewIDs.begin(), this->ThreeDViewIDs.end(), value);
  return it != this->ThreeDViewIDs.end();
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::IsDisplayableInThreeDView(const char* viewNodeID) const
{
  return this->GetNumberOfThreeDViewIDs() == 0 || this->IsThreeDViewIDPresent(viewNodeID);
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::SetOrientation(const char* orientation)
{
  if (!orientation)
  {
    vtkErrorMacro("SetOrientation: invalid input orientation");
    return false;
  }

  vtkMatrix3x3* orientationPreset = this->GetSliceOrientationPreset(orientation);
  if (orientationPreset == nullptr)
  {
    return false;
  }

  for (int ii = 0; ii < 3; ++ii)
  {
    for (int jj = 0; jj < 3; ++jj)
    {
      this->SliceToRAS->SetElement(ii, jj, orientationPreset->GetElement(ii, jj));
    }
  }

  // SetOrientationReference() behavior will be maintained for backward compatibility
  this->SetOrientationReference(orientation);

  this->UpdateMatrices();
  return true;
}

//----------------------------------------------------------------------------
std::string vtkMRMLSliceNode::GetOrientation()
{
  return this->GetOrientation(this->SliceToRAS);
}

//----------------------------------------------------------------------------
const char* vtkMRMLSliceNode::GetOrientationString()
{
  std::string current = this->GetOrientation();
  if (current != this->OrientationString)
  {
    this->OrientationString = current;
  }
  return this->OrientationString.c_str();
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::SetOrientationToAxial()
{
  return this->SetOrientation("Axial");
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::SetOrientationToSagittal()
{
  return this->SetOrientation("Sagittal");
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::SetOrientationToCoronal()
{
  return this->SetOrientation("Coronal");
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::MatrixAreEqual(const vtkMatrix4x4* m1, const vtkMatrix4x4* m2)
{
  return vtkAddonMathUtilities::MatrixAreEqual(m1, m2);
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::MatrixAreEqual(const vtkMatrix4x4* matrix, const vtkMatrix3x3* orientationMatrix)
{
  return vtkAddonMathUtilities::MatrixAreEqual(matrix, orientationMatrix);
}

//----------------------------------------------------------------------------
vtkMatrix3x3* vtkMRMLSliceNode::GetSliceOrientationPreset(const std::string& name)
{
  std::vector<OrientationPresetType>::iterator it;
  for (it = this->OrientationMatrices.begin(); it != this->OrientationMatrices.end(); ++it)
  {
    if (it->first == name)
    {
      return it->second;
    }
  }

  vtkErrorMacro("GetSliceOrientationPreset: invalid orientation preset name: " << name);
  return nullptr;
}

//----------------------------------------------------------------------------
std::string vtkMRMLSliceNode::GetSliceOrientationPresetName(vtkMatrix3x3* orientationMatrix)
{
  if (!orientationMatrix)
  {
    vtkErrorMacro("GetSliceOrientationPresetName: invalid input Matrix.");
    return std::string();
  }

  std::vector<OrientationPresetType>::reverse_iterator it;
  for (it = this->OrientationMatrices.rbegin(); it != this->OrientationMatrices.rend(); ++it)
  {
    std::string presetName = it->first;
    vtkMatrix3x3* presetOrientationMatrix = this->GetSliceOrientationPreset(presetName);
    if (vtkAddonMathUtilities::MatrixAreEqual(orientationMatrix, presetOrientationMatrix))
    {
      return presetName;
    }
  }
  return std::string();
}

//----------------------------------------------------------------------------
std::string vtkMRMLSliceNode::GetOrientation(vtkMatrix4x4* sliceToRAS)
{
  vtkNew<vtkMatrix3x3> orientationMatrix;
  vtkAddonMathUtilities::GetOrientationMatrix(sliceToRAS, orientationMatrix.GetPointer());

  std::string orientation = this->GetSliceOrientationPresetName(orientationMatrix.GetPointer());
  if (orientation != "")
  {
    return orientation;
  }
  return vtkMRMLSliceNode::GetReformatOrientationName();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::GetSliceOrientationPresetNames(vtkStringArray* presetOrientationNames)
{
  if (presetOrientationNames == nullptr)
  {
    vtkErrorMacro("GetSliceOrientationPresetNames: invalid input vtkStringArray.");
    return;
  }

  presetOrientationNames->SetNumberOfValues(this->OrientationMatrices.size());

  std::vector<OrientationPresetType>::iterator it;
  int id = 0;
  for (it = this->OrientationMatrices.begin(); it != this->OrientationMatrices.end(); ++it)
  {
    presetOrientationNames->SetValue(id, it->first);
    id++;
  }
}

//----------------------------------------------------------------------------
int vtkMRMLSliceNode::GetNumberOfSliceOrientationPresets() const
{
  return static_cast<int>(this->OrientationMatrices.size());
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::AddSliceOrientationPreset(const std::string& name, vtkMatrix3x3* orientationMatrix)
{
  if (name == vtkMRMLSliceNode::GetReformatOrientationName())
  {
    vtkWarningMacro("AddSliceOrientationPreset: Reformat refer to any arbitrary orientation. "
                    "Therefore, it can not be used a preset name.");
    return false;
  }

  std::vector<OrientationPresetType>::iterator it;
  for (it = this->OrientationMatrices.begin(); it != this->OrientationMatrices.end(); ++it)
  {
    if (it->first == name)
    {
      it->second->DeepCopy(orientationMatrix);
      return true;
    }
  }

  this->OrientationMatrices.emplace_back(name, orientationMatrix);
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::RemoveSliceOrientationPreset(const std::string& name)
{
  std::vector<OrientationPresetType>::iterator it;
  for (it = this->OrientationMatrices.begin(); it != this->OrientationMatrices.end(); ++it)
  {
    if (it->first == name)
    {
      this->OrientationMatrices.erase(it);
      return true;
    }
  }

  vtkErrorMacro("RemoveSliceOrientationPreset: the orientation preset " << name << " is not stored.");
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::RenameSliceOrientationPreset(const std::string& name, const std::string& updatedName)
{
  if (name == vtkMRMLSliceNode::GetReformatOrientationName()
      || updatedName == vtkMRMLSliceNode::GetReformatOrientationName())
  {
    vtkErrorMacro("RenameSliceOrientationPreset: 'Reformat' refers to any "
                  "arbitrary orientation. It can NOT be used as a preset name.");
    return false;
  }
  if (name == updatedName)
  {
    return false;
  }

  if (!name.compare(this->GetOrientationReference()))
  {
    this->SetOrientationReference(updatedName.c_str());
  }

  std::vector<OrientationPresetType>::iterator it;
  for (it = this->OrientationMatrices.begin(); it != this->OrientationMatrices.end(); ++it)
  {
    if (it->first == name)
    {
      it->first = updatedName;
      return true;
    }
  }

  vtkErrorMacro("RenameSliceOrientationPreset: The orientation preset "
                "'"
                << name << "' does NOT exist.");
  return false;
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::HasSliceOrientationPreset(const std::string& name)
{
  if (name == vtkMRMLSliceNode::GetReformatOrientationName())
  {
    vtkWarningMacro("HasSliceOrientationPreset: 'Reformat' refers to any "
                    "arbitrary orientation. It can NOT be used as a preset name.");
    return false;
  }

  std::vector<OrientationPresetType>::iterator it;
  for (it = this->OrientationMatrices.begin(); it != this->OrientationMatrices.end(); ++it)
  {
    if (it->first == name)
    {
      return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::GetAxialSliceToRASMatrix(vtkMatrix3x3* orientationMatrix,
                                                bool patientRightIsScreenLeft /*=true*/)
{
  if (!orientationMatrix)
  {
    return;
  }
  if (patientRightIsScreenLeft)
  {
    // L
    orientationMatrix->SetElement(0, 0, -1.0);
    orientationMatrix->SetElement(1, 0, 0.0);
    orientationMatrix->SetElement(2, 0, 0.0);
  }
  else
  {
    // R
    orientationMatrix->SetElement(0, 0, 1.0);
    orientationMatrix->SetElement(1, 0, 0.0);
    orientationMatrix->SetElement(2, 0, 0.0);
  }

  // A
  orientationMatrix->SetElement(0, 1, 0.0);
  orientationMatrix->SetElement(1, 1, 1.0);
  orientationMatrix->SetElement(2, 1, 0.0);

  // S (to make arrow up/right to go superior)
  orientationMatrix->SetElement(0, 2, 0.0);
  orientationMatrix->SetElement(1, 2, 0.0);
  orientationMatrix->SetElement(2, 2, 1.0);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::GetSagittalSliceToRASMatrix(vtkMatrix3x3* orientationMatrix,
                                                   bool patientRightIsScreenLeft /*=true*/)
{
  if (!orientationMatrix)
  {
    return;
  }
  // P
  orientationMatrix->SetElement(0, 0, 0.0);
  orientationMatrix->SetElement(1, 0, -1.0);
  orientationMatrix->SetElement(2, 0, 0.0);

  // S
  orientationMatrix->SetElement(0, 1, 0.0);
  orientationMatrix->SetElement(1, 1, 0.0);
  orientationMatrix->SetElement(2, 1, 1.0);

  if (patientRightIsScreenLeft)
  {
    // L (to make arrow up/right to go left)
    orientationMatrix->SetElement(0, 2, -1.0);
    orientationMatrix->SetElement(1, 2, 0.0);
    orientationMatrix->SetElement(2, 2, 0.0);
  }
  else
  {
    // R (to make arrow up/right to go right)
    orientationMatrix->SetElement(0, 2, 1.0);
    orientationMatrix->SetElement(1, 2, 0.0);
    orientationMatrix->SetElement(2, 2, 0.0);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::GetCoronalSliceToRASMatrix(vtkMatrix3x3* orientationMatrix,
                                                  bool patientRightIsScreenLeft /*=true*/)
{
  if (!orientationMatrix)
  {
    return;
  }
  if (patientRightIsScreenLeft)
  {
    // L
    orientationMatrix->SetElement(0, 0, -1.0);
    orientationMatrix->SetElement(1, 0, 0.0);
    orientationMatrix->SetElement(2, 0, 0.0);
  }
  else
  {
    // R
    orientationMatrix->SetElement(0, 0, 1.0);
    orientationMatrix->SetElement(1, 0, 0.0);
    orientationMatrix->SetElement(2, 0, 0.0);
  }

  // S
  orientationMatrix->SetElement(0, 1, 0.0);
  orientationMatrix->SetElement(1, 1, 0.0);
  orientationMatrix->SetElement(2, 1, 1.0);

  // A (to make arrow up/right to go anterior)
  orientationMatrix->SetElement(0, 2, 0.0);
  orientationMatrix->SetElement(1, 2, 1.0);
  orientationMatrix->SetElement(2, 2, 0.0);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(vtkMRMLScene* scene, bool patientRightIsScreenLeft /*=true*/)
{
  if (!scene)
  {
    return;
  }

  // Setting Orientation Matrices presets
  vtkNew<vtkMatrix3x3> axialSliceToRAS;
  vtkMRMLSliceNode::GetAxialSliceToRASMatrix(axialSliceToRAS, patientRightIsScreenLeft);

  vtkNew<vtkMatrix3x3> sagittalSliceToRAS;
  vtkMRMLSliceNode::GetSagittalSliceToRASMatrix(sagittalSliceToRAS, patientRightIsScreenLeft);

  vtkNew<vtkMatrix3x3> coronalSliceToRAS;
  vtkMRMLSliceNode::GetCoronalSliceToRASMatrix(coronalSliceToRAS, patientRightIsScreenLeft);

  // Setting a Slice Default Node
  vtkSmartPointer<vtkMRMLNode> defaultNode = scene->GetDefaultNodeByClass("vtkMRMLSliceNode");
  if (!defaultNode)
  {
    defaultNode.TakeReference(scene->CreateNodeByClass("vtkMRMLSliceNode"));
    scene->AddDefaultNode(defaultNode);
  }
  vtkMRMLSliceNode* defaultSliceNode = vtkMRMLSliceNode::SafeDownCast(defaultNode);
  defaultSliceNode->AddSliceOrientationPreset("Axial", axialSliceToRAS);
  defaultSliceNode->AddSliceOrientationPreset("Sagittal", sagittalSliceToRAS);
  defaultSliceNode->AddSliceOrientationPreset("Coronal", coronalSliceToRAS);
}

//----------------------------------------------------------------------------
//  Set the SliceToRAS matrix by the position and orientation of the locator
//
void vtkMRMLSliceNode::SetSliceToRASByNTP(double Nx,
                                          double Ny,
                                          double Nz,
                                          double Tx,
                                          double Ty,
                                          double Tz,
                                          double Px,
                                          double Py,
                                          double Pz,
                                          int Orientation)
{
  vnl_double_3 n, t, c;
  vnl_double_3 negN, negT, negC;

  n[0] = Nx;
  n[1] = Ny;
  n[2] = Nz;
  t[0] = Tx;
  t[1] = Ty;
  t[2] = Tz;

  // Ensure N, T orthogonal:
  //    C = N x T
  //    T = C x N
  c = vnl_cross_3d(n, t);
  t = vnl_cross_3d(c, n);

  // Ensure vectors are normalized
  n.normalize();
  t.normalize();
  c.normalize();

  // Get negative vectors
  negN = -n;
  negT = -t;
  negC = -c;

  this->SliceToRAS->Identity();
  // Tip location
  this->SliceToRAS->SetElement(0, 3, Px);
  this->SliceToRAS->SetElement(1, 3, Py);
  this->SliceToRAS->SetElement(2, 3, Pz);

  switch (Orientation)
  {
    // para-Axial
    case 0:
      // N
      this->SliceToRAS->SetElement(0, 2, n[0]);
      this->SliceToRAS->SetElement(1, 2, n[1]);
      this->SliceToRAS->SetElement(2, 2, n[2]);

      // C
      this->SliceToRAS->SetElement(0, 1, c[0]);
      this->SliceToRAS->SetElement(1, 1, c[1]);
      this->SliceToRAS->SetElement(2, 1, c[2]);
      // T
      this->SliceToRAS->SetElement(0, 0, t[0]);
      this->SliceToRAS->SetElement(1, 0, t[1]);
      this->SliceToRAS->SetElement(2, 0, t[2]);

      break;

    // para-Sagittal
    case 1:
      // T
      this->SliceToRAS->SetElement(0, 2, t[0]);
      this->SliceToRAS->SetElement(1, 2, t[1]);
      this->SliceToRAS->SetElement(2, 2, t[2]);

      // negN
      this->SliceToRAS->SetElement(0, 1, negN[0]);
      this->SliceToRAS->SetElement(1, 1, negN[1]);
      this->SliceToRAS->SetElement(2, 1, negN[2]);
      // negC
      this->SliceToRAS->SetElement(0, 0, negC[0]);
      this->SliceToRAS->SetElement(1, 0, negC[1]);
      this->SliceToRAS->SetElement(2, 0, negC[2]);

      break;

    // para-Coronal
    case 2:
      // C
      this->SliceToRAS->SetElement(0, 2, c[0]);
      this->SliceToRAS->SetElement(1, 2, c[1]);
      this->SliceToRAS->SetElement(2, 2, c[2]);
      // negN
      this->SliceToRAS->SetElement(0, 1, negN[0]);
      this->SliceToRAS->SetElement(1, 1, negN[1]);
      this->SliceToRAS->SetElement(2, 1, negN[2]);
      // T
      this->SliceToRAS->SetElement(0, 0, t[0]);
      this->SliceToRAS->SetElement(1, 0, t[1]);
      this->SliceToRAS->SetElement(2, 0, t[2]);

      break;
  }

  this->UpdateMatrices();
}

//----------------------------------------------------------------------------
//  Calculate XYToSlice and XYToRAS
//  Inputs: Dimenionss, FieldOfView, SliceToRAS
//
void vtkMRMLSliceNode::UpdateMatrices()
{
  if (this->IsUpdatingMatrices)
  {
    return;
  }
  else
  {
    this->IsUpdatingMatrices = 1;
  }
  double spacing[3];
  unsigned int i;
  vtkNew<vtkMatrix4x4> xyToSlice;
  vtkNew<vtkMatrix4x4> xyToRAS;

  int disabledModify = this->StartModify();

  // the mapping from XY output slice pixels to Slice Plane coordinate
  xyToSlice->Identity();
  if (this->Dimensions[0] > 0 && this->Dimensions[1] > 0 && this->Dimensions[2] > 0)
  {
    for (i = 0; i < 3; i++)
    {
      spacing[i] = this->FieldOfView[i] / this->Dimensions[i];
      xyToSlice->SetElement(i, i, spacing[i]);
      xyToSlice->SetElement(i, 3, -this->FieldOfView[i] / 2. + this->XYZOrigin[i]);
    }
    // vtkWarningMacro( << "FieldOfView[2] = " << this->FieldOfView[2] << ", Dimensions[2] = " << this->Dimensions[2] );
    // xyToSlice->SetElement(2, 2, 1.);

    xyToSlice->SetElement(2, 3, 0.);
  }

  // the mapping from slice plane coordinates to RAS
  // (the Orientation as in Axial, Sagittal, Coronal)
  //
  // The combined transform:
  //
  // | R | = [Slice to RAS ] [ XY to Slice ]  | X |
  // | A |                                    | Y |
  // | S |                                    | Z |
  // | 1 |                                    | 1 |
  //
  // or
  //
  // RAS = XYToRAS * XY
  //
  vtkMatrix4x4::Multiply4x4(this->SliceToRAS, xyToSlice.GetPointer(), xyToRAS.GetPointer());

  bool modified = false;

  // check to see if the matrix actually changed
  if (!MatrixAreEqual(xyToRAS.GetPointer(), this->XYToRAS))
  {
    this->XYToSlice->DeepCopy(xyToSlice.GetPointer());
    this->XYToRAS->DeepCopy(xyToRAS.GetPointer());
    modified = true;
  }

  // the mapping from XY output slice pixels to Slice Plane coordinate
  this->UVWToSlice->Identity();
  if (this->UVWDimensions[0] > 0 && this->UVWDimensions[1] > 0 && this->UVWDimensions[2] > 0)
  {
    for (i = 0; i < 2; i++)
    {
      spacing[i] = this->UVWExtents[i] / (this->UVWDimensions[i]);
      this->UVWToSlice->SetElement(i, i, spacing[i]);
      this->UVWToSlice->SetElement(i, 3, -this->UVWExtents[i] / 2. + this->UVWOrigin[i]);
    }
    this->UVWToSlice->SetElement(2, 2, 1.0);
    this->UVWToSlice->SetElement(2, 3, 0.);
  }

  vtkNew<vtkMatrix4x4> uvwToRAS;

  vtkMatrix4x4::Multiply4x4(this->SliceToRAS, this->UVWToSlice, uvwToRAS.GetPointer());

  if (!MatrixAreEqual(uvwToRAS.GetPointer(), this->UVWToRAS))
  {
    this->UVWToRAS->DeepCopy(uvwToRAS.GetPointer());
    modified = true;
  }

  if (modified)
  {
    this->Modified();
  }

  // as UpdateMatrices can be called with DisableModifiedEvent
  // (typically when the scene is closed, slice nodes are reset but shouldn't
  // fire events. We should respect the modifiedWasDisabled flag.
  this->EndModify(disabledModify);

  this->IsUpdatingMatrices = 0;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLVectorMacro(fieldOfView, FieldOfView, double, 3);
  vtkMRMLWriteXMLVectorMacro(dimensions, Dimensions, int, 3);
  vtkMRMLWriteXMLVectorMacro(xyzOrigin, XYZOrigin, double, 3);
  vtkMRMLWriteXMLIntMacro(sliceResolutionMode, SliceResolutionMode);
  vtkMRMLWriteXMLVectorMacro(uvwExtents, UVWExtents, double, 3);
  vtkMRMLWriteXMLVectorMacro(uvwDimensions, UVWDimensions, int, 3);
  vtkMRMLWriteXMLVectorMacro(uvwOrigin, UVWOrigin, double, 3);
  vtkMRMLWriteXMLIntMacro(activeSlice, ActiveSlice);
  vtkMRMLWriteXMLIntMacro(layoutGridRows, LayoutGridRows);
  vtkMRMLWriteXMLIntMacro(layoutGridColumns, LayoutGridColumns);
  vtkMRMLWriteXMLMatrix4x4Macro(sliceToRAS, SliceToRAS);

  // orientationMatrix
  std::vector<OrientationPresetType>::iterator it;
  for (it = this->OrientationMatrices.begin(); it != this->OrientationMatrices.end(); ++it)
  {
    std::stringstream ss;
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        ss << it->second->GetElement(i, j);
        if (!(i == 2 && j == 2))
        {
          ss << " ";
        }
      }
    }
    of << " orientationMatrix" << this->URLEncodeString(it->first.c_str()) << "=\"" << ss.str().c_str() << "\"";
  }

  vtkMRMLWriteXMLStdStringMacro(orientation, Orientation);
  vtkMRMLWriteXMLStringMacro(defaultOrientation, DefaultOrientation);
  vtkMRMLWriteXMLStringMacro(orientationReference, OrientationReference);
  vtkMRMLWriteXMLIntMacro(jumpMode, JumpMode);
  vtkMRMLWriteXMLBooleanMacro(sliceVisibility, SliceVisible);
  vtkMRMLWriteXMLBooleanMacro(widgetVisibility, WidgetVisible);
  vtkMRMLWriteXMLBooleanMacro(widgetOutlineVisibility, WidgetOutlineVisible);
  vtkMRMLWriteXMLBooleanMacro(useLabelOutline, UseLabelOutline);
  vtkMRMLWriteXMLIntMacro(sliceSpacingMode, SliceSpacingMode);
  vtkMRMLWriteXMLVectorMacro(prescribedSliceSpacing, PrescribedSliceSpacing, double, 3);

  // threeDViewNodeRef
  if (this->ThreeDViewIDs.size() > 0)
  {
    vtkMRMLWriteXMLStdStringVectorMacro(threeDViewNodeRef, ThreeDViewIDs, std::vector);
  }

  vtkMRMLWriteXMLBooleanMacro(slabReconstructionEnabled, SlabReconstructionEnabled);
  vtkMRMLWriteXMLEnumMacro(slabReconstructionType, SlabReconstructionType);
  vtkMRMLWriteXMLFloatMacro(slabReconstructionThickness, SlabReconstructionThickness);
  vtkMRMLWriteXMLFloatMacro(slabReconstructionOversamplingFactor, SlabReconstructionOversamplingFactor);

  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  bool layoutColorFound = false;
  bool layoutLabelFound = false;

  vtkMRMLReadXMLBeginMacro(atts);
  if (!strcmp(xmlReadAttName, "layoutLabel"))
  {
    // Layout label is set in Superclass
    layoutLabelFound = true;
  }
  if (!strcmp(xmlReadAttName, "layoutColor"))
  {
    // Layout color is set in Superclass
    layoutColorFound = true;
  }
  vtkMRMLReadXMLVectorMacro(fieldOfView, FieldOfView, double, 3);
  vtkMRMLReadXMLVectorMacro(xyzOrigin, XYZOrigin, double, 3);
  vtkMRMLReadXMLVectorMacro(uvwOrigin, UVWOrigin, double, 3);
  vtkMRMLReadXMLVectorMacro(uvwExtents, UVWExtents, double, 3);
  vtkMRMLReadXMLVectorMacro(uvwDimensions, UVWDimensions, int, 3);
  vtkMRMLReadXMLIntMacro(sliceResolutionMode, SliceResolutionMode);
  vtkMRMLReadXMLIntMacro(activeSlice, ActiveSlice);
  vtkMRMLReadXMLIntMacro(layoutGridRows, LayoutGridRows);
  vtkMRMLReadXMLIntMacro(layoutGridColumns, LayoutGridColumns);
  vtkMRMLReadXMLIntMacro(jumpMode, JumpMode);
  vtkMRMLReadXMLBooleanMacro(sliceVisibility, SliceVisible);
  vtkMRMLReadXMLBooleanMacro(widgetVisibility, WidgetVisible);
  vtkMRMLReadXMLBooleanMacro(widgetOutlineVisibility, WidgetOutlineVisible);
  vtkMRMLReadXMLBooleanMacro(useLabelOutline, UseLabelOutline);
  vtkMRMLReadXMLStdStringMacro(orientation, Orientation);
  vtkMRMLReadXMLStringMacro(defaultOrientation, DefaultOrientation);
  vtkMRMLReadXMLStringMacro(orientationReference, OrientationReference);
  vtkMRMLReadXMLStringMacro(layoutName, LayoutName);
  vtkMRMLReadXMLVectorMacro(dimensions, Dimensions, int, 3);

  // resliceDimensions: Setting of UVWDimensions based of the "resliceDimensions" attribute
  // was originally introduced in 2012 through commit Slicer@01ffcb5326 (ENH 9124. Added
  // new options for displaying slice models in 3d views)
  vtkMRMLReadXMLVectorMacro(resliceDimensions, UVWDimensions, int, 3);

  vtkMRMLReadXMLOwnedMatrix4x4Macro(sliceToRAS, SliceToRAS);

  // orientationMatrix
  if (!strncmp(this->URLDecodeString(xmlReadAttName), "orientationMatrix", 17))
  {
    std::string name = std::string(this->URLDecodeString(xmlReadAttName));
    std::stringstream ss;
    double val;
    vtkNew<vtkMatrix3x3> orientationMatrix;
    orientationMatrix->Identity();
    ss << xmlReadAttValue;
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        ss >> val;
        orientationMatrix->SetElement(i, j, val);
      }
    }
    name.erase(0, 17);
    this->AddSliceOrientationPreset(name, orientationMatrix.GetPointer());
  }

  vtkMRMLReadXMLVectorMacro(prescribedSliceSpacing, PrescribedSliceSpacing, double, 3);
  vtkMRMLReadXMLIntMacro(sliceSpacingMode, SliceSpacingMode);

  // threeDViewNodeRef
  if (!strcmp(xmlReadAttName, "threeDViewNodeRef"))
  {
    std::stringstream ss(xmlReadAttValue);
    while (!ss.eof())
    {
      std::string id;
      ss >> id;
      this->AddThreeDViewID(id.c_str());
    }
  }

  vtkMRMLReadXMLBooleanMacro(slabReconstructionEnabled, SlabReconstructionEnabled);
  vtkMRMLReadXMLEnumMacro(slabReconstructionType, SlabReconstructionType);
  vtkMRMLReadXMLFloatMacro(slabReconstructionThickness, SlabReconstructionThickness);
  vtkMRMLReadXMLFloatMacro(slabReconstructionOversamplingFactor, SlabReconstructionOversamplingFactor);

  vtkMRMLReadXMLEndMacro();

  if (!layoutColorFound)
  {
    std::string layoutName(this->GetLayoutName() ? this->GetLayoutName() : "");
    // Slicer3 scene file. Grok a color
    if (layoutName.find("Compare") == 0)
    {
      this->SetLayoutColor(vtkMRMLAbstractViewNode::GetCompareColor());
    }
    else if (layoutName == "Red")
    {
      this->SetLayoutColor(vtkMRMLAbstractViewNode::GetRedColor());
    }
    else if (layoutName == "Yellow")
    {
      this->SetLayoutColor(vtkMRMLAbstractViewNode::GetYellowColor());
    }
    else if (layoutName == "Green")
    {
      this->SetLayoutColor(vtkMRMLAbstractViewNode::GetGreenColor());
    }
    else
    {
      this->SetLayoutColor(vtkMRMLAbstractViewNode::GetGrayColor());
    }
  }

  if (!layoutLabelFound)
  {
    std::string layoutName(this->GetLayoutName() ? this->GetLayoutName() : "");
    // Slicer3 scene file. Grok a label
    if (layoutName.find("Compare") == 0)
    {
      std::string name(this->GetLayoutName());
      std::string number(name.substr(7, name.size() - 7));
      this->SetLayoutLabel(number.c_str());
    }
    else if (layoutName == "Red")
    {
      this->SetLayoutLabel("R");
    }
    else if (layoutName == "Yellow")
    {
      this->SetLayoutLabel("Y");
    }
    else if (layoutName == "Green")
    {
      this->SetLayoutLabel("G");
    }
  }

  this->UpdateMatrices();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::CopyContent(vtkMRMLNode* anode, bool deepCopy /*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkMRMLSliceNode* node = vtkMRMLSliceNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(anode);

  vtkMRMLCopyBooleanMacro(SliceVisible);
  vtkMRMLCopyOwnedMatrix4x4Macro(SliceToRAS);

  vtkNew<vtkStringArray> namedOrientations;
  node->GetSliceOrientationPresetNames(namedOrientations.GetPointer());
  for (int i = 0; i < namedOrientations->GetNumberOfValues(); i++)
  {
    this->AddSliceOrientationPreset(namedOrientations->GetValue(i),
                                    node->GetSliceOrientationPreset(namedOrientations->GetValue(i)));
  }

  std::string orientation = node->GetOrientation();
  if (orientation != vtkMRMLSliceNode::GetReformatOrientationName())
  {
    this->SetOrientation(orientation.c_str());
  }

  vtkMRMLCopyStringMacro(DefaultOrientation);
  vtkMRMLCopyStringMacro(OrientationReference);

  vtkMRMLCopyIntMacro(JumpMode);
  vtkMRMLCopyIntMacro(ActiveSlice);

  vtkMRMLCopyIntMacro(LayoutGridRows);
  vtkMRMLCopyIntMacro(LayoutGridColumns);

  vtkMRMLCopyIntMacro(SliceSpacingMode);

  vtkMRMLCopyBooleanMacro(WidgetVisible);
  vtkMRMLCopyBooleanMacro(WidgetOutlineVisible);
  vtkMRMLCopyBooleanMacro(UseLabelOutline);

  vtkMRMLCopyIntMacro(SliceResolutionMode);

  vtkMRMLCopyVectorMacro(FieldOfView, double, 3);
  vtkMRMLCopyVectorMacro(Dimensions, int, 3);
  vtkMRMLCopyVectorMacro(XYZOrigin, double, 3);
  vtkMRMLCopyVectorMacro(UVWDimensions, int, 3);
  vtkMRMLCopyVectorMacro(UVWExtents, double, 3);
  vtkMRMLCopyVectorMacro(UVWOrigin, double, 3);
  vtkMRMLCopyVectorMacro(UVWMaximumDimensions, int, 3);
  vtkMRMLCopyVectorMacro(PrescribedSliceSpacing, double, 3);

  vtkMRMLCopyBooleanMacro(SlabReconstructionEnabled);
  vtkMRMLCopyEnumMacro(SlabReconstructionType);
  vtkMRMLCopyFloatMacro(SlabReconstructionThickness);
  vtkMRMLCopyFloatMacro(SlabReconstructionOversamplingFactor);

  vtkMRMLCopyEndMacro();

  this->UpdateMatrices();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::Reset(vtkMRMLNode* defaultNode)
{
  // The LayoutName is preserved by vtkMRMLNode::Reset, however the orientation,
  // and the layout color (typically associated with the layoutName) are not
  // preserved automatically.
  // This require a custom behavior implemented here.
  std::string orientation = this->GetOrientation();
  std::string defaultOrientation = (this->GetDefaultOrientation() ? this->GetDefaultOrientation() : "");
  double layoutColor[3] = { 0.0, 0.0, 0.0 };
  this->GetLayoutColor(layoutColor);
  this->Superclass::Reset(defaultNode);
  int wasModified = this->StartModify();
  if (orientation != vtkMRMLSliceNode::GetReformatOrientationName())
  {
    this->SetOrientation(orientation.c_str());
  }
  if (!defaultOrientation.empty())
  {
    this->SetDefaultOrientation(defaultOrientation.c_str());
  }
  this->SetLayoutColor(layoutColor);
  this->EndModify(wasModified);
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLSliceNode::GetSliceToRAS()
{
  return this->SliceToRAS;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintVectorMacro(FieldOfView, double, 3);
  vtkMRMLPrintVectorMacro(Dimensions, int, 3);
  vtkMRMLPrintVectorMacro(XYZOrigin, double, 3);
  vtkMRMLPrintVectorMacro(UVWDimensions, int, 3);
  vtkMRMLPrintVectorMacro(UVWExtents, double, 3);
  vtkMRMLPrintVectorMacro(UVWOrigin, double, 3);

  vtkMRMLPrintIntMacro(SliceResolutionMode);

  os << indent << "Layout grid: " << this->LayoutGridRows << "x" << this->LayoutGridColumns << "\n";
  vtkMRMLPrintIntMacro(ActiveSlice);

  vtkMRMLPrintBooleanMacro(SliceVisible);
  vtkMRMLPrintBooleanMacro(WidgetVisible);
  vtkMRMLPrintBooleanMacro(WidgetOutlineVisible);
  vtkMRMLPrintBooleanMacro(UseLabelOutline);

  os << indent << "Jump mode: ";
  if (this->JumpMode == CenteredJumpSlice)
  {
    os << indent << "Centered\n";
  }
  else
  {
    os << indent << "Offset\n";
  }

  vtkMRMLPrintMatrix4x4Macro(SliceToRAS);

  std::vector<OrientationPresetType>::iterator it;
  for (it = this->OrientationMatrices.begin(); it != this->OrientationMatrices.end(); ++it)
  {
    os << indent << "OrientationMatrix" << this->URLEncodeString(it->first.c_str()) << ": \n";
    it->second->PrintSelf(os, indent.GetNextIndent());
  }

  os << indent << "XYToRAS: \n";
  this->XYToRAS->PrintSelf(os, indent.GetNextIndent());

  os << indent
     << "SliceSpacingMode: " << (this->SliceSpacingMode == AutomaticSliceSpacingMode ? "Automatic" : "Prescribed")
     << "\n";

  vtkMRMLPrintVectorMacro(PrescribedSliceSpacing, double, 3);

  os << indent << "Interacting: " << (this->Interacting ? "on" : "off") << "\n";

  if (this->ThreeDViewIDs.size() > 0)
  {
    vtkMRMLPrintStdStringVectorMacro(ThreeDViewIDs, std::vector);
  }

  vtkMRMLPrintStringMacro(DefaultOrientation);

  vtkMRMLPrintBooleanMacro(SlabReconstructionEnabled);
  vtkMRMLPrintEnumMacro(SlabReconstructionType);
  vtkMRMLPrintFloatMacro(SlabReconstructionThickness);
  vtkMRMLPrintFloatMacro(SlabReconstructionOversamplingFactor);

  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::JumpSlice(double r, double a, double s)
{
  if (this->JumpMode == CenteredJumpSlice)
  {
    this->JumpSliceByCentering(r, a, s);
  }
  else if (this->JumpMode == OffsetJumpSlice)
  {
    this->JumpSliceByOffsetting(r, a, s);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::JumpSliceByCentering(double r, double a, double s)
{
  vtkMatrix4x4* sliceToRAS = this->GetSliceToRAS();
  double sr = sliceToRAS->GetElement(0, 3);
  double sa = sliceToRAS->GetElement(1, 3);
  double ss = sliceToRAS->GetElement(2, 3);

  // deduce the slice spacing
  vtkMatrix4x4* xyzToRAS = this->GetXYToRAS();

  double p1xyz[4] = { 0.0, 0.0, 0.0, 1.0 };
  double p2xyz[4] = { 0.0, 0.0, 1.0, 1.0 };

  double p1ras[4], p2ras[4];

  xyzToRAS->MultiplyPoint(p1xyz, p1ras);
  xyzToRAS->MultiplyPoint(p2xyz, p2ras);

  double sliceSpacing = sqrt(vtkMath::Distance2BetweenPoints(p2ras, p1ras));

  if (r != sr || a != sa || s != ss || this->XYZOrigin[0] != 0 || this->XYZOrigin[1] != 0 || this->XYZOrigin[2] != 0)
  {
    sliceToRAS->SetElement(0, 3, r - this->ActiveSlice * sliceSpacing * sliceToRAS->GetElement(0, 2));
    sliceToRAS->SetElement(1, 3, a - this->ActiveSlice * sliceSpacing * sliceToRAS->GetElement(1, 2));
    sliceToRAS->SetElement(2, 3, s - this->ActiveSlice * sliceSpacing * sliceToRAS->GetElement(2, 2));
    this->SetSliceOrigin(0, 0, 0); // move sliceToRAS origin into the center of the slice view
    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::JumpSliceByOffsetting(double r, double a, double s)
{
  vtkMatrix4x4* sliceToRAS = this->GetSliceToRAS();
  double sr = sliceToRAS->GetElement(0, 3);
  double sa = sliceToRAS->GetElement(1, 3);
  double ss = sliceToRAS->GetElement(2, 3);

  // deduce the slice spacing
  vtkMatrix4x4* xyzToRAS = this->GetXYToRAS();

  double p1xyz[4] = { 0.0, 0.0, 0.0, 1.0 };
  double p2xyz[4] = { 0.0, 0.0, 1.0, 1.0 };

  double p1ras[4], p2ras[4];

  xyzToRAS->MultiplyPoint(p1xyz, p1ras);
  xyzToRAS->MultiplyPoint(p2xyz, p2ras);

  double sliceSpacing = sqrt(vtkMath::Distance2BetweenPoints(p2ras, p1ras));

  double d;
  d = (r - sr) * sliceToRAS->GetElement(0, 2) + (a - sa) * sliceToRAS->GetElement(1, 2)
      + (s - ss) * sliceToRAS->GetElement(2, 2);
  sr += (d - this->ActiveSlice * sliceSpacing) * sliceToRAS->GetElement(0, 2);
  sa += (d - this->ActiveSlice * sliceSpacing) * sliceToRAS->GetElement(1, 2);
  ss += (d - this->ActiveSlice * sliceSpacing) * sliceToRAS->GetElement(2, 2);

  sliceToRAS->SetElement(0, 3, sr);
  sliceToRAS->SetElement(1, 3, sa);
  sliceToRAS->SetElement(2, 3, ss);
  this->UpdateMatrices();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::JumpSliceByOffsetting(int k, double r, double a, double s)
{
  // Jump the slice such that the kth slice is at the specified
  // ras. If there are not k slices, then jump the first slice to the
  // specified ras

  if (!(k >= 0 && k < this->LayoutGridColumns * this->LayoutGridRows))
  {
    k = 0;
  }

  // int oldActiveSlice = this->ActiveSlice;
  this->ActiveSlice = k;
  this->JumpSliceByOffsetting(r, a, s);
  // this->ActiveSlice = oldActiveSlice;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::JumpAllSlices(double r, double a, double s)
{
  vtkMRMLSliceNode::JumpAllSlices(
    this->GetScene(), r, a, s, vtkMRMLSliceNode::DefaultJumpSlice, this->GetViewGroup(), this);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::JumpAllSlices(vtkMRMLScene* scene,
                                     double r,
                                     double a,
                                     double s,
                                     int jumpMode /* =vtkMRMLSliceNode::DefaultJumpSlice */,
                                     int viewGroup /* =-1 */,
                                     vtkMRMLSliceNode* exclude /*=nullptr*/)
{
  if (!scene)
  {
    return;
  }

  int nnodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  for (int n = 0; n < nnodes; n++)
  {
    vtkMRMLSliceNode* node = vtkMRMLSliceNode::SafeDownCast(scene->GetNthNodeByClass(n, "vtkMRMLSliceNode"));
    if (node == nullptr || node == exclude)
    {
      continue;
    }
    if (viewGroup >= 0 && viewGroup != node->GetViewGroup())
    {
      continue;
    }
    if (jumpMode == vtkMRMLSliceNode::DefaultJumpSlice)
    {
      node->JumpSlice(r, a, s);
    }
    else if (jumpMode == CenteredJumpSlice)
    {
      node->JumpSliceByCentering(r, a, s);
    }
    else if (jumpMode == OffsetJumpSlice)
    {
      node->JumpSliceByOffsetting(r, a, s);
    }
    else
    {
      vtkGenericWarningMacro("vtkMRMLSliceNode::JumpAllSlices failed: invalid jump mode " << jumpMode);
    }
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetFieldOfView(double x, double y, double z)
{
  bool modified = false;
  if (x != this->FieldOfView[0] || y != this->FieldOfView[1] || z != this->FieldOfView[2])
  {
    modified = true;
    this->FieldOfView[0] = x;
    this->FieldOfView[1] = y;
    this->FieldOfView[2] = z;
  }

  if (modified)
  {
    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetXYZOrigin(double x, double y, double z)
{
  if (x != this->XYZOrigin[0] || y != this->XYZOrigin[1] || z != this->XYZOrigin[2])
  {
    this->XYZOrigin[0] = x;
    this->XYZOrigin[1] = y;
    this->XYZOrigin[2] = z;
    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetUVWOrigin(double x, double y, double z)
{
  if (x != this->UVWOrigin[0] || y != this->UVWOrigin[1] || z != this->UVWOrigin[2])
  {
    this->UVWOrigin[0] = x;
    this->UVWOrigin[1] = y;
    this->UVWOrigin[2] = z;
    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetDimensions(int x, int y, int z)
{
  if (x != this->Dimensions[0] || y != this->Dimensions[1] || z != this->Dimensions[2])
  {
    this->Dimensions[0] = x;
    this->Dimensions[1] = y;
    this->Dimensions[2] = z;
    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetUVWExtents(double x, double y, double z)
{
  if (x != this->UVWExtents[0] || y != this->UVWExtents[1] || z != this->UVWExtents[2])
  {
    this->UVWExtents[0] = x;
    this->UVWExtents[1] = y;
    this->UVWExtents[2] = z;
    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetUVWExtents(double xyz[3])
{
  this->SetUVWExtents(xyz[0], xyz[1], xyz[2]);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetSliceResolutionMode(int mode)
{
  if (this->SliceResolutionMode != mode)
  {
    this->SliceResolutionMode = mode;
    if (this->SliceResolutionMode == vtkMRMLSliceNode::SliceResolutionMatchVolumes)
    {
      this->SetUVWOrigin(0, 0, 0);
    }
    else if (this->SliceResolutionMode == vtkMRMLSliceNode::SliceResolutionMatch2DView
             || this->SliceResolutionMode == vtkMRMLSliceNode::SliceFOVMatch2DViewSpacingMatchVolumes
             || this->SliceResolutionMode == vtkMRMLSliceNode::SliceFOVMatchVolumesSpacingMatch2DView)
    {
      this->SetUVWOrigin(this->GetXYZOrigin());
    }
    this->XYToRAS->Identity();
    this->UVWToRAS->Identity();
    this->Modified();
    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetUVWDimensions(int xyz[3])
{
  this->SetUVWDimensions(xyz[0], xyz[1], xyz[2]);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetUVWMaximumDimensions(int xyz[3])
{
  this->SetUVWMaximumDimensions(xyz[0], xyz[1], xyz[2]);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetUVWOrigin(double xyz[3])
{
  this->SetUVWOrigin(xyz[0], xyz[1], xyz[2]);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetUVWMaximumDimensions(int x, int y, int z)
{
  if (x != this->UVWMaximumDimensions[0] || y != this->UVWMaximumDimensions[1] || z != this->UVWMaximumDimensions[2])
  {
    this->UVWMaximumDimensions[0] = x;
    this->UVWMaximumDimensions[1] = y;
    this->UVWMaximumDimensions[2] = z;
    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetUVWDimensions(int x, int y, int z)
{
  if (x != this->UVWDimensions[0] || y != this->UVWDimensions[1] || z != this->UVWDimensions[2])
  {
    if (x > this->UVWMaximumDimensions[0])
    {
      x = this->UVWMaximumDimensions[0];
    }
    if (y > this->UVWMaximumDimensions[1])
    {
      y = this->UVWMaximumDimensions[1];
    }
    if (z > this->UVWMaximumDimensions[2])
    {
      z = this->UVWMaximumDimensions[2];
    }
    this->UVWDimensions[0] = x;
    this->UVWDimensions[1] = y;
    this->UVWDimensions[2] = z;
    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetSliceOrigin(double x, double y, double z)
{
  bool modified = false;
  double u = x;
  double v = y;
  double w = z;

  if (this->SliceResolutionMode == vtkMRMLSliceNode::SliceResolutionMatchVolumes)
  {
    u = 0;
    v = 0;
    w = 0;
  }
  if (this->SliceResolutionMode != vtkMRMLSliceNode::SliceResolutionMatch2DView
      && this->SliceResolutionMode != vtkMRMLSliceNode::SliceResolutionCustom)
  {
    if (u != this->UVWOrigin[0] || v != this->UVWOrigin[1] || w != this->UVWOrigin[2])
    {
      this->UVWOrigin[0] = u;
      this->UVWOrigin[1] = v;
      this->UVWOrigin[2] = w;
      modified = true;
    }
  }
  if (x != this->XYZOrigin[0] || y != this->XYZOrigin[1] || z != this->XYZOrigin[2])
  {
    this->XYZOrigin[0] = x;
    this->XYZOrigin[1] = y;
    this->XYZOrigin[2] = z;
    modified = true;
  }

  if (modified)
  {
    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetSliceOrigin(double xyz[3])
{
  this->SetSliceOrigin(xyz[0], xyz[1], xyz[2]);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetUVWExtentsAndDimensions(double extents[3], int dimensions[3])
{
  bool modified = false;

  if (extents[0] != this->UVWExtents[0] || extents[1] != this->UVWExtents[1] || extents[2] != this->UVWExtents[2])
  {
    modified = true;
    this->UVWExtents[0] = extents[0];
    this->UVWExtents[1] = extents[1];
    this->UVWExtents[2] = extents[2];
  }

  if (dimensions[0] != this->UVWDimensions[0] || dimensions[1] != this->UVWDimensions[1]
      || dimensions[2] != this->UVWDimensions[2])
  {
    modified = true;
    if (dimensions[0] > this->UVWMaximumDimensions[0])
    {
      dimensions[0] = this->UVWMaximumDimensions[0];
    }
    if (dimensions[1] > this->UVWMaximumDimensions[1])
    {
      dimensions[1] = this->UVWMaximumDimensions[1];
    }
    if (dimensions[2] > this->UVWMaximumDimensions[2])
    {
      dimensions[2] = this->UVWMaximumDimensions[2];
    }
    this->UVWDimensions[0] = dimensions[0];
    this->UVWDimensions[1] = dimensions[1];
    this->UVWDimensions[2] = dimensions[2];
  }

  if (modified)
  {
    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLSliceNode::GetXYToSlice()
{
  return this->XYToSlice;
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLSliceNode::GetXYToRAS()
{
  return this->XYToRAS;
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLSliceNode::GetUVWToSlice()
{
  return this->UVWToSlice;
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLSliceNode::GetUVWToRAS()
{
  return this->UVWToRAS;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetLayoutGrid(int rows, int columns)
{
  // Much of this code looks more like application logic than data
  // code. Should the adjustments to Dimensions and FieldOfView be
  // pulled out the SetLayoutGrid*() methods and put in the logic/gui
  // level?
  if ((rows != this->LayoutGridRows) || (columns != this->LayoutGridColumns))
  {
    // Calculate the scaling and "scaling magnitudes"
    double scaling[3];
    scaling[0] = this->LayoutGridColumns / (double)columns;
    scaling[1] = this->LayoutGridRows / (double)rows;
    scaling[2] = 1.0; // ???

    double scaleMagnitude[3];
    scaleMagnitude[0] = (scaling[0] < 1.0 ? 1.0 / scaling[0] : scaling[0]);
    scaleMagnitude[1] = (scaling[1] < 1.0 ? 1.0 / scaling[1] : scaling[1]);
    scaleMagnitude[2] = 1.0;

    // A change in the LightBox layout changes the dimensions of the
    // slice and the FieldOfView in Z
    this->Dimensions[0] = int(this->Dimensions[0] * scaling[0]);
    this->Dimensions[1] = int(this->Dimensions[1] * scaling[1]);
    this->Dimensions[2] = rows * columns;

    // adjust the field of view in x and y to maintain aspect ratio
    if (scaleMagnitude[0] < scaleMagnitude[1])
    {
      // keep x fov the same, adjust y
      this->FieldOfView[1] *= (scaling[1] / scaling[0]);
    }
    else
    {
      // keep y fov the same, adjust x
      this->FieldOfView[0] *= (scaling[0] / scaling[1]);
    }

    // keep the same pixel spacing in z, i.e. update FieldOfView[2]
    this->FieldOfView[2] *= (rows * columns / (double)(this->LayoutGridRows * this->LayoutGridColumns));

    // cache the layout
    this->LayoutGridRows = rows;
    this->LayoutGridColumns = columns;

    // if the active slice is not on the lightbox, then reset active
    // slice to the last slice in the lightbox
    if (this->ActiveSlice >= this->LayoutGridRows * this->LayoutGridColumns)
    {
      this->ActiveSlice = this->LayoutGridRows * this->LayoutGridColumns - 1;
    }

    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetLayoutGridRows(int rows)
{
  // Much of this code looks more like application logic than data
  // code. Should the adjustments to Dimensions and FieldOfView be
  // pulled out the SetLayoutGrid*() methods and put in the logic/gui
  // level?
  if (rows != this->LayoutGridRows)
  {
    // Calculate the scaling
    double scaling;
    scaling = this->LayoutGridRows / (double)rows;

    // A change in the LightBox layout changes the dimensions of the
    // slice and the FieldOfView in Z
    this->Dimensions[1] = int(this->Dimensions[1] * scaling);
    this->Dimensions[2] = rows * this->LayoutGridColumns;

    // adjust the field of view in x to maintain aspect ratio
    this->FieldOfView[0] /= scaling;

    // keep the same pixel spacing in z, i.e. update FieldOfView[2]
    this->FieldOfView[2] *= (rows / (double)this->LayoutGridRows);

    // cache the layout
    this->LayoutGridRows = rows;

    // if the active slice is not on the lightbox, then reset active
    // slice to the last slice in the lightbox
    if (this->ActiveSlice >= this->LayoutGridRows * this->LayoutGridColumns)
    {
      this->ActiveSlice = this->LayoutGridRows * this->LayoutGridColumns - 1;
    }

    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetLayoutGridColumns(int cols)
{
  // Much of this code looks more like application logic than data
  // code. Should the adjustments to Dimensions and FieldOfView be
  // pulled out the SetLayoutGrid*() methods and put in the logic/gui
  // level?
  if (cols != this->LayoutGridColumns)
  {
    // Calculate the scaling
    double scaling;
    scaling = this->LayoutGridColumns / (double)cols;

    // A change in the LightBox layout changes the dimensions of the
    // slice and the FieldOfView in Z
    this->Dimensions[0] = int(this->Dimensions[0] * (this->LayoutGridColumns / (double)cols));
    this->Dimensions[2] = this->LayoutGridRows * cols;

    // adjust the field of view in y to maintain aspect ratio
    this->FieldOfView[1] /= scaling;

    // keep the same pixel spacing in z, i.e. update FieldOfView[2]
    this->FieldOfView[2] *= (cols / (double)this->LayoutGridColumns);

    // cache the layout
    this->LayoutGridColumns = cols;

    // if the active slice is not on the lightbox, then reset active
    // slice to the last slice in the lightbox
    if (this->ActiveSlice >= this->LayoutGridRows * this->LayoutGridColumns)
    {
      this->ActiveSlice = this->LayoutGridRows * this->LayoutGridColumns - 1;
    }

    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetSliceSpacingModeToAutomatic()
{
  this->SetSliceSpacingMode(AutomaticSliceSpacingMode);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetSliceSpacingModeToPrescribed()
{
  this->SetSliceSpacingMode(PrescribedSliceSpacingMode);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetJumpModeToCentered()
{
  this->SetJumpMode(CenteredJumpSlice);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetJumpModeToOffset()
{
  this->SetJumpMode(OffsetJumpSlice);
}

//----------------------------------------------------------------------------
// Get/Set the current distance from the origin to the slice plane
double vtkMRMLSliceNode::GetSliceOffset()
{
  //
  // - get the current translation in RAS space and convert it to Slice space
  //   by transforming it by the inverse of the upper 3x3 of SliceToRAS
  // - pull out the Z translation part
  //

  vtkNew<vtkMatrix4x4> sliceToRAS;
  sliceToRAS->DeepCopy(this->GetSliceToRAS());
  for (int i = 0; i < 3; i++)
  {
    sliceToRAS->SetElement(i, 3, 0.0); // Zero out the translation portion
  }
  sliceToRAS->Invert();
  double v1[4], v2[4];
  for (int i = 0; i < 4; i++)
  { // get the translation back as a vector
    v1[i] = this->GetSliceToRAS()->GetElement(i, 3);
  }
  // bring the translation into slice space
  // and overwrite the z part
  sliceToRAS->MultiplyPoint(v1, v2);

  return (v2[2]);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetSliceOffset(double offset)
{
  //
  // Set the Offset
  // - get the current translation in RAS space and convert it to Slice space
  //   by transforming it by the inverse of the upper 3x3 of SliceToRAS
  // - replace the z value of the translation with the new value given by the slider
  // - this preserves whatever translation was already in place
  //

  double oldOffset = this->GetSliceOffset();
  if (fabs(offset - oldOffset) <= 1.0e-6)
  {
    return;
  }

  vtkNew<vtkMatrix4x4> sliceToRAS;
  sliceToRAS->DeepCopy(this->GetSliceToRAS());
  for (int i = 0; i < 3; i++)
  {
    sliceToRAS->SetElement(i, 3, 0.0); // Zero out the translation portion
  }
  vtkNew<vtkMatrix4x4> sliceToRASInverted; // inverse sliceToRAS
  sliceToRASInverted->DeepCopy(sliceToRAS.GetPointer());
  sliceToRASInverted->Invert();
  double v1[4], v2[4], v3[4];
  for (int i = 0; i < 4; i++)
  { // get the translation back as a vector
    v1[i] = this->GetSliceToRAS()->GetElement(i, 3);
  }
  // bring the translation into slice space
  // and overwrite the z part
  sliceToRASInverted->MultiplyPoint(v1, v2);

  v2[2] = offset;

  // Now bring the new translation vector back into RAS space
  sliceToRAS->MultiplyPoint(v2, v3);

  // if the translation has changed, update the rest of the matrices
  double eps = 1.0e-6;
  if (fabs(v1[0] - v3[0]) > eps || fabs(v1[1] - v3[1]) > eps || fabs(v1[2] - v3[2]) > eps)
  {
    // copy new translation into sliceToRAS
    for (int i = 0; i < 4; i++)
    {
      sliceToRAS->SetElement(i, 3, v3[i]);
    }
    this->GetSliceToRAS()->DeepCopy(sliceToRAS.GetPointer());
    this->UpdateMatrices();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::RotateToVolumePlane(vtkMRMLVolumeNode* volumeNode, bool forceSlicePlaneToSingleSlice /*=true*/)
{
  if (volumeNode == nullptr)
  {
    return;
  }

  vtkNew<vtkMatrix4x4> ijkToRAS;
  volumeNode->GetIJKToRASMatrix(ijkToRAS.GetPointer());

  // apply the transform
  vtkMRMLTransformNode* transformNode = volumeNode->GetParentTransformNode();
  if (transformNode != nullptr)
  {
    if (transformNode->IsTransformToWorldLinear())
    {
      vtkNew<vtkMatrix4x4> rasToRAS;
      transformNode->GetMatrixTransformToWorld(rasToRAS.GetPointer());
      rasToRAS->Multiply4x4(rasToRAS.GetPointer(), ijkToRAS.GetPointer(), ijkToRAS.GetPointer());
    }
    else
    {
      vtkErrorMacro("Cannot handle non-linear transforms");
    }
  }

  int volumeAxisIndexForSliceZ = -1;
  if (forceSlicePlaneToSingleSlice && volumeNode->GetImageData() != nullptr)
  {
    int dims[3] = { 0, 0, 0 };
    volumeNode->GetImageData()->GetDimensions(dims);
    for (int volumeAxisIndex = 0; volumeAxisIndex < 3; volumeAxisIndex++)
    {
      if (dims[volumeAxisIndex] == 1)
      {
        volumeAxisIndexForSliceZ = volumeAxisIndex;
        break;
      }
    }
  }

  this->RotateToAxes(ijkToRAS.GetPointer(), volumeAxisIndexForSliceZ);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::RotateToAxes(vtkMatrix4x4* referenceToRAS, int sliceNormalAxisIndex /*=-1*/)
{
  if (referenceToRAS == nullptr)
  {
    return;
  }

  double testedDirections[6][4] = { { 1, 0, 0, 0 },  { -1, 0, 0, 0 }, { 0, 1, 0, 0 },
                                    { 0, -1, 0, 0 }, { 0, 0, 1, 0 },  { 0, 0, -1, 0 } };

  // To keep left/right handedness of the input referenceToRAS coordinate system,
  // we need to determine it here and reproduce in the end result later.
  vtkNew<vtkMatrix3x3> sliceToRasOrientation;
  vtkAddonMathUtilities::GetOrientationMatrix(this->SliceToRAS, sliceToRasOrientation.GetPointer());
  bool sliceToRasRightHanded = (sliceToRasOrientation->Determinant() >= 0);

  // background is a single-slice volume
  // find combination of volume axis directions that are closest to current slice X, Y
  // axis directions

  // 8 combinations of X, Y vector directions, 2 vectors (slice X and Y)
  double sliceXAxisDirection[3] = { 0.0 };
  double sliceYAxisDirection[3] = { 0.0 };
  double sliceZAxisDirection[3] = { 0.0 };
  vtkAddonMathUtilities::GetOrientationMatrixColumn(this->SliceToRAS, 0, sliceXAxisDirection);
  vtkAddonMathUtilities::GetOrientationMatrixColumn(this->SliceToRAS, 1, sliceYAxisDirection);
  vtkAddonMathUtilities::GetOrientationMatrixColumn(this->SliceToRAS, 2, sliceZAxisDirection);
  double minAngleDiff = -1;
  for (int testedDirectionIndexX = 0; testedDirectionIndexX < 6; testedDirectionIndexX++)
  {
    int volumeAxisAsSliceX = testedDirectionIndexX / 2;
    if (sliceNormalAxisIndex >= 0 && volumeAxisAsSliceX == sliceNormalAxisIndex)
    {
      // X slice axis must not be snapped to volume plane normal axis
      continue;
    }
    for (int testedDirectionIndexY = 0; testedDirectionIndexY < 6; testedDirectionIndexY++)
    {
      int volumeAxisAsSliceY = testedDirectionIndexY / 2;
      if (volumeAxisAsSliceY == volumeAxisAsSliceX
          || (sliceNormalAxisIndex >= 0 && volumeAxisAsSliceY == sliceNormalAxisIndex))
      {
        // Y slice axis must not be snapped to X slice axis or volume plane normal axis
        continue;
      }
      double sliceXAxisVolumeAxisDirection[4] = { 0.0 };
      double sliceYAxisVolumeAxisDirection[4] = { 0.0 };
      double sliceZAxisVolumeAxisDirection[4] = { 0.0 };
      referenceToRAS->MultiplyPoint(testedDirections[testedDirectionIndexX], sliceXAxisVolumeAxisDirection);
      referenceToRAS->MultiplyPoint(testedDirections[testedDirectionIndexY], sliceYAxisVolumeAxisDirection);
      vtkMath::Cross(sliceXAxisVolumeAxisDirection, sliceYAxisVolumeAxisDirection, sliceZAxisVolumeAxisDirection);

      // Comparison metric is sum of angle between x, y, z axis
      double zAxisAngleDiff = fabs(vtkMath::AngleBetweenVectors(sliceZAxisVolumeAxisDirection, sliceZAxisDirection));
      if (zAxisAngleDiff > vtkMath::Pi() / 2.0)
      {
        // we ignore z axis flip (we will compute final z direction from on x and y axes and sliceToRasRightHanded)
        zAxisAngleDiff = vtkMath::Pi() - zAxisAngleDiff;
      }
      double angleDiff = fabs(vtkMath::AngleBetweenVectors(sliceXAxisVolumeAxisDirection, sliceXAxisDirection))
                         + fabs(vtkMath::AngleBetweenVectors(sliceYAxisVolumeAxisDirection, sliceYAxisDirection))
                         + zAxisAngleDiff;

      if (angleDiff < minAngleDiff || minAngleDiff < 0.)
      {
        minAngleDiff = angleDiff;
        vtkMath::Normalize(sliceXAxisVolumeAxisDirection);
        vtkMath::Normalize(sliceYAxisVolumeAxisDirection);
        vtkAddonMathUtilities::SetOrientationMatrixColumn(this->SliceToRAS, 0, sliceXAxisVolumeAxisDirection);
        vtkAddonMathUtilities::SetOrientationMatrixColumn(this->SliceToRAS, 1, sliceYAxisVolumeAxisDirection);
      }
    }
  }

  // Get final X, Y axis directions
  vtkAddonMathUtilities::GetOrientationMatrixColumn(this->SliceToRAS, 0, sliceXAxisDirection);
  vtkAddonMathUtilities::GetOrientationMatrixColumn(this->SliceToRAS, 1, sliceYAxisDirection);
  // Set slice Z axis
  if (sliceToRasRightHanded)
  {
    vtkMath::Cross(sliceXAxisDirection, sliceYAxisDirection, sliceZAxisDirection);
  }
  else
  {
    vtkMath::Cross(sliceYAxisDirection, sliceXAxisDirection, sliceZAxisDirection);
  }
  vtkAddonMathUtilities::SetOrientationMatrixColumn(this->SliceToRAS, 2, sliceZAxisDirection);

  this->UpdateMatrices();
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceNode::SetOrientationToDefault()
{
  if (!this->GetDefaultOrientation() || strlen(this->GetDefaultOrientation()) == 0
      || !strcmp(this->GetDefaultOrientation(), vtkMRMLSliceNode::GetReformatOrientationName()))
  {
    return false;
  }
  return this->SetOrientation(this->GetDefaultOrientation());
}

//---------------------------------------------------------------------------
const char* vtkMRMLSliceNode::GetSlabReconstructionTypeAsString(int slabReconstructionType)
{
  switch (slabReconstructionType)
  {
    case VTK_IMAGE_SLAB_MAX:
      return "Max";
    case VTK_IMAGE_SLAB_MIN:
      return "Min";
    case VTK_IMAGE_SLAB_MEAN:
      return "Mean";
    case VTK_IMAGE_SLAB_SUM:
      return "Sum";
    default:
      vtkGenericWarningMacro("Unknown reconstruction type: " << slabReconstructionType);
      return "";
  }
}

//-----------------------------------------------------------
int vtkMRMLSliceNode::GetSlabReconstructionTypeFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  // VTK_IMAGE_SLAB enum doesn't use last
  for (int ii = 0; ii < 4; ii++)
  {
    if (strcmp(name, GetSlabReconstructionTypeAsString(ii)) == 0)
    {
      // found a matching name
      return ii;
    }
  }
  // unknown name
  return -1;
}
