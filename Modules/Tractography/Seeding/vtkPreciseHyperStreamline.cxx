/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkPreciseHyperStreamline.cxx,v $
  Date:      $Date: 2006/03/06 21:07:33 $
  Version:   $Revision: 1.10 $

=========================================================================auto=*/

#include <math.h>

#include "vtkPreciseHyperStreamline.h"

#include "vtkCellArray.h"
#include "vtkDataSet.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkTensorImplicitFunctionToFunctionSet.h"
#include "vtkPreciseHyperArray.h"
#include "vtkPreciseHyperPoint.h"
#include "vtkDiffusionTensorMathematics.h"


vtkCxxRevisionMacro(vtkPreciseHyperStreamline, "$Revision: 1.10 $");
vtkStandardNewMacro(vtkPreciseHyperStreamline);


#define VTK_START_FROM_POSITION 0
#define VTK_START_FROM_LOCATION 1

vtkPreciseHyperPoint::vtkPreciseHyperPoint()
{
  this->V[0] = this->V0;
  this->V[1] = this->V1;
  this->V[2] = this->V2;
  this->D = -1.0;
}

vtkPreciseHyperPoint& vtkPreciseHyperPoint::operator=(const vtkPreciseHyperPoint& hp)
{
  int i, j;

  for (i=0; i<3; i++) 
    {
      this->X[i] = hp.X[i];
      this->P[i] = hp.P[i];
      this->W[i] = hp.W[i];
      for (j=0; j<3; j++)
    {
      this->V[j][i] = hp.V[j][i];
    }
    }
  this->CellId = hp.CellId;
  this->SubId = hp.SubId;
  this->S = hp.S;
  this->D = hp.D;

  return *this;
}

vtkPreciseHyperArray::vtkPreciseHyperArray()
{
  this->MaxId = -1; 
  this->Array = new vtkPreciseHyperPoint[1000];
  this->Size = 1000;
  this->Extend = 5000;
  this->Direction = VTK_INTEGRATE_FORWARD;
  this->MaxAngleLastId = 0;
}

vtkFloatingPointType vtkPreciseHyperArray::CosineOfAngle(void) 
{
  vtkFloatingPointType vect[3];
  int i;
  vtkFloatingPointType length = 0.0;
  vtkFloatingPointType res=1.0;
  if ( this->MaxId > this->MaxAngleLastId ) 
    {
      for ( i = 0 ; i < 3 ; i++ ) 
    {
      vect[i] = this->Array[this->MaxId].X[i] - this->Array[this->MaxAngleLastId].X[i];
      length += vect[i]*vect[i];
    }
      if ( length > 0.0 )
    {
      length = sqrt(length);
      for ( i = 0 ; i < 3 ; i++ )
        {
          vect[i] = vect[i] / length;
        }
      length = 0.0;
      res = 0.0;
      for ( i = 0 ; i < 3 ; i++ ) 
        {
          res += vect[i] * ( this->Array[this->MaxAngleLastId+1].X[i] - this->Array[this->MaxAngleLastId].X[i] );
          length += ( this->Array[this->MaxAngleLastId+1].X[i] - this->Array[this->MaxAngleLastId].X[i] ) * ( this->Array[this->MaxAngleLastId+1].X[i] - this->Array[this->MaxAngleLastId].X[i] );
        }
      if ( length > 0.0 )
        res = res / sqrt(length);
      else
        res = 1.0;
    }
    }
  return res;
}

vtkPreciseHyperPoint *vtkPreciseHyperArray::Resize(vtkIdType sz)
{
  vtkPreciseHyperPoint *newArray;
  vtkIdType newSize, i;

  if (sz >= this->Size)
    {
      newSize = this->Size + 
    this->Extend*(((sz-this->Size)/this->Extend)+1);
    }
  else
    {
      newSize = sz;
    }

  newArray = new vtkPreciseHyperPoint[newSize];

  for (i=0; i<sz; i++)
    {
      newArray[i] = this->Array[i];
    }

  this->Size = newSize;
  delete [] this->Array;
  this->Array = newArray;

  return this->Array;
}

// Construct object with initial starting position (0,0,0); integration step 
// length 0.2; step length 0.01; forward integration; terminal eigenvalue 0.0;
// number of sides 6; radius 0.5; and logarithmic scaling off.
vtkPreciseHyperStreamline::vtkPreciseHyperStreamline()
{
  this->StartFrom = VTK_START_FROM_POSITION;
  this->StartPosition[0] = this->StartPosition[1] = this->StartPosition[2] = 0.0;

  this->StartCell = 0;
  this->StartSubId = 0;
  this->StartPCoords[0] = this->StartPCoords[1] = this->StartPCoords[2] = 0.5;

  this->Streamers = NULL;

  this->MaximumPropagationDistance = 100.0;
  this->MinimumPropagationDistance =0.0;
  this->IntegrationStepLength = 0.2;
  this->StepLength = 0.01;
  this->IntegrationDirection = VTK_INTEGRATE_FORWARD;
  this->TerminalEigenvalue = 0.0;
  this->TerminalFractionalAnisotropy = 0.0;
  this->RegularizationBias = 0.0;
  this->NumberOfSides = 6;
  this->Radius = 0.5;
  this->LogScaling = 0;
  this->method = 0;
  this->MinStep = 0.001;
  this->MaxStep = 1.0;
  this->MaxError = 0.000001;
  this->IntegrationEigenvector = VTK_INTEGRATE_MAJOR_EIGENVECTOR;
  this->MaxAngle = 0.866;
  this->LengthOfMaxAngle = 3; // length in mm
}

vtkPreciseHyperStreamline::~vtkPreciseHyperStreamline()
{
  if ( this->Streamers )
    {
      delete [] this->Streamers;
    }
}

// Specify the start of the hyperstreamline in the cell coordinate system. 
// That is, cellId and subId (if composite cell), and parametric coordinates.
void vtkPreciseHyperStreamline::SetStartLocation(vtkIdType cellId, int subId,
                         vtkFloatingPointType pcoords[3])
{
  if ( cellId != this->StartCell || subId != this->StartSubId ||
       pcoords[0] !=  this->StartPCoords[0] || 
       pcoords[1] !=  this->StartPCoords[1] || 
       pcoords[2] !=  this->StartPCoords[2] )
    {
      this->Modified();
      this->StartFrom = VTK_START_FROM_LOCATION;

      this->StartCell = cellId;
      this->StartSubId = subId;
      this->StartPCoords[0] = pcoords[0];
      this->StartPCoords[1] = pcoords[1];
      this->StartPCoords[2] = pcoords[2];
    }
}

// Specify the start of the hyperstreamline in the cell coordinate system. 
// That is, cellId and subId (if composite cell), and parametric coordinates.
void vtkPreciseHyperStreamline::SetStartLocation(vtkIdType cellId, int subId,
                         vtkFloatingPointType r, vtkFloatingPointType s, vtkFloatingPointType t)
{
  vtkFloatingPointType pcoords[3];
  pcoords[0] = r;
  pcoords[1] = s;
  pcoords[2] = t;

  this->SetStartLocation(cellId, subId, pcoords);
}

// Get the starting location of the hyperstreamline in the cell coordinate
// system. Returns the cell that the starting point is in.
vtkIdType vtkPreciseHyperStreamline::GetStartLocation(int& subId, vtkFloatingPointType pcoords[3])
{
  subId = this->StartSubId;
  pcoords[0] = this->StartPCoords[0];
  pcoords[1] = this->StartPCoords[1];
  pcoords[2] = this->StartPCoords[2];
  return this->StartCell;
}

// Specify the start of the hyperstreamline in the global coordinate system. 
// Starting from position implies that a search must be performed to find 
// initial cell to start integration from.
void vtkPreciseHyperStreamline::SetStartPosition(vtkFloatingPointType x[3])
{
  if ( x[0] != this->StartPosition[0] || x[1] != this->StartPosition[1] || 
       x[2] != this->StartPosition[2] )
    {
      this->Modified();
      this->StartFrom = VTK_START_FROM_POSITION;

      this->StartPosition[0] = x[0];
      this->StartPosition[1] = x[1];
      this->StartPosition[2] = x[2];
    }
}

// Specify the start of the hyperstreamline in the global coordinate system. 
// Starting from position implies that a search must be performed to find 
// initial cell to start integration from.
void vtkPreciseHyperStreamline::SetStartPosition(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z)
{
  vtkFloatingPointType pos[3];
  pos[0] = x;
  pos[1] = y;
  pos[2] = z;

  this->SetStartPosition(pos);
}

// Get the start position of the hyperstreamline in global x-y-z coordinates.
vtkFloatingPointType *vtkPreciseHyperStreamline::GetStartPosition()
{
  return this->StartPosition;
}

// Make sure coordinate systems are consistent
static void FixVectors(vtkFloatingPointType **prev, vtkFloatingPointType **current, int iv, int ix, int iy)
{
  vtkFloatingPointType p0[3], p1[3], p2[3];
  vtkFloatingPointType v0[3], v1[3], v2[3];
  vtkFloatingPointType temp[3];
  int i;

  for (i=0; i<3; i++)
    {
      v0[i] = current[i][iv];
      v1[i] = current[i][ix];
      v2[i] = current[i][iy];
    }

  if ( prev == NULL ) //make sure coord system is right handed
    {
      vtkMath::Cross(v0,v1,temp);
      if ( vtkMath::Dot(v2,temp) < 0.0 )
    {
      for (i=0; i<3; i++)
        {
          current[i][iy] *= -1.0;
        }
    }
    }

  else //make sure vectors consistent from one point to the next
    {
      for (i=0; i<3; i++)
    {
      p0[i] = prev[i][iv];
      p1[i] = prev[i][ix];
      p2[i] = prev[i][iy];
    }
      if ( vtkMath::Dot(p0,v0) < 0.0 )
    {
      for (i=0; i<3; i++)
        {
          current[i][iv] *= -1.0;
        }
    }
      if ( vtkMath::Dot(p1,v1) < 0.0 )
    {
      for (i=0; i<3; i++)
        {
          current[i][ix] *= -1.0;
        }
    }
      if ( vtkMath::Dot(p2,v2) < 0.0 )
    {
      for (i=0; i<3; i++)
        {
          current[i][iy] *= -1.0;
        }
    }
    }
}

void vtkPreciseHyperStreamline::Execute()
{
  vtkDataSet *input = this->GetInput();
  vtkPointData *pd=input->GetPointData();
  vtkDataArray *inScalars;
  vtkDataArray *inTensors;
  vtkPreciseHyperPoint *sNext, *sPtr;
  int i, ptId, iv, ix, iy;
  vtkFloatingPointType xNext[3];
  vtkCell *cell;
  vtkFloatingPointType tol2;
  vtkFloatingPointType d, step;
  vtkFloatingPointType *w;
  vtkFloatingPointType deitActual, error, dirStart[3];
  vtkFloatingPointType *m[3], *v[3];
  vtkFloatingPointType totalLength =0.0;
  //   float m0[3], m1[3], m2[3];
  //   float v0[3], v1[3], v2[3];
  vtkFloatingPointType m0[9];
  vtkFloatingPointType v0[9];
  vtkDataArray *cellTensors;
  vtkDataArray *cellScalars;
  // set up working matrices
  v[0] = v0; v[1] = &(v0[3]); v[2] = &(v0[6]); 
  m[0] = m0; m[1] = &(m0[3]); m[2] = &(m0[6]); 

  vtkDebugMacro(<<"Generating hyperstreamline(s)");
  this->NumberOfStreamers = 0;
  if ( ! (inTensors=pd->GetTensors()) )
    //   if ( ! (pd->GetTensors()) )
    {
      vtkErrorMacro(<<"No tensor data defined!");
      return;
    }
  w = new vtkFloatingPointType[input->GetMaxCellSize()];

  inScalars = pd->GetScalars();
  //inScalars = 0;
  cellTensors = vtkDataArray::CreateDataArray(inTensors->GetDataType());
  cellScalars = vtkDataArray::CreateDataArray(inScalars->GetDataType());
  int numComp;
  if (inTensors)
    {
      numComp = inTensors->GetNumberOfComponents();
      cellTensors->SetNumberOfComponents(numComp);
      cellTensors->SetNumberOfTuples(VTK_CELL_SIZE);
    }
  if (inScalars)
    {
      numComp = inScalars->GetNumberOfComponents();
      cellScalars->SetNumberOfComponents(numComp);
      cellScalars->SetNumberOfTuples(VTK_CELL_SIZE);
    }
  
  
  tol2 = input->GetLength() / 1000.0;
  tol2 = tol2 * tol2;

  iv = this->IntegrationEigenvector;
  ix = (iv + 1) % 3;
  iy = (iv + 2) % 3;
  //
  // Create starting points
  //
  this->NumberOfStreamers = 1;
 
  if ( this->IntegrationDirection == VTK_INTEGRATE_BOTH_DIRECTIONS )
    {
      this->NumberOfStreamers *= 2;
    }

  this->Streamers = new vtkPreciseHyperArray[this->NumberOfStreamers];
  if ( this->StartFrom == VTK_START_FROM_POSITION )
    {
      sPtr = this->Streamers[0].InsertNextPreciseHyperPoint();
      for (i=0; i<3; i++)
      {
        sPtr->X[i] = this->StartPosition[i];
      }
      sPtr->CellId = input->FindCell(this->StartPosition, NULL, (-1), 0.0, 
                     sPtr->SubId, sPtr->P, w);
    }

  else //VTK_START_FROM_LOCATION
    {
      sPtr = this->Streamers[0].InsertNextPreciseHyperPoint();
      cell =  input->GetCell(sPtr->CellId);
      cell->EvaluateLocation(sPtr->SubId, sPtr->P, sPtr->X, w);
    }
  //
  // Finish initializing each hyperstreamline
  //
  this->Streamers[0].Direction = 1.0;
  sPtr = this->Streamers[0].GetPreciseHyperPoint(0);
  sPtr->D = 0.0;
  if ( sPtr->CellId >= 0 ) //starting point in dataset
    {
      cell = input->GetCell(sPtr->CellId);
      cell->EvaluateLocation(sPtr->SubId, sPtr->P, xNext, w);
      //    inTensors->GetTuples(cell->PointIds, cellTensors);
      
      // interpolate tensor, compute eigenfunctions
      ((vtkTensorImplicitFunctionToFunctionSet *)this->GetMethod()->GetFunctionSet())->GetTensor(xNext,m0);
      if ( vtkDiffusionTensorMathematics::TeemEigenSolver(m, sPtr->W, sPtr->V) ) {
        //vtkMath::Jacobi(m, sPtr->W, sPtr->V);
        FixVectors(NULL, sPtr->V, iv, ix, iy);
    
        if ( sPtr->W[iv] > 0 ) {
            sPtr->S = 0.5*(( sPtr->W[iv] - sPtr->W[ix] ) *( sPtr->W[iv] - sPtr->W[ix] )  +  ( sPtr->W[ix] - sPtr->W[iy] )*( sPtr->W[ix] - sPtr->W[iy] ) +  ( sPtr->W[iv] - sPtr->W[iy] )*(                                  sPtr->W[iv] - sPtr->W[iy] ));
            sPtr->S = sqrt(sPtr->S/(sPtr->W[iv]*sPtr->W[iv] + sPtr->W[iy]*sPtr->W[iy] + sPtr->W[ix]*sPtr->W[ix] ));
        }
        else
            sPtr->S = -1.0;
        if ( this->IntegrationDirection == VTK_INTEGRATE_BOTH_DIRECTIONS )
            {
            this->Streamers[1].Direction = -1.0;
            sNext = this->Streamers[1].InsertNextPreciseHyperPoint();
            *sNext = *sPtr;
            }
        else if ( this->IntegrationDirection == VTK_INTEGRATE_BACKWARD )
            {
            this->Streamers[0].Direction = -1.0;
            }
      }
      else {
        sPtr->W[0] = 0;
        sPtr->W[1] = 0;
        sPtr->W[2] = 0;
        sPtr->V[0][0] = 1;
        sPtr->V[1][0] = 0;
        sPtr->V[2][0] = 0;
        sPtr->V[0][1] = 0;
        sPtr->V[1][1] = 1;
        sPtr->V[2][1] = 0;
        sPtr->V[0][2] = 0;
        sPtr->V[1][2] = 0;
        sPtr->V[2][2] = 1;
        sPtr->D = 0.0;
        sPtr->S = 0;
      }
    }
  //
  // For each hyperstreamline, integrate in appropriate direction (using supplied IVP solver).
  //
  for (ptId=0; ptId < this->NumberOfStreamers; ptId++)
    {
      //get starting step
      sPtr = this->Streamers[ptId].GetPreciseHyperPoint(0);
      for ( i = 0 ; i < 3 ; i++ )
    dirStart[i] = sPtr->V[i][this->IntegrationEigenvector];

      ((vtkTensorImplicitFunctionToFunctionSet *)this->GetMethod()->GetFunctionSet())->SetIntegrationDirection(dirStart);

      for ( i = 0 ; i < 3 ; i++ )
    dirStart[i] *= MinStep;

      if ( this->GetMethod() == 0 ) 
    {
      vtkErrorMacro(<<"No initial value problem solver defined");
      continue;
    }
      else 
    {
      if ( this->Streamers[ptId].Direction > 0 )
        {
          ((vtkTensorImplicitFunctionToFunctionSet *)this->GetMethod()->GetFunctionSet())->SetChangeIntegrationDirection();
          for ( i = 0 ; i < 3 ; i++ ) {
        sPtr->V[i][iv] *= -1.0;
        sPtr->V[i][ix] *= -1.0;
        sPtr->V[i][iy] *= -1.0;
          }
        }
    }
      cell = input->GetCell(sPtr->CellId);
      cell->EvaluateLocation(sPtr->SubId, sPtr->P, xNext, w);
      step = this->IntegrationStepLength * sqrt((double)cell->GetLength2());

      while ( sPtr->CellId >= 0 && fabs(sPtr->W[0]) >= this->TerminalEigenvalue && sPtr->S >= this->TerminalFractionalAnisotropy &&
          sPtr->D < this->MaximumPropagationDistance && this->Streamers[ptId].CosineOfAngle() > MaxAngle &&
          ((vtkTensorImplicitFunctionToFunctionSet *)this->GetMethod()->GetFunctionSet())->IsInsideImage(sPtr->X) )
    {
      while ( this->Streamers[ptId].DistanceSoFarMaxAngle() > LengthOfMaxAngle )
        {
          this->Streamers[ptId].IncrementMaxAngleLastId();
        }
      if ( !(this->method->ComputeNextStep(sPtr->X,xNext,sPtr->D,step,deitActual,MinStep,MaxStep,MaxError,error)) ) {
        deitActual = step;
        sNext = this->Streamers[ptId].InsertNextPreciseHyperPoint();
        
        for (i=0; i<3; i++)
          {
        sNext->X[i] = xNext[i];
          }

        cell->EvaluateLocation(sNext->SubId, sNext->P, xNext, w);
        ((vtkTensorImplicitFunctionToFunctionSet *)this->GetMethod()->GetFunctionSet())->GetLastEigenvalues(sNext->W);
        ((vtkTensorImplicitFunctionToFunctionSet *)this->GetMethod()->GetFunctionSet())->GetLastEigenvectors(sNext->V);

        FixVectors(sPtr->V, sNext->V, iv, ix, iy);
        for ( i=0;i<3;i++)
          dirStart[i] = sPtr->V[i][this->IntegrationEigenvector];
        
        ((vtkTensorImplicitFunctionToFunctionSet *)this->GetMethod()->GetFunctionSet())->SetIntegrationDirection(dirStart);

        sNext->S = ((vtkTensorImplicitFunctionToFunctionSet *)this->GetMethod()->GetFunctionSet())->GetLastFractionalAnisotropy();

        for (i=0; i<3; i++)
          {
        xNext[i] = sNext->X[i];
          }
        sNext->CellId = input->FindCell(xNext, cell, sPtr->CellId, tol2, 
                        sNext->SubId, sNext->P, w);
        if ( sNext->CellId >= 0 ) //make sure not out of dataset
          {
        cell = input->GetCell(sNext->CellId);
        inTensors->GetTuples(cell->PointIds, cellTensors);
        if (inScalars){inScalars->GetTuples(cell->PointIds, cellScalars);}
          }
        d = sqrt((double)vtkMath::Distance2BetweenPoints(sPtr->X,sNext->X));
        sNext->D = sPtr->D + d;
        
        
        sPtr = sNext;
      }
      
      else {
        sNext->W[0] = 0;
        sNext->W[1] = 0;
        sNext->W[2] = 0;
        sNext->V[0][0] = 1;
        sNext->V[1][0] = 0;
        sNext->V[2][0] = 0;
        sNext->V[0][1] = 0;
        sNext->V[1][1] = 1;
        sNext->V[2][1] = 0;
        sNext->V[0][2] = 0;
        sNext->V[1][2] = 0;
        sNext->V[2][2] = 1;
        sNext->D = sPtr->D;
        sNext->S = 0;
      }
    }
      totalLength += sPtr->D;
    } //for each hyperstreamline
  if ( totalLength < this->MinimumPropagationDistance ) {
    delete [] this->Streamers;
    this->NumberOfStreamers = 0;
    this->Streamers = NULL;
  }
  this->BuildTube();
  delete [] w;
  cellTensors->Delete();
  cellScalars->Delete();  

}

void vtkPreciseHyperStreamline::BuildTube()
{
  vtkPreciseHyperPoint *sPrev, *sPtr;
  vtkPoints *newPts;
  vtkFloatArray *newVectors;
  vtkFloatArray *newNormals;
  vtkFloatArray *newScalars=NULL;
  vtkCellArray *newStrips;
  vtkIdType i, npts, ptOffset=0;
  int ptId, j, id, k, i1, i2;
  vtkFloatingPointType dOffset, x[3], v[3], s, r, r1[3], r2[3], stepLength;
  vtkFloatingPointType xT[3], sFactor, normal[3], w[3];
  vtkFloatingPointType theta=2.0*vtkMath::Pi()/this->NumberOfSides;
  vtkPointData *outPD;
  vtkDataSet *input = this->GetInput();
  vtkPolyData *output = this->GetOutput();
  int iv, ix, iy;
  vtkIdType numIntPts;
  //
  // Initialize
  //
  vtkDebugMacro(<<"Creating hyperstreamline tube");
  if ( this->NumberOfStreamers <= 0 )
    {
      return;
    }

  stepLength = input->GetLength() * this->StepLength;
  outPD = output->GetPointData();

  iv = this->IntegrationEigenvector;
  ix = (iv+1) % 3;
  iy = (iv+2) % 3;
  //
  // Allocate
  //
  newPts  = vtkPoints::New();
  newPts ->Allocate(2500);
  if ( input->GetPointData()->GetScalars() )
    {
      newScalars = vtkFloatArray::New();
      newScalars->Allocate(2500);
    }
  newVectors = vtkFloatArray::New();
  newVectors->SetNumberOfComponents(3);
  newVectors->Allocate(7500);
  newNormals = vtkFloatArray::New();
  newNormals->SetNumberOfComponents(3);
  newNormals->Allocate(7500);
  newStrips = vtkCellArray::New();
  newStrips->Allocate(newStrips->EstimateSize(3*this->NumberOfStreamers,
                                              VTK_CELL_SIZE));
  //
  // Loop over all hyperstreamlines generating points
  //
  for (ptId=0; ptId < this->NumberOfStreamers; ptId++)
    {
      if ( (numIntPts=this->Streamers[ptId].GetNumberOfPoints()) < 2 )
    {
      continue;
    }
      sPrev = this->Streamers[ptId].GetPreciseHyperPoint(0);
      sPtr = this->Streamers[ptId].GetPreciseHyperPoint(1);

      // compute scale factor
      i = (sPrev->W[ix] > sPrev->W[iy] ? ix : iy);
      if ( sPrev->W[i] == 0.0 )
    {
      sFactor = 1.0;
    }
      else
    {
      sFactor = this->Radius / sPrev->W[i];
    }

      if ( numIntPts == 2 && sPtr->CellId < 0 )
    {
      continue;
    }

      dOffset = sPrev->D;

      for ( npts=0, i=1; i < numIntPts && sPtr->CellId >= 0;
        i++, sPrev=sPtr, sPtr=this->Streamers[ptId].GetPreciseHyperPoint(i) )
    {
      //
      // Bracket steps and construct tube points
      //
      while ( dOffset >= sPrev->D && dOffset < sPtr->D )
        {
          r = (dOffset - sPrev->D) / (sPtr->D - sPrev->D);

          for (j=0; j<3; j++) //compute point in center of tube
        {
          x[j] = sPrev->X[j] + r * (sPtr->X[j] - sPrev->X[j]);
          v[j] = sPrev->V[j][iv] + r * (sPtr->V[j][iv] - sPrev->V[j][iv]);
          r1[j] = sPrev->V[j][ix] + r * (sPtr->V[j][ix] - sPrev->V[j][ix]);
          r2[j] = sPrev->V[j][iy] + r * (sPtr->V[j][iy] - sPrev->V[j][iy]);
          w[j] = sPrev->W[j] + r * (sPtr->W[j] - sPrev->W[j]);
        }

          // construct points around tube
          for (k=0; k < this->NumberOfSides; k++)
        {
          for (j=0; j<3; j++) 
            {
              normal[j] = w[ix]*r1[j]*cos((double)k*theta) + 
                        w[iy]*r2[j]*sin((double)k*theta);
              xT[j] = x[j] + sFactor * normal[j];
            }
          id = newPts->InsertNextPoint(xT);
          newVectors->InsertTuple(id,v);
          vtkMath::Normalize(normal);
          newNormals->InsertTuple(id,normal);
        }

          if ( newScalars ) //add scalars around tube
        {
          s = sPrev->S + r * (sPtr->S - sPrev->S);
          for (k=0; k<this->NumberOfSides; k++)
            {
              newScalars->InsertNextTuple(&s);
            }
        }

          npts++;
          dOffset += stepLength;

        } //while
    } //for this hyperstreamline

      //
      // Generate the strips for this hyperstreamline
      //
      for (k=0; k<this->NumberOfSides; k++)
    {
      i1 = (k+1) % this->NumberOfSides;
      newStrips->InsertNextCell(npts*2);
      for (i=0; i < npts; i++) 
        {
          //make sure strip definition consistent with normals
          if (this->Streamers[ptId].Direction > 0.0)
        {
          i2 = i*this->NumberOfSides;
        }
          else
        {
          i2 = (npts - i - 1) * this->NumberOfSides;
        }
          newStrips->InsertCellPoint(ptOffset+i2+k);
          newStrips->InsertCellPoint(ptOffset+i2+i1);
        }
    }//for all tube sides

      ptOffset += this->NumberOfSides*npts;

    } //for all hyperstreamlines
  //
  // Update ourselves
  //
  output->SetPoints(newPts);
  newPts->Delete();

  output->SetStrips(newStrips);
  newStrips->Delete();

  if ( newScalars )
    {
      outPD->SetScalars(newScalars);
      newScalars->Delete();
    }

  outPD->SetNormals(newNormals);
  newNormals->Delete();

  outPD->SetVectors(newVectors);
  newVectors->Delete();

  output->Squeeze();
}

void vtkPreciseHyperStreamline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->StartFrom == VTK_START_FROM_POSITION )
    {
      os << indent << "Starting Position: (" << this->StartPosition[0] << ","
     << this->StartPosition[1] << ", " << this->StartPosition[2] << ")\n";
    }
  else 
    {
      os << indent << "Starting Location:\n\tCell: " << this->StartCell 
     << "\n\tSubId: " << this->StartSubId << "\n\tP.Coordinates: ("
     << this->StartPCoords[0] << ", " 
     << this->StartPCoords[1] << ", " 
     << this->StartPCoords[2] << ")\n";
    }

  os << indent << "Maximum Propagation Distance: " 
     << this->MaximumPropagationDistance << "\n";

  if ( this->IntegrationDirection == VTK_INTEGRATE_FORWARD )
    {
      os << indent << "Integration Direction: FORWARD\n";
    }
  else if ( this->IntegrationDirection == VTK_INTEGRATE_BACKWARD )
    {
      os << indent << "Integration Direction: BACKWARD\n";
    }
  else
    {
      os << indent << "Integration Direction: FORWARD & BACKWARD\n";
    }

  os << indent << "Integration Step Length: " << this->IntegrationStepLength << "\n";
  os << indent << "Step Length: " << this->StepLength << "\n";

  os << indent << "Terminal Eigenvalue: " << this->TerminalEigenvalue << "\n";

  os << indent << "Radius: " << this->Radius << "\n";
  os << indent << "Number Of Sides: " << this->NumberOfSides << "\n";
  os << indent << "Logarithmic Scaling: " << (this->LogScaling ? "On\n" : "Off\n");
  
  if ( this->IntegrationEigenvector == 0 )
    {
      os << indent << "Integrate Along Major Eigenvector\n";
    }
  else if ( this->IntegrationEigenvector == 1 )
    {
      os << indent << "Integrate Along Medium Eigenvector\n";
    }
  else
    {
      os << indent << "Integrate Along Minor Eigenvector\n";
    }
}


