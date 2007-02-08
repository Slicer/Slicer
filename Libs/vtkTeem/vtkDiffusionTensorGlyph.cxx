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
#include "vtkTensorMathematics.h"

#include <time.h>

vtkCxxSetObjectMacro(vtkDiffusionTensorGlyph,ScalarMask,vtkImageData);
vtkCxxSetObjectMacro(vtkDiffusionTensorGlyph,VolumePositionMatrix,vtkMatrix4x4);
vtkCxxSetObjectMacro(vtkDiffusionTensorGlyph,TensorRotationMatrix,vtkMatrix4x4);

//------------------------------------------------------------------------------
vtkDiffusionTensorGlyph* vtkDiffusionTensorGlyph::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDiffusionTensorGlyph");
  if(ret)
    {
    return (vtkDiffusionTensorGlyph*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkDiffusionTensorGlyph;
}




// Construct object with defaults from superclass: these are
// scaling on and scale factor 1.0. Eigenvalues are 
// extracted, glyphs are colored with input scalar data, and logarithmic
// scaling is turned off.
vtkDiffusionTensorGlyph::vtkDiffusionTensorGlyph()
{
  // Instead of coloring glyphs by passing through input
  // scalars, color according to features we are computing.
  //this->ColorGlyphsWithLinearMeasure();
  this->ScalarMeasure = 0; // Need to initialed var before callling ColorGlyphsWithDirection
  this->ColorGlyphsWithDirection();

  this->VolumePositionMatrix = NULL;
  this->TensorRotationMatrix = NULL;

  this->MaskGlyphsWithScalars = 0;
  this->ScalarMask = NULL;
  this->Resolution = 1;
}

vtkDiffusionTensorGlyph::~vtkDiffusionTensorGlyph()
{

}

void vtkDiffusionTensorGlyph::ColorGlyphsWithLinearMeasure() {
  this->ColorGlyphsWith(VTK_LINEAR_MEASURE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsWithSphericalMeasure() {
  this->ColorGlyphsWith(VTK_SPHERICAL_MEASURE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsWithPlanarMeasure() {
  this->ColorGlyphsWith(VTK_PLANAR_MEASURE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsWithMaxEigenvalue() {
  this->ColorGlyphsWith(VTK_MAX_EIGENVAL_MEASURE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsWithMiddleEigenvalue() {
  this->ColorGlyphsWith(VTK_MIDDLE_EIGENVAL_MEASURE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsWithMinEigenvalue() {
  this->ColorGlyphsWith(VTK_MIN_EIGENVAL_MEASURE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsWithMaxMinusMidEigenvalue() {
  this->ColorGlyphsWith(VTK_EIGENVAL_DIFFERENCE_MAX_MID_MEASURE);
}

void vtkDiffusionTensorGlyph::ColorGlyphsWithDirection() {
  this->ColorGlyphsWith(VTK_DIRECTION_MEASURE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsWithRelativeAnisotropy() {
  this->ColorGlyphsWith(VTK_RELATIVE_ANISOTROPY_MEASURE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsWithFractionalAnisotropy() {
  this->ColorGlyphsWith(VTK_FRACTIONAL_ANISOTROPY_MEASURE);
}

void vtkDiffusionTensorGlyph::ColorGlyphsWith(int measure) {
  if (this->ScalarMeasure != measure) 
    {
      this->ColorGlyphs = 0;
      this->ColorGlyphsWithAnisotropy = 1;
      this->ScalarMeasure = measure;
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
  if (this->ScalarMask)
    {
      inMask = this->ScalarMask->GetPointData()->GetScalars();
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
  //if (inMask && this->MaskGlyphsWithScalars)
  //doMasking = 1;
  if (this->MaskGlyphsWithScalars)
    {
      if (inMask)
    {
      doMasking = 1;
    }
      else 
    {
      vtkErrorMacro("User has not set input mask, but has requested MaskGlyphsWithScalars");
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
      vtkFloatingPointType trace = tensor[0][0] + tensor[1][1] + tensor[2][2];

      
      // only display this glyph if either:
      // a) we are masking and the mask is 1 at this location.
      // b) the trace is positive and we are not masking (default).
      // (If the trace is 0 we don't need to go through the code just to
      // display nothing at the end, since we expect that our data has
      // non-negative eigenvalues.)
      if ((doMasking && inMask->GetTuple1(inPtId)) || (!this->MaskGlyphsWithScalars && trace > 0)) 
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
         vtkTensorMathematics::TeemEigenSolver(m,w,v);
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

      // output scalars before modifying the value of 
      // the eigenvalues (scaling, etc)
      if ( inScalars && this->ColorGlyphs ) 
        {
          // Copy point data from source
          s = inScalars->GetTuple1(inPtId);
        }
      else 
        {
          //Correct for negative eigenvalues: used logic coded in vtkTensorMathematics
        vtkTensorMathematics::FixNegativeEigenvalues(w);

        switch (this->ScalarMeasure) 
        {
        case VTK_LINEAR_MEASURE:
          s = vtkTensorMathematics::LinearMeasure(w);
          break;
        case VTK_PLANAR_MEASURE:
          s = vtkTensorMathematics::PlanarMeasure(w);
          break;
        case VTK_SPHERICAL_MEASURE:
          s = vtkTensorMathematics::SphericalMeasure(w);
          break;
        case VTK_MAX_EIGENVAL_MEASURE:
          s = w[0];
          break;
        case VTK_MIDDLE_EIGENVAL_MEASURE:
          s = w[1];
          break;
        case VTK_MIN_EIGENVAL_MEASURE:
          s = w[2]; 
          break;
        case VTK_EIGENVAL_DIFFERENCE_MAX_MID_MEASURE:
          s = w[0] - w[2]; 
          break;
        case VTK_DIRECTION_MEASURE:
          // vary color only with x and y, since unit vector
          // these two determine z component.
          // use max evector for direction
          //s = fabs(xv[0])/(fabs(yv[0]) + eps);
          double v_maj[3];
          v_maj[0]=v[0][0];
          v_maj[1]=v[1][0];
          v_maj[2]=v[2][0];
          if (this->TensorRotationMatrix)
            {
              rotate->SetMatrix(this->TensorRotationMatrix);
              rotate->TransformPoint(v_maj,v_maj);
            }
          
          this->RGBToIndex(fabs(v_maj[0]),fabs(v_maj[1]),fabs(v_maj[2]),s);
          
          break;
        case VTK_RELATIVE_ANISOTROPY_MEASURE:
          s = vtkTensorMathematics::RelativeAnisotropy(w);
          break;
        case VTK_FRACTIONAL_ANISOTROPY_MEASURE:
          s = vtkTensorMathematics::FractionalAnisotropy(w);
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

  cout << "glyph time: " << clock() - tStart << endl;
}

void vtkDiffusionTensorGlyph::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkTensorGlyph::PrintSelf(os,indent);

  //  os << indent << "ColorGlyphsWithAnisotropy: " << this->ColorGlyphsWithAnisotropy << "\n";
}

//----------------------------------------------------------------------------
// Account for the MTime of objects we use
//
unsigned long int vtkDiffusionTensorGlyph::GetMTime()
{
  unsigned long mTime=this->vtkObject::GetMTime();
  unsigned long time;

  if ( this->ScalarMask != NULL )
    {
      time = this->ScalarMask->GetMTime();
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


// This is sort of the inverse of code from Gordon Kindlmann for mapping
// the mode (index value) to RGB. See vtkTensorMathematics for that code.
// There may be a simpler way to do this but this works.
// Note this expects a "0 1" Hue Range in the vtkLookupTable used to
// display the glyphs.
void vtkDiffusionTensorGlyph::RGBToIndex(double R, double G, 
                                           double B, double &index) 
{

  // remove the gray part of the color.
  // this is so we can use the model where either R,G, or B is 0.
  // then we scale so that the max of the other two is one.
  double min = R;
  int minIdx = 0;
  if (G < min)
    {
      min = G;
      minIdx = 1;
    }
  if (B < min)
    {
      min = B;
      minIdx = 2;
    }

  // make the smallest of R,G,B equal 0
  R = R - min;
  G = G - min;
  B = B - min;

  // now take the max, and scale it to be 1.
  double max = R;
  int maxIdx = 0;
  if (G > max)
    {
      max = G;
      maxIdx = 1;
    }
  if (B > max)
    {
      max = B;
      maxIdx = 2;
    }

  R = R/max;
  G = G/max;
  B = B/max;


  // now using the inverse sextants, map this into an index.
  // switch (sextant) {
  //   case 0: { R = 1;      G = frac;   B = 0;      break; }
  //   case 1: { R = 1-frac; G = 1;      B = 0;      break; }
  //   case 2: { R = 0;      G = 1;      B = frac;   break; }
  //   case 3: { R = 0;      G = 1-frac; B = 1;      break; }
  //   case 4: { R = frac;   G = 0;      B = 1;      break; }
  //   case 5: { R = 1;      G = 0;      B = 1-frac; break; }
  // }
  int sextant;
  if (maxIdx == 0 && minIdx == 2) sextant = 0;
  if (maxIdx == 1 && minIdx == 2) sextant = 1;
  if (maxIdx == 1 && minIdx == 0) sextant = 2;
  if (maxIdx == 2 && minIdx == 0) sextant = 3;
  if (maxIdx == 2 && minIdx == 1) sextant = 4;
  if (maxIdx == 0 && minIdx == 1) sextant = 5;

  double offset;
  offset = 256/6;

  switch (sextant) 
    {
    case 0: { index =  G*offset;     break; }
    case 1: { index = offset + (1-R)*offset;      break; }
    case 2: { index = offset*2 + B*offset;   break; }
    case 3: { index = offset*3 + (1-G)*offset;      break; }
    case 4: { index = offset*4 + R*offset;      break; }
    case 5: { index = offset*5 + (1-B)*offset; break; }
    }

}

