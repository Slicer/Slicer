/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/

#include "vtkTransformVisualizerGlyph3D.h"

// STD includes
#include <vector>

// VTK includes
#include <vtkGlyph3D.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCell.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkTransform.h>

static const int ARRAY_INDEX_SCALARS=0;
static const int ARRAY_INDEX_VECTORS=1;
// static const int ARRAY_INDEX_NORMALS=2; // unused
static const int ARRAY_INDEX_COLORS=3;

vtkStandardNewMacro(vtkTransformVisualizerGlyph3D);

//------------------------------------------------------------------------------
vtkTransformVisualizerGlyph3D::vtkTransformVisualizerGlyph3D()
{
  this->ScaleDirectional = true;

  this->MagnitudeThresholding = false;
  this->MagnitudeThresholdLower = 0.0;
  this->MagnitudeThresholdUpper = 100.0;
}

//------------------------------------------------------------------------------
int vtkTransformVisualizerGlyph3D::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkDataSet *input = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPointData* outputPD = output->GetPointData();

  vtkDataArray *inVectors = this->GetInputArrayToProcess(ARRAY_INDEX_VECTORS,inputVector);
  vtkDataArray *inCScalars = this->GetInputArrayToProcess(ARRAY_INDEX_COLORS,inputVector);; // Scalars for Coloring
  if (inCScalars == nullptr)
    {
    vtkDataArray *inSScalars = this->GetInputArrayToProcess(ARRAY_INDEX_SCALARS,inputVector); // Scalars for Scaling
    inCScalars = inSScalars;
    }

  vtkIdType numPts = input->GetNumberOfPoints();
  if (numPts < 1)
    {
    vtkDebugMacro("No points to glyph!");
    return 1;
    }

  if ( inCScalars==nullptr )
    {
    vtkDebugMacro("No scalar data is available!");
    return 1;
    }

  // Allocate storage for output PolyData
  outputPD->CopyVectorsOff();
  outputPD->CopyNormalsOff();
  outputPD->CopyTCoordsOff();

  vtkPolyData *source = this->GetSource(0, inputVector[1]);
  vtkPoints *sourcePts = source->GetPoints();
  vtkIdType numSourcePts = sourcePts->GetNumberOfPoints();
  vtkIdType numSourceCells = source->GetNumberOfCells();

  // Prepare to copy output.
  vtkPointData *pd = input->GetPointData();
  outputPD->CopyAllocate(pd,numPts*numSourcePts);

  vtkNew<vtkPoints> newPts;
  newPts->Allocate(numPts*numSourcePts);

  vtkSmartPointer<vtkDataArray> newScalars=vtkSmartPointer<vtkDataArray>::Take(inCScalars->NewInstance());
  newScalars->SetNumberOfComponents(inCScalars->GetNumberOfComponents());
  newScalars->Allocate(inCScalars->GetNumberOfComponents()*numPts*numSourcePts);
  newScalars->SetName(inCScalars->GetName());

  // Setting up for calls to PolyData::InsertNextCell()
  output->Allocate(source, 3*numPts*numSourceCells, numPts*numSourceCells);

  vtkSmartPointer<vtkPoints> transformedSourcePts = vtkSmartPointer<vtkPoints>::New();
  transformedSourcePts->SetDataTypeToDouble();
  transformedSourcePts->Allocate(numSourcePts);

  vtkNew<vtkTransform> trans;

  vtkNew<vtkIdList> pts;
  pts->Allocate(VTK_CELL_SIZE);

  // Traverse all Input points, transforming Source points and copying
  // point attributes.
  vtkIdType ptIncr=0;
  vtkIdType cellIncr=0;
  double v[3] = {0};
  for (vtkIdType inPtId=0; inPtId < numPts; inPtId++)
    {
    if ( ! (inPtId % 10000) )
      {
      this->UpdateProgress(static_cast<double>(inPtId)/numPts);
      if (this->GetAbortExecute())
        {
        break;
        }
      }

    // Get the scalar and vector data
    double scalarValue = inCScalars->GetComponent(inPtId, 0);
    if (this->MagnitudeThresholding && (scalarValue<this->MagnitudeThresholdLower || scalarValue>this->MagnitudeThresholdUpper))
      {
      continue;
      }

    inVectors->GetTuple(inPtId, v);
    double vMag = vtkMath::Norm(v);

    // Now begin copying/transforming glyph
    trans->Identity();

    // Copy all topology (transformation independent)
    for (vtkIdType cellId=0; cellId < numSourceCells; cellId++)
      {
      vtkCell *cell = source->GetCell(cellId);
      vtkIdList *cellPts = cell->GetPointIds();
      int npts = cellPts->GetNumberOfIds();
      pts->Reset();
      for (vtkIdType i=0; i < npts; i++)
        {
        pts->InsertId(i,cellPts->GetId(i) + ptIncr);
        }
      output->InsertNextCell(cell->GetCellType(),pts.GetPointer());
      }

    // translate Source to Input point
    trans->Translate(input->GetPoint(inPtId));

    // Orient the glyph
    if (this->Orient && (vMag > 0.0))
      {
      // if there is no y or z component
      if ( v[1] == 0.0 && v[2] == 0.0 )
        {
        if (v[0] < 0) //just flip x if we need to
          {
          trans->RotateWXYZ(180.0,0,1,0);
          }
        }
      else
        {
        trans->RotateWXYZ(180.0, (v[0]+vMag)/2.0, v[1]/2.0, v[2]/2.0);
        }
      }

    // Scale data if appropriate
    if ( this->Scaling )
      {
      if ( this->ScaleDirectional )
        {
        double scale = vMag * this->ScaleFactor;
        if ( scale == 0.0 )
          {
          scale = 1.0e-10;
          }
        trans->Scale(scale,1.0,1.0);
        }
      else
        {
        double scale = this->ScaleFactor;
        if (this->ScaleMode==VTK_SCALE_BY_SCALAR)
          {
          scale *= scalarValue;
          }
        else
          {
          scale *= vMag;
          }
        if ( scale == 0.0 )
          {
          scale = 1.0e-10;
          }
        trans->Scale(scale,scale,scale);
        }
      }

    // Multiply points and normals by resulting matrix
    if (this->SourceTransform)
      {
      transformedSourcePts->Reset();
      this->SourceTransform->TransformPoints(sourcePts, transformedSourcePts);
      trans->TransformPoints(transformedSourcePts, newPts.GetPointer());
      }
    else
      {
      trans->TransformPoints(sourcePts,newPts.GetPointer());
      }

    // Copy point data from source
    for (vtkIdType i=0; i < numSourcePts; i++)
      {
      outputPD->CopyTuple(inCScalars, newScalars, inPtId, ptIncr+i);
      }

    ptIncr += numSourcePts;
    cellIncr += numSourceCells;
    }

  // Update ourselves and release memory
  output->SetPoints(newPts.GetPointer());

  if (newScalars)
    {
    int idx = outputPD->AddArray(newScalars);
    outputPD->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
    }

  output->Squeeze();

  return 1;
}

//------------------------------------------------------------------------------
void vtkTransformVisualizerGlyph3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//------------------------------------------------------------------------------
void vtkTransformVisualizerGlyph3D::SetColorArray(const char* colorArrayName)
{
  this->SetInputArrayToProcess(ARRAY_INDEX_COLORS,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,colorArrayName);
}

//------------------------------------------------------------------------------
void vtkTransformVisualizerGlyph3D::SetVectorArray(const char* vectorArrayName)
{
  this->SetInputArrayToProcess(ARRAY_INDEX_VECTORS,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,vectorArrayName);
}

//------------------------------------------------------------------------------
void vtkTransformVisualizerGlyph3D::SetScalarArray(const char* scalarArrayName)
{
  this->SetInputArrayToProcess(ARRAY_INDEX_SCALARS,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,scalarArrayName);
}
