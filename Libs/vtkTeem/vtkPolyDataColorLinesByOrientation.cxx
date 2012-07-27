/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkPolyDataColorLinesByOrientation.cxx,v $

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPolyDataColorLinesByOrientation.h"

#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"

#include "vtkDiffusionTensorMathematics.h"
#include <math.h>



vtkCxxRevisionMacro(vtkPolyDataColorLinesByOrientation, "$Revision: 1.41 $");
vtkStandardNewMacro(vtkPolyDataColorLinesByOrientation);

// Construct with lower threshold=0, upper threshold=1, and threshold 
// function=upper.
vtkPolyDataColorLinesByOrientation::vtkPolyDataColorLinesByOrientation()
{
  this->ScalarArrayName = NULL;
  this->SetScalarArrayName("MeanOrientation");
  this->CopyOriginalDataOff();
  this->ColorMode = vtkPolyDataColorLinesByOrientation::colorModeMeanFiberOrientation;
}

vtkPolyDataColorLinesByOrientation::~vtkPolyDataColorLinesByOrientation()
{
  if (this->ScalarArrayName)
  {
    delete [] this->ScalarArrayName;
  }
}


int vtkPolyDataColorLinesByOrientation::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints *points, *newPoints;
  vtkPointData *pd, *outPD;
  vtkCellData *cd, *outCD;

  vtkIdType ptId, numPts, *pts;
  double x[3], point[3], lastPoint[3], displacement[3];
  double color;

  vtkDebugMacro(<< "Executing color cells by mean orientation");

  points = input->GetPoints();
  pd = input->GetPointData();
  cd = input->GetCellData();
  numPts = input->GetNumberOfPoints();

  outPD = output->GetPointData();
  outCD = output->GetCellData();

  vtkCellArray *sourceCells, *cells;  
  vtkCell *cell;
  vtkIdType cellId;
  vtkIdList *cellPts;
  int npts;

  if (this->CopyOriginalData)
  {
    outPD->DeepCopy(pd);
    outCD->DeepCopy(cd);
  }

  if ( (sourceCells=input->GetVerts())->GetNumberOfCells() > 0 )
  {
    cells = vtkCellArray::New();
    cells->Allocate(sourceCells->GetSize());
    output->SetVerts(cells);
    cells->Delete();
  }
  if ( (sourceCells=input->GetLines())->GetNumberOfCells() > 0 )
  { 
    cells = vtkCellArray::New();
    cells->Allocate(sourceCells->GetSize());
    output->SetLines(cells);
    cells->Delete();
  }
  if ((sourceCells=input->GetPolys())->GetNumberOfCells() > 0 )
  {
    cells = vtkCellArray::New();
    cells->Allocate(sourceCells->GetSize());
    output->SetPolys(cells);
    cells->Delete();
  }
  if ( (sourceCells=input->GetStrips())->GetNumberOfCells() > 0 )
  {
    cells = vtkCellArray::New();
    cells->Allocate(sourceCells->GetSize());
    output->SetStrips(cells);
    cells->Delete();
  }


  int i;
  int numSourceCells = input->GetNumberOfCells();

  pts = new vtkIdType[input->GetMaxCellSize()];

  // copy topology

  vtkFloatArray *cellColor = vtkFloatArray::New() ;
  cellColor->SetNumberOfComponents(1);
  cellColor->SetNumberOfTuples(numSourceCells);
  cellColor->SetName(this->ScalarArrayName);

  vtkFloatArray *pointMeanFiberColor = vtkFloatArray::New() ;
  pointMeanFiberColor->SetNumberOfComponents(1);
  pointMeanFiberColor->SetNumberOfTuples(numPts);
  pointMeanFiberColor->SetName(this->ScalarArrayName);

  // Check that the scalars of each point satisfy the threshold criterion
  int abort=0;
  // Check that the scalars of each point satisfy the threshold criterion
  vtkIdType progressInterval = numPts/20+1;
  vtkIdType ptOffset = 0;

  newPoints = vtkPoints::New();  
  for (ptId=0; ptId < numPts && !abort; ptId++)
  {
    if ( !(ptId % progressInterval) )
    {
      this->UpdateProgress((double)ptId/numPts);
      abort = this->GetAbortExecute();
    }


    input->GetPoint(ptId, x);
    newPoints->InsertNextPoint(x);

    ptOffset++;

  } // for all points
  output->SetPoints(newPoints);
  newPoints->Delete();

  progressInterval = 1;
  for (cellId=0; cellId < numSourceCells && !abort; cellId++)
  {
    if ( !(cellId % progressInterval) )
    {
      this->UpdateProgress((double)cellId/numSourceCells);
      abort = this->GetAbortExecute();
    }

    cell = input->GetCell(cellId);
    cellPts = cell->GetPointIds();
    npts = cellPts->GetNumberOfIds();
    x[0] = x[1] = x[2] = 0.;

    if (npts > 0)
    {
      pts[0] = cellPts->GetId(0);
      points->GetPoint(pts[0], lastPoint);

      for (i=1; i < npts; i++)
      {
        pts[i] = cellPts->GetId(i);
        points->GetPoint(pts[i], point);

        displacement[0] = fabs(point[0]-lastPoint[0]);
        displacement[1] = fabs(point[1]-lastPoint[1]);
        displacement[2] = fabs(point[2]-lastPoint[2]);

        const double norm = sqrt(
            displacement[0] * displacement[0] +
            displacement[1] * displacement[1] +
            displacement[2] * displacement[2]
            );

        displacement[0] /= norm;
        displacement[1] /= norm;
        displacement[2] /= norm;

        if (this->ColorMode == vtkPolyDataColorLinesByOrientation::colorModePointFiberOrientation)
        {
          vtkDiffusionTensorMathematics::RGBToIndex(displacement[0], displacement[1], displacement[2], color);
          pointMeanFiberColor->SetTuple(pts[i], &color);
        } else if (this->ColorMode == vtkPolyDataColorLinesByOrientation::colorModeMeanFiberOrientation)
        {
          x[0] += displacement[0];
          x[1] += displacement[1];
          x[2] += displacement[2];
        }


        lastPoint[0] = point[0];
        lastPoint[1] = point[1];
        lastPoint[2] = point[2];
      }

      if (this->ColorMode == vtkPolyDataColorLinesByOrientation::colorModePointFiberOrientation)
      {
        if (npts > 1)
        {
          pointMeanFiberColor->GetTuple(pts[1], &color);
          pointMeanFiberColor->SetTuple(pts[0], &color);
        }
      } else {
        x[0] = x[0] / npts;
        x[1] = x[1] / npts;
        x[2] = x[2] / npts;
      }
    }
  

    if (this->ColorMode == vtkPolyDataColorLinesByOrientation::colorModeMeanFiberOrientation)
    { 
      vtkDiffusionTensorMathematics::RGBToIndex(x[0],x[1],x[2],color);
      for (i=0; i < npts; i++)
      {
        pointMeanFiberColor->SetTuple(pts[i], &color);
      }
      cellColor->InsertTuple(cellId, &color);
    }

    output->InsertNextCell(cell->GetCellType(),npts,pts);

  }

  int idx = outPD->AddArray(pointMeanFiberColor);
  if (idx >= 0)
    outPD->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
  pointMeanFiberColor->Delete();

  if (this->ColorMode == vtkPolyDataColorLinesByOrientation::colorModeMeanFiberOrientation)
  { 
    idx = outCD->AddArray(cellColor);
    if (idx >= 0)
      outCD->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
    cellColor->Delete();
  }

  output->Squeeze();
  delete [] pts;

  return 1;
}

int vtkPolyDataColorLinesByOrientation::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

void vtkPolyDataColorLinesByOrientation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
