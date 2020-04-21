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
  // Default glyph scale used to be 3.0 (in Slicer-4.10 and earlier).
  // This display scale factor value produces similar appearance of markup points.
  this->ScreenScaleFactor = 0.2;

  this->PickingTolerance = 30.0;
  this->NeedToRender = false;

  this->AlwaysOnTop = false;
}

//----------------------------------------------------------------------
vtkMRMLAbstractWidgetRepresentation::~vtkMRMLAbstractWidgetRepresentation() = default;

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
  os << indent << "PickingTolerance : " << this->PickingTolerance <<"\n";
  os << indent << "ScreenScaleFactor: " << this->ScreenScaleFactor << "\n";
  os << indent << "Always On Top: " << (this->AlwaysOnTop ? "On\n" : "Off\n");
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
