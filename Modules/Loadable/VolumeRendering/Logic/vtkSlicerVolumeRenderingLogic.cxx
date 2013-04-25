/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumeRenderingLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// Volume Rendering includes
#include "vtkMRMLNCIRayCastVolumeRenderingDisplayNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkMRMLVolumeRenderingScenarioNode.h"
#include "vtkSlicerVolumeRenderingLogic.h"
#include "vtkMRMLCPURayCastVolumeRenderingDisplayNode.h"
#include "vtkMRMLNCIRayCastVolumeRenderingDisplayNode.h"
#include "vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode.h"
#include "vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode.h"
#include "vtkMRMLGPURayCastVolumeRenderingDisplayNode.h"

// Annotations includes
#include <vtkMRMLAnnotationROINode.h>

// MRML includes
#include <vtkCacheManager.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVolumePropertyNode.h>
#include <vtkMRMLVolumePropertyStorageNode.h>

// VTKSYS includes
#include <itksys/SystemTools.hxx>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkgl.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkVolumeProperty.h>

// STD includes
#include <algorithm>
#include <cassert>

//----------------------------------------------------------------------------
double nextHigher(double value)
{
  // Increment the value by the smallest offset possible
  typedef union {
      long long i64;
      double d64;
    } dbl_64;
  dbl_64 d;
  d.d64 = value;
  d.i64 += (value >= 0) ? 1 : -1;
  return d.d64;
}

//----------------------------------------------------------------------------
double higherAndUnique(double value, double &previousValue)
{
  value = std::max(value, previousValue);
  if (value == previousValue)
    {
    value = nextHigher(value);
    }
  assert (value != previousValue);
  previousValue = value;
  return value;
}

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerVolumeRenderingLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerVolumeRenderingLogic);

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic::vtkSlicerVolumeRenderingLogic()
{
  this->DefaultRenderingMethod = NULL;
  this->UseLinearRamp = true;
  this->PresetsScene = 0;

  this->RegisterRenderingMethod("VTK CPU Ray Casting",
                                "vtkMRMLCPURayCastVolumeRenderingDisplayNode");
  this->RegisterRenderingMethod("VTK GPU Ray Casting",
                                "vtkMRMLGPURayCastVolumeRenderingDisplayNode");
  this->RegisterRenderingMethod("VTK OpenGL 3D Texture Mapping",
                                "vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode");
  this->RegisterRenderingMethod("NCI GPU Ray Casting",
                                "vtkMRMLNCIRayCastVolumeRenderingDisplayNode");
  //this->RegisterRenderingMethod("NCI GPU MultiVolume Ray Casting",
  //                              "vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode");
}

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic::~vtkSlicerVolumeRenderingLogic()
{
  if (this->DefaultRenderingMethod)
    {
    delete [] this->DefaultRenderingMethod;
    }
  if (this->PresetsScene)
    {
    this->PresetsScene->Delete();
    }
  this->RemoveAllVolumeRenderingDisplayNodes();
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Display nodes:" << std::endl;
  for (unsigned int i = 0; i < this->DisplayNodes.size(); ++i)
    {
    os << indent << this->DisplayNodes[i]->GetID() << std::endl;
    }
  const char *gl_vendor=reinterpret_cast<const char *>(glGetString(GL_VENDOR));
  os << indent << "Vendor: " << gl_vendor << std::endl;
  const char *gl_version=reinterpret_cast<const char *>(glGetString(GL_VERSION));
  os << indent << "Version: " << gl_version << std::endl;
  const char *glsl_version=
    reinterpret_cast<const char *>(glGetString(vtkgl::SHADING_LANGUAGE_VERSION));
  os << indent << "Shading Language Version: " << glsl_version << std::endl;
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::RegisterNodes()
{
  assert(this->GetMRMLScene());

  vtkNew<vtkMRMLVolumePropertyNode> vpn;
  this->GetMRMLScene()->RegisterNodeClass( vpn.GetPointer() );

  vtkNew<vtkMRMLVolumePropertyStorageNode> vpsn;
  this->GetMRMLScene()->RegisterNodeClass( vpsn.GetPointer() );

  vtkNew<vtkMRMLVolumeRenderingScenarioNode> vrsNode;
  this->GetMRMLScene()->RegisterNodeClass( vrsNode.GetPointer() );

  vtkNew<vtkMRMLCPURayCastVolumeRenderingDisplayNode> cpuVRNode;
  this->GetMRMLScene()->RegisterNodeClass( cpuVRNode.GetPointer() );
  // Volume rendering nodes used to have the tag "VolumeRenderingParameters"
  // in scenes prior to Slicer 4.2
#if MRML_SUPPORT_VERSION < 0x040200
  this->GetMRMLScene()->RegisterNodeClass( cpuVRNode.GetPointer(),
                                           "VolumeRenderingParameters");
#endif

  vtkNew<vtkMRMLNCIRayCastVolumeRenderingDisplayNode> nciNode;
  this->GetMRMLScene()->RegisterNodeClass( nciNode.GetPointer() );

  vtkNew<vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode> nciMVNode;
  this->GetMRMLScene()->RegisterNodeClass( nciMVNode.GetPointer() );

  vtkNew<vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode> tmNode;
  this->GetMRMLScene()->RegisterNodeClass( tmNode.GetPointer() );

  vtkNew<vtkMRMLGPURayCastVolumeRenderingDisplayNode> gpuNode;
  this->GetMRMLScene()->RegisterNodeClass( gpuNode.GetPointer() );
}
//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::RegisterRenderingMethod(const char* methodName, const char* displayNodeClassName)
{
  this->RenderingMethods[methodName] = displayNodeClassName;
  this->Modified();
}

//----------------------------------------------------------------------------
std::map<std::string, std::string> vtkSlicerVolumeRenderingLogic
::GetRenderingMethods()
{
  return this->RenderingMethods;
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::AddVolumeRenderingDisplayNode(vtkMRMLVolumeRenderingDisplayNode* node)
{
  DisplayNodesType::iterator it =
    std::find(this->DisplayNodes.begin(), this->DisplayNodes.end(), node);
  if (it != this->DisplayNodes.end())
    {
    // already added
    return;
    }
  // push empty...
  it = this->DisplayNodes.insert(this->DisplayNodes.end(), static_cast<vtkMRMLNode*>(0));
  // .. then set and observe
  vtkSetAndObserveMRMLNodeMacro(*it, node);
  this->UpdateVolumeRenderingDisplayNode(node);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::RemoveVolumeRenderingDisplayNode(vtkMRMLVolumeRenderingDisplayNode* node)
{
  DisplayNodesType::iterator it =
    std::find(this->DisplayNodes.begin(), this->DisplayNodes.end(), node);
  if (it == this->DisplayNodes.end())
    {
    return;
    }
  // unobserve
  vtkSetAndObserveMRMLNodeMacro(*it, 0);
  this->DisplayNodes.erase(it);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::RemoveAllVolumeRenderingDisplayNodes()
{
  for (; this->DisplayNodes.size();)
    {
    this->RemoveVolumeRenderingDisplayNode(
      vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(this->DisplayNodes[0]));
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::AddAllVolumeRenderingDisplayNodes()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  std::vector<vtkMRMLNode*> volumeRenderingDisplayNodes;
  this->GetMRMLScene()->GetNodesByClass(
    "vtkMRMLVolumeRenderingDisplayNode", volumeRenderingDisplayNodes);
  std::vector<vtkMRMLNode*>::const_iterator it;
  for (it = volumeRenderingDisplayNodes.begin();
       it != volumeRenderingDisplayNodes.end(); ++it)
    {
    this->AddVolumeRenderingDisplayNode(
      vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(*it));
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::UpdateVolumeRenderingDisplayNode(vtkMRMLVolumeRenderingDisplayNode* node)
{
  assert(node);
  if (!node->GetVolumeNode())
    {
    return;
    }
  vtkMRMLVolumeDisplayNode* displayNode =
    vtkMRMLVolumeDisplayNode::SafeDownCast(
      node->GetVolumeNode()->GetDisplayNode());
  if (!displayNode)
    {
    return;
    }
  if (node->GetFollowVolumeDisplayNode())
    {
    // observe display node if not already observing it
    if (!this->GetMRMLNodesObserverManager()->GetObservationsCount(displayNode))
      {
      vtkObserveMRMLNodeMacro(displayNode);
      }
    this->CopyDisplayToVolumeRenderingDisplayNode(node);
    }
  else
    {
    // unobserve display node
    vtkUnObserveMRMLNodeMacro(displayNode);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::SetMRMLSceneInternal(vtkMRMLScene* scene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(scene, sceneEvents.GetPointer());

  this->RemoveAllVolumeRenderingDisplayNodes();
  this->AddAllVolumeRenderingDisplayNodes();
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  vtkMRMLVolumeRenderingDisplayNode* vrDisplayNode =
    vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);
  if (vrDisplayNode)
    {
    this->AddVolumeRenderingDisplayNode(vrDisplayNode);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkMRMLVolumeRenderingDisplayNode* vrDisplayNode =
    vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);
  if (vrDisplayNode)
    {
    this->RemoveVolumeRenderingDisplayNode(vrDisplayNode);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::OnMRMLNodeModified(vtkMRMLNode* node)
{
  vtkMRMLVolumeRenderingDisplayNode* vrDisplayNode =
    vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);
  if (vrDisplayNode)
    {
    this->UpdateVolumeRenderingDisplayNode(vrDisplayNode);
    }
  vtkMRMLVolumeDisplayNode* volumeDisplayNode =
    vtkMRMLVolumeDisplayNode::SafeDownCast(node);
  if (volumeDisplayNode)
    {
    for (unsigned int i = 0; i < this->DisplayNodes.size(); ++i)
      {
      vrDisplayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
        this->DisplayNodes[i]);
      if (vrDisplayNode->GetVolumeNode()->GetDisplayNode() ==
          volumeDisplayNode &&
          vrDisplayNode->GetFollowVolumeDisplayNode())
        {
        this->CopyDisplayToVolumeRenderingDisplayNode(
          vrDisplayNode, volumeDisplayNode);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::UpdateTranferFunctionRangeFromImage(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  std::cout << "vtkSlicerVolumeRenderingLogic::UpdateTranferFunctionRangeFromImage()" << std::endl;
  if (vspNode == 0 || vspNode->GetVolumeNode() == 0 || vspNode->GetVolumePropertyNode() == 0)
  {
    return;
  }
  vtkImageData *input = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData();
  vtkVolumeProperty *prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();
  if (input == NULL || prop == NULL)
    {
    return;
    }

  //update scalar range
  vtkColorTransferFunction *functionColor = prop->GetRGBTransferFunction();

  vtkDataArray* scalars = input->GetPointData()->GetScalars();
  if (!scalars)
    {
    return;
    }

  double rangeNew[2];
  scalars->GetRange(rangeNew);
  functionColor->AdjustRange(rangeNew);
  std::cout << "Color range: "
            << functionColor->GetRange()[0] << " " << functionColor->GetRange()[1]
            << std::endl;

  vtkPiecewiseFunction *functionOpacity = prop->GetScalarOpacity();
  functionOpacity->AdjustRange(rangeNew);

  std::cout << "Opacity range: "
            << functionOpacity->GetRange()[0] << " " << functionOpacity->GetRange()[1]
            << std::endl;

  rangeNew[1] = (rangeNew[1] - rangeNew[0])*0.25;
  rangeNew[0] = 0;

  functionOpacity = prop->GetGradientOpacity();
  functionOpacity->RemovePoint(255);//Remove the standard value
  functionOpacity->AdjustRange(rangeNew);
  std::cout << "Gradient Opacity range: "
            << functionOpacity->GetRange()[0] << " " << functionOpacity->GetRange()[1]
            << std::endl;
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::SetThresholdToVolumeProp(double scalarRange[2],
                           double threshold[2],
                           vtkVolumeProperty* volumeProp,
                           bool linearRamp,
                           bool stayUpAtUpperLimit)
{
  assert(scalarRange && threshold && volumeProp);
  // Sanity check
  threshold[0] = std::max(std::min(threshold[0], scalarRange[1]), scalarRange[0]);
  threshold[1] = std::min(std::max(threshold[1], scalarRange[0]), scalarRange[1]);
  std::cout << "Threshold: " << threshold[0] << " " << threshold[1] << std::endl;

  double previous = VTK_DOUBLE_MIN;

  vtkNew<vtkPiecewiseFunction> opacity;
  // opacity doesn't support duplicate points
  opacity->AddPoint(higherAndUnique(scalarRange[0], previous), 0.0);
  opacity->AddPoint(higherAndUnique(threshold[0], previous), 0.0);
  if (!linearRamp)
    {
    opacity->AddPoint(higherAndUnique(threshold[0], previous), 1.0);
    }
  opacity->AddPoint(higherAndUnique(threshold[1], previous), 1.0);
  double endValue = stayUpAtUpperLimit ? 1.0 : 0.0;
  if (!stayUpAtUpperLimit)
    {
    opacity->AddPoint(higherAndUnique(threshold[1], previous), endValue);
    }
  opacity->AddPoint(higherAndUnique(scalarRange[1], previous), endValue);

  vtkPiecewiseFunction *volumePropOpacity = volumeProp->GetScalarOpacity();
  if (this->IsDifferentFunction(opacity.GetPointer(), volumePropOpacity))
    {
    volumePropOpacity->DeepCopy(opacity.GetPointer());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::SetWindowLevelToVolumeProp(double scalarRange[2],
                             double windowLevel[2],
                             vtkLookupTable* lut,
                             vtkVolumeProperty* volumeProp)
{
  assert(scalarRange && windowLevel && volumeProp);

  double windowLevelMinMax[2];
  windowLevelMinMax[0] = windowLevel[1] - 0.5 * windowLevel[0];
  windowLevelMinMax[1] = windowLevel[1] + 0.5 * windowLevel[0];

  double previous = VTK_DOUBLE_MIN;

  vtkNew<vtkColorTransferFunction> colorTransfer;

  const int size = lut ? lut->GetNumberOfTableValues() : 0;
  if (size == 0)
    {
    const double black[3] = {0., 0., 0.};
    const double white[3] = {1., 1., 1.};
    colorTransfer->AddRGBPoint(scalarRange[0], black[0], black[1], black[2]);
    colorTransfer->AddRGBPoint(windowLevelMinMax[0], black[0], black[1], black[2]);
    colorTransfer->AddRGBPoint(windowLevelMinMax[1], white[0], white[1], white[2]);
    colorTransfer->AddRGBPoint(scalarRange[1], white[0], white[1], white[2]);
    }
  else if (size == 1)
    {
    double color[4];
    lut->GetTableValue(0, color);

    colorTransfer->AddRGBPoint(higherAndUnique(scalarRange[0], previous),
                               color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(higherAndUnique(windowLevelMinMax[0], previous),
                               color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(higherAndUnique(windowLevelMinMax[1], previous),
                               color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(higherAndUnique(scalarRange[1], previous),
                               color[0], color[1], color[2]);
    }
  else // if (size > 1)
    {
    previous = VTK_DOUBLE_MIN;

    double color[4];
    lut->GetTableValue(0, color);
    colorTransfer->AddRGBPoint(higherAndUnique(scalarRange[0], previous),
                               color[0], color[1], color[2]);

    double value = windowLevelMinMax[0];

    double step = windowLevel[0] / (size - 1);

    int downSamplingFactor = 64;
    for (int i = 0; i < size; i += downSamplingFactor,
                              value += downSamplingFactor*step)
      {
      lut->GetTableValue(i, color);
      colorTransfer->AddRGBPoint(higherAndUnique(value, previous),
                                 color[0], color[1], color[2]);
      }

    lut->GetTableValue(size - 1, color);
    colorTransfer->AddRGBPoint(higherAndUnique(windowLevelMinMax[1], previous),
                               color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(higherAndUnique(scalarRange[1], previous),
                               color[0], color[1], color[2]);
    }

  vtkColorTransferFunction *volumePropColorTransfer = volumeProp->GetRGBTransferFunction();
  if (this->IsDifferentFunction(colorTransfer.GetPointer(), volumePropColorTransfer))
    {
    volumePropColorTransfer->DeepCopy(colorTransfer.GetPointer());
    }

  volumeProp->SetInterpolationTypeToLinear();
  volumeProp->ShadeOn();
  volumeProp->SetAmbient(0.30);
  volumeProp->SetDiffuse(0.60);
  volumeProp->SetSpecular(0.50);
  volumeProp->SetSpecularPower(40);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::SetGradientOpacityToVolumeProp(double scalarRange[2],
                                 vtkVolumeProperty* volumeProp)
{
  assert(scalarRange && volumeProp);

  double gradientRange[2];
  gradientRange[0] = 0.;
  gradientRange[1] = (scalarRange[1] - scalarRange[0]); // *0.25;

  double previous = VTK_DOUBLE_MIN;
  vtkNew<vtkPiecewiseFunction> opacity;
  // opacity doesn't support duplicate points
  opacity->AddPoint(higherAndUnique(scalarRange[0], previous), 1.0);
  opacity->AddPoint(higherAndUnique(scalarRange[1], previous), 1.0);

  vtkPiecewiseFunction *volumePropGradientOpacity = volumeProp->GetGradientOpacity();
  if (this->IsDifferentFunction(opacity.GetPointer(), volumePropGradientOpacity))
    {
    volumePropGradientOpacity->DeepCopy(opacity.GetPointer());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::SetLabelMapToVolumeProp(vtkScalarsToColors* colors,
                          vtkVolumeProperty* volumeProp)
{
  assert(colors && volumeProp);

  vtkNew<vtkPiecewiseFunction> opacity;
  vtkNew<vtkColorTransferFunction> colorTransfer;

  vtkLookupTable* lut = vtkLookupTable::SafeDownCast(colors);
  const int colorCount = colors->GetNumberOfAvailableColors();
  double value = colors->GetRange()[0];
  double step = (colors->GetRange()[1] - colors->GetRange()[0] + 1.) / colorCount;
  double color[4] = {0., 0., 0., 1.};
  for (int i = 0; i < colorCount; ++i, value += step)
    {
    // Short circuit for luts as it is faster
    if (lut)
      {
      lut->GetTableValue(i, color);
      }
    else
      {
      colors->GetColor(value, color);
      }
    opacity->AddPoint(value, color[3]);
    colorTransfer->AddRGBPoint(value, color[0], color[1], color[2]);
    }

  vtkPiecewiseFunction *volumePropOpacity = volumeProp->GetScalarOpacity();
  if (this->IsDifferentFunction(opacity.GetPointer(), volumePropOpacity))
    {
    volumePropOpacity->DeepCopy(opacity.GetPointer());
    }

  vtkColorTransferFunction *volumePropColorTransfer = volumeProp->GetRGBTransferFunction();
  if (this->IsDifferentFunction(colorTransfer.GetPointer(), volumePropColorTransfer))
    {
    volumePropColorTransfer->DeepCopy(colorTransfer.GetPointer());
    }

  volumeProp->SetInterpolationTypeToNearest();
  volumeProp->ShadeOn();
  volumeProp->SetAmbient(0.30);
  volumeProp->SetDiffuse(0.60);
  volumeProp->SetSpecular(0.50);
  volumeProp->SetSpecularPower(40);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::CopyDisplayToVolumeRenderingDisplayNode(
  vtkMRMLVolumeRenderingDisplayNode* vspNode,
  vtkMRMLVolumeDisplayNode* displayNode)
{
  assert(vspNode);
  if (!displayNode)
    {
    vtkMRMLVolumeNode* volumeNode = vspNode->GetVolumeNode();
    if (!volumeNode)
      {
      vtkWarningMacro("Volume Rendering display node does not reference a "
                      "volume node.");
      return;
      }
    displayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(
      volumeNode->GetDisplayNode());
    }
  if (!displayNode)
    {
    vtkWarningMacro("No display node to copy");
    return;
    }
  if (vtkMRMLScalarVolumeDisplayNode::SafeDownCast(displayNode))
    {
    this->CopyScalarDisplayToVolumeRenderingDisplayNode(vspNode,
      vtkMRMLScalarVolumeDisplayNode::SafeDownCast(displayNode));
    }
  else if (vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(displayNode))
    {
    this->CopyLabelMapDisplayToVolumeRenderingDisplayNode(vspNode,
      vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(displayNode));
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::CopyScalarDisplayToVolumeRenderingDisplayNode(
  vtkMRMLVolumeRenderingDisplayNode* vspNode,
  vtkMRMLScalarVolumeDisplayNode* vpNode)
{
  assert(vspNode);
  assert(vspNode->GetVolumePropertyNode());

  if (!vpNode)
    {
    vpNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      vspNode->GetVolumeNode()->GetDisplayNode());
    }
  assert(vpNode);

  bool ignoreVolumeDisplayNodeThreshold =
    vspNode->GetIgnoreVolumeDisplayNodeThreshold();
  double scalarRange[2];
  vpNode->GetDisplayScalarRange(scalarRange);

  double windowLevel[2];
  windowLevel[0] = vpNode->GetWindow();
  windowLevel[1] = vpNode->GetLevel();

  double threshold[2];
  if (!ignoreVolumeDisplayNodeThreshold)
    {
    threshold[0] = vpNode->GetLowerThreshold();
    threshold[1] = vpNode->GetUpperThreshold();
    }
  else
    {
    threshold[0] = vpNode->GetWindowLevelMin();
    threshold[1] = vpNode->GetWindowLevelMax();
    }

  vtkLookupTable* lut = vpNode->GetColorNode() ?
    vpNode->GetColorNode()->GetLookupTable() : 0;
  vtkVolumeProperty *prop =
    vspNode->GetVolumePropertyNode()->GetVolumeProperty();

  int disabledModify = vspNode->StartModify();
  int vpNodeDisabledModify = vspNode->GetVolumePropertyNode()->StartModify();

  this->SetThresholdToVolumeProp(
    scalarRange, threshold, prop,
    this->UseLinearRamp, ignoreVolumeDisplayNodeThreshold);
  if (vtkMRMLNCIRayCastVolumeRenderingDisplayNode::SafeDownCast(vspNode))
    {
    // NCI raycast mapper applies a second threshold in addition to the opacity
    // transfer function
    vtkMRMLNCIRayCastVolumeRenderingDisplayNode::SafeDownCast(vspNode)
      ->SetDepthPeelingThreshold(scalarRange[0]);
    }
  this->SetWindowLevelToVolumeProp(
    scalarRange, windowLevel, lut, prop);
  this->SetGradientOpacityToVolumeProp(scalarRange, prop);

  vspNode->GetVolumePropertyNode()->EndModify(vpNodeDisabledModify);
  vspNode->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::CopyLabelMapDisplayToVolumeRenderingDisplayNode(
  vtkMRMLVolumeRenderingDisplayNode* vspNode,
  vtkMRMLLabelMapVolumeDisplayNode* vpNode)
{
  assert(vspNode);
  assert(vspNode->GetVolumePropertyNode());

  if (!vpNode)
    {
    vpNode = vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(
      vspNode->GetVolumeNode()->GetDisplayNode());
    }
  assert(vpNode);

  vtkScalarsToColors* colors = vpNode->GetColorNode() ?
    vpNode->GetColorNode()->GetScalarsToColors() : 0;

  vtkVolumeProperty *prop =
    vspNode->GetVolumePropertyNode()->GetVolumeProperty();

  int disabledModify = vspNode->StartModify();
  int vpNodeDisabledModify = vspNode->GetVolumePropertyNode()->StartModify();

  this->SetLabelMapToVolumeProp(colors, prop);
  this->SetGradientOpacityToVolumeProp(colors->GetRange(), prop);

  vspNode->GetVolumePropertyNode()->EndModify(vpNodeDisabledModify);
  vspNode->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::FitROIToVolume(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  // resize the ROI to fit the volume
  vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::SafeDownCast(vspNode->GetROINode());
  vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode());


  if (volumeNode && roiNode)
  {
    int disabledModify = roiNode->StartModify();

    double xyz[3];
    double center[3];

    vtkMRMLSliceLogic::GetVolumeRASBox(volumeNode, xyz,  center);
    for (int i = 0; i < 3; i++)
    {
      xyz[i] *= 0.5;
    }

    roiNode->SetXYZ(center);
    roiNode->SetRadiusXYZ(xyz);

    roiNode->EndModify(disabledModify);
  }
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic
::CreateVolumeRenderingDisplayNode(const char* renderingClassName)
{
  vtkMRMLVolumeRenderingDisplayNode *node = NULL;

  if (this->GetMRMLScene() == 0)
    {
    return node;
    }
  bool volumeRenderingUniqueName = true;
  if (renderingClassName == 0 || strlen(renderingClassName)==0)
    {
    renderingClassName = this->DefaultRenderingMethod;
    }
  else
    {
    volumeRenderingUniqueName = false;
    }
  if (renderingClassName == 0 || strlen(renderingClassName)==0)
    {
    renderingClassName = "vtkMRMLCPURayCastVolumeRenderingDisplayNode";
    }
  node = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
    this->GetMRMLScene()->CreateNodeByClass(renderingClassName));
  if (volumeRenderingUniqueName)
    {
    node->SetName(this->GetMRMLScene()->GenerateUniqueName("VolumeRendering").c_str());
    }

  return node;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingScenarioNode* vtkSlicerVolumeRenderingLogic::CreateScenarioNode()
{
  vtkMRMLVolumeRenderingScenarioNode *node = NULL;

  if (this->GetMRMLScene())
    {
    node = vtkMRMLVolumeRenderingScenarioNode::New();
    }

  return node;
}

// Description:
// Remove ViewNode from VolumeRenderingDisplayNode for a VolumeNode,
//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::RemoveViewFromVolumeDisplayNodes(
                                          vtkMRMLVolumeNode *volumeNode,
                                          vtkMRMLViewNode *viewNode)
{
  if (viewNode == NULL || volumeNode == NULL)
    {
    return;
    }

  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode)
      {
      dnode->RemoveViewNodeID(viewNode->GetID());
      }
    }
}

// Description:
// Find volume rendering display node reference in the volume
//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetVolumeRenderingDisplayNodeByID(
                                                              vtkMRMLVolumeNode *volumeNode,
                                                              char *displayNodeID)
{
  if (displayNodeID == NULL || volumeNode == NULL)
    {
    return NULL;
    }

  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode && !strcmp(displayNodeID, dnode->GetID()))
      {
      return dnode;
      }
    }
  return NULL;
}

// Description:
// Find first volume rendering display node
//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetFirstVolumeRenderingDisplayNode(
                                                              vtkMRMLVolumeNode *volumeNode)
{
  if (volumeNode == NULL)
    {
    return NULL;
    }
  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode)
      {
      return dnode;
      }
    }
  return NULL;
}

// Description:
// Find volume rendering display node referencing the view node and volume node
//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetVolumeRenderingDisplayNodeForViewNode(
                                                              vtkMRMLVolumeNode *volumeNode,
                                                              vtkMRMLViewNode *viewNode)
{
  if (viewNode == NULL || volumeNode == NULL)
    {
    return NULL;
    }
  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));

    if (dnode // display node is not necessarily volume rendering display node.
        && dnode->IsDisplayableInView(viewNode->GetID()))
      {
      return dnode;
      }
    }
  return NULL;
}

// Description:
// Find volume rendering display node referencing the view node in the scene
//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetVolumeRenderingDisplayNodeForViewNode(
                                                              vtkMRMLViewNode *viewNode)
{
  if (viewNode == NULL || viewNode->GetScene() == NULL)
    {
    return NULL;
    }
  std::vector<vtkMRMLNode *> nodes;
  viewNode->GetScene()->GetNodesByClass("vtkMRMLVolumeRenderingDisplayNode", nodes);

  for (unsigned int i=0; i<nodes.size(); i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      nodes[i]);
    if (dnode && dnode->IsViewNodeIDPresent(viewNode->GetID()))
      {
      return dnode;
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic
::GetFirstVolumeRenderingDisplayNodeByROINode(vtkMRMLAnnotationROINode* roiNode)
{
  if (roiNode == NULL || roiNode->GetScene() == NULL)
    {
    return NULL;
    }
  std::vector<vtkMRMLNode *> nodes;
  roiNode->GetScene()->GetNodesByClass("vtkMRMLVolumeRenderingDisplayNode", nodes);

  for (unsigned int i = 0; i < nodes.size(); ++i)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode =
      vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(nodes[i]);
    if (dnode && dnode->GetROINodeID() &&
        !strcmp(dnode->GetROINodeID(), roiNode->GetID()))
      {
      return dnode;
      }
    }
  return NULL;
}

// Description:
// Update vtkMRMLVolumeRenderingDisplayNode from VolumeNode,
// if needed create vtkMRMLVolumePropertyNode and vtkMRMLAnnotationROINode
// and initioalize them from VolumeNode
//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::UpdateDisplayNodeFromVolumeNode(
                                          vtkMRMLVolumeRenderingDisplayNode *displayNode,
                                          vtkMRMLVolumeNode *volumeNode,
                                          vtkMRMLVolumePropertyNode **propNode,
                                          vtkMRMLAnnotationROINode **roiNode)
{

  if (volumeNode == NULL)
    {
    displayNode->SetAndObserveVolumeNodeID(NULL);
    return;
    }

  displayNode->SetAndObserveVolumeNodeID(volumeNode->GetID());

  if (*propNode == NULL)
    {
    *propNode = vtkMRMLVolumePropertyNode::New();
    this->GetMRMLScene()->AddNode(*propNode);
    (*propNode)->Delete();
    }
  displayNode->SetAndObserveVolumePropertyNodeID((*propNode)->GetID());

  if (*roiNode == NULL)
    {
    *roiNode = vtkMRMLAnnotationROINode::New();
    // By default, the ROI is interactive. It could be an application setting.
    (*roiNode)->SetInteractiveMode(1);
    (*roiNode)->Initialize(this->GetMRMLScene());
    // by default, show the ROI only if cropping is enabled
    (*roiNode)->SetDisplayVisibility(displayNode->GetCroppingEnabled());
    (*roiNode)->Delete();
    }
  displayNode->SetAndObserveROINodeID((*roiNode)->GetID());

  //this->UpdateVolumePropertyFromImageData(displayNode);
  this->CopyDisplayToVolumeRenderingDisplayNode(displayNode);

  this->FitROIToVolume(displayNode);
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkSlicerVolumeRenderingLogic
::AddVolumePropertyFromFile (const char* filename)
{
  vtkMRMLVolumePropertyNode *vpNode = vtkMRMLVolumePropertyNode::New();
  vtkMRMLVolumePropertyStorageNode *vpStorageNode = vtkMRMLVolumePropertyStorageNode::New();

  // check for local or remote files
  int useURI = 0; // false;
  if (this->GetMRMLScene()->GetCacheManager() != NULL)
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    }

  itksys_stl::string name;
  const char *localFile;
  if (useURI)
    {
    vpStorageNode->SetURI(filename);
     // reset filename to the local file name
    localFile = ((this->GetMRMLScene())->GetCacheManager())->GetFilenameFromURI(filename);
    }
  else
    {
    vpStorageNode->SetFileName(filename);
    localFile = filename;
    }
  const itksys_stl::string fname(localFile);
  // the model name is based on the file name (itksys call should work even if
  // file is not on disk yet)
  name = itksys::SystemTools::GetFilenameName(fname);

  // check to see which node can read this type of file
  if (!vpStorageNode->SupportedFileType(name.c_str()))
    {
    vpStorageNode->Delete();
    vpStorageNode = NULL;
    }

  /* don't read just yet, need to add to the scene first for remote reading
  if (vpStorageNode->ReadData(vpNode) != 0)
    {
    storageNode = vpStorageNode;
    }
  */
  if (vpStorageNode != NULL)
    {
    std::string uname( this->GetMRMLScene()->GetUniqueNameByString(name.c_str()));

    vpNode->SetName(uname.c_str());

    this->GetMRMLScene()->SaveStateForUndo();

    vpNode->SetScene(this->GetMRMLScene());
    vpStorageNode->SetScene(this->GetMRMLScene());

    this->GetMRMLScene()->AddNode(vpStorageNode);
    vpNode->SetAndObserveStorageNodeID(vpStorageNode->GetID());

    this->GetMRMLScene()->AddNode(vpNode);

    // the scene points to it still
    vpNode->Delete();

    // now set up the reading
    int retval = vpStorageNode->ReadData(vpNode);
    if (retval != 1)
      {
      vtkErrorMacro("AddVolumePropertyFromFile: error reading " << filename);
      this->GetMRMLScene()->RemoveNode(vpNode);
      this->GetMRMLScene()->RemoveNode(vpStorageNode);
      vpNode = NULL;
      }
    }
  else
    {
    vtkDebugMacro("Couldn't read file, returning null model node: " << filename);
    vpNode->Delete();
    vpNode = NULL;
    }
  if (vpStorageNode)
    {
    vpStorageNode->Delete();
    }
  return vpNode;
}

//---------------------------------------------------------------------------
vtkMRMLScene* vtkSlicerVolumeRenderingLogic::GetPresetsScene()
{
  if (!this->PresetsScene)
    {
    this->PresetsScene = vtkMRMLScene::New();
    this->LoadPresets(this->PresetsScene);
    }
  return this->PresetsScene;
}

//---------------------------------------------------------------------------
bool vtkSlicerVolumeRenderingLogic::LoadPresets(vtkMRMLScene* scene)
{
  this->PresetsScene->RegisterNodeClass(vtkNew<vtkMRMLVolumePropertyNode>().GetPointer());

  if (this->GetModuleShareDirectory().empty())
    {
    vtkErrorMacro(<< "Failed to load presets: Share directory *NOT* set !");
    return false;
    }

  std::string presetFileName = this->GetModuleShareDirectory() + "/presets.xml";
  scene->SetURL(presetFileName.c_str());
  int connected = scene->Connect();
  if (connected != 1)
    {
    vtkErrorMacro(<< "Failed to load presets [" << presetFileName << "]");
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerVolumeRenderingLogic::IsDifferentFunction(
  vtkPiecewiseFunction* function1, vtkPiecewiseFunction* function2)const
{
  if ((function1 != 0) ^ (function2 != 0))
    {
    return true;
    }
  if (function1->GetSize() != function2->GetSize())
    {
    return true;
    }
  bool different = false;
  for (int i = 0; i < function1->GetSize(); ++i)
    {
    double node1[4];
    function1->GetNodeValue(i, node1);
    double node2[4];
    function2->GetNodeValue(i, node2);
    for (unsigned int j = 0; j < 4; ++j)
      {
      if (node1[j] != node2[j])
        {
        different = true;
        break;
        }
      }
    if (different)
      {
      break;
      }
    }
  return different;
}


//---------------------------------------------------------------------------
bool vtkSlicerVolumeRenderingLogic::IsDifferentFunction(
  vtkColorTransferFunction* function1, vtkColorTransferFunction* function2)const
{
  if ((function1 != 0) ^ (function2 != 0))
    {
    return true;
    }
  if (function1->GetSize() != function2->GetSize())
    {
    return true;
    }
  bool different = false;
  for (int i = 0; i < function1->GetSize(); ++i)
    {
    double node1[6];
    function1->GetNodeValue(i, node1);
    double node2[6];
    function2->GetNodeValue(i, node2);
    for (unsigned int j = 0; j < 6; ++j)
      {
      if (node1[j] != node2[j])
        {
        different = true;
        break;
        }
      }
    if (different)
      {
      break;
      }
    }
  return different;
}
