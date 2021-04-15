/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#include "vtkSlicerTestLineRepresentation2D.h"

#include <vtkActor2D.h>
#include <vtkGlyphSource2D.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkSampleImplicitFunctionFilter.h>
//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTestLineRepresentation2D);

//------------------------------------------------------------------------------
vtkSlicerTestLineRepresentation2D::vtkSlicerTestLineRepresentation2D()
{
  this->MiddlePointSource = vtkSmartPointer<vtkGlyphSource2D>::New();
  this->MiddlePointSource->SetCenter(0.0, 0.0, 0.0);
  this->MiddlePointSource->SetScale(5);

  this->MiddlePointDataMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->MiddlePointDataMapper->SetInputConnection(this->MiddlePointSource->GetOutputPort());

  this->MiddlePointActor = vtkSmartPointer<vtkActor2D>::New();
  this->MiddlePointActor->SetMapper(this->MiddlePointDataMapper);
}

//------------------------------------------------------------------------------
vtkSlicerTestLineRepresentation2D::~vtkSlicerTestLineRepresentation2D() = default;

//------------------------------------------------------------------------------
void vtkSlicerTestLineRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}
// -----------------------------------------------------------------------------
void vtkSlicerTestLineRepresentation2D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
{
  Superclass::UpdateFromMRML(caller, event, callData);

  this->NeedToRenderOn();

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || !this->IsDisplayable())
    {
    this->VisibilityOff();
    return;
    }

  this->VisibilityOn();

  this->MiddlePointActor->SetVisibility(markupsNode->GetNumberOfDefinedControlPoints(true) == 2);

  // Hide the line actor if it doesn't intersect the current slice
  this->SliceDistance->Update();
  if (!this->IsRepresentationIntersectingSlice(vtkPolyData::SafeDownCast(this->SliceDistance->GetOutput()), this->SliceDistance->GetScalarArrayName()))
    {
    this->MiddlePointActor->SetVisibility(false);
    }

  if (markupsNode->GetNumberOfDefinedControlPoints(true) == 2)
    {
    double p1[3] = { 0.0 };
    double p2[3] = { 0.0 };
    this->GetNthControlPointDisplayPosition(0, p1);
    this->GetNthControlPointDisplayPosition(1, p2);
    double middlePointPos[2] = { (p1[0] + p2[0]) / 2.0, (p1[1] + p2[1]) / 2.0 };
    // this->MiddlePointActor->SetDisplayPosition(static_cast<int>(middlePointPos[0]),
    //                                            static_cast<int>(middlePointPos[1]));
    this->MiddlePointSource->SetCenter(middlePointPos[0], middlePointPos[1], 0.0);
    this->MiddlePointSource->Update();
    }
  else
    {
    this->MiddlePointActor->SetVisibility(false);
    }

  this->MiddlePointActor->SetProperty(this->GetControlPointsPipeline(Active)->Property);
}

//----------------------------------------------------------------------
void vtkSlicerTestLineRepresentation2D::GetActors(vtkPropCollection *pc)
{
  this->MiddlePointActor->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerTestLineRepresentation2D::ReleaseGraphicsResources(vtkWindow *win)
{
  this->MiddlePointActor->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerTestLineRepresentation2D::RenderOverlay(vtkViewport *viewport)
{
  int count=0;
  if (this->MiddlePointActor->GetVisibility())
    {
    count +=  this->MiddlePointActor->RenderOverlay(viewport);
    }
  count += this->Superclass::RenderOverlay(viewport);
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerTestLineRepresentation2D::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int count=0;
  if (this->MiddlePointActor->GetVisibility())
    {
    count += this->MiddlePointActor->RenderOpaqueGeometry(viewport);
    }
  count = this->Superclass::RenderOpaqueGeometry(viewport);
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerTestLineRepresentation2D::RenderTranslucentPolygonalGeometry(vtkViewport *viewport)
{
  int count=0;
  if (this->MiddlePointActor->GetVisibility())
    {
    count += this->MiddlePointActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  count = this->Superclass::RenderTranslucentPolygonalGeometry(viewport);
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerTestLineRepresentation2D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->MiddlePointActor->GetVisibility() && this->MiddlePointActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  return false;
}
