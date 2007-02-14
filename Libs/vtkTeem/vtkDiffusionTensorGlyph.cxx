/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkDiffusionTensorGlyph.cxx,v $
  Date:      $Date: 2006/04/18 17:32:59 $
  Version:   $Revision: 1.16 $

=========================================================================auto=*/
#include "vtkDiffusionTensorGlyph.h"

#include "vtkObjectFactory.h"
#include "vtkTransform.h"
#include "vtkMath.h"
#include "vtkTensor.h"
#include "vtkFloatArray.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkImageData.h"
#include "vtkDiffusionTensorMathematics.h"

#include <time.h>

vtkCxxSetObjectMacro(vtkDiffusionTensorGlyph,Mask,vtkImageData);
vtkCxxSetObjectMacro(vtkDiffusionTensorGlyph,VolumePositionMatrix,vtkMatrix4x4);
vtkCxxSetObjectMacro(vtkDiffusionTensorGlyph,TensorRotationMatrix,vtkMatrix4x4);


vtkCxxRevisionMacro(vtkDiffusionTensorGlyph, "$Revision: 1.57.12.1 $");
vtkStandardNewMacro(vtkDiffusionTensorGlyph);


// Construct object with default values for diffusion tensor data.
vtkDiffusionTensorGlyph::vtkDiffusionTensorGlyph()
{
  // Color according to FA scalar invariant by default
  this->ColorGlyphs = 1;
  this->ColorMode = vtkTensorGlyph::COLOR_BY_EIGENVALUES;
  this->ScalarInvariant = vtkDiffusionTensorMathematics::FractionalAnisotropyScalar;

  // These can be optionally set by the user
  this->VolumePositionMatrix = NULL;
  this->TensorRotationMatrix = NULL;

  this->MaskGlyphs = 0;
  this->Mask = NULL;

  // Default to highest rendering resolution
  this->Resolution = 1;

  // Default large scalar factor for diffusion data. 
  // Display small magnitude eigenvalues in mm space.
  this->ScaleFactor = 1000;

  // TO DO: Use correct scaling by sqrt of eigenvalues for DTI!
}

vtkDiffusionTensorGlyph::~vtkDiffusionTensorGlyph()
{

  // Delete all objects (reduce ref count by one)
  if ( this->VolumePositionMatrix != NULL )
    {
    this->VolumePositionMatrix->Delete( );
    }

  if ( this->TensorRotationMatrix != NULL )
    {
    this->TensorRotationMatrix->Delete( );
    }

  if ( this->Mask != NULL )
    {
    this->Mask->Delete( );
    }

}

void vtkDiffusionTensorGlyph::ColorGlyphsByLinearMeasure() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::LinearMeasureScalar);
}
void vtkDiffusionTensorGlyph::ColorGlyphsBySphericalMeasure() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::SphericalMeasureScalar);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByPlanarMeasure() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::PlanarMeasureScalar);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByMaxEigenvalue() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::MaxEigenvalueScalar);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByMidEigenvalue() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::MidEigenvalueScalar);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByMinEigenvalue() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::MinEigenvalueScalar);
}

void vtkDiffusionTensorGlyph::ColorGlyphsByOrientation() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::ColorOrientationScalar);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByRelativeAnisotropy() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::RelativeAnisotropyScalar);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByFractionalAnisotropy() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::FractionalAnisotropyScalar);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByTrace() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::TraceScalar);
}

void vtkDiffusionTensorGlyph::ColorGlyphsBy(int invariant) {
  if (this->ScalarInvariant != invariant) 
    {
    // superclass flag to output scalars
    this->ColorGlyphs = 1;

    // superclass flag to calculate scalars from tensors
    // (rather than passing through input scalars)
    this->SetColorModeToEigenvalues();

    // type of scalar we'll calculate
    this->ScalarInvariant = invariant;

    // we have been modified
    this->Modified();
    }
}

void vtkDiffusionTensorGlyph::Execute()
{
  vtkDataArray *inTensors;
  vtkFloatingPointType tensor[3][3];
  vtkDataArray *inScalars;
  int numPts, numSourcePts, numSourceCells;
  int inPtId, i, j;
  vtkPoints *sourcePts;
  vtkDataArray *sourceNormals;
  vtkCellArray *sourceCells, *cells;  
  vtkPoints *newPts;
  vtkFloatArray *newScalars=NULL;
  vtkFloatArray *newNormals=NULL;
  vtkFloatingPointType *x, s;
  vtkTransform *trans = vtkTransform::New();
  vtkTransform *rotate= vtkTransform::New();;
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  vtkCell *cell;
  vtkIdList *cellPts;
  int npts;
  vtkIdType *pts;
  int cellId;
  int ptOffset=0;
  vtkFloatingPointType *m[3], w[3], *v[3];
  vtkFloatingPointType m0[3], m1[3], m2[3];
  vtkFloatingPointType v0[3], v1[3], v2[3];
  vtkFloatingPointType xv[3], yv[3], zv[3];
  vtkFloatingPointType maxScale;
  vtkPointData *pd, *outPD;
#if (VTK_MAJOR_VERSION >= 5)
  vtkDataSet *input = this->GetPolyDataInput(0);
#else
  vtkDataSet *input = this->GetInput();
#endif
  vtkPolyData *output = this->GetOutput();

  if (this->GetSource() == NULL)
    {
    vtkDebugMacro("No source.");
    return;
    }
    
  pts = new vtkIdType[this->GetSource()->GetMaxCellSize()];

  vtkDataArray *inMask;
  int doMasking;
  // time
  clock_t tStart=0;
  tStart = clock();


  // set up working matrices
  m[0] = m0; m[1] = m1; m[2] = m2; 
  v[0] = v0; v[1] = v1; v[2] = v2; 

  vtkDebugMacro(<<"Generating tensor glyphs");

  pd = input->GetPointData();
  outPD = output->GetPointData();
  inTensors = pd->GetTensors();
  inScalars = pd->GetScalars();
  numPts = input->GetNumberOfPoints();
  inMask = NULL;
  if (this->Mask)
    {
      inMask = this->Mask->GetPointData()->GetScalars();
    }

  if ( !inTensors || numPts < 1 )
    {
      vtkErrorMacro(<<"No data to glyph!");
      return;
    }
  //
  // Allocate storage for output PolyData
  //
  sourcePts = this->GetSource()->GetPoints();
  numSourcePts = sourcePts->GetNumberOfPoints();
  numSourceCells = this->GetSource()->GetNumberOfCells();

  newPts = vtkPoints::New();
  newPts->Allocate(numPts*numSourcePts);

  // Setting up for calls to PolyData::InsertNextCell()
  if ( (sourceCells=this->GetSource()->GetVerts())->GetNumberOfCells() > 0 )
    {
      cells = vtkCellArray::New();
      cells->Allocate(numPts*sourceCells->GetSize());
      output->SetVerts(cells);
      cells->Delete();
    }
  if ( (sourceCells=this->GetSource()->GetLines())->GetNumberOfCells() > 0 )
    {
      cells = vtkCellArray::New();
      cells->Allocate(numPts*sourceCells->GetSize());
      output->SetLines(cells);
      cells->Delete();
    }
  if ( (sourceCells=this->GetSource()->GetPolys())->GetNumberOfCells() > 0 )
    {
      cells = vtkCellArray::New();
      cells->Allocate(numPts*sourceCells->GetSize());
      output->SetPolys(cells);
      cells->Delete();
    }
  if ( (sourceCells=this->GetSource()->GetStrips())->GetNumberOfCells() > 0 )
    {
      cells = vtkCellArray::New();
      cells->Allocate(numPts*sourceCells->GetSize());
      output->SetStrips(cells);
      cells->Delete();
    }

  // copy point data through or create it here
  pd = this->GetSource()->GetPointData();

  // always output scalars
  newScalars = vtkFloatArray::New();
  newScalars->Allocate(numPts*numSourcePts);
  
  if ( (sourceNormals = pd->GetNormals()) )
    {
      newNormals = vtkFloatArray::New();
      // vtk4.0, copied from tensor glyph filter
      newNormals->SetNumberOfComponents(3);
      newNormals->Allocate(3*numPts*numSourcePts);
      //newNormals->Allocate(numPts*numSourcePts);
    }

  // Figure out whether we are using a mask (if the user has
  // asked us to mask and also has set the mask input).
  doMasking = 0;
  //if (inMask && this->MaskGlyphs)
  //doMasking = 1;
  if (this->MaskGlyphs)
    {
      if (inMask)
    {
      doMasking = 1;
    }
      else 
    {
      vtkErrorMacro("User has not set input mask, but has requested MaskGlyphs");
    }
    }

  // figure out if we are transforming output point locations
  vtkTransform *userVolumeTransform = vtkTransform::New();
  if (this->VolumePositionMatrix)
    {
      userVolumeTransform->SetMatrix(this->VolumePositionMatrix);
      userVolumeTransform->PreMultiply();
    }

  //
  // Traverse all Input points, transforming glyph at Source points
  //
  trans->PreMultiply();

  //cout << "glyph time before pt traversal: " << clock() - tStart << endl;

  for (inPtId=0; inPtId < numPts; inPtId=inPtId+this->Resolution)
    {
      
      if ( ! (inPtId % 10000) ) 
    {
      this->UpdateProgress ((vtkFloatingPointType)inPtId/numPts);
      if (this->GetAbortExecute())
        {
          break;
        }
    }

      //ptIncr = inPtId * numSourcePts;

      //tensor = inTensors->GetTuple(inPtId);
      inTensors->GetTuple(inPtId,(vtkFloatingPointType *)tensor);

      trans->Identity();

      // threshold: if trace is <= 0, don't do expensive computations
      // This used to be: tensor ->GetComponent(0,0) + 
      // tensor->GetComponent(1,1) + tensor->GetComponent(2,2);
      double trace = vtkDiffusionTensorMathematics::Trace(tensor);
      
      // only display this glyph if either:
      // a) we are masking and the mask is 1 at this location.
      // b) the trace is positive and we are not masking (default).
      // (If the trace is <= 0 we don't need to go through the code just to
      // display nothing at the end, since we expect that our data has
      // non-negative eigenvalues.)
      if ((doMasking && inMask->GetTuple1(inPtId)) || (!this->MaskGlyphs && trace > 0)) 
    {
      // copy topology
      for (cellId=0; cellId < numSourceCells; cellId++)
        {
          cell = this->GetSource()->GetCell(cellId);
          cellPts = cell->GetPointIds();
          npts = cellPts->GetNumberOfIds();
          for (i=0; i < npts; i++)
        {
          //pts[i] = cellPts->GetId(i) + ptIncr;
          pts[i] = cellPts->GetId(i) + ptOffset;
        }
          output->InsertNextCell(cell->GetCellType(),npts,pts);
        }

      // translate Source to Input point
      x = input->GetPoint(inPtId);
      // If we have a user-specified matrix determining the points
      vtkFloatingPointType x2[3];
      if (this->VolumePositionMatrix)
        {
          userVolumeTransform->TransformPoint(x,x2);
          // point x to x2 now
          x = x2;
        }  
      trans->Translate(x[0], x[1], x[2]);

      // compute orientation vectors and scale factors from tensor
      if ( this->ExtractEigenvalues ) // extract appropriate eigenfunctions
        {
          for (j=0; j<3; j++)
        {
          for (i=0; i<3; i++)
            {
              // transpose
              //m[i][j] = tensor[i+3*j];
              m[i][j] = tensor[j][i];
            }
        }
         //vtkMath::Jacobi(m, w, v);
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
          //xv[i] = tensor[i];
          //yv[i] = tensor[i+3];
          //zv[i] = tensor[i+6];
          xv[i] = tensor[0][i];
          yv[i] = tensor[1][i];
          zv[i] = tensor[2][i];
        }
          w[0] = vtkMath::Normalize(xv);
          w[1] = vtkMath::Normalize(yv);
          w[2] = vtkMath::Normalize(zv);
        }

      // pass through input scalars if requested
      if ( inScalars && this->ColorGlyphs && ( this->ColorMode == vtkTensorGlyph::COLOR_BY_SCALARS ) ) 
        {
          // Copy point data from source
          s = inScalars->GetTuple1(inPtId);
        }

      // output scalar invariants if requested 
      // (before modifying the value of the eigenvalues for glyph scaling)      
      if ( this->ColorGlyphs && ( this->ColorMode == vtkTensorGlyph::COLOR_BY_EIGENVALUES ) ) 
        {
          //Correct for negative eigenvalues: used logic coded in vtkDiffusionTensorMathematics
        vtkDiffusionTensorMathematics::FixNegativeEigenvaluesMethod(w);

        switch (this->ScalarInvariant) 
        {
        case vtkDiffusionTensorMathematics::LinearMeasureScalar:
          s = vtkDiffusionTensorMathematics::LinearMeasure(w);
          break;
        case vtkDiffusionTensorMathematics::PlanarMeasureScalar:
          s = vtkDiffusionTensorMathematics::PlanarMeasure(w);
          break;
        case vtkDiffusionTensorMathematics::SphericalMeasureScalar:
          s = vtkDiffusionTensorMathematics::SphericalMeasure(w);
          break;
        case vtkDiffusionTensorMathematics::MaxEigenvalueScalar:
          s = w[0];
          break;
        case vtkDiffusionTensorMathematics::MidEigenvalueScalar:
          s = w[1];
          break;
        case vtkDiffusionTensorMathematics::MinEigenvalueScalar:
          s = w[2]; 
          break;
        case vtkDiffusionTensorMathematics::ColorOrientationScalar:

          // TO DO: here output as RGB. Need to allocate scalars first.
          
          break;
        case vtkDiffusionTensorMathematics::RelativeAnisotropyScalar:
          s = vtkDiffusionTensorMathematics::RelativeAnisotropy(w);
          break;
        case vtkDiffusionTensorMathematics::FractionalAnisotropyScalar:
          s = vtkDiffusionTensorMathematics::FractionalAnisotropy(w);
          break;
        case vtkDiffusionTensorMathematics::TraceScalar:
          s = vtkDiffusionTensorMathematics::Trace(w);
          break;
        default:
          s = 0;
          break;
        }
        }          

      for (i=0; i < numSourcePts; i++) 
        {
          //newScalars->InsertScalar(ptIncr+i, s);
          newScalars->InsertNextTuple1(s);
        }        

      // compute scale factors
      w[0] *= this->ScaleFactor;
      w[1] *= this->ScaleFactor;
      w[2] *= this->ScaleFactor;
    
      if ( this->ClampScaling )
        {
          for (maxScale=0.0, i=0; i<3; i++)
        {
          if ( maxScale < fabs(w[i]) )
            {
              maxScale = fabs(w[i]);
            }
        }
          if ( maxScale > this->MaxScaleFactor )
        {
          maxScale = this->MaxScaleFactor / maxScale;
          for (i=0; i<3; i++)
            {
              w[i] *= maxScale; //preserve overall shape of glyph
            }
        }
        }

      // If we have a user-specified matrix rotating the tensor
       if (this->TensorRotationMatrix)
         {
           trans->Concatenate(this->TensorRotationMatrix);
         }


      // normalized eigenvectors rotate object
      // odonnell: test -y for display 
      int yFlipFlag = 1;
      matrix->Element[0][0] = xv[0];
      matrix->Element[0][1] = yFlipFlag*yv[0];
      matrix->Element[0][2] = zv[0];
      matrix->Element[1][0] = xv[1];
      matrix->Element[1][1] = yFlipFlag*yv[1];
      matrix->Element[1][2] = zv[1];
      matrix->Element[2][0] = xv[2];
      matrix->Element[2][1] = yFlipFlag*yv[2];
      matrix->Element[2][2] = zv[2];
      trans->Concatenate(matrix);

      // make sure scale is okay (non-zero) and scale data
      for (maxScale=0.0, i=0; i<3; i++)
        {
          if ( w[i] > maxScale )
        {
          maxScale = w[i];
        }
        }
      if ( maxScale == 0.0 )
        {
          maxScale = 1.0;
        }
      for (i=0; i<3; i++)
        {
          if ( w[i] == 0.0 )
        {
          w[i] = maxScale * 1.0e-06;
        }
        }
      trans->Scale(w[0], w[1], w[2]);

      // multiply points (and normals if available) by resulting matrix
      // this also appends them to the output "new" data
      trans->TransformPoints(sourcePts,newPts);
      if ( newNormals )
        {
          trans->TransformNormals(sourceNormals,newNormals);
        }

      ptOffset += numSourcePts;

    }  // end if mask is 1 OR trace is ok
    }

  vtkDebugMacro(<<"Generated " << numPts <<" tensor glyphs");
  //
  // Update output and release memory
  //
  delete [] pts;

  output->SetPoints(newPts);
  newPts->Delete();

  if ( newScalars )
    {
      outPD->SetScalars(newScalars);
      newScalars->Delete();
    }

  if ( newNormals )
    {
      outPD->SetNormals(newNormals);
      newNormals->Delete();
    }

  // reclaim extra memory we allocated
  output->Squeeze();

  rotate->Delete();
  userVolumeTransform->Delete();
  trans->Delete();
  matrix->Delete();

  vtkDebugMacro("glyph time: " << clock() - tStart );
}

void vtkDiffusionTensorGlyph::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Color Glyphs by Scalar Invariant: " << this->ScalarInvariant << "\n";
  os << indent << "Mask Glyphs: " << (this->MaskGlyphs ? "On\n" : "Off\n");
  os << indent << "Resolution: " << this->Resolution << endl;

  // print objects
  if ( this->VolumePositionMatrix )
    {
    os << indent << "VolumePositionMatrix:\n";
    this->VolumePositionMatrix->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "VolumePositionMatrix: (none)\n";
    }

  if ( this->TensorRotationMatrix )
    {
    os << indent << "TensorRotationMatrix:\n";
    this->TensorRotationMatrix->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "TensorRotationMatrix: (none)\n";
    }

  if ( this->Mask )
    {
    os << indent << "Mask:\n";
    this->Mask->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Mask: (none)\n";
    }

}

//----------------------------------------------------------------------------
// Account for the MTime of objects we use
//
unsigned long int vtkDiffusionTensorGlyph::GetMTime()
{
  unsigned long mTime=this->vtkObject::GetMTime();
  unsigned long time;

  if ( this->Mask != NULL )
    {
      time = this->Mask->GetMTime();
      mTime = ( time > mTime ? time : mTime );
    }

  if ( this->VolumePositionMatrix != NULL )
    {
      time = this->VolumePositionMatrix->GetMTime();
      mTime = ( time > mTime ? time : mTime );
    }

  if ( this->TensorRotationMatrix != NULL )
    {
      time = this->TensorRotationMatrix->GetMTime();
      mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}


