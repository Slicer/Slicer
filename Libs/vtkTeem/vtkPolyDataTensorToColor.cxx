/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPolyDataTensorToColor.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPolyDataTensorToColor.h"

#include "vtkCellArray.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"

#include "vtkDiffusionTensorMathematics.h"

#include <time.h>


vtkCxxRevisionMacro(vtkPolyDataTensorToColor, "$Revision: 1.41 $");
vtkStandardNewMacro(vtkPolyDataTensorToColor);

// Construct with lower threshold=0, upper threshold=1, and threshold 
// function=upper.
vtkPolyDataTensorToColor::vtkPolyDataTensorToColor()
{
  this->ColorMode = vtkTensorGlyph::COLOR_BY_EIGENVALUES;
  this->ScalarInvariant = vtkDiffusionTensorMathematics::VTK_TENS_FRACTIONAL_ANISOTROPY;
  this->ExtractScalar = 0;
  this->ExtractEigenvalues = 1;
}


void vtkPolyDataTensorToColor::ColorGlyphsByLinearMeasure() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_LINEAR_MEASURE);
}
void vtkPolyDataTensorToColor::ColorGlyphsBySphericalMeasure() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_SPHERICAL_MEASURE);
}
void vtkPolyDataTensorToColor::ColorGlyphsByPlanarMeasure() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_PLANAR_MEASURE);
}
void vtkPolyDataTensorToColor::ColorGlyphsByMaxEigenvalue() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_MAX_EIGENVALUE);
}
void vtkPolyDataTensorToColor::ColorGlyphsByMidEigenvalue() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_MID_EIGENVALUE);
}
void vtkPolyDataTensorToColor::ColorGlyphsByMinEigenvalue() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_MIN_EIGENVALUE);
}

void vtkPolyDataTensorToColor::ColorGlyphsByRelativeAnisotropy() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_RELATIVE_ANISOTROPY);
}
void vtkPolyDataTensorToColor::ColorGlyphsByFractionalAnisotropy() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_FRACTIONAL_ANISOTROPY);
}
void vtkPolyDataTensorToColor::ColorGlyphsByTrace() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_TRACE);
}

void vtkPolyDataTensorToColor::ColorGlyphsBy(int invariant) {
  if (this->ScalarInvariant != invariant) 
    {
    // superclass flag to output scalars
    // superclass flag to calculate scalars from tensors
    // (rather than passing through input scalars)
    this->SetColorModeToEigenvalues();
    
    this->ExtractScalar = 1;

    // type of scalar we'll calculate
    this->ScalarInvariant = invariant;

    // we have been modified
    this->Modified();
    }
}

int vtkPolyDataTensorToColor::RequestData(
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

  vtkDataArray *inScalars;
  vtkPoints *newPoints;
  vtkPointData *pd, *outPD;
  //vtkCellArray *verts;
  vtkIdType ptId, numPts, *pts;
  double x[3];
  double s = 0;

  vtkDebugMacro(<< "Executing threshold points filter");
     
  numPts = input->GetNumberOfPoints();
  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts);
  pd = input->GetPointData();
  vtkDataArray *inTensors = pd->GetTensors();
  inScalars = pd->GetScalars();

  outPD = output->GetPointData();
  //outPD->CopyAllocate(pd);
  //verts = vtkCellArray::New();
  //verts->Allocate(verts->EstimateSize(numPts,1));

  // Setting up for calls to PolyData::InsertNextCell()

  vtkCellArray *sourceCells, *cells;  
  vtkCell *cell;
  vtkIdType cellId;
  vtkIdList *cellPts;
  int npts;

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
  if ( (sourceCells=input->GetPolys())->GetNumberOfCells() > 0 )
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
  int j;
  int numSourceCells = input->GetNumberOfCells();

  pts = new vtkIdType[input->GetMaxCellSize()];
  
  // copy topology 
  for (cellId=0; cellId < numSourceCells; cellId++)
    {
    cell = input->GetCell(cellId);
    cellPts = cell->GetPointIds();
    npts = cellPts->GetNumberOfIds();
    for (i=0; i < npts; i++)
      {
      pts[i] = cellPts->GetId(i);
      }

    output->InsertNextCell(cell->GetCellType(),npts,pts);
    }

  vtkFloatArray *newScalars=NULL;

  double tensor[3][3];
  double *m[3], w[3], *v[3];
  double m0[3], m1[3], m2[3];
  double v0[3], v1[3], v2[3];
  double xv[3], yv[3], zv[3];

  // set up working matrices
  m[0] = m0; m[1] = m1; m[2] = m2; 
  v[0] = v0; v[1] = v1; v[2] = v2; 

  if (this->ExtractScalar && ((this->ColorMode == vtkTensorGlyph::COLOR_BY_EIGENVALUES) || 
       (inScalars && (this->ColorMode == vtkTensorGlyph::COLOR_BY_SCALARS)))  )
    {
    newScalars = vtkFloatArray::New();
    newScalars->Allocate(numPts);
    }
  else
    {
    // only copy scalar data through
    // (superclass does this but why? if user has not asked for ColorGlyphs)
    outPD->CopyAllOff();
    outPD->CopyScalarsOn();
    outPD->CopyAllocate(pd,numPts);
    }

  // Check that the scalars of each point satisfy the threshold criterion
  int abort=0;
  vtkIdType progressInterval = numPts/20+1;
  vtkIdType ptOffset = 0;

  for (ptId=0; ptId < numPts && !abort; ptId++)
    {
    if ( !(ptId % progressInterval) )
      {
      this->UpdateProgress((double)ptId/numPts);
      abort = this->GetAbortExecute();
      }


    input->GetPoint(ptId, x);
    pts[0] = newPoints->InsertNextPoint(x);
    
    if (!this->ExtractScalar) 
      {
      continue;
      }
    inTensors->GetTuple(ptId, (double *)tensor);

    // Decide whether this tensor will be glyphed:
    // Threshold by trace ( must be > 0)
    double trace = vtkDiffusionTensorMathematics::Trace(tensor);

    //outPD->CopyData(pd,ptId,pts[0]);
    //verts->InsertNextCell(1,pts);



    // compute orientation vectors and scale factors from tensor
    if ( this->ExtractEigenvalues ) // extract appropriate eigenfunctions
      {
      for (j=0; j<3; j++)
        {
        for (i=0; i<3; i++)
          {
          // this line from vtkTensorGlyph actually transposes
          //m[i][j] = tensor[i+3*j];  
          // simpler code with 3x3 array:
          m[i][j] = tensor[j][i];
          }
        }

      //vtkMath::Jacobi(m, w, v);
      // Use superior eigensolve from teem.
      vtkDiffusionTensorMathematics::TeemEigenSolver(m,w,v);
      
      //copy eigenvectors
      xv[0] = v[0][0]; xv[1] = v[1][0]; xv[2] = v[2][0];
      yv[0] = v[0][1]; yv[1] = v[1][1]; yv[2] = v[2][1];
      zv[0] = v[0][2]; zv[1] = v[1][2]; zv[2] = v[2][2];
      }
    else //use tensor columns as eigenvectors
      {
      for (i=0; i<3; i++)
        {
        //xv[i] = tensor[i]; // from vtkTensorGlyph
        //yv[i] = tensor[i+3]; 
        //zv[i] = tensor[i+6];
        xv[i] = tensor[0][i]; // with 3x3 matrix
        yv[i] = tensor[1][i];
        zv[i] = tensor[2][i];
        }
      w[0] = vtkMath::Normalize(xv);
      w[1] = vtkMath::Normalize(yv);
      w[2] = vtkMath::Normalize(zv);
      }
    
    
    // Calculate output scalars before computing glyph scale factors from eigenvalues.
    // First, pass through input scalars if requested.
    if ( inScalars  && ( this->ColorMode == vtkTensorGlyph::COLOR_BY_SCALARS ) ) 
      {
      // Copy point data from source
      s = inScalars->GetComponent(ptId, 0);
      }

    // Output scalar invariants if requested 
    else if (  this->ColorMode == vtkTensorGlyph::COLOR_BY_EIGENVALUES ) 
      {

      // Correct for negative eigenvalues: use logic coded in vtkDiffusionTensorMathematics
      vtkDiffusionTensorMathematics::FixNegativeEigenvaluesMethod(w);
      
      switch (this->ScalarInvariant) 
        {
        case vtkDiffusionTensorMathematics::VTK_TENS_LINEAR_MEASURE:
          s = vtkDiffusionTensorMathematics::LinearMeasure(w);
          break;
        case vtkDiffusionTensorMathematics::VTK_TENS_PLANAR_MEASURE:
          s = vtkDiffusionTensorMathematics::PlanarMeasure(w);
          break;
        case vtkDiffusionTensorMathematics::VTK_TENS_SPHERICAL_MEASURE:
          s = vtkDiffusionTensorMathematics::SphericalMeasure(w);
          break;
        case vtkDiffusionTensorMathematics::VTK_TENS_MAX_EIGENVALUE:
          s = w[0];
          break;
        case vtkDiffusionTensorMathematics::VTK_TENS_MID_EIGENVALUE:
          s = w[1];
          break;
        case vtkDiffusionTensorMathematics::VTK_TENS_MIN_EIGENVALUE:
          s = w[2]; 
          break;
        case vtkDiffusionTensorMathematics::VTK_TENS_COLOR_ORIENTATION:
          double v_maj[3];
          v_maj[0]=v[0][0];
          v_maj[1]=v[1][0];
          v_maj[2]=v[2][0];
          
          // TO DO: here output as RGB. Need to allocate 3-component scalars first.
          s = 0;
          break;
        case vtkDiffusionTensorMathematics::VTK_TENS_RELATIVE_ANISOTROPY:
          s = vtkDiffusionTensorMathematics::RelativeAnisotropy(w);
          break;
        case vtkDiffusionTensorMathematics::VTK_TENS_FRACTIONAL_ANISOTROPY:
          s = vtkDiffusionTensorMathematics::FractionalAnisotropy(w);
          break;
        case vtkDiffusionTensorMathematics::VTK_TENS_TRACE:
          s = vtkDiffusionTensorMathematics::Trace(w);
          break;
        default:
          s = 0;
          break;
        }

      }

    // Actually output the scalar invariant calculated above
    if ( newScalars != NULL )
      {
      newScalars->InsertTuple(ptOffset, &s);
      }        
    else
      {
      // TO DO: why does superclass have this if no scalar output?
      // in this case it appears copy scalars is on (above in
      // scalar allocation section).
      outPD->CopyData(pd,0,ptOffset);
      }

    // Keep track of the number of points output so far.
    ptOffset++;
    
    } // for all points

  vtkDebugMacro(<< "Extracted " << output->GetNumberOfPoints() << " points.");
  
  // Update ourselves and release memory
  //
  output->SetPoints(newPoints);
  newPoints->Delete();
  
  if ( newScalars )
    {
    int idx = outPD->AddArray(newScalars);
    outPD->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
    newScalars->Delete();
    }

  //output->SetVerts(verts);
  //verts->Delete();
  
  output->Squeeze();

  return 1;
}

int vtkPolyDataTensorToColor::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

void vtkPolyDataTensorToColor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

}
