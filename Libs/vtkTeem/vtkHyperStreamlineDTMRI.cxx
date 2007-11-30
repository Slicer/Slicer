/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkHyperStreamlineDTMRI.cxx,v $
  Date:      $Date: 2007/03/19 14:39:53 $
  Version:   $Revision: 1.32.2.1 $

=========================================================================auto=*/
#include "vtkHyperStreamlineDTMRI.h"

#include "vtkCellArray.h"
#include "vtkDataSet.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkTractographyPointAndArray.h"

// the superclass had these classes in the vtkHyperStreamline.cxx
// file: being compiled via CMakeListsLocal.txt
#if (VTK_MAJOR_VERSION == 4 && VTK_MINOR_VERSION >= 3)
//#include "vtkHyperPointandArray.cxx"
#endif

vtkCxxRevisionMacro(vtkHyperStreamlineDTMRI, "$Revision: 1.32.2.1 $");
vtkStandardNewMacro(vtkHyperStreamlineDTMRI);

vtkHyperStreamlineDTMRI::vtkHyperStreamlineDTMRI()
{
  // defaults copied from superclass for now:
  this->StartFrom = VTK_START_FROM_POSITION;
  this->StartPosition[0] = this->StartPosition[1] = this->StartPosition[2] = 0.0;

  this->StartCell = 0;
  this->StartSubId = 0;
  this->StartPCoords[0] = this->StartPCoords[1] = this->StartPCoords[2] = 0.5;

  this->Streamers = NULL;

  this->MaximumPropagationDistance = 600.0;

  this->RadiusOfCurvature = 0.87;

  // in mm.
  this->IntegrationStepLength = 0.5;

  this->IntegrationDirection = VTK_INTEGRATE_BOTH_DIRECTIONS;
  this->TerminalEigenvalue = 0.0;

  this->IntegrationEigenvector = VTK_INTEGRATE_MAJOR_EIGENVECTOR;

  this->StoppingMode = vtkDiffusionTensorMathematics::VTK_TENS_LINEAR_MEASURE;
  this->StoppingThreshold=0.15;

  this->OutputTensors = 0;
  this->OneTrajectoryPerSeedPoint = 0;
}

vtkHyperStreamlineDTMRI::~vtkHyperStreamlineDTMRI()
{
}

// copied from superclass
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

int vtkHyperStreamlineDTMRI::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
vtkInformation *outInfo = outputVector->GetInformationObject(0);

//Get the input and output
vtkDataSet *input = vtkDataSet::SafeDownCast(
  inInfo->Get(vtkDataObject::DATA_OBJECT()));
vtkPolyData *output = vtkPolyData::SafeDownCast(
  outInfo->Get(vtkDataObject::DATA_OBJECT()));


  vtkPointData *pd=input->GetPointData();
  vtkDataArray *inScalars;
  vtkDataArray *inTensors;
  vtkFloatingPointType *tensor;
  vtkTractographyPoint *sNext, *sPtr;
  int i, j, k, ptId, subId, iv, ix, iy;
  vtkCell *cell;
  vtkFloatingPointType ev[3];
  vtkFloatingPointType xNext[3];
  vtkFloatingPointType d, step, dir, tol2, p[3];
  vtkFloatingPointType *w;
  vtkFloatingPointType dist2;
  vtkFloatingPointType closestPoint[3];
  vtkFloatingPointType *m[3], *v[3];
  vtkFloatingPointType m0[3], m1[3], m2[3];
  vtkFloatingPointType v0[3], v1[3], v2[3];
  vtkDataArray *cellTensors;
  vtkDataArray *cellScalars;
  int pointCount;
  vtkTractographyPoint *sPrev, *sPrevPrev;
  vtkFloatingPointType kv1[3], kv2[3], ku1[3], ku2[3], kl1, kl2, kn[3], K;
  // set up working matrices
  v[0] = v0; v[1] = v1; v[2] = v2;
  m[0] = m0; m[1] = m1; m[2] = m2;
  float stop;
  //static const float sqrt3halves = sqrt((float)3/2);
  int keepIntegrating;

  vtkDebugMacro(<<"Generating hyperstreamline(s)");
  this->NumberOfStreamers = 0;

  if ( ! (inTensors=pd->GetTensors()) )
    {
    vtkErrorMacro(<<"No tensor data defined!");
    return 0;
    }
  w = new vtkFloatingPointType[input->GetMaxCellSize()];

  inScalars = pd->GetScalars();
  if (inTensors)
    {
    cellTensors = vtkDataArray::CreateDataArray(inTensors->GetDataType());
    }
  if (inScalars)
    {
    cellScalars = vtkDataArray::CreateDataArray(inScalars->GetDataType());
    }
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

  this->Streamers = new vtkTractographyArray[this->NumberOfStreamers];

  if ( this->StartFrom == VTK_START_FROM_POSITION )
    {
    sPtr = this->Streamers[0].InsertNextTractographyPoint();
    for (i=0; i<3; i++)
      {
      sPtr->X[i] = this->StartPosition[i];
      }
    sPtr->CellId = input->FindCell(this->StartPosition, NULL, (-1), 0.0, 
                                   sPtr->SubId, sPtr->P, w);
    }

  else //VTK_START_FROM_LOCATION
    {
    sPtr = this->Streamers[0].InsertNextTractographyPoint();
    cell =  input->GetCell(sPtr->CellId);
    cell->EvaluateLocation(sPtr->SubId, sPtr->P, sPtr->X, w);
    }
  //
  // Finish initializing each hyperstreamline
  //
  this->Streamers[0].Direction = 1.0;
  sPtr = this->Streamers[0].GetTractographyPoint(0);
  sPtr->D = 0.0;
  if ( sPtr->CellId >= 0 ) //starting point in dataset
    {
    cell = input->GetCell(sPtr->CellId);
    cell->EvaluateLocation(sPtr->SubId, sPtr->P, xNext, w);

    inTensors->GetTuples(cell->PointIds, cellTensors);

    // interpolate tensor, compute eigenfunctions
    for (j=0; j<3; j++)
      {
      for (i=0; i<3; i++)
        {
        m[i][j] = 0.0;
        }
      }
    for (k=0; k < cell->GetNumberOfPoints(); k++)
      {
      tensor = cellTensors->GetTuple(k);
      for (j=0; j<3; j++) 
        {
        for (i=0; i<3; i++) 
          {
          m[i][j] += tensor[i+3*j] * w[k];

          }
        }
      }
    
    // store tensor at start point
    for (j=0; j<3; j++) 
      {
      for (i=0; i<3; i++) 
        {
        sPtr->T[i][j] = m[i][j];
        }
      }

    //vtkMath::Jacobi(m, sPtr->W, sPtr->V);
    vtkDiffusionTensorMathematics::TeemEigenSolver(m,sPtr->W,sPtr->V);
    FixVectors(NULL, sPtr->V, iv, ix, iy);

    if ( inScalars )
      {
      inScalars->GetTuples(cell->PointIds, cellScalars);
      for (sPtr->S=0, i=0; i < cell->GetNumberOfPoints(); i++)
        {
        sPtr->S += cellScalars->GetTuple(i)[0] * w[i];
        // for curvature coloring for debugging purposes:
        //sPtr->S =0;
        }
      }

    if ( this->IntegrationDirection == VTK_INTEGRATE_BOTH_DIRECTIONS )
      {
      this->Streamers[1].Direction = -1.0;
      sNext = this->Streamers[1].InsertNextTractographyPoint();
      *sNext = *sPtr;
      }
    else if ( this->IntegrationDirection == VTK_INTEGRATE_BACKWARD )
      {
      this->Streamers[0].Direction = -1.0;
      }
    } //for hyperstreamline in dataset

  //
  // For each hyperstreamline, integrate in appropriate direction (using RK2).
  //
  for (ptId=0; ptId < this->NumberOfStreamers; ptId++)
    {
    //get starting step
    sPtr = this->Streamers[ptId].GetTractographyPoint(0);
    if ( sPtr->CellId < 0 )
      {
      continue;
      }

    dir = this->Streamers[ptId].Direction;
    cell = input->GetCell(sPtr->CellId);
    cell->EvaluateLocation(sPtr->SubId, sPtr->P, xNext, w);
    step = this->IntegrationStepLength;
    inTensors->GetTuples(cell->PointIds, cellTensors);
    if ( inScalars ) {inScalars->GetTuples(cell->PointIds, cellScalars);}


    // This is the flag for integration to continue if FA, curvature
    // are within limits
    keepIntegrating=1;
    // init index for curvature calculation
    pointCount=0;

    //integrate until distance has been exceeded
    while ( sPtr->CellId >= 0 && fabs(sPtr->W[0]) > this->TerminalEigenvalue &&
            sPtr->D < this->MaximumPropagationDistance &&
            keepIntegrating)
      {
        // Test curvature
        if ( pointCount > 2 )
          {

           // v2=p3-p2;  % vector from point 2 to point 3
            // v1=p2-p1;  % vector from point 1 to point 2
            // u2=v2/norm(v2);  % unit vector in the direction of v2
            // u1=v1/norm(v1);  % unit vector in the direction of v1

            // kn is curvature times the unit normal vector
            // it's the change in the unit normal over half the distance 
            // from p1 to p3
            // kn=2*(u2-u1)/(norm(v1)+norm(v2));
            // absk=norm(kn);  % absolute value of the curvature

            sPrev = this->Streamers[ptId].GetTractographyPoint(pointCount-1);
            sPrevPrev = this->Streamers[ptId].GetTractographyPoint(pointCount-2);
            kl2=0;
            kl1=0;
            for (i=0; i<3; i++)
              {
              // vectors
              kv2[i]=sPrevPrev->X[i] - sPrev->X[i];
              kv1[i]=sPrev->X[i] - sPtr->X[i];
              // lengths
              kl2+=kv2[i]*kv2[i];
              kl1+=kv1[i]*kv1[i];
              }
            kl2=sqrt(kl2);
            kl1=sqrt(kl1);
            // normalize
            for (i=0; i<3; i++)
              {
              // unit vectors
              ku2[i]=kv2[i]/kl2;
              ku1[i]=kv1[i]/kl1;
              }
            // compute curvature
            for (i=0; i<3; i++)
              {
              kn[i]=2*(ku2[i]-ku1[i])/(kl1+kl2);
              K+=kn[i]*kn[i];
              }
            K=sqrt(K);
            // units are radians per mm.
            // Convert to radius of curvature (in mm) 
            // and compare to allowed radius.
            if (K != 0)
              {
                if ((1/K) < this->RadiusOfCurvature) 
                  {
                    keepIntegrating=0;
                  }
              }
          }
        else 
          {
          K=0;
          }


      //compute updated position using this step (Euler integration)
      for (i=0; i<3; i++)
        {
        xNext[i] = sPtr->X[i] + dir * step * sPtr->V[i][iv];
        }

      //compute updated position using updated step
      cell->EvaluatePosition(xNext, closestPoint, subId, p, dist2, w);

      //interpolate tensor
      for (j=0; j<3; j++)
        {
        for (i=0; i<3; i++)
          {
          m[i][j] = 0.0;
          }
        }
      for (k=0; k < cell->GetNumberOfPoints(); k++)
        {
        tensor = cellTensors->GetTuple(k);
        for (j=0; j<3; j++)
          {
          for (i=0; i<3; i++)
            {
            m[i][j] += tensor[i+3*j] * w[k];
            }
          }
        }

      //vtkMath::Jacobi(m, ev, v);
      vtkDiffusionTensorMathematics::TeemEigenSolver(m,ev,v);
      FixVectors(sPtr->V, v, iv, ix, iy);

      //now compute final position
      for (i=0; i<3; i++)
        {
        xNext[i] = sPtr->X[i] + 
                   dir * (step/2.0) * (sPtr->V[i][iv] + v[i][iv]);
        }
      sNext = this->Streamers[ptId].InsertNextTractographyPoint();

      if ( cell->EvaluatePosition(xNext, closestPoint, sNext->SubId, 
      sNext->P, dist2, w) )
        { //integration still in cell
        for (i=0; i<3; i++)
          {
          sNext->X[i] = closestPoint[i];
          }
        sNext->CellId = sPtr->CellId;
        sNext->SubId = sPtr->SubId;
        }
      else
        { //integration has passed out of cell
        sNext->CellId = input->FindCell(xNext, cell, sPtr->CellId, tol2, 
                                        sNext->SubId, sNext->P, w);
        if ( sNext->CellId >= 0 ) //make sure not out of dataset
          {
          for (i=0; i<3; i++)
            {
            sNext->X[i] = xNext[i];
            }
          cell = input->GetCell(sNext->CellId);
          inTensors->GetTuples(cell->PointIds, cellTensors);
          if (inScalars){inScalars->GetTuples(cell->PointIds, cellScalars);}
          step = this->IntegrationStepLength;
          }
        }

      if ( sNext->CellId >= 0 )
        {
        cell->EvaluateLocation(sNext->SubId, sNext->P, xNext, w);
        for (j=0; j<3; j++)
          {
          for (i=0; i<3; i++)
            {
            m[i][j] = 0.0;
            }
          }
        for (k=0; k < cell->GetNumberOfPoints(); k++)
          {
          tensor = cellTensors->GetTuple(k);
          for (j=0; j<3; j++) 
            {
            for (i=0; i<3; i++) 
              {
              m[i][j] += tensor[i+3*j] * w[k];
              }
            }
          }

        //vtkMath::Jacobi(m, sNext->W, sNext->V);
        vtkDiffusionTensorMathematics::TeemEigenSolver(m,sNext->W,sNext->V);
        FixVectors(sPtr->V, sNext->V, iv, ix, iy);

        // compute invariants at final position                                         
        switch (this->GetStoppingMode()) {
        case vtkDiffusionTensorMathematics::VTK_TENS_FRACTIONAL_ANISOTROPY:
            stop = vtkDiffusionTensorMathematics::FractionalAnisotropy(sNext->W);
            break;
        case vtkDiffusionTensorMathematics::VTK_TENS_LINEAR_MEASURE:
            stop = vtkDiffusionTensorMathematics::LinearMeasure(sNext->W);
            break;
        case vtkDiffusionTensorMathematics::VTK_TENS_PLANAR_MEASURE:
            stop = vtkDiffusionTensorMathematics::PlanarMeasure(sNext->W);
            break;
        case vtkDiffusionTensorMathematics::VTK_TENS_SPHERICAL_MEASURE:
            stop =  vtkDiffusionTensorMathematics::SphericalMeasure(sNext->W);
            break;
        default:
            vtkErrorMacro( << "Should not happen" );
        }

        // test FA cutoff
        if (stop < this->StoppingThreshold)
          {
          keepIntegrating=0;
          }

        if ( inScalars )
          {
          for (sNext->S=0.0, i=0; i < cell->GetNumberOfPoints(); i++)
            {
              // output interpolated scalar data
              sNext->S += cellScalars->GetTuple(i)[0] * w[i];
              // for curvature coloring for debugging purposes:
              //sNext->S =K;

            }
          }

        // output tensor at final position
        for (j=0; j<3; j++) 
            {
            for (i=0; i<3; i++) 
              {
                sNext->T[i][j] = m[i][j];
              }
            }


        d = sqrt((double)vtkMath::Distance2BetweenPoints(sPtr->X,sNext->X));
        sNext->D = sPtr->D + d;
        }

      sPtr = sNext;

      pointCount++;

      }//for elapsed time

    } //for each hyperstreamline

  this->BuildLines(input,output);

  delete [] w;
  if (inTensors)
    {
    cellTensors->Delete();
    }
  if (inScalars)
    {
    cellScalars->Delete();
    }

  // note: these two lines fix memory leak in code copied from vtk
  delete [] this->Streamers;
  this->Streamers = NULL;
  return 1;
}

void vtkHyperStreamlineDTMRI::BuildLines(vtkDataSet *input, vtkPolyData *output)
{

  if ( this->OneTrajectoryPerSeedPoint == 1  )
    this->BuildLinesForSingleTrajectory(input, output);
  else
    this->BuildLinesForTwoTrajectories(input, output);
}

void vtkHyperStreamlineDTMRI::BuildLinesForTwoTrajectories(vtkDataSet *input, vtkPolyData *output)
{
  vtkTractographyPoint *sPtr;
  vtkPoints *newPoints;
  vtkCellArray *newLines;
  vtkFloatArray *newScalars=NULL;
  vtkFloatArray *newTensors=NULL;

  vtkPointData *outPD = output->GetPointData();

  vtkIdType numIntPts;
  //
  // Initialize
  //
  vtkDebugMacro(<<"Creating hyperstreamline tube");
  if ( this->NumberOfStreamers <= 0 )
    {
    return;
    }

  //
  // Allocate
  //
  newPoints  = vtkPoints::New();
  numIntPts = 0;
  for (int ptId=0; ptId < this->NumberOfStreamers; ptId++)
    {
      numIntPts+=this->Streamers[ptId].GetNumberOfPoints();
    }
  newPoints ->Allocate(numIntPts);
  newLines = vtkCellArray::New();

  if ( input->GetPointData()->GetScalars() )
    {
    newScalars = vtkFloatArray::New();
    newScalars->Allocate(numIntPts);
    }

  if ( this->OutputTensors )
    {
    newTensors = vtkFloatArray::New();
    newTensors->SetNumberOfComponents(9);
    newTensors->Allocate(numIntPts * 9);
    }

  // index into the whole point array
  int strIdx = 0;

  for (int ptId=0; ptId < this->NumberOfStreamers; ptId++)
    {
      // if no points give up
    if ( (numIntPts=this->Streamers[ptId].GetNumberOfPoints()) < 1 )
      {
      continue;
      }

      // cell indicates line connectivity
      newLines->InsertNextCell(numIntPts);

      // loop through all points on the path and make a line
      int i=0;
      sPtr=this->Streamers[ptId].GetTractographyPoint(i);
      while (i < numIntPts && sPtr->CellId >= 0)
        {
          newPoints->InsertPoint(strIdx,sPtr->X);
          newLines->InsertCellPoint(strIdx);

          if ( newScalars ) // add scalars at points
            {
              double s = sPtr->S;
              newScalars->InsertNextTuple(&s);
            }

          if ( newTensors ) // add tensors at points
            {
              double tensor[9];
              int idx;
              idx =0;
              for (int row = 0; row < 3; row++)
                {
                  for (int col = 0; col < 3; col++)
                    {
                      tensor[idx] = sPtr->T[row][col];
                      idx++;
                    }
                }  

              newTensors->InsertNextTuple(tensor);
            }

          i++;
          sPtr=this->Streamers[ptId].GetTractographyPoint(i);
          strIdx++;
          
        } //while

      // in case we ended earlier than numIntPts because sPtr->CellID=0
      // this gets rid of empty cell points at the end
      newLines->UpdateCellCount(i);

    } //for this hyperstreamline

  //
  // Update ourselves
  //
  output->SetPoints(newPoints);
  newPoints->Delete();

  if ( newScalars )
    {
    int idx = outPD->AddArray(newScalars);
    outPD->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
    newScalars->Delete();
    }

  if ( newTensors )
    {
    int idx = outPD->AddArray(newTensors);
    outPD->SetActiveAttribute(idx, vtkDataSetAttributes::TENSORS);
    newTensors->Delete();
    }

  output->SetLines(newLines);
  newLines->Delete();
    
  output->Squeeze();

}


void vtkHyperStreamlineDTMRI::BuildLinesForSingleTrajectory(vtkDataSet *input, vtkPolyData *output)
{
  vtkTractographyPoint *sPtr;
  vtkPoints *newPoints;
  vtkCellArray *newLines;
  vtkFloatArray *newScalars=NULL;
  vtkFloatArray *newTensors=NULL;

  vtkPointData *outPD = output->GetPointData();

  vtkIdType numIntPts;
  //
  // Initialize
  //
  vtkDebugMacro(<<"Creating hyperstreamline tube");
  if ( this->NumberOfStreamers <= 0 )
    {
    return;
    }

  //
  // Allocate
  //
  newPoints  = vtkPoints::New();
  numIntPts = 0;
  for (int streamerIdx=0; streamerIdx < this->NumberOfStreamers; streamerIdx++)
    {
      numIntPts+=this->Streamers[streamerIdx].GetNumberOfPoints();
    }

  // don't count the seed point twice
  numIntPts--;

  newPoints ->Allocate(numIntPts);
  newLines = vtkCellArray::New();

  if ( input->GetPointData()->GetScalars() )
    {
    newScalars = vtkFloatArray::New();
    newScalars->Allocate(numIntPts);
    }

  if ( this->OutputTensors )
    {
    newTensors = vtkFloatArray::New();
    newTensors->SetNumberOfComponents(9);
    newTensors->Allocate(numIntPts * 9);
    }

  // index into the whole point array
  int strIdx = 0;


  // single cell indicates line connectivity
  newLines->InsertNextCell(numIntPts);

  vtkDebugMacro("Handling first streamer");

  // go backwards through first streamer and skip seed point
  // --------------------------------------------------------
  int streamerId = 0;

  // if no points give up
  if ( (numIntPts=this->Streamers[streamerId].GetNumberOfPoints()) > 0 )
    {
    
    // loop through all points on the path and make a line
    int i = numIntPts-1;
    sPtr=this->Streamers[streamerId].GetTractographyPoint(i);
    // > 0 skips initial (seed) point
    while (i > 0 )
      {
 
      // if this was a good point add it
      if ( sPtr->CellId >= 0)
        {
        newPoints->InsertPoint(strIdx,sPtr->X);
        newLines->InsertCellPoint(strIdx);
        
        if ( newScalars ) // add scalars at points
          {
          double s = sPtr->S;
          newScalars->InsertNextTuple(&s);
          }
        
        if ( newTensors ) // add tensors at points
          {
          double tensor[9];
          int idx;
          idx =0;
          for (int row = 0; row < 3; row++)
            {
            for (int col = 0; col < 3; col++)
              {
              tensor[idx] = sPtr->T[row][col];
              idx++;
              }
            }  
          
          newTensors->InsertNextTuple(tensor);
          }

        // count of number of points added
        strIdx++;
        
        }
      
      i--;
      sPtr=this->Streamers[streamerId].GetTractographyPoint(i);
        
      } //while
    


    } //for this hyperstreamline
  
  vtkDebugMacro("Handling second streamer");

  // go forwards through second streamer and include seed point
  // --------------------------------------------------------
  streamerId = 1;

  // if no points give up
  if ( (numIntPts=this->Streamers[streamerId].GetNumberOfPoints()) > 0 )
    {
    
    // loop through all points on the path and make a line
    int i=0;
    sPtr=this->Streamers[streamerId].GetTractographyPoint(i);
    while (i < numIntPts && sPtr->CellId >= 0)
      {
      newPoints->InsertPoint(strIdx,sPtr->X);
      newLines->InsertCellPoint(strIdx);
      
      if ( newScalars ) // add scalars at points
        {
        double s = sPtr->S;
        newScalars->InsertNextTuple(&s);
        }
      
      if ( newTensors ) // add tensors at points
        {
        double tensor[9];
//        int row, col;
        int idx;
        idx =0;
        for (int row = 0; row < 3; row++)
          {
          for (int col = 0; col < 3; col++)
            {
            tensor[idx] = sPtr->T[row][col];
            idx++;
            }
          }  
        
        newTensors->InsertNextTuple(tensor);
        }
      
      i++;
      sPtr=this->Streamers[streamerId].GetTractographyPoint(i);
      strIdx++;
          
      }
    }

  // in case we ended earlier than numIntPts because sPtr->CellID=0
  // this gets rid of empty cell points at the end
  newLines->UpdateCellCount(strIdx);
    
  vtkDebugMacro("Assigning output values");

  //
  // Update ourselves
  //
  output->SetPoints(newPoints);
  newPoints->Delete();
  
  if ( newScalars )
    {
    int idx = outPD->AddArray(newScalars);
    outPD->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
    newScalars->Delete();
    }

  if ( newTensors )
    {
    int idx = outPD->AddArray(newTensors);
    outPD->SetActiveAttribute(idx, vtkDataSetAttributes::TENSORS);
    newTensors->Delete();
    }

  output->SetLines(newLines);
  newLines->Delete();
    
  output->Squeeze();

}

void vtkHyperStreamlineDTMRI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Radius of Curvature "
    << this->RadiusOfCurvature << "\n";
}


