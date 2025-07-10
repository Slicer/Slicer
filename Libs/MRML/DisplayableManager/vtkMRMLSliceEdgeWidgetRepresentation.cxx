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

  This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
  and development was supported in part by CI3.

==============================================================================*/

// VTK includes
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkFeatureEdges.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkStripper.h>
#include <vtkTubeFilter.h>

// MRML includes
#include <vtkMRMLAbstractThreeDViewDisplayableManager.h>
#include <vtkMRMLInteractionEventData.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLViewNode.h>

#include <vtkMRMLSliceEdgeWidgetRepresentation.h>

vtkStandardNewMacro(vtkMRMLSliceEdgeWidgetRepresentation);

//----------------------------------------------------------------------
vtkMRMLSliceEdgeWidgetRepresentation::vtkMRMLSliceEdgeWidgetRepresentation()
{
  this->ViewScaleFactorMmPerPixel = 1.0;
  this->ScreenSizePixel = 1000;
  this->NeedToRender = false;
  this->AlwaysOnTop = true;
  this->Pipeline = nullptr;
}

//----------------------------------------------------------------------
void vtkMRMLSliceEdgeWidgetRepresentation::SetupSliceEdgePipeline()
{
  this->Pipeline = new SliceEdgePipeline();
  this->NeedToRenderOn();
}

//----------------------------------------------------------------------
vtkMRMLSliceEdgeWidgetRepresentation::~vtkMRMLSliceEdgeWidgetRepresentation()
{
  // Force deleting variables to prevent circular dependency keeping objects alive
  if (this->Pipeline != nullptr)
  {
    delete this->Pipeline;
    this->Pipeline = nullptr;
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLSliceEdgeWidgetRepresentation::PrintSelf(ostream& os,
                                                      vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------
void vtkMRMLSliceEdgeWidgetRepresentation::UpdateFromMRML(
    vtkMRMLNode* vtkNotUsed(caller), unsigned long vtkNotUsed(event), void* vtkNotUsed(callData))
{
  if (!this->Pipeline)
  {
    this->SetupSliceEdgePipeline();
  }

  if (this->GetSliceNode())
  {
    this->UpdateSliceEdgeFromSliceNode();
  }

  if (this->Pipeline)
  {
    this->UpdateSliceEdgePipeline();
  }
}

//----------------------------------------------------------------------
void vtkMRMLSliceEdgeWidgetRepresentation::UpdateSliceEdgePipeline()
{
  if (!this->Pipeline)
  {
    return;
  }

  double radius = this->ViewScaleFactorMmPerPixel * this->SliceEdgeSize;
  double previousRadius = this->Pipeline->TubeFilter->GetRadius();
  if (fabs(radius - previousRadius) < 1e-6)
  {
    return;
  }

  this->Pipeline->TubeFilter->SetRadius(radius);
  this->NeedToRenderOn();
}

//----------------------------------------------------------------------
void vtkMRMLSliceEdgeWidgetRepresentation::GetActors(vtkPropCollection* pc)
{
  vtkProp* actor = this->GetSliceEdgeActor();
  if (actor)
  {
    actor->GetActors(pc);
  }
}

//----------------------------------------------------------------------
void vtkMRMLSliceEdgeWidgetRepresentation::ReleaseGraphicsResources(vtkWindow* window)
{
  vtkProp* actor = this->GetSliceEdgeActor();
  if (actor)
  {
    actor->ReleaseGraphicsResources(window);
  }
}

//----------------------------------------------------------------------
int vtkMRMLSliceEdgeWidgetRepresentation::RenderOverlay(vtkViewport* viewport)
{
  int count = 0;
  vtkProp* actor = this->GetSliceEdgeActor();
  if (this->Pipeline && actor->GetVisibility())
  {
    count += actor->RenderOverlay(viewport);
  }
  return count;
}

//----------------------------------------------------------------------
int vtkMRMLSliceEdgeWidgetRepresentation::RenderOpaqueGeometry(vtkViewport* viewport)
{
  if (!this->Pipeline)
  {
    this->SetupSliceEdgePipeline();
  }

  int count = 0;
  vtkProp* actor = this->GetSliceEdgeActor();
  if (actor && actor->GetVisibility())
  {
    this->UpdateSliceEdgeFromSliceNode();
    this->UpdateViewScaleFactor();
    this->UpdateSliceEdgePipeline();
    count += actor->RenderOpaqueGeometry(viewport);
  }
  return count;
}

//----------------------------------------------------------------------
int vtkMRMLSliceEdgeWidgetRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
{
  int count = 0;
  vtkProp* actor = this->GetSliceEdgeActor();
  if (actor && actor->GetVisibility())
  {
    actor->SetPropertyKeys(this->GetPropertyKeys());
    count += actor->RenderTranslucentPolygonalGeometry(viewport);
  }
  return count;
}

//----------------------------------------------------------------------
vtkTypeBool vtkMRMLSliceEdgeWidgetRepresentation::HasTranslucentPolygonalGeometry()
{
  vtkProp* actor = this->GetSliceEdgeActor();
  if (actor && actor->GetVisibility() && //
      actor->HasTranslucentPolygonalGeometry())
  {
    return true;
  }
  return false;
}

//----------------------------------------------------------------------
vtkMRMLSliceEdgeWidgetRepresentation::SliceEdgePipeline::SliceEdgePipeline()
{
  this->FeatureEdges = vtkSmartPointer<vtkFeatureEdges>::New();
  this->FeatureEdges->SetOutputPointsPrecision(vtkAlgorithm::SINGLE_PRECISION);
  this->FeatureEdges->SetBoundaryEdges(true);
  this->FeatureEdges->SetFeatureEdges(false);
  this->FeatureEdges->SetNonManifoldEdges(false);
  this->FeatureEdges->SetManifoldEdges(false);
  this->FeatureEdges->SetColoring(false);

  this->Stripper = vtkSmartPointer<vtkStripper>::New();
  this->Stripper->SetInputConnection(FeatureEdges->GetOutputPort());

  this->TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  this->TubeFilter->SetInputConnection(this->Stripper->GetOutputPort());
  this->TubeFilter->SetRadius(0.75);
  this->TubeFilter->SetNumberOfSides(12);

  this->Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->Mapper->SetInputConnection(TubeFilter->GetOutputPort());

  this->Actor = vtkSmartPointer<vtkActor>::New();
  this->Actor->SetMapper(this->Mapper);
  this->Actor->PickableOff();

  vtkProperty* prop = this->Actor->GetProperty();
  prop->SetRepresentationToSurface();
  prop->SetAmbient(0.5);
  prop->SetDiffuse(0.5);
  prop->SetSpecular(0);
  prop->SetShading(true);
  prop->SetSpecularPower(1);
  prop->SetOpacity(1.);
  prop->SetEdgeVisibility(false);
  prop->SetVertexVisibility(false);
}

//----------------------------------------------------------------------
vtkProp* vtkMRMLSliceEdgeWidgetRepresentation::GetSliceEdgeActor()
{
  if (!this->Pipeline)
  {
    return nullptr;
  }
  return this->Pipeline->Actor;
}

//----------------------------------------------------------------------
vtkPolyData* vtkMRMLSliceEdgeWidgetRepresentation::GetSliceEdgePolydata()
{
  if (!this->Pipeline)
  {
    return nullptr;
  }
  return this->Pipeline->TubeFilter->GetOutput();
}

//----------------------------------------------------------------------
void vtkMRMLSliceEdgeWidgetRepresentation::GetSliceEdgeColor(double color[4])
{
  if (!this->Pipeline || !color)
  {
    return;
  }

  double rgb[3];
  this->Pipeline->Actor->GetProperty()->GetColor(rgb);
  color[0] = rgb[0];
  color[1] = rgb[1];
  color[2] = rgb[2];
  color[3] = this->Pipeline->Actor->GetProperty()->GetOpacity();
}

//----------------------------------------------------------------------
void vtkMRMLSliceEdgeWidgetRepresentation::setSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode || this->SliceNode == sliceNode)
  {
    return;
  }

  this->SliceNode = sliceNode;
  this->UpdateSliceEdgeFromSliceNode();
}

//----------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceEdgeWidgetRepresentation::GetSliceNode()
{
  return this->SliceNode;
}

//----------------------------------------------------------------------
void vtkMRMLSliceEdgeWidgetRepresentation::setSliceModelNode(vtkMRMLModelNode* sliceModelNode)
{
  if (!this->Pipeline || this->SliceModelNode == sliceModelNode)
  {
    // no change
    return;
  }

  this->SliceModelNode = sliceModelNode;
  this->Pipeline->FeatureEdges->SetInputConnection(sliceModelNode ? sliceModelNode->GetPolyDataConnection() : nullptr);

  this->NeedToRenderOn();
}

//----------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLSliceEdgeWidgetRepresentation::GetSliceModelNode()
{
  return this->SliceModelNode;
}

//----------------------------------------------------------------------
void vtkMRMLSliceEdgeWidgetRepresentation::UpdateSliceEdgeFromSliceNode()
{
  if (!this->GetSliceNode() || !this->Pipeline)
  {
    return;
  }

  vtkProperty* prop = this->Pipeline->Actor->GetProperty();
  double rgb[3];
  prop->GetColor(rgb);
  double* layoutColor = this->GetSliceNode()->GetLayoutColor();
  const double tolerance = 1.e-6;
  if (fabs(layoutColor[0] - rgb[0]) < tolerance && //
      fabs(layoutColor[1] - rgb[1]) < tolerance && //
      fabs(layoutColor[2] - rgb[2]) < tolerance)
  {
    // no change
    return;
  }

  prop->SetColor(layoutColor);
  this->NeedToRenderOn();
}

//----------------------------------------------------------------------
void vtkMRMLSliceEdgeWidgetRepresentation::UpdateViewScaleFactor()
{
  this->ViewScaleFactorMmPerPixel = 1.0;
  this->ScreenSizePixel = 1000.0;
  if (!this->Renderer || !this->Renderer->GetActiveCamera() || !this->Renderer->GetRenderWindow())
  {
    return;
  }

  if (this->Renderer->GetRenderWindow()->GetNeverRendered())
  {
    // In VR, calling GetScreenSize() without rendering can cause a crash.
    return;
  }

  const int* screenSize = this->Renderer->GetRenderWindow()->GetScreenSize();
  double screenSizePixel = sqrt(screenSize[0] * screenSize[0] + screenSize[1] * screenSize[1]);
  if (screenSizePixel < 1.0)
  {
    // render window is not fully initialized yet
    return;
  }
  this->ScreenSizePixel = screenSizePixel;

  double cameraFP[3] = { 0.0 };
  this->Renderer->GetActiveCamera()->GetFocalPoint(cameraFP);
  this->ViewScaleFactorMmPerPixel = vtkMRMLAbstractThreeDViewDisplayableManager::
    GetViewScaleFactorAtPosition(this->Renderer, cameraFP);
}
