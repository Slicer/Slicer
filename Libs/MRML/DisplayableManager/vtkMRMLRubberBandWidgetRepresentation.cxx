/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "vtkMRMLRubberBandWidgetRepresentation.h"

#include "vtkActor2D.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPropCollection.h"
#include "vtkProperty2D.h"

vtkStandardNewMacro(vtkMRMLRubberBandWidgetRepresentation);

class vtkMRMLRubberBandWidgetRepresentation::vtkInternal
{
public:
  vtkInternal(vtkMRMLRubberBandWidgetRepresentation * external);
  ~vtkInternal();

  static int IntersectWithFinitePlane(double n[3], double o[3], double pOrigin[3], double px[3], double py[3], double x0[3], double x1[3]);

  vtkMRMLRubberBandWidgetRepresentation* External;

  vtkSmartPointer<vtkPolyData> PolyData;

  vtkSmartPointer<vtkPolyDataMapper2D> ShadowMapper;
  vtkSmartPointer<vtkProperty2D> ShadowProperty;
  vtkSmartPointer<vtkActor2D> ShadowActor;

  vtkSmartPointer<vtkPolyDataMapper2D> Mapper;
  vtkSmartPointer<vtkProperty2D> Property;
  vtkSmartPointer<vtkActor2D> Actor;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLRubberBandWidgetRepresentation::vtkInternal
::vtkInternal(vtkMRMLRubberBandWidgetRepresentation * external)
{
  this->External = external;

  this->ShadowMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->ShadowActor = vtkSmartPointer<vtkActor2D>::New();

  this->Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->Actor = vtkSmartPointer<vtkActor2D>::New();

  this->PolyData = vtkSmartPointer<vtkPolyData>::New();

  vtkNew<vtkPoints> points;
  vtkNew<vtkCellArray> lines;
  this->PolyData->SetPoints(points);
  this->PolyData->SetLines(lines);

  for (int i = 0; i < 4; i++)
    {
    points->InsertNextPoint(0.0, 0.0, 0.0);
    vtkNew<vtkIdList> idList;
    idList->InsertNextId(i);
    idList->InsertNextId((i+1) % 4);
    this->PolyData->InsertNextCell(VTK_LINE, idList);
    }

  this->ShadowMapper->SetInputData(this->PolyData);
  this->ShadowActor->SetMapper(this->ShadowMapper);
  this->ShadowProperty = vtkSmartPointer<vtkProperty2D>::New();
  this->ShadowProperty->SetColor(0.1, 0.1, 0.0);
  this->ShadowProperty->SetLineWidth(3);
  this->ShadowActor->SetProperty(this->ShadowProperty);

  this->Mapper->SetInputData(this->PolyData);
  this->Actor->SetMapper(this->Mapper);
  this->Property = vtkSmartPointer<vtkProperty2D>::New();
  this->Property->SetColor(1.0, 1.0, 0);
  this->Property->SetLineWidth(1);
  this->Actor->SetProperty(this->Property);
}

//---------------------------------------------------------------------------
vtkMRMLRubberBandWidgetRepresentation::vtkInternal::~vtkInternal() = default;

//----------------------------------------------------------------------
vtkMRMLRubberBandWidgetRepresentation::vtkMRMLRubberBandWidgetRepresentation()
{
  this->CornerPoint1[0] = 0;
  this->CornerPoint1[1] = 0;
  this->CornerPoint2[0] = 0;
  this->CornerPoint2[1] = 0;
  this->Internal = new vtkInternal(this);
}

//----------------------------------------------------------------------
vtkMRMLRubberBandWidgetRepresentation::~vtkMRMLRubberBandWidgetRepresentation()
{
  delete this->Internal;
}

//----------------------------------------------------------------------
void vtkMRMLRubberBandWidgetRepresentation::GetActors2D(vtkPropCollection *pc)
{
  pc->AddItem(this->Internal->ShadowActor);
  pc->AddItem(this->Internal->Actor);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkMRMLRubberBandWidgetRepresentation::ReleaseGraphicsResources(vtkWindow *win)
{
  this->Internal->Actor->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkMRMLRubberBandWidgetRepresentation::RenderOverlay(vtkViewport *viewport)
{
  vtkPoints* points = this->Internal->PolyData->GetPoints();
  if (this->GetMTime() > points->GetMTime())
    {
    points->SetPoint(0, this->CornerPoint1[0], this->CornerPoint1[1], 0.0);
    points->SetPoint(1, this->CornerPoint2[0], this->CornerPoint1[1], 0.0);
    points->SetPoint(2, this->CornerPoint2[0], this->CornerPoint2[1], 0.0);
    points->SetPoint(3, this->CornerPoint1[0], this->CornerPoint2[1], 0.0);
    points->Modified();
    }
  int count = 0;
  if (this->Internal->Actor->GetVisibility())
    {
    count += this->Internal->ShadowActor->RenderOverlay(viewport);
    count += this->Internal->Actor->RenderOverlay(viewport);
    }
  count += this->Superclass::RenderOverlay(viewport);
  return count;
}

//----------------------------------------------------------------------
void vtkMRMLRubberBandWidgetRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
