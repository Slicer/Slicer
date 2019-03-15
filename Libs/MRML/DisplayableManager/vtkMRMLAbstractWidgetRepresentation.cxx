/*=========================================================================

 Copyright (c) ProxSim ltd., Kwun Tong, Hong Kong. All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
 and development was supported by ProxSim ltd.

=========================================================================*/

#include "vtkMRMLAbstractWidgetRepresentation.h"
#include "vtkCamera.h"
#include "vtkMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"

//----------------------------------------------------------------------
vtkMRMLAbstractWidgetRepresentation::vtkMRMLAbstractWidgetRepresentation()
{
  this->ViewScaleFactor = 1.0;

  this->Tolerance = 2.0;
  this->PixelTolerance = 1;
  this->NeedToRender = false;

  this->AlwaysOnTop = false;
}

//----------------------------------------------------------------------
vtkMRMLAbstractWidgetRepresentation::~vtkMRMLAbstractWidgetRepresentation()
{
}

//----------------------------------------------------------------------
void vtkMRMLAbstractWidgetRepresentation::UpdateViewScaleFactor()
{
  if (!this->Renderer || !this->Renderer->GetActiveCamera())
    {
    this->ViewScaleFactor = 1.0;
    }

  double p1[4], p2[4];
  this->Renderer->GetActiveCamera()->GetFocalPoint(p1);
  p1[3] = 1.0;
  this->Renderer->SetWorldPoint(p1);
  this->Renderer->WorldToView();
  this->Renderer->GetViewPoint(p1);

  double depth = p1[2];
  double aspect[2];
  this->Renderer->ComputeAspect();
  this->Renderer->GetAspect(aspect);

  p1[0] = -aspect[0];
  p1[1] = -aspect[1];
  this->Renderer->SetViewPoint(p1);
  this->Renderer->ViewToWorld();
  this->Renderer->GetWorldPoint(p1);

  p2[0] = aspect[0];
  p2[1] = aspect[1];
  p2[2] = depth;
  p2[3] = 1.0;
  this->Renderer->SetViewPoint(p2);
  this->Renderer->ViewToWorld();
  this->Renderer->GetWorldPoint(p2);

  double distance = sqrt(vtkMath::Distance2BetweenPoints(p1, p2));

  int *size = this->Renderer->GetRenderWindow()->GetSize();
  double viewport[4];
  this->Renderer->GetViewport(viewport);

  double x, y, distance2;

  x = size[0] * (viewport[2] - viewport[0]);
  y = size[1] * (viewport[3] - viewport[1]);

  distance2 = sqrt(x * x + y * y);
  this->ViewScaleFactor = distance2 / distance;
}


//----------------------------------------------------------------------
void vtkMRMLAbstractWidgetRepresentation
::GetRendererComputedDisplayPositionFromWorldPosition(const double worldPos[3],
                                                      double displayPos[2])
{
  double pos[4];
  pos[0] = worldPos[0];
  pos[1] = worldPos[1];
  pos[2] = worldPos[2];
  pos[3] = 1.0;

  this->Renderer->SetWorldPoint(pos);
  this->Renderer->WorldToDisplay();
  this->Renderer->GetDisplayPoint(pos);

  displayPos[0] = static_cast<int>(pos[0]);
  displayPos[1] = static_cast<int>(pos[1]);
}

//-----------------------------------------------------------------------------
void vtkMRMLAbstractWidgetRepresentation::SetRenderer(vtkRenderer *ren)
{
  if ( ren == this->Renderer )
    {
    return;
    }
  this->Renderer = ren;
  this->Modified();
}

//-----------------------------------------------------------------------------
vtkRenderer* vtkMRMLAbstractWidgetRepresentation::GetRenderer()
{
  return this->Renderer;
}

//-----------------------------------------------------------------------------
void vtkMRMLAbstractWidgetRepresentation::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (viewNode == this->ViewNode)
    {
    return;
    }
  this->ViewNode = viewNode;
  this->Modified();
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractViewNode* vtkMRMLAbstractWidgetRepresentation::GetViewNode()
{
  return this->ViewNode;
}

//-----------------------------------------------------------------------------
void vtkMRMLAbstractWidgetRepresentation::PrintSelf(ostream& os,
                                                      vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Tolerance: " << this->Tolerance <<"\n";

  os << indent << "Current Operation: ";

  os << indent << "Always On Top: "
     << (this->AlwaysOnTop ? "On\n" : "Off\n");
}


//-----------------------------------------------------------------------------
void vtkMRMLAbstractWidgetRepresentation::AddActorsBounds(vtkBoundingBox& boundingBox,
  const std::vector<vtkProp*> &actors, double* additionalBounds /*=nullptr*/)
{
  for (auto actor : actors)
    {
    if (!actor->GetVisibility())
      {
      continue;
      }
    double* bounds = actor->GetBounds();
    if (!bounds)
      {
      continue;
      }
    boundingBox.AddBounds(bounds);
    }
  if (additionalBounds)
    {
    boundingBox.AddBounds(additionalBounds);
    }
}

//----------------------------------------------------------------------
void vtkMRMLAbstractWidgetRepresentation::UpdateFromMRML(
    vtkMRMLNode* vtkNotUsed(caller), unsigned long vtkNotUsed(event), void *vtkNotUsed(callData))
{
}

//-----------------------------------------------------------------------------
void vtkMRMLAbstractWidgetRepresentation::UpdateRelativeCoincidentTopologyOffsets(vtkMapper* mapper)
{
  if (this->AlwaysOnTop)
    {
    // max value 65536 so we subtract 66000 to make sure we are
    // zero or negative
    mapper->SetRelativeCoincidentTopologyLineOffsetParameters(0, -66000);
    mapper->SetRelativeCoincidentTopologyPolygonOffsetParameters(0, -66000);
    mapper->SetRelativeCoincidentTopologyPointOffsetParameter(-66000);
    }
  else
    {
    mapper->SetRelativeCoincidentTopologyLineOffsetParameters(-1, -1);
    mapper->SetRelativeCoincidentTopologyPolygonOffsetParameters(-1, -1);
    mapper->SetRelativeCoincidentTopologyPointOffsetParameter(-1);
    }
}
