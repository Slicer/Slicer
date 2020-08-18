/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumeRenderingLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkSlicerConfigure.h" // Slicer_VTK_RENDERING_USE_{OpenGL|OpenGL2}_BACKEND

// Volume Rendering includes
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkSlicerVolumeRenderingLogic.h"
#include "vtkMRMLCPURayCastVolumeRenderingDisplayNode.h"
#include "vtkMRMLGPURayCastVolumeRenderingDisplayNode.h"
#include "vtkMRMLMultiVolumeRenderingDisplayNode.h"

// Annotations includes
#include <vtkMRMLAnnotationROINode.h>

// MRML includes
#include <vtkCacheManager.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVolumePropertyNode.h>
#include <vtkMRMLVolumePropertyStorageNode.h>
#include <vtkMRMLShaderPropertyNode.h>
#include <vtkMRMLShaderPropertyStorageNode.h>

// VTKSYS includes
#include <itksys/SystemTools.hxx>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkVolumeProperty.h>

#if defined(Slicer_VTK_RENDERING_USE_OpenGL_BACKEND)
#include <vtkOpenGLExtensionManager.h>
#include <vtkgl.h>
#endif

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerVolumeRenderingLogic);

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic::vtkSlicerVolumeRenderingLogic()
{
  this->DefaultRenderingMethod = nullptr;
  this->UseLinearRamp = true;
  this->PresetsScene = nullptr;

  this->RegisterRenderingMethod("VTK CPU Ray Casting",
                                "vtkMRMLCPURayCastVolumeRenderingDisplayNode");
  this->RegisterRenderingMethod("VTK GPU Ray Casting",
                                "vtkMRMLGPURayCastVolumeRenderingDisplayNode");
  this->RegisterRenderingMethod("VTK Multi-Volume (experimental)",
                                "vtkMRMLMultiVolumeRenderingDisplayNode");
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
#if defined(Slicer_VTK_RENDERING_USE_OpenGL_BACKEND)
  const char *gl_vendor=reinterpret_cast<const char *>(glGetString(GL_VENDOR));
  os << indent << "Vendor: " << gl_vendor << std::endl;
  const char *gl_version=reinterpret_cast<const char *>(glGetString(GL_VERSION));
  os << indent << "Version: " << gl_version << std::endl;
  const char *glsl_version=
    reinterpret_cast<const char *>(glGetString(vtkgl::SHADING_LANGUAGE_VERSION));
  os << indent << "Shading Language Version: " << glsl_version << std::endl;
#endif
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::RegisterNodes()
{
  if(!this->GetMRMLScene())
    {
    vtkWarningMacro("RegisterNodes: No MRML scene.");
    return;
    }

  vtkNew<vtkMRMLVolumePropertyNode> vpn;
  this->GetMRMLScene()->RegisterNodeClass( vpn.GetPointer() );

  vtkNew<vtkMRMLVolumePropertyStorageNode> vpsn;
  this->GetMRMLScene()->RegisterNodeClass( vpsn.GetPointer() );

  vtkNew<vtkMRMLShaderPropertyNode> spn;
  this->GetMRMLScene()->RegisterNodeClass( spn.GetPointer() );

  vtkNew<vtkMRMLShaderPropertyStorageNode> spsn;
  this->GetMRMLScene()->RegisterNodeClass( spsn.GetPointer() );

  vtkNew<vtkMRMLCPURayCastVolumeRenderingDisplayNode> cpuVRNode;
  this->GetMRMLScene()->RegisterNodeClass( cpuVRNode.GetPointer() );
  // Volume rendering nodes used to have the tag "VolumeRenderingParameters"
  // in scenes prior to Slicer 4.2
#if MRML_SUPPORT_VERSION < 0x040200
  this->GetMRMLScene()->RegisterNodeClass( cpuVRNode.GetPointer(), "VolumeRenderingParameters");
#endif

  vtkNew<vtkMRMLGPURayCastVolumeRenderingDisplayNode> gpuNode;
  this->GetMRMLScene()->RegisterNodeClass( gpuNode.GetPointer() );

  vtkNew<vtkMRMLMultiVolumeRenderingDisplayNode> multiNode;
  this->GetMRMLScene()->RegisterNodeClass( multiNode.GetPointer() );
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::RegisterRenderingMethod(const char* methodName, const char* displayNodeClassName)
{
  this->RenderingMethods[methodName] = displayNodeClassName;
  this->Modified();
}

//----------------------------------------------------------------------------
std::map<std::string, std::string> vtkSlicerVolumeRenderingLogic::GetRenderingMethods()
{
  return this->RenderingMethods;
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::AddVolumeRenderingDisplayNode(vtkMRMLVolumeRenderingDisplayNode* node)
{
  DisplayNodesType::iterator it = std::find(this->DisplayNodes.begin(), this->DisplayNodes.end(), node);
  if (it != this->DisplayNodes.end())
    {
    // already added
    return;
    }
  // push empty...
  it = this->DisplayNodes.insert(this->DisplayNodes.end(), static_cast<vtkMRMLNode*>(nullptr));
  // .. then set and observe
  vtkSetAndObserveMRMLNodeMacro(*it, node);

  //Don't update volume rendering while a scene is being imported
  if (!this->GetMRMLScene()->IsImporting())
    {
    this->UpdateVolumeRenderingDisplayNode(node);
    }

}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::RemoveVolumeRenderingDisplayNode(vtkMRMLVolumeRenderingDisplayNode* node)
{
  DisplayNodesType::iterator it = std::find(this->DisplayNodes.begin(), this->DisplayNodes.end(), node);
  if (it == this->DisplayNodes.end())
    {
    return;
    }
  // unobserve
  vtkSetAndObserveMRMLNodeMacro(*it, 0);
  this->DisplayNodes.erase(it);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::RemoveAllVolumeRenderingDisplayNodes()
{
  while (!this->DisplayNodes.empty())
    {
    this->RemoveVolumeRenderingDisplayNode(vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(this->DisplayNodes[0]));
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::AddAllVolumeRenderingDisplayNodes()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  std::vector<vtkMRMLNode*> volumeRenderingDisplayNodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLVolumeRenderingDisplayNode", volumeRenderingDisplayNodes);
  std::vector<vtkMRMLNode*>::const_iterator it;
  for (it = volumeRenderingDisplayNodes.begin(); it != volumeRenderingDisplayNodes.end(); ++it)
    {
    this->AddVolumeRenderingDisplayNode(vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(*it));
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::ChangeVolumeRenderingMethod(const char* displayNodeClassName/*=0*/)
{
  if (this->DisplayNodes.empty())
    {
    // There are no display nodes, nothing to do
    return;
    }
  if (!this->GetMRMLScene())
    {
    return;
    }
  if (displayNodeClassName == nullptr || strlen(displayNodeClassName) == 0)
    {
    displayNodeClassName = this->DefaultRenderingMethod;
    }
  else
    {
    // In case of a non-empty class name set the default rendering method
    this->SetDefaultRenderingMethod(displayNodeClassName);
    }
  if (displayNodeClassName == nullptr || strlen(displayNodeClassName) == 0)
    {
    displayNodeClassName = "vtkMRMLCPURayCastVolumeRenderingDisplayNode";
    }
  if (!strcmp(this->DisplayNodes[0]->GetClassName(), displayNodeClassName))
    {
    // Type of existing display nodes match the requested type, nothing to do
    return;
    }

  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  // Create a display node of the requested type for all existing display nodes
  DisplayNodesType displayNodesCopy(this->DisplayNodes);
  DisplayNodesType::iterator displayIt;
  for (displayIt = displayNodesCopy.begin(); displayIt != displayNodesCopy.end(); ++displayIt)
    {
    vtkMRMLVolumeRenderingDisplayNode* oldDisplayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(*displayIt);
    if (!oldDisplayNode)
      {
      // node may have been deleted
      continue;
      }
    vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode> newDisplayNode =
      vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode>::Take(this->CreateVolumeRenderingDisplayNode(displayNodeClassName));
    if (!newDisplayNode)
      {
      vtkErrorMacro("ChangeVolumeRenderingMethod: Failed to create display node of type " << displayNodeClassName);
      continue;
      }
    this->GetMRMLScene()->AddNode(newDisplayNode);
    newDisplayNode->vtkMRMLVolumeRenderingDisplayNode::Copy(oldDisplayNode);
    vtkMRMLDisplayableNode* displayableNode = oldDisplayNode->GetDisplayableNode();
    this->GetMRMLScene()->RemoveNode(oldDisplayNode);
    // Assign updated display node to displayable node.
    // There may be orphan volume rendering display nodes in the scene (without being assigned to a displayable node),
    // but we leave them alone, as maybe they are just temporarily not used.
    if (displayableNode)
      {
      displayableNode->AddAndObserveDisplayNodeID(newDisplayNode->GetID());
      }
    }

  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::UpdateVolumeRenderingDisplayNode(vtkMRMLVolumeRenderingDisplayNode* node)
{
  if (!node)
    {
    vtkWarningMacro("UpdateVolumeRenderingDisplayNode: Volume Rendering display node does not exist.");
    return;
    }
  if (!node->GetVolumeNode())
    {
    return;
    }
  vtkMRMLVolumeDisplayNode* displayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(
    node->GetVolumeNode()->GetDisplayNode() );
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
void vtkSlicerVolumeRenderingLogic::SetMRMLSceneInternal(vtkMRMLScene* scene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(scene, sceneEvents.GetPointer());
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::ObserveMRMLScene()
{
  this->RemoveAllVolumeRenderingDisplayNodes();
  this->AddAllVolumeRenderingDisplayNodes();
  this->Superclass::ObserveMRMLScene();
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  vtkMRMLVolumeRenderingDisplayNode* vrDisplayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);
  if (vrDisplayNode)
    {
    this->AddVolumeRenderingDisplayNode(vrDisplayNode);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkMRMLVolumeRenderingDisplayNode* vrDisplayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);
  if (vrDisplayNode)
    {
    this->RemoveVolumeRenderingDisplayNode(vrDisplayNode);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::OnMRMLNodeModified(vtkMRMLNode* node)
{
  vtkMRMLVolumeRenderingDisplayNode* vrDisplayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);
  if (vrDisplayNode)
    {
    this->UpdateVolumeRenderingDisplayNode(vrDisplayNode);
    }
  vtkMRMLVolumeDisplayNode* volumeDisplayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(node);
  if (volumeDisplayNode)
    {
    for (unsigned int i = 0; i < this->DisplayNodes.size(); ++i)
      {
      vrDisplayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(this->DisplayNodes[i]);
      if (vrDisplayNode->GetVolumeNode()->GetDisplayNode() == volumeDisplayNode &&
          vrDisplayNode->GetFollowVolumeDisplayNode())
        {
        this->CopyDisplayToVolumeRenderingDisplayNode(vrDisplayNode, volumeDisplayNode);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::UpdateTranferFunctionRangeFromImage(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  vtkDebugMacro("vtkSlicerVolumeRenderingLogic::UpdateTranferFunctionRangeFromImage()");
  if (vspNode == nullptr || vspNode->GetVolumeNode() == nullptr || vspNode->GetVolumePropertyNode() == nullptr)
    {
    return;
    }
  vtkImageData *input = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData();
  vtkVolumeProperty *prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();
  if (input == nullptr || prop == nullptr)
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
  vtkDebugMacro("Color range: "<< functionColor->GetRange()[0] << " " << functionColor->GetRange()[1]);

  vtkPiecewiseFunction *functionOpacity = prop->GetScalarOpacity();
  functionOpacity->AdjustRange(rangeNew);

  vtkDebugMacro("Opacity range: " << functionOpacity->GetRange()[0] << " " << functionOpacity->GetRange()[1]);

  rangeNew[1] = (rangeNew[1] - rangeNew[0])*0.25;
  rangeNew[0] = 0;

  functionOpacity = prop->GetGradientOpacity();
  functionOpacity->RemovePoint(255); //Remove the standard value
  functionOpacity->AdjustRange(rangeNew);
  vtkDebugMacro("Gradient Opacity range: " << functionOpacity->GetRange()[0] << " " << functionOpacity->GetRange()[1]);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::SetThresholdToVolumeProp(
  double scalarRange[2], double threshold[2], vtkVolumeProperty* volumeProp, bool linearRamp, bool stayUpAtUpperLimit)
{

  if (!volumeProp || !scalarRange || !threshold)
    {
    vtkWarningMacro("SetThresholdToVolumeProp: Inputs do not exist.");
    return;
    }

  // Sanity check
  threshold[0] = std::max(std::min(threshold[0], scalarRange[1]), scalarRange[0]);
  threshold[1] = std::min(std::max(threshold[1], scalarRange[0]), scalarRange[1]);
  vtkDebugMacro("Threshold: " << threshold[0] << " " << threshold[1]);

  double previous = VTK_DOUBLE_MIN;

  vtkNew<vtkPiecewiseFunction> opacity;
  // opacity doesn't support duplicate points
  opacity->AddPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(scalarRange[0], previous), 0.0);
  opacity->AddPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(threshold[0], previous), 0.0);
  if (!linearRamp)
    {
    opacity->AddPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(threshold[0], previous), 1.0);
    }
  opacity->AddPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(threshold[1], previous), 1.0);
  double endValue = stayUpAtUpperLimit ? 1.0 : 0.0;
  if (!stayUpAtUpperLimit)
    {
    opacity->AddPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(threshold[1], previous), endValue);
    }
  opacity->AddPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(scalarRange[1], previous), endValue);

  vtkPiecewiseFunction *volumePropOpacity = volumeProp->GetScalarOpacity();
  if (this->IsDifferentFunction(opacity.GetPointer(), volumePropOpacity))
    {
    volumePropOpacity->DeepCopy(opacity.GetPointer());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::SetWindowLevelToVolumeProp(
  double scalarRange[2], double windowLevel[2], vtkLookupTable* lut, vtkVolumeProperty* volumeProp)
{
  if (!volumeProp || !scalarRange || !windowLevel)
    {
    vtkWarningMacro("SetWindowLevelToVolumeProp: Inputs do not exist.");
    return;
    }

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

    colorTransfer->AddRGBPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(scalarRange[0], previous),
                               color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(windowLevelMinMax[0], previous),
                               color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(windowLevelMinMax[1], previous),
                               color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(scalarRange[1], previous),
                               color[0], color[1], color[2]);
    }
  else // if (size > 1)
    {
    previous = VTK_DOUBLE_MIN;

    double color[4];
    lut->GetTableValue(0, color);
    colorTransfer->AddRGBPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(scalarRange[0], previous),
                               color[0], color[1], color[2]);

    double value = windowLevelMinMax[0];

    double step = windowLevel[0] / (size - 1);

    int downSamplingFactor = 64;
    for (int i = 0; i < size; i += downSamplingFactor,
                              value += downSamplingFactor*step)
      {
      lut->GetTableValue(i, color);
      colorTransfer->AddRGBPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(value, previous),
                                 color[0], color[1], color[2]);
      }

    lut->GetTableValue(size - 1, color);
    colorTransfer->AddRGBPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(windowLevelMinMax[1], previous),
                               color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(scalarRange[1], previous),
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
void vtkSlicerVolumeRenderingLogic::SetGradientOpacityToVolumeProp(double scalarRange[2], vtkVolumeProperty* volumeProp)
{
  if (!scalarRange || !volumeProp)
    {
    vtkWarningMacro("SetGradientOpacityToVolumeProp: Inputs do not exist.");
    return;
    }

  double previous = VTK_DOUBLE_MIN;
  vtkNew<vtkPiecewiseFunction> opacity;
  // opacity doesn't support duplicate points
  opacity->AddPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(scalarRange[0], previous), 1.0);
  opacity->AddPoint(vtkMRMLVolumePropertyNode::HigherAndUnique(scalarRange[1], previous), 1.0);

  vtkPiecewiseFunction *volumePropGradientOpacity = volumeProp->GetGradientOpacity();
  if (this->IsDifferentFunction(opacity.GetPointer(), volumePropGradientOpacity))
    {
    volumePropGradientOpacity->DeepCopy(opacity.GetPointer());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::SetLabelMapToVolumeProp(vtkScalarsToColors* colors, vtkVolumeProperty* volumeProp)
{
  if (!colors || !volumeProp)
    {
    vtkWarningMacro("SetLabelMapToVolumeProp: Inputs do not exist.");
    return;
    }

  vtkNew<vtkPiecewiseFunction> opacity;
  vtkNew<vtkColorTransferFunction> colorTransfer;

  vtkLookupTable* lut = vtkLookupTable::SafeDownCast(colors);
  const int colorCount = colors->GetNumberOfAvailableColors();
  double value = colors->GetRange()[0];
  double step = (colors->GetRange()[1] - colors->GetRange()[0] + 1.) / colorCount;
  double color[4] = {0., 0., 0., 1.};
  const double midPoint = 0.5;
  const double sharpness = 1.0;
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
    opacity->AddPoint(value, color[3], midPoint, sharpness);
    colorTransfer->AddRGBPoint(value, color[0], color[1], color[2], midPoint, sharpness);
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
void vtkSlicerVolumeRenderingLogic::CopyDisplayToVolumeRenderingDisplayNode(
  vtkMRMLVolumeRenderingDisplayNode* vspNode, vtkMRMLVolumeDisplayNode* displayNode)
{
  if (!vspNode)
    {
    vtkWarningMacro("CopyDisplayToVolumeRenderingDisplayNode: Volume Rendering display node does not exist.");
    return;
    }
  if (!displayNode)
    {
    vtkMRMLVolumeNode* volumeNode = vspNode->GetVolumeNode();
    if (!volumeNode)
      {
      vtkWarningMacro("CopyDisplayToVolumeRenderingDisplayNode: Volume Rendering display node does not reference a volume node.");
      return;
      }
    displayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(volumeNode->GetDisplayNode());
    }
  if (!displayNode)
    {
    vtkWarningMacro("CopyDisplayToVolumeRenderingDisplayNode: No display node to copy.");
    return;
    }
  if (vtkMRMLScalarVolumeDisplayNode::SafeDownCast(displayNode))
    {
    this->CopyScalarDisplayToVolumeRenderingDisplayNode(vspNode, vtkMRMLScalarVolumeDisplayNode::SafeDownCast(displayNode));
    }
  else if (vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(displayNode))
    {
    this->CopyLabelMapDisplayToVolumeRenderingDisplayNode(vspNode, vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(displayNode));
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::CopyScalarDisplayToVolumeRenderingDisplayNode(
  vtkMRMLVolumeRenderingDisplayNode* vspNode, vtkMRMLScalarVolumeDisplayNode* vpNode)
{
  if (!vspNode)
    {
    vtkWarningMacro("CopyScalarDisplayToVolumeRenderingDisplayNode: No volume rendering display node.");
    return;
    }
  if (!vspNode->GetVolumePropertyNode())
    {
    vtkWarningMacro("CopyScalarDisplayToVolumeRenderingDisplayNode: No volume property node.");
    return;
    }

  if (!vpNode)
    {
    vpNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(vspNode->GetVolumeNode()->GetDisplayNode());
    }

  if (!vpNode)
    {
    vtkWarningMacro("CopyScalarDisplayToVolumeRenderingDisplayNode: No volume display node.");
    return;
    }

  bool ignoreVolumeDisplayNodeThreshold = vspNode->GetIgnoreVolumeDisplayNodeThreshold();
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

  vtkLookupTable* lut = vpNode->GetColorNode() ? vpNode->GetColorNode()->GetLookupTable() : nullptr;
  vtkVolumeProperty *prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();

  int disabledModify = vspNode->StartModify();
  int vpNodeDisabledModify = vspNode->GetVolumePropertyNode()->StartModify();

  this->SetThresholdToVolumeProp(scalarRange, threshold, prop, this->UseLinearRamp, ignoreVolumeDisplayNodeThreshold);

  this->SetWindowLevelToVolumeProp(scalarRange, windowLevel, lut, prop);
  this->SetGradientOpacityToVolumeProp(scalarRange, prop);

  vspNode->GetVolumePropertyNode()->EndModify(vpNodeDisabledModify);
  vspNode->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::CopyLabelMapDisplayToVolumeRenderingDisplayNode(
  vtkMRMLVolumeRenderingDisplayNode* vspNode, vtkMRMLLabelMapVolumeDisplayNode* vpNode)
{
  if (!vspNode)
    {
    vtkWarningMacro("CopyLabelMapDisplayToVolumeRenderingDisplayNode: No volume rendering display node.");
    return;
    }
  if (!vspNode->GetVolumePropertyNode())
    {
    vtkWarningMacro("CopyLabelMapDisplayToVolumeRenderingDisplayNode: No volume property node.");
    return;
    }

  if (!vpNode)
    {
    vpNode = vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(vspNode->GetVolumeNode()->GetDisplayNode());
    }
  if (!vpNode)
    {
    vtkWarningMacro("CopyLabelMapDisplayToVolumeRenderingDisplayNode: No volume display node.");
    return;
    }

  vtkScalarsToColors* colors = vpNode->GetColorNode() ? vpNode->GetColorNode()->GetScalarsToColors() : nullptr;

  vtkVolumeProperty *prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();

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
  // Resize the ROI to fit the volume
  vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::SafeDownCast(vspNode->GetROINode());
  vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode());

  if (volumeNode && roiNode)
  {
    int disabledModify = roiNode->StartModify();

    double xyz[3] = {0.0};
    double center[3] = {0.0};

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
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::CreateDefaultVolumeRenderingNodes(vtkMRMLVolumeNode* volumeNode)
{
  if (!volumeNode)
    {
    vtkErrorMacro("CreateVolumeRenderingNodesForVolume: No volume node given");
    return nullptr;
    }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkErrorMacro("CreateVolumeRenderingNodesForVolume: Invalid MRML scene");
    return nullptr;
    }

  vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode> displayNode = this->GetFirstVolumeRenderingDisplayNode(volumeNode);
  if (!displayNode)
    {
    displayNode = vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode>::Take(this->CreateVolumeRenderingDisplayNode());
    scene->AddNode(displayNode);

    // Add all 3D views to the display node
    std::vector<vtkMRMLNode*> viewNodes;
    scene->GetNodesByClass("vtkMRMLViewNode", viewNodes);
    for (std::vector<vtkMRMLNode*>::iterator nodeIt=viewNodes.begin(); nodeIt != viewNodes.end(); ++nodeIt)
      {
      displayNode->AddViewNodeID((*nodeIt)->GetID());
      }

    volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    }
  if (!displayNode)
    {
    vtkErrorMacro("CreateVolumeRenderingNodesForVolume: Failed to create volume rendering display node for scalar volume node " << volumeNode->GetName());
    return nullptr;
    }

  vtkMRMLVolumePropertyNode* volumePropertyNode = displayNode->GetVolumePropertyNode();
  if (!volumePropertyNode)
    {
    this->UpdateDisplayNodeFromVolumeNode(displayNode, volumeNode);
    this->SetRecommendedVolumeRenderingProperties(displayNode);
    volumePropertyNode = displayNode->GetVolumePropertyNode();
    }
  if (!volumePropertyNode)
    {
    vtkErrorMacro("CreateVolumeRenderingNodesForVolume: Failed to create volume property node for scalar volume node " << volumeNode->GetName());
    return displayNode;
    }

  return displayNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::CreateVolumeRenderingDisplayNode(const char* renderingClassName)
{
  vtkMRMLVolumeRenderingDisplayNode *node = nullptr;

  if (this->GetMRMLScene() == nullptr)
    {
    return node;
    }
  bool volumeRenderingUniqueName = true;
  if (renderingClassName == nullptr || strlen(renderingClassName) == 0)
    {
    renderingClassName = this->DefaultRenderingMethod;
    }
  else
    {
    volumeRenderingUniqueName = false;
    }
  if (renderingClassName == nullptr || strlen(renderingClassName) == 0)
    {
    renderingClassName = "vtkMRMLCPURayCastVolumeRenderingDisplayNode";
    }
  node = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass(renderingClassName));
  if (volumeRenderingUniqueName)
    {
    node->SetName(this->GetMRMLScene()->GenerateUniqueName("VolumeRendering").c_str());
    }

  return node;
}

// Description:
// Remove ViewNode from VolumeRenderingDisplayNode for a VolumeNode,
//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::RemoveViewFromVolumeDisplayNodes(
  vtkMRMLVolumeNode *volumeNode, vtkMRMLViewNode *viewNode)
{
  if (viewNode == nullptr || volumeNode == nullptr)
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
  vtkMRMLVolumeNode *volumeNode, char *displayNodeID)
{
  if (displayNodeID == nullptr || volumeNode == nullptr)
    {
    return nullptr;
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
  return nullptr;
}

// Description:
// Find first volume rendering display node
//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetFirstVolumeRenderingDisplayNode(vtkMRMLVolumeNode *volumeNode)
{
  if (volumeNode == nullptr)
    {
    return nullptr;
    }
  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(volumeNode->GetNthDisplayNode(i));
    if (!dnode)
      {
      // not a volume rendering display node
      continue;
      }
    if (dnode->GetVolumeNode() != volumeNode)
      {
      // Invalid volume node reference, ignore it (it would show a display node on the GUI that cannot be used to show a volume).
      // TODO: volume node reference is supposed to be always valid, but in some cases it becomes invalid.
      //   Mechanism that links volume node to display node would need to be redesigned to be more stable.
      continue;
      }
    return dnode;
    }
  return nullptr;
}

// Description:
// Find volume rendering display node referencing the view node and volume node
//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetVolumeRenderingDisplayNodeForViewNode(
  vtkMRMLVolumeNode *volumeNode, vtkMRMLViewNode *viewNode)
{
  if (viewNode == nullptr || volumeNode == nullptr)
    {
    return nullptr;
    }
  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(volumeNode->GetNthDisplayNode(i));

    if (dnode // display node is not necessarily volume rendering display node.
        && dnode->IsDisplayableInView(viewNode->GetID()))
      {
      return dnode;
      }
    }
  return nullptr;
}

// Description:
// Find volume rendering display node referencing the view node in the scene
//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetVolumeRenderingDisplayNodeForViewNode(vtkMRMLViewNode *viewNode)
{
  if (viewNode == nullptr || viewNode->GetScene() == nullptr)
    {
    return nullptr;
    }
  std::vector<vtkMRMLNode *> nodes;
  viewNode->GetScene()->GetNodesByClass("vtkMRMLVolumeRenderingDisplayNode", nodes);

  for (unsigned int i=0; i<nodes.size(); i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(nodes[i]);
    if (dnode && dnode->IsViewNodeIDPresent(viewNode->GetID()))
      {
      return dnode;
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic
::GetFirstVolumeRenderingDisplayNodeByROINode(vtkMRMLAnnotationROINode* roiNode)
{
  if (roiNode == nullptr || roiNode->GetScene() == nullptr)
    {
    return nullptr;
    }
  std::vector<vtkMRMLNode *> nodes;
  roiNode->GetScene()->GetNodesByClass("vtkMRMLVolumeRenderingDisplayNode", nodes);

  for (unsigned int i = 0; i < nodes.size(); ++i)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(nodes[i]);
    if (dnode && dnode->GetROINodeID() && !strcmp(dnode->GetROINodeID(), roiNode->GetID()))
      {
      return dnode;
      }
    }
  return nullptr;
}

// Description:
// Update vtkMRMLVolumeRenderingDisplayNode from VolumeNode,
// if needed create vtkMRMLVolumePropertyNode and vtkMRMLAnnotationROINode
// and initialize them from VolumeNode
//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::UpdateDisplayNodeFromVolumeNode(
  vtkMRMLVolumeRenderingDisplayNode *displayNode, vtkMRMLVolumeNode *volumeNode,
  vtkMRMLVolumePropertyNode *propNode /*=nullptr*/, vtkMRMLAnnotationROINode *roiNode /*=nullptr*/ )
{
  if (displayNode == nullptr)
    {
    vtkErrorMacro("vtkSlicerVolumeRenderingLogic::UpdateDisplayNodeFromVolumeNode: display node pointer is null.");
    return;
    }

  if (volumeNode == nullptr)
    {
    return;
    }

  if (propNode == nullptr && displayNode->GetVolumePropertyNode() == nullptr)
    {
    propNode = vtkMRMLVolumePropertyNode::New();
    this->GetMRMLScene()->AddNode(propNode);
    propNode->Delete();
    }
  if (propNode != nullptr)
    {
    displayNode->SetAndObserveVolumePropertyNodeID(propNode->GetID());
    }

  if (roiNode == nullptr && displayNode->GetROINode() == nullptr)
    {
    roiNode = vtkMRMLAnnotationROINode::New();
    // By default, the ROI is interactive. It could be an application setting.
    roiNode->SetInteractiveMode(1);
    roiNode->Initialize(this->GetMRMLScene());
    // by default, show the ROI only if cropping is enabled
    roiNode->SetDisplayVisibility(displayNode->GetCroppingEnabled());
    roiNode->Delete();
    }
  if (roiNode != nullptr)
    {
    displayNode->SetAndObserveROINodeID(roiNode->GetID());
    }

  this->CopyDisplayToVolumeRenderingDisplayNode(displayNode);

  this->FitROIToVolume(displayNode);
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkSlicerVolumeRenderingLogic::AddVolumePropertyFromFile(const char* filename)
{
  if (!this->GetMRMLScene())
    {
    return nullptr;
    }
  if (!filename || !strcmp(filename, ""))
    {
    vtkErrorMacro("AddVolumePropertyFromFile: can't load volume properties from empty file name");
    return nullptr;
    }

  vtkSmartPointer<vtkMRMLVolumePropertyNode> vpNode = vtkSmartPointer<vtkMRMLVolumePropertyNode>::New();
  vtkSmartPointer<vtkMRMLVolumePropertyStorageNode> vpStorageNode = vtkSmartPointer<vtkMRMLVolumePropertyStorageNode>::New();

  // check for local or remote files
  int useURI = 0; // false;
  if (this->GetMRMLScene()->GetCacheManager() != nullptr)
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    }

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
  const std::string fname(localFile);

  // check to see which node can read this type of file
  if (!vpStorageNode->SupportedFileType(fname.c_str()))
    {
    vtkDebugMacro("Couldn't read file, returning null volume property node: " << filename);
    return nullptr;
    }

  // the node name is based on the file name
  const std::string name = vpStorageNode->GetFileNameWithoutExtension(fname.c_str());
  std::string uname( this->GetMRMLScene()->GetUniqueNameByString(name.c_str()));
  vpNode->SetName(uname.c_str());
  this->GetMRMLScene()->AddNode(vpNode);
  this->GetMRMLScene()->AddNode(vpStorageNode);
  vpNode->SetAndObserveStorageNodeID(vpStorageNode->GetID());

  // now set up the reading
  int retval = vpStorageNode->ReadData(vpNode);
  if (retval != 1)
    {
    vtkErrorMacro("AddVolumePropertyFromFile: error reading " << filename);
    this->GetMRMLScene()->RemoveNode(vpNode);
    this->GetMRMLScene()->RemoveNode(vpStorageNode);
    return nullptr;
    }

  return vpNode;
}

//---------------------------------------------------------------------------
vtkMRMLShaderPropertyNode* vtkSlicerVolumeRenderingLogic::AddShaderPropertyFromFile(const char* filename)
{
  if (!this->GetMRMLScene())
    {
    return nullptr;
    }
  if (!filename || !strcmp(filename, ""))
    {
    vtkErrorMacro(<<"AddShaderPropertyFromFile: can't load shader properties from empty file name");
    return nullptr;
    }

  vtkNew<vtkMRMLShaderPropertyNode> spNode;
  vtkNew<vtkMRMLShaderPropertyStorageNode> spStorageNode;

  // check for local or remote files
  int useURI = 0; // false;
  if (this->GetMRMLScene()->GetCacheManager() != nullptr)
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    }

  const char *localFile = nullptr;
  if (useURI)
    {
    spStorageNode->SetURI(filename);
     // reset filename to the local file name
    localFile = ((this->GetMRMLScene())->GetCacheManager())->GetFilenameFromURI(filename);
    }
  else
    {
    spStorageNode->SetFileName(filename);
    localFile = filename;
    }
  const std::string fname(localFile);
  // the node name is based on the file name
  const std::string name = spStorageNode->GetFileNameWithoutExtension(fname.c_str());

  // check to see which node can read this type of file
  if (spStorageNode->SupportedFileType(fname.c_str()))
    {
    std::string uname( this->GetMRMLScene()->GetUniqueNameByString(name.c_str()));

    spNode->SetName(uname.c_str());

    spNode->SetScene(this->GetMRMLScene());
    spStorageNode->SetScene(this->GetMRMLScene());

    this->GetMRMLScene()->AddNode(spStorageNode);
    spNode->SetAndObserveStorageNodeID(spStorageNode->GetID());

    this->GetMRMLScene()->AddNode(spNode);

    // now set up the reading
    int retval = spStorageNode->ReadData(spNode);
    if (retval != 1)
      {
      vtkErrorMacro("AddVolumePropertyFromFile: error reading " << filename);
      this->GetMRMLScene()->RemoveNode(spNode);
      this->GetMRMLScene()->RemoveNode(spStorageNode);
      return nullptr;
      }
    return spNode;
    }
  else
    {
    vtkDebugMacro("Couldn't read file, returning null model node: " << filename);
    return nullptr;
    }
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
vtkMRMLVolumePropertyNode* vtkSlicerVolumeRenderingLogic::GetPresetByName(const char* presetName)
{
  vtkMRMLScene * presetsScene = this->GetPresetsScene();
  if (!presetsScene || !presetName)
    {
    return nullptr;
    }
  vtkSmartPointer<vtkCollection> presets;
  presets.TakeReference(presetsScene->GetNodesByClassByName("vtkMRMLVolumePropertyNode", presetName));
  if (presets->GetNumberOfItems() == 0)
    {
    return nullptr;
    }
  return vtkMRMLVolumePropertyNode::SafeDownCast(presets->GetItemAsObject(0));
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
  if (!connected)
    {
    vtkErrorMacro(<< "Failed to load presets [" << presetFileName << "]");
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerVolumeRenderingLogic::IsDifferentFunction(vtkPiecewiseFunction* function1, vtkPiecewiseFunction* function2)const
{
  if ((function1 != nullptr) ^ (function2 != nullptr))
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
bool vtkSlicerVolumeRenderingLogic::IsDifferentFunction(vtkColorTransferFunction* function1, vtkColorTransferFunction* function2)const
{
  if ((function1 != nullptr) ^ (function2 != nullptr))
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

//---------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::AddPreset(vtkMRMLVolumePropertyNode* preset, vtkImageData* icon /* = nullptr */)
{
  if (preset == nullptr)
    {
    vtkErrorMacro("vtkSlicerVolumeRenderingLogic::AddPreset failed: preset is invalid");
    return;
    }
  if (icon == nullptr)
    {
    // use the icon assigned to the preset node if available
    vtkMRMLVolumeNode* iconNode = vtkMRMLVolumeNode::SafeDownCast(
      preset->GetNodeReference(vtkSlicerVolumeRenderingLogic::GetIconVolumeReferenceRole()));
    if (iconNode)
      {
      icon = iconNode->GetImageData();
      }
    }
  vtkMRMLScene* presetScene = this->GetPresetsScene();
  if (icon != nullptr)
    {
    // vector volume is chosen because usually icons are RGB color images
    vtkNew<vtkMRMLVectorVolumeNode> iconNode;
    iconNode->SetAndObserveImageData(icon);
    vtkMRMLNode* addedIconNode = presetScene->AddNode(iconNode.GetPointer());
    // Need to set the node reference before adding the node to the scene to make sure the icon
    // is available immediately when the node is added (otherwise widgets may add the item without an icon)
    preset->SetNodeReferenceID(vtkSlicerVolumeRenderingLogic::GetIconVolumeReferenceRole(), addedIconNode->GetID());
    }
  presetScene->AddNode(preset);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::RemovePreset(vtkMRMLVolumePropertyNode* preset)
{
  if (preset == nullptr)
    {
    return;
    }
  vtkMRMLScene* presetScene = this->GetPresetsScene();
  vtkMRMLNode* iconNode = preset->GetNodeReference(vtkSlicerVolumeRenderingLogic::GetIconVolumeReferenceRole());
  if (iconNode != nullptr)
    {
    presetScene->RemoveNode(iconNode);
    }
  presetScene->RemoveNode(preset);
}

//---------------------------------------------------------------------------
int vtkSlicerVolumeRenderingLogic::LoadCustomPresetsScene(const char* sceneFilePath)
{
  if (!this->PresetsScene)
    {
    this->PresetsScene = vtkMRMLScene::New();
    }
  else
    {
    this->PresetsScene->Clear(1);
    }

  this->PresetsScene->SetURL(sceneFilePath);
  return this->PresetsScene->Import();
}

//---------------------------------------------------------------------------
bool vtkSlicerVolumeRenderingLogic::SetRecommendedVolumeRenderingProperties(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode == nullptr || vspNode->GetVolumePropertyNode() == nullptr)
    {
    vtkErrorMacro("SetRecommendedVolumeRenderingProperties: invalid input display or volume property node");
    return false;
    }
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode());
  if (!volumeNode || !volumeNode->GetImageData())
    {
    vtkErrorMacro("SetRecommendedVolumeRenderingProperties: invalid volume node");
    return false;
    }

  if (volumeNode->IsA("vtkMRMLLabelMapVolumeNode"))
    {
    return false;
    }

  double* scalarRange = volumeNode->GetImageData()->GetScalarRange();
  double scalarRangeSize = scalarRange[1] - scalarRange[0];

  if (scalarRangeSize > 50.0 && scalarRangeSize < 1500.0 && this->GetPresetByName("MR-Default"))
    {
    // small dynamic range, probably MRI
    vspNode->GetVolumePropertyNode()->Copy(this->GetPresetByName("MR-Default"));
    return true;
    }

  if (scalarRangeSize >= 1500.0 && scalarRangeSize < 10000.0 && this->GetPresetByName("CT-Chest-Contrast-Enhanced"))
    {
    // larger dynamic range, probably CT
    vspNode->GetVolumePropertyNode()->Copy(this->GetPresetByName("CT-Chest-Contrast-Enhanced"));
    return true;
    }

  return false;
}
