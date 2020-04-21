/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLViewNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLViewNode);

//----------------------------------------------------------------------------
vtkMRMLViewNode::vtkMRMLViewNode()
{
  this->BoxVisible = 1;
  this->AxisLabelsVisible = 1;
  this->AxisLabelsCameraDependent = 1;
  this->FiducialsVisible = 1;
  this->FiducialLabelsVisible = 1;
  this->FieldOfView = 200;
  this->LetterSize = 0.05;
  this->AnimationMode = vtkMRMLViewNode::Off;
  this->ViewAxisMode = vtkMRMLViewNode::LookFrom;
  this->SpinDegrees = 2.0;
  this->RotateDegrees = 5.0;
  this->SpinDirection = vtkMRMLViewNode::YawLeft;
  this->AnimationMs = 5;
  this->RockLength = 200;
  this->RockCount = 0;
  this->StereoType = vtkMRMLViewNode::NoStereo;
  this->RenderMode = vtkMRMLViewNode::Perspective;
  this->BackgroundColor[0] = this->defaultBackgroundColor()[0];
  this->BackgroundColor[1] = this->defaultBackgroundColor()[1];
  this->BackgroundColor[2] = this->defaultBackgroundColor()[2];
  this->BackgroundColor2[0] = this->defaultBackgroundColor2()[0];
  this->BackgroundColor2[1] = this->defaultBackgroundColor2()[1];
  this->BackgroundColor2[2] = this->defaultBackgroundColor2()[2];
  this->UseDepthPeeling = 1;
  this->FPSVisible = 0;
  this->OrientationMarkerEnabled = true;
  this->RulerEnabled = true;
  this->GPUMemorySize = 0; // Means application default
  this->ExpectedFPS = 8.;
  this->VolumeRenderingQuality = vtkMRMLViewNode::Adaptive;
  this->RaycastTechnique = vtkMRMLViewNode::Composite;
  this->VolumeRenderingSurfaceSmoothing = false;
  this->VolumeRenderingOversamplingFactor = 2.0;
  this->LinkedControl = 0;
  this->Interacting = 0;
  this->InteractionFlags = 0;
}

//----------------------------------------------------------------------------
vtkMRMLViewNode::~vtkMRMLViewNode() = default;

//----------------------------------------------------------------------------
const char* vtkMRMLViewNode::GetNodeTagName()
{
  return "View";
}

//----------------------------------------------------------------------------
void vtkMRMLViewNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLFloatMacro(fieldOfView, FieldOfView);
  vtkMRMLWriteXMLFloatMacro(letterSize, LetterSize);
  vtkMRMLWriteXMLBooleanMacro(boxVisible, BoxVisible);
  vtkMRMLWriteXMLBooleanMacro(fiducialsVisible, FiducialsVisible);
  vtkMRMLWriteXMLBooleanMacro(fiducialLabelsVisible, FiducialLabelsVisible);
  vtkMRMLWriteXMLBooleanMacro(axisLabelsVisible, AxisLabelsVisible);
  vtkMRMLWriteXMLBooleanMacro(axisLabelsCameraDependent, AxisLabelsCameraDependent);
  vtkMRMLWriteXMLEnumMacro(animationMode, AnimationMode);
  vtkMRMLWriteXMLEnumMacro(viewAxisMode, ViewAxisMode);
  vtkMRMLWriteXMLFloatMacro(spinDegrees, SpinDegrees);
  vtkMRMLWriteXMLFloatMacro(spinMs, AnimationMs);
  vtkMRMLWriteXMLEnumMacro(spinDirection, SpinDirection);
  vtkMRMLWriteXMLFloatMacro(rotateDegrees, RotateDegrees);
  vtkMRMLWriteXMLIntMacro(rockLength, RockLength);
  vtkMRMLWriteXMLIntMacro(rockCount, RockCount);
  vtkMRMLWriteXMLEnumMacro(stereoType, StereoType);
  vtkMRMLWriteXMLEnumMacro(renderMode, RenderMode);
  vtkMRMLWriteXMLIntMacro(useDepthPeeling, UseDepthPeeling);
  vtkMRMLWriteXMLIntMacro(gpuMemorySize, GPUMemorySize);
  vtkMRMLWriteXMLFloatMacro(expectedFPS, ExpectedFPS);
  vtkMRMLWriteXMLEnumMacro(volumeRenderingQuality, VolumeRenderingQuality);
  vtkMRMLWriteXMLEnumMacro(raycastTechnique, RaycastTechnique);
  vtkMRMLWriteXMLIntMacro(volumeRenderingSurfaceSmoothing, VolumeRenderingSurfaceSmoothing);
  vtkMRMLWriteXMLFloatMacro(volumeRenderingOversamplingFactor, VolumeRenderingOversamplingFactor);
  vtkMRMLWriteXMLIntMacro(linkedControl, LinkedControl);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLFloatMacro(fieldOfView, FieldOfView);
  vtkMRMLReadXMLFloatMacro(letterSize, LetterSize);
  vtkMRMLReadXMLBooleanMacro(boxVisible, BoxVisible);
  vtkMRMLReadXMLBooleanMacro(fiducialsVisible, FiducialsVisible);
  vtkMRMLReadXMLBooleanMacro(fiducialLabelsVisible, FiducialLabelsVisible);
  vtkMRMLReadXMLBooleanMacro(axisLabelsVisible, AxisLabelsVisible);
  vtkMRMLReadXMLBooleanMacro(axisLabelsCameraDependent, AxisLabelsCameraDependent);
  vtkMRMLReadXMLEnumMacro(animationMode, AnimationMode);
  vtkMRMLReadXMLEnumMacro(viewAxisMode, ViewAxisMode);
  vtkMRMLReadXMLFloatMacro(spinDegrees, SpinDegrees);
  vtkMRMLReadXMLFloatMacro(spinMs, AnimationMs);
  vtkMRMLReadXMLEnumMacro(spinDirection, SpinDirection);
  vtkMRMLReadXMLFloatMacro(rotateDegrees, RotateDegrees);
  vtkMRMLReadXMLIntMacro(rockLength, RockLength);
  vtkMRMLReadXMLIntMacro(rockCount, RockCount);
  vtkMRMLReadXMLEnumMacro(stereoType, StereoType);
  vtkMRMLReadXMLEnumMacro(renderMode, RenderMode);
  vtkMRMLReadXMLIntMacro(useDepthPeeling, UseDepthPeeling);
  vtkMRMLReadXMLIntMacro(gpuMemorySize, GPUMemorySize);
  vtkMRMLReadXMLFloatMacro(expectedFPS, ExpectedFPS);
  vtkMRMLReadXMLEnumMacro(volumeRenderingQuality, VolumeRenderingQuality);
  vtkMRMLReadXMLEnumMacro(raycastTechnique, RaycastTechnique);
  vtkMRMLReadXMLIntMacro(volumeRenderingSurfaceSmoothing, VolumeRenderingSurfaceSmoothing);
  vtkMRMLReadXMLFloatMacro(volumeRenderingOversamplingFactor, VolumeRenderingOversamplingFactor);
  vtkMRMLReadXMLIntMacro(linkedControl, LinkedControl);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLViewNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyFloatMacro(FieldOfView);
  vtkMRMLCopyFloatMacro(LetterSize);
  vtkMRMLCopyBooleanMacro(BoxVisible);
  vtkMRMLCopyBooleanMacro(FiducialsVisible);
  vtkMRMLCopyBooleanMacro(FiducialLabelsVisible);
  vtkMRMLCopyBooleanMacro(AxisLabelsVisible);
  vtkMRMLCopyBooleanMacro(AxisLabelsCameraDependent);
  vtkMRMLCopyEnumMacro(AnimationMode);
  vtkMRMLCopyEnumMacro(ViewAxisMode);
  vtkMRMLCopyFloatMacro(SpinDegrees);
  vtkMRMLCopyFloatMacro(AnimationMs);
  vtkMRMLCopyEnumMacro(SpinDirection);
  vtkMRMLCopyFloatMacro(RotateDegrees);
  vtkMRMLCopyIntMacro(RockLength);
  vtkMRMLCopyIntMacro(RockCount);
  vtkMRMLCopyEnumMacro(StereoType);
  vtkMRMLCopyEnumMacro(RenderMode);
  vtkMRMLCopyIntMacro(UseDepthPeeling);
  vtkMRMLCopyIntMacro(GPUMemorySize);
  vtkMRMLCopyFloatMacro(ExpectedFPS);
  vtkMRMLCopyIntMacro(VolumeRenderingQuality);
  vtkMRMLCopyIntMacro(RaycastTechnique);
  vtkMRMLCopyIntMacro(VolumeRenderingSurfaceSmoothing);
  vtkMRMLCopyFloatMacro(VolumeRenderingOversamplingFactor);
  vtkMRMLCopyIntMacro(LinkedControl);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintFloatMacro(FieldOfView);
  vtkMRMLPrintFloatMacro(LetterSize);
  vtkMRMLPrintBooleanMacro(BoxVisible);
  vtkMRMLPrintBooleanMacro(FiducialsVisible);
  vtkMRMLPrintBooleanMacro(FiducialLabelsVisible);
  vtkMRMLPrintBooleanMacro(AxisLabelsVisible);
  vtkMRMLPrintBooleanMacro(AxisLabelsCameraDependent);
  vtkMRMLPrintEnumMacro(AnimationMode);
  vtkMRMLPrintEnumMacro(ViewAxisMode);
  vtkMRMLPrintFloatMacro(SpinDegrees);
  vtkMRMLPrintFloatMacro(AnimationMs);
  vtkMRMLPrintEnumMacro(SpinDirection);
  vtkMRMLPrintFloatMacro(RotateDegrees);
  vtkMRMLPrintIntMacro(RockLength);
  vtkMRMLPrintIntMacro(RockCount);
  vtkMRMLPrintEnumMacro(StereoType);
  vtkMRMLPrintEnumMacro(RenderMode);
  vtkMRMLPrintIntMacro(UseDepthPeeling);
  vtkMRMLPrintIntMacro(GPUMemorySize);
  vtkMRMLPrintFloatMacro(ExpectedFPS);
  vtkMRMLPrintIntMacro(VolumeRenderingQuality);
  vtkMRMLPrintIntMacro(RaycastTechnique);
  vtkMRMLPrintIntMacro(VolumeRenderingSurfaceSmoothing);
  vtkMRMLPrintFloatMacro(VolumeRenderingOversamplingFactor);
  vtkMRMLPrintIntMacro(Interacting);
  vtkMRMLPrintIntMacro(LinkedControl);
  vtkMRMLPrintEndMacro();
}

//------------------------------------------------------------------------------
double* vtkMRMLViewNode::defaultBackgroundColor()
{
  //static double backgroundColor[3] = {0.70196, 0.70196, 0.90588};
  static double backgroundColor[3] = {0.7568627450980392,
                                      0.7647058823529412,
                                      0.9098039215686275};
  return backgroundColor;
}

//------------------------------------------------------------------------------
double* vtkMRMLViewNode::defaultBackgroundColor2()
{
  static double backgroundColor2[3] = {0.4549019607843137,
                                       0.4705882352941176,
                                       0.7450980392156863};
  return backgroundColor2;
}

//---------------------------------------------------------------------------
const char* vtkMRMLViewNode::GetAnimationModeAsString(int id)
{
  switch (id)
    {
    case Off: return "Off";
    case Spin: return "Spin";
    case Rock: return "Rock";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLViewNode::GetAnimationModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < AnimationMode_Last; ii++)
    {
    if (strcmp(name, GetAnimationModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkMRMLViewNode::GetViewAxisModeAsString(int id)
{
  switch (id)
    {
    case LookFrom: return "LookFrom";
    case RotateAround: return "RotateAround";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLViewNode::GetViewAxisModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < ViewAxisMode_Last; ii++)
    {
    if (strcmp(name, GetViewAxisModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkMRMLViewNode::GetSpinDirectionAsString(int id)
{
  switch (id)
    {
    case PitchUp: return "PitchUp";
    case PitchDown: return "PitchDown";
    case RollLeft: return "RollLeft";
    case RollRight: return "RollRight";
    case YawLeft: return "YawLeft";
    case YawRight: return "YawRight";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLViewNode::GetSpinDirectionFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < SpinDirection_Last; ii++)
    {
    if (strcmp(name, GetSpinDirectionAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkMRMLViewNode::GetStereoTypeAsString(int id)
{
  switch (id)
    {
    case NoStereo: return "NoStereo";
    case RedBlue: return "RedBlue";
    case Anaglyph: return "Anaglyph";
    case QuadBuffer: return "QuadBuffer";
    case Interlaced: return "Interlaced";
    case UserDefined_1: return "UserDefined_1";
    case UserDefined_2: return "UserDefined_2";
    case UserDefined_3: return "UserDefined_3";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLViewNode::GetStereoTypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < StereoType_Last; ii++)
    {
    if (strcmp(name, GetStereoTypeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkMRMLViewNode::GetRenderModeAsString(int id)
{
  switch (id)
    {
    case Perspective: return "Perspective";
    case Orthographic: return "Orthographic";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLViewNode::GetRenderModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < RenderMode_Last; ii++)
    {
    if (strcmp(name, GetRenderModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkMRMLViewNode::GetVolumeRenderingQualityAsString(int id)
{
  switch (id)
    {
    case Adaptive: return "Adaptive";
    case Normal: return "Normal";
    case Maximum: return "Maximum";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLViewNode::GetVolumeRenderingQualityFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < VolumeRenderingQuality_Last; ii++)
    {
    if (strcmp(name, GetVolumeRenderingQualityAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//---------------------------------------------------------------------------
const char* vtkMRMLViewNode::GetRaycastTechniqueAsString(int id)
{
  switch (id)
    {
    case Composite: return "Composite";
    case CompositeEdgeColoring: return "CompositeEdgeColoring";
    case MaximumIntensityProjection: return "MaximumIntensityProjection";
    case MinimumIntensityProjection: return "MinimumIntensityProjection";
    case GradiantMagnitudeOpacityModulation: return "GradiantMagnitudeOpacityModulation";
    case IllustrativeContextPreservingExploration: return "IllustrativeContextPreservingExploration";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLViewNode::GetRaycastTechniqueFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < RaycastTechnique_Last; ii++)
    {
    if (strcmp(name, GetRaycastTechniqueAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
void vtkMRMLViewNode::SetInteracting(int interacting)
{
  // Don't call Modified()
  this->Interacting = interacting;
}

//-----------------------------------------------------------
void vtkMRMLViewNode::SetInteractionFlags(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlags = flags;
}
