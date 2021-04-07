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

#include "vtkSlicerTestLineRepresentation3D.h"

#include "vtkMRMLMarkupsTestLineNode.h"

// VTK includes
#include <vtkActor.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>


//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTestLineRepresentation3D);

//------------------------------------------------------------------------------
vtkSlicerTestLineRepresentation3D::vtkSlicerTestLineRepresentation3D()
  :TargetOrgan(nullptr)
{

  this->SlicingPlane = vtkSmartPointer<vtkPlane>::New();

  this->Cutter =  vtkSmartPointer<vtkCutter>::New();
  this->Cutter->SetInputData(this->TargetOrgan);
  this->Cutter->SetCutFunction(this->SlicingPlane);
  this->Cutter->SetNumberOfContours(1);
  this->Cutter->GenerateTrianglesOn();
  this->Cutter->GenerateCutScalarsOff();

  this->ContourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ContourMapper->SetInputConnection(this->Cutter->GetOutputPort());

  this->ContourActor = vtkSmartPointer<vtkActor>::New();
  this->ContourActor->SetMapper(this->ContourMapper);

  this->MiddlePointSource = vtkSmartPointer<vtkSphereSource>::New();

  this->MiddlePointMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->MiddlePointMapper->SetInputConnection(this->MiddlePointSource->GetOutputPort());

  this->MiddlePointActor = vtkSmartPointer<vtkActor>::New();
  this->MiddlePointActor->SetMapper(this->MiddlePointMapper);
}

//------------------------------------------------------------------------------
vtkSlicerTestLineRepresentation3D::~vtkSlicerTestLineRepresentation3D() = default;

//------------------------------------------------------------------------------
void vtkSlicerTestLineRepresentation3D::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
void vtkSlicerTestLineRepresentation3D::GetActors(vtkPropCollection* pc)
{
  this->Superclass::GetActors(pc);

  if (this->TargetOrgan)
    {
    this->ContourActor->GetActors(pc);
    }

  this->MiddlePointActor->GetActors(pc);
}

//------------------------------------------------------------------------------
void vtkSlicerTestLineRepresentation3D::ReleaseGraphicsResources(vtkWindow* win)
{
  this->Superclass::ReleaseGraphicsResources(win);

  if (this->TargetOrgan)
    {
    this->ContourActor->ReleaseGraphicsResources(win);
    }

  this->MiddlePointActor->ReleaseGraphicsResources(win);
}

//------------------------------------------------------------------------------
int vtkSlicerTestLineRepresentation3D::RenderOverlay(vtkViewport* viewport)
{
  int count = this->Superclass::RenderOverlay(viewport);
  if (this->TargetOrgan && this->ContourActor->GetVisibility())
    {
    count += this->ContourActor->RenderOverlay(viewport);
    }

  if (this->MiddlePointActor->GetVisibility())
    {
    count += this->MiddlePointActor->RenderOverlay(viewport);
    }
  return count;
}

//------------------------------------------------------------------------------
int vtkSlicerTestLineRepresentation3D::RenderOpaqueGeometry(vtkViewport* viewport)
{
  int count = this->Superclass::RenderOpaqueGeometry(viewport);
  if (this->TargetOrgan && this->ContourActor->GetVisibility())
    {
    count += this->ContourActor->RenderOpaqueGeometry(viewport);
    }

  if (this->MiddlePointActor->GetVisibility())
    {
    count += this->MiddlePointActor->RenderOpaqueGeometry(viewport);
    }

  return count;
}

//------------------------------------------------------------------------------
int vtkSlicerTestLineRepresentation3D::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
{
  int count = this->Superclass::RenderTranslucentPolygonalGeometry(viewport);
  if (this->TargetOrgan && this->ContourActor->GetVisibility())
    {
    this->ContourActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->ContourActor->RenderTranslucentPolygonalGeometry(viewport);
    }

  if (this->MiddlePointActor->GetVisibility())
    {
    this->MiddlePointActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->MiddlePointActor->RenderTranslucentPolygonalGeometry(viewport);
    }

  return count;
}

//------------------------------------------------------------------------------
vtkTypeBool vtkSlicerTestLineRepresentation3D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }

  if (this->TargetOrgan && this->ContourActor->GetVisibility() &&
      this->ContourActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }

  if (this->MiddlePointActor->GetVisibility() &&
      this->MiddlePointActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------
void vtkSlicerTestLineRepresentation3D::UpdateFromMRML(vtkMRMLNode* caller,
                                                           unsigned long event,
                                                           void *callData /*=nullptr*/)
{
 this->Superclass::UpdateFromMRML(caller, event, callData);

 this->NeedToRenderOn();

 this->BuildMiddlePoint();
 this->BuildSlicingPlane();

 vtkMRMLMarkupsTestLineNode* liverMarkupsTestLineNode=
   vtkMRMLMarkupsTestLineNode::SafeDownCast(this->GetMarkupsNode());

 if (!liverMarkupsTestLineNode)
   {
   return;
   }

 this->TargetOrgan = liverMarkupsTestLineNode->GetTargetOrgan();
 this->Cutter->SetInputData(this->TargetOrgan);
}

//------------------------------------------------------------------------------
void vtkSlicerTestLineRepresentation3D::BuildMiddlePoint()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  if (markupsNode->GetNumberOfControlPoints() != 2)
    {
    return;
    }

  double p1[3] = { 0.0 };
  double p2[3] = { 0.0 };
  double center[3] = {0.0};
  markupsNode->GetNthControlPointPositionWorld(0, p1);
  markupsNode->GetNthControlPointPositionWorld(1, p2);
  center[0] = (p1[0] + p2[0]) / 2.0;
  center[1] = (p1[1] + p2[1]) / 2.0;
  center[2] = (p1[2] + p2[2]) / 2.0;

  this->MiddlePointSource->SetCenter(center);
  this->MiddlePointSource->SetRadius(this->ControlPointSize);
}

//------------------------------------------------------------------------------
void vtkSlicerTestLineRepresentation3D::BuildSlicingPlane()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  if (markupsNode->GetNumberOfControlPoints() != 2)
    {
    return;
    }

  double p1[3] = { 0.0 };
  double p2[3] = { 0.0 };
  double origin[3] = {0.0};
  double normal[3] = {0.0};

  markupsNode->GetNthControlPointPositionWorld(0, p1);
  markupsNode->GetNthControlPointPositionWorld(1, p2);

  origin[0] = (p1[0] + p2[0]) / 2.0;
  origin[1] = (p1[1] + p2[1]) / 2.0;
  origin[2] = (p1[2] + p2[2]) / 2.0;

  normal[0] = p2[0] - p1[0];
  normal[1] = p2[1] - p1[1];
  normal[2] = p2[2] - p1[2];

  this->SlicingPlane->SetOrigin(origin);
  this->SlicingPlane->SetNormal(normal);
}
