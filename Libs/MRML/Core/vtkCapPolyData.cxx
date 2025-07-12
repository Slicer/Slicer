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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/

// vtkAddon includes
#include "vtkCapPolyData.h"

// VTK includes
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkClipPolyData.h>
#include <vtkCutter.h>
#include <vtkContourTriangulator.h>
#include <vtkFloatArray.h>
#include <vtkGeneralTransform.h>
#include <vtkImplicitBoolean.h>
#include <vtkImplicitFunctionCollection.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkPlaneCollection.h>
#include <vtkPlanes.h>
#include <vtkPointData.h>
#include <vtkPolygon.h>
#include <vtkReverseSense.h>

vtkStandardNewMacro(vtkCapPolyData);

//----------------------------------------------------------------------------
vtkCapPolyData::vtkCapPolyData() = default;

//----------------------------------------------------------------------------
vtkCapPolyData::~vtkCapPolyData() = default;

//----------------------------------------------------------------------------
void vtkCapPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Clip Function:" << std::endl;
  if (this->ClipFunction)
  {
    this->ClipFunction->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << indent.GetNextIndent() << "(none)" << std::endl;
  }
  os << indent << "Generate Outline: " << (this->GenerateOutline ? "On" : "Off") << "\n";
  os << indent << "Generate Cell Type Scalars: " << (this->GenerateCellTypeScalars ? "On" : "Off") << "\n";
}

//----------------------------------------------------------------------------
void vtkCapPolyData::GetPlanes(vtkImplicitFunction* function, vtkPlaneCollection* planes, vtkAbstractTransform* parentTransform /*=nullptr*/)
{
  if (!function || !planes)
  {
    return;
  }

  vtkSmartPointer<vtkAbstractTransform> transform = nullptr;
  vtkAbstractTransform* functionTransform = function->GetTransform();
  if (functionTransform && parentTransform)
  {
    vtkSmartPointer<vtkGeneralTransform> generalTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    generalTransform->Concatenate(parentTransform);
    generalTransform->Concatenate(functionTransform);
    transform = generalTransform;
  }
  else if (parentTransform)
  {
    transform = parentTransform;
  }
  else if (functionTransform)
  {
    transform = functionTransform;
  }

  vtkImplicitBoolean* booleanFunction = vtkImplicitBoolean::SafeDownCast(function);
  if (booleanFunction)
  {
    vtkImplicitFunctionCollection* functions = booleanFunction->GetFunction();
    for (int i = 0; i < functions->GetNumberOfItems(); ++i)
    {
      vtkImplicitFunction* function = vtkImplicitFunction::SafeDownCast(functions->GetItemAsObject(i));
      vtkCapPolyData::GetPlanes(function, planes, transform);
    }
    return;
  }

  vtkPlanes* planesFunction = vtkPlanes::SafeDownCast(function);
  if (planesFunction)
  {
    for (int i = 0; i < planesFunction->GetNumberOfPlanes(); ++i)
    {
      vtkCapPolyData::GetPlanes(planesFunction->GetPlane(i), planes, transform);
    }
    return;
  }

  vtkPlane* plane = vtkPlane::SafeDownCast(function);
  if (plane)
  {
    double planeOrigin[3] = { 0.0, 0.0, 0.0 };
    plane->GetOrigin(planeOrigin);
    double planeNormal[3] = { 0.0, 0.0, 0.0 };
    plane->GetNormal(planeNormal);
    if (transform)
    {
      // vtkCutter with vtkPlane doesn't take transform into account.
      // Need to apply the transform to the plane manually.
      vtkAbstractTransform* inverseTransform = transform->GetInverse();
      inverseTransform->TransformPoint(planeOrigin, planeOrigin);
      inverseTransform->TransformNormalAtPoint(planeOrigin, planeNormal, planeNormal);
    }

    vtkNew<vtkPlane> newPlane;
    newPlane->SetNormal(planeNormal);
    newPlane->SetOrigin(planeOrigin);
    planes->AddItem(newPlane);
  }
}

//----------------------------------------------------------------------------
void vtkCapPolyData::CreateEndCap(vtkPlaneCollection* planes, vtkPolyData* originalPolyData, vtkImplicitFunction* cutFunction, vtkPolyData* outputEndCap)
{
  vtkNew<vtkAppendPolyData> appendFilter;
  for (int i = 0; i < planes->GetNumberOfItems(); ++i)
  {
    vtkPlane* plane = planes->GetItem(i);

    vtkNew<vtkCutter> cutter;
    cutter->SetCutFunction(plane);
    cutter->SetInputData(originalPolyData);

    vtkNew<vtkContourTriangulator> contourTriangulator;
    contourTriangulator->SetInputConnection(cutter->GetOutputPort());
    contourTriangulator->Update();

    vtkNew<vtkPolyData> endCapPolyData;

    if (this->GenerateOutline)
    {
      vtkNew<vtkAppendPolyData> append;
      append->AddInputData(contourTriangulator->GetOutput());
      append->AddInputData(cutter->GetOutput());
      append->SetOutput(endCapPolyData);
      append->Update();
    }
    else
    {
      endCapPolyData->ShallowCopy(contourTriangulator->GetOutput());
    }

    // Create a seam along the intersection of each plane with the triangulated contour.
    // This allows the contour to be split correctly later.
    for (int j = 0; j < planes->GetNumberOfItems(); ++j)
    {
      if (i == j)
      {
        continue;
      }
      vtkPlane* plane2 = planes->GetItem(j);
      vtkNew<vtkClipPolyData> clipper;
      clipper->SetInputData(endCapPolyData);
      clipper->SetClipFunction(plane2);
      clipper->SetValue(0.0);
      clipper->GenerateClippedOutputOn();
      vtkNew<vtkAppendPolyData> appendCut;
      appendCut->AddInputConnection(clipper->GetOutputPort());
      appendCut->AddInputConnection(clipper->GetClippedOutputPort());
      appendCut->Update();
      endCapPolyData->ShallowCopy(appendCut->GetOutput());
    }

    // Remove all triangles that do not lie at 0.0.
    double epsilon = 1e-4;
    vtkNew<vtkClipPolyData> clipper;
    clipper->SetInputData(endCapPolyData);
    clipper->SetClipFunction(cutFunction);
    clipper->InsideOutOff();
    clipper->SetValue(-epsilon);
    vtkNew<vtkClipPolyData> clipper2;
    clipper2->SetInputConnection(clipper->GetOutputPort());
    clipper2->SetClipFunction(cutFunction);
    clipper2->InsideOutOn();
    clipper2->SetValue(epsilon);
    clipper2->Update();
    endCapPolyData->ShallowCopy(clipper2->GetOutput());

    double planeNormal[3] = { 0.0 };
    plane->GetNormal(planeNormal);

    vtkCellArray* endCapPolys = endCapPolyData->GetPolys();
    if (endCapPolys && endCapPolyData->GetNumberOfPolys() > 0)
    {
      vtkNew<vtkIdList> polyPointIds;
      endCapPolys->GetCell(0, polyPointIds);
      double polyNormal[3] = { 0.0 };

      // The normal of the triangles generated by vtkContourTriangulator are based on the clockwise/counter-clockwise direction of the largest contour.
      // This normal will not always line up with the desired normal as defined by the plane, so if the normal generated by vtkContourTriangulator faces
      // the wrong  direction, then we need to flip the normals of the polys so that it matches the expected normal.
      vtkPolygon::ComputeNormal(endCapPolyData->GetPoints(), polyPointIds->GetNumberOfIds(), polyPointIds->GetPointer(0), polyNormal);
      if (vtkMath::Dot(polyNormal, planeNormal) < 0.0)
      {
        vtkNew<vtkReverseSense> reverseSense;
        reverseSense->SetInputData(endCapPolyData);
        reverseSense->ReverseCellsOn();
        reverseSense->Update();
        endCapPolyData->ShallowCopy(reverseSense->GetOutput());
      }
    }

    vtkNew<vtkFloatArray> normals;
    normals->SetName("Normals");
    normals->SetNumberOfComponents(3);
    normals->SetNumberOfTuples(endCapPolyData->GetNumberOfPoints());
    for (int i = 0; i < endCapPolyData->GetNumberOfPoints(); ++i)
    {
      normals->SetTuple3(i, planeNormal[0], planeNormal[1], planeNormal[2]);
    }
    endCapPolyData->GetPointData()->SetNormals(normals);
    appendFilter->AddInputData(endCapPolyData);
  }

  appendFilter->Update();
  outputEndCap->ShallowCopy(appendFilter->GetOutput());

  if (this->GenerateCellTypeScalars)
  {
    this->UpdateCellTypeArray(outputEndCap);
  }
}

//------------------------------------------------------------------------------
void vtkCapPolyData::UpdateCellTypeArray(vtkPolyData* polyData)
{
  if (!polyData || !polyData->GetCellData())
  {
    return;
  }

  vtkSmartPointer<vtkIdTypeArray> cellTypes = vtkIdTypeArray::SafeDownCast(polyData->GetCellData()->GetArray("CellType"));
  if (!cellTypes)
  {
    cellTypes = vtkSmartPointer<vtkIdTypeArray>::New();
  }

  cellTypes->SetNumberOfComponents(1);
  cellTypes->SetNumberOfTuples(polyData->GetNumberOfCells());
  for (vtkIdType i = 0; i < polyData->GetNumberOfCells(); ++i)
  {
    cellTypes->SetValue(i, polyData->GetCellType(i));
  }
  cellTypes->SetName("CellType");
  polyData->GetCellData()->AddArray(cellTypes);
  polyData->GetCellData()->SetActiveScalars("CellType");
}

//------------------------------------------------------------------------------
int vtkCapPolyData::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData* input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDebugMacro(<< "Clipping polygonal data");

  vtkIdType numPts = input->GetNumberOfPoints();
  vtkPoints* inPts = input->GetPoints();
  if (numPts < 1 || inPts == nullptr)
  {
    vtkDebugMacro(<< "No data to clip");
    return 1;
  }
  if (!this->ClipFunction)
  {
    return 1;
  }

  vtkNew<vtkPlaneCollection> planes;
  vtkCapPolyData::GetPlanes(this->ClipFunction, planes);
  vtkCapPolyData::CreateEndCap(planes, input, this->ClipFunction, output);

  return 1;
}

//------------------------------------------------------------------------------
// Overload standard modified time function. If Clip functions is modified,
// then this object is modified as well.
vtkMTimeType vtkCapPolyData::GetMTime()
{
  vtkMTimeType mTime = this->Superclass::GetMTime();
  vtkMTimeType time;

  if (this->ClipFunction != nullptr)
  {
    time = this->ClipFunction->GetMTime();
    mTime = (time > mTime ? time : mTime);
  }
  return mTime;
}
