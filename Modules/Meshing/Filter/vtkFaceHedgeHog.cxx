/*=========================================================================

  Program:   FaceHedgeHog
  Module:    $RCSfile: vtkFaceHedgeHog.cxx,v $

  Copyright KnowledgeVis,LLC
  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE. 

  Description:  Draw oriented vectors from the center of the faces of 
                            a polygonal object. 
                            
=========================================================================*/




#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkCell.h"
#include "vtkMath.h"

#include "vtkFaceHedgeHog.h"

vtkStandardNewMacro(vtkFaceHedgeHog);
vtkCxxRevisionMacro(vtkFaceHedgeHog, "$Revision: 1.1 $");

vtkFaceHedgeHog::vtkFaceHedgeHog()
{
  this->ScaleFactor = 1.0;
  this->VectorMode = VTK_USE_VECTOR;
}

int vtkFaceHedgeHog::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType numPts;
  vtkIdType numCells;
  vtkPoints *newPts;
  vtkPointData *pd;
  vtkDataArray *inVectors;
  vtkDataArray *inNormals;
  vtkIdType ptId;
  int i;
  vtkIdType pts[2];
  vtkCellArray *newLines;
  double x[3], v[3], center[3];
  double newX[3];
  vtkPointData *outputPD = output->GetPointData();
  
  // Initialize
  //
  numPts = input->GetNumberOfPoints();
  numCells = input->GetNumberOfCells();
  pd = input->GetPointData();
  inVectors = pd->GetVectors();
  
  if ( numPts < 1 )
    {
    vtkErrorMacro(<<"No input data");
    return 1;
    }
  if ( !inVectors && this->VectorMode == VTK_USE_VECTOR)
    {
    vtkErrorMacro(<<"No vectors in input data");
    return 1;
    }

  inNormals = pd->GetNormals();
  
  if ( !inNormals && this->VectorMode == VTK_USE_NORMAL)
    {
    vtkErrorMacro(<<"No normals in input data");
    return 1;
    }
  outputPD->Allocate(2*numCells);

  newPts = vtkPoints::New(); newPts->SetNumberOfPoints(2*numCells);
  newLines = vtkCellArray::New();
  newLines->Allocate(newLines->EstimateSize(numCells,2));

  // Loop over all cells, creating oriented line
  //
  for (vtkIdType cellId=0; cellId < numCells; cellId++)
    {
    if ( ! (cellId % 100) ) //abort/progress
      {
      this->UpdateProgress ((double)cellId/numCells);
      if (this->GetAbortExecute())
        {
        break;
        }
      }
    
      vtkCell *thisCell  = input->GetCell(cellId);
    
    // find center of cell by averaging the points, since origin of line
    // is the cell center. We scale the point contributions by 1/numPtsInCell
    // to avoid the number getting enlarged by the summation of multiple points
    
     vtkIdType numPtsInCell = thisCell->GetNumberOfPoints();
     center[0] = 0.0; center[1] = 0.0; center[2] = 0.0;
     double adjustForSum = 1.0/ (double)numPtsInCell;
     for ( ptId=0; ptId < numPtsInCell; ptId++)
       {
         center[0] += adjustForSum*(thisCell->GetPoints()->GetPoint(ptId))[0];
         center[1] += adjustForSum*(thisCell->GetPoints()->GetPoint(ptId))[1];
         center[2] += adjustForSum*(thisCell->GetPoints()->GetPoint(ptId))[2];
       }
    
    
    if (this->VectorMode == VTK_USE_VECTOR)
      {
        inVectors->GetTuple(cellId, v);
      }
    else
      {
        this->CalculateFaceNormal(thisCell, v);
      }
    for (i=0; i<3; i++)
      {
       // divide the center value by the number of points we added
       // to find the point at the far end of the normal vector.  Allow
       // a scaling factor 
       newX[i] = center[i] + this->ScaleFactor * v[i];
      }

    pts[0] = cellId;
    pts[1] = cellId + numCells;

    newPts->SetPoint(pts[0], center);
    newPts->SetPoint(pts[1], newX);

    newLines->InsertNextCell(2,pts);

    outputPD->CopyData(pd,cellId,pts[0]);
    outputPD->CopyData(pd,cellId,pts[1]);
    }

  // Update ourselves and release memory
  //
  output->SetPoints(newPts);
  newPts->Delete();

  output->SetLines(newLines);
  newLines->Delete();

  return 1;
}

int vtkFaceHedgeHog::CalculateFaceNormal(vtkCell* cell, double* normal)
{
        double v1[3], v2[3];
        v1[0] = cell->GetPoints()->GetPoint(0)[0] - cell->GetPoints()->GetPoint(1)[0];
        v1[1] = cell->GetPoints()->GetPoint(0)[1] - cell->GetPoints()->GetPoint(1)[1];
        v1[2] = cell->GetPoints()->GetPoint(0)[2] - cell->GetPoints()->GetPoint(1)[2];
        v2[0] = cell->GetPoints()->GetPoint(2)[0] - cell->GetPoints()->GetPoint(1)[0];
        v2[1] = cell->GetPoints()->GetPoint(2)[1] - cell->GetPoints()->GetPoint(1)[1];
        v2[2] = cell->GetPoints()->GetPoint(2)[2] - cell->GetPoints()->GetPoint(1)[2];
        vtkMath::Cross(v2,v1,normal);

        return 1;
}

int vtkFaceHedgeHog::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

void vtkFaceHedgeHog::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Scale Factor: " << this->ScaleFactor << "\n";
  os << indent << "Orient Mode: " << (this->VectorMode == VTK_USE_VECTOR ? 
                                       "Orient by vector\n" : "Orient by normal\n");
}
