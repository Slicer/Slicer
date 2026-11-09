/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// MRML includes
#include "vtkVectorFieldGrid.h"
#include "vtkVectorFieldSampler.h"

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkClipPolyData.h>
#include <vtkDataArray.h>
#include <vtkFieldData.h>
#include <vtkGeometryFilter.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkTubeFilter.h>
#include <vtkUnsignedCharArray.h>

// STD includes
#include <vector>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkVectorFieldGrid);

//----------------------------------------------------------------------------
vtkVectorFieldGrid::vtkVectorFieldGrid()
{
  this->SetNumberOfOutputPorts(2);
  this->Clipper->SetValue(0.5);
  this->Tuber->CappingOn();
  this->Tuber->SetNumberOfSides(8);
  // The active vectors of the point data, unless an array is selected by name
  this->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::VECTORS);
}

//----------------------------------------------------------------------------
vtkVectorFieldGrid::~vtkVectorFieldGrid() = default;

//----------------------------------------------------------------------------
void vtkVectorFieldGrid::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Subdivision: " << this->Subdivision << "\n";
  os << indent << "ScaleFactor: " << this->ScaleFactor << "\n";
  os << indent << "DeformedLinesVisibility: " << (this->DeformedLinesVisibility ? "On" : "Off") << "\n";
  os << indent << "UndeformedLinesVisibility: " << (this->UndeformedLinesVisibility ? "On" : "Off") << "\n";
  os << indent << "Fill: " << (this->Fill ? "On" : "Off") << "\n";
  os << indent << "LineDiameter: " << this->LineDiameter << "\n";
  os << indent << "ClipArrayName: " << this->ClipArrayName << "\n";
}

//----------------------------------------------------------------------------
void vtkVectorFieldGrid::SetClipArrayName(const std::string& name)
{
  if (this->ClipArrayName == name)
  {
    return;
  }
  this->ClipArrayName = name;
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkVectorFieldGrid::FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPointSet");
  return 1;
}

//----------------------------------------------------------------------------
void vtkVectorFieldGrid::BuildLatticeLines(vtkPointSet* input, vtkPolyData* output)
{
  int latticeSize[3] = { 0, 0, 0 };
  vtkIntArray* latticeSizeArray = vtkIntArray::SafeDownCast(input->GetFieldData()->GetArray(vtkVectorFieldSampler::GetLatticeSizeArrayName()));
  if (latticeSizeArray && latticeSizeArray->GetNumberOfValues() >= 3)
  {
    for (int axis = 0; axis < 3; ++axis)
    {
      latticeSize[axis] = latticeSizeArray->GetValue(axis);
    }
  }
  if (latticeSize[0] < 1 || latticeSize[1] < 1 || latticeSize[2] < 1)
  {
    // The samples are not arranged in a lattice, there is no grid to draw
    return;
  }

  output->SetPoints(input->GetPoints());
  output->GetPointData()->PassData(input->GetPointData());

  auto pointId = [&latticeSize](int i, int j, int k)
  { return static_cast<vtkIdType>(i) + static_cast<vtkIdType>(j) * latticeSize[0] + static_cast<vtkIdType>(k) * latticeSize[0] * latticeSize[1]; };

  // Grid lines stop where the field had no value and start again after it, instead of running undeformed
  // through the empty space inside the bounding box.
  vtkUnsignedCharArray* validSamples = vtkUnsignedCharArray::SafeDownCast(input->GetPointData()->GetArray(vtkVectorFieldSampler::GetValidSampleArrayName()));
  auto sampleIsValid = [validSamples](vtkIdType id) { return (!validSamples || (id < validSamples->GetNumberOfValues() && validSamples->GetValue(id) != 0)); };

  vtkNew<vtkCellArray> lines;
  int index[3] = { 0, 0, 0 };
  std::vector<vtkIdType> run;
  // A run of valid samples along a row is a polyline, if it has at least two
  auto endRun = [&lines, &run]()
  {
    if (run.size() > 1)
    {
      lines->InsertNextCell(static_cast<int>(run.size()), run.data());
    }
    run.clear();
  };
  // One polyline along each axis, for every Subdivision-th row of the two other axes
  for (int lineAxis = 0; lineAxis < 3; ++lineAxis)
  {
    if (latticeSize[lineAxis] < 2)
    {
      // A single sample along this axis is a point, not a line
      continue;
    }
    int firstOtherAxis = (lineAxis + 1) % 3;
    int secondOtherAxis = (lineAxis + 2) % 3;
    for (int second = 0; second < latticeSize[secondOtherAxis]; second += this->Subdivision)
    {
      for (int first = 0; first < latticeSize[firstOtherAxis]; first += this->Subdivision)
      {
        index[firstOtherAxis] = first;
        index[secondOtherAxis] = second;
        for (int along = 0; along < latticeSize[lineAxis]; ++along)
        {
          index[lineAxis] = along;
          vtkIdType id = pointId(index[0], index[1], index[2]);
          if (sampleIsValid(id))
          {
            run.push_back(id);
          }
          else
          {
            endRun();
          }
        }
        endRun();
      }
    }
  }
  output->SetLines(lines);
}

//----------------------------------------------------------------------------
void vtkVectorFieldGrid::DeformPoints(vtkPointSet* pointSet, vtkDataArray* vectors)
{
  vtkPoints* points = pointSet->GetPoints();
  if (!points || !vectors || vectors->GetNumberOfComponents() < 3 || vectors->GetNumberOfTuples() != points->GetNumberOfPoints())
  {
    return;
  }
  vtkNew<vtkPoints> deformedPoints;
  deformedPoints->SetDataType(points->GetDataType());
  vtkIdType numberOfPoints = points->GetNumberOfPoints();
  deformedPoints->SetNumberOfPoints(numberOfPoints);
  double point[3] = { 0.0, 0.0, 0.0 };
  double vector[3] = { 0.0, 0.0, 0.0 };
  for (vtkIdType pointIndex = 0; pointIndex < numberOfPoints; ++pointIndex)
  {
    points->GetPoint(pointIndex, point);
    for (int axis = 0; axis < 3; ++axis)
    {
      vector[axis] = vectors->GetComponent(pointIndex, axis);
    }
    deformedPoints->SetPoint(pointIndex, //
                             point[0] + this->ScaleFactor * vector[0],
                             point[1] + this->ScaleFactor * vector[1],
                             point[2] + this->ScaleFactor * vector[2]);
  }
  pointSet->SetPoints(deformedPoints);
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkVectorFieldGrid::Clip(vtkPolyData* polyData)
{
  vtkDataArray* clipArray = this->ClipArrayName.empty() ? nullptr : polyData->GetPointData()->GetArray(this->ClipArrayName.c_str());
  if (!clipArray)
  {
    return polyData;
  }
  vtkNew<vtkPolyData> clipInput;
  clipInput->ShallowCopy(polyData);
  clipInput->GetPointData()->SetActiveScalars(this->ClipArrayName.c_str());
  this->Clipper->SetInputData(clipInput);
  this->Clipper->Update();
  vtkSmartPointer<vtkPolyData> clipped = vtkSmartPointer<vtkPolyData>::New();
  clipped->ShallowCopy(this->Clipper->GetOutput());
  this->Clipper->SetInputData(nullptr);
  return clipped;
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkVectorFieldGrid::FinishLines(vtkPolyData* lines)
{
  vtkSmartPointer<vtkPolyData> finished = this->Clip(lines);
  if (this->LineDiameter > 0.0 && finished->GetNumberOfLines() > 0)
  {
    this->Tuber->SetInputData(finished);
    this->Tuber->SetRadius(0.5 * this->LineDiameter);
    this->Tuber->Update();
    finished = vtkSmartPointer<vtkPolyData>::New();
    finished->ShallowCopy(this->Tuber->GetOutput());
    this->Tuber->SetInputData(nullptr);
  }
  return finished;
}

//----------------------------------------------------------------------------
int vtkVectorFieldGrid::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkPointSet* input = vtkPointSet::GetData(inputVector[0]);
  vtkPolyData* output = vtkPolyData::GetData(outputVector, 0);
  vtkPolyData* undeformedLinesOutput = vtkPolyData::GetData(outputVector, UndeformedLinesPort);
  if (!input || !output || !undeformedLinesOutput)
  {
    return 0;
  }
  output->Initialize();
  undeformedLinesOutput->Initialize();

  vtkDataArray* vectors = this->GetInputArrayToProcess(0, inputVector);
  std::string vectorArrayName = (vectors && vectors->GetName()) ? vectors->GetName() : "";

  // Grid lines: the deformed ones, the undeformed ones, or both. Showing the undeformed grid
  // next to the deformed one makes the deformation easier to read.
  vtkSmartPointer<vtkPolyData> lines;
  if (this->DeformedLinesVisibility || this->UndeformedLinesVisibility)
  {
    vtkNew<vtkPolyData> latticeLines;
    this->BuildLatticeLines(input, latticeLines);
    if (this->DeformedLinesVisibility)
    {
      vtkNew<vtkPolyData> deformedLines;
      deformedLines->ShallowCopy(latticeLines);
      this->DeformPoints(deformedLines, vectors);
      lines = this->FinishLines(deformedLines);
    }
    if (this->UndeformedLinesVisibility)
    {
      undeformedLinesOutput->ShallowCopy(this->FinishLines(latticeLines));
    }
  }

  // The lattice surface (quads over a plane, outer cell faces over a region), deformed, cut, and colored
  // like the grid lines.
  vtkSmartPointer<vtkPolyData> fill;
  if (this->Fill && input->GetNumberOfCells() > 0)
  {
    this->FillSurface->SetInputData(input);
    this->FillSurface->Update();
    fill = vtkSmartPointer<vtkPolyData>::New();
    fill->ShallowCopy(this->FillSurface->GetOutput());
    this->FillSurface->SetInputData(nullptr);
    vtkDataArray* fillVectors = vectorArrayName.empty() ? nullptr : fill->GetPointData()->GetArray(vectorArrayName.c_str());
    this->DeformPoints(fill, fillVectors);
    fill = this->Clip(fill);
  }

  if (lines && fill)
  {
    this->OutputAppender->RemoveAllInputs();
    this->OutputAppender->AddInputData(lines);
    this->OutputAppender->AddInputData(fill);
    this->OutputAppender->Update();
    output->ShallowCopy(this->OutputAppender->GetOutput());
    this->OutputAppender->RemoveAllInputs();
  }
  else if (lines)
  {
    output->ShallowCopy(lines);
  }
  else if (fill)
  {
    output->ShallowCopy(fill);
  }
  return 1;
}
