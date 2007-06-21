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


#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkDataSet.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkTransform.h"

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
  this->ScalarInvariant = vtkDiffusionTensorMathematics::VTK_TENS_FRACTIONAL_ANISOTROPY;

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
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_LINEAR_MEASURE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsBySphericalMeasure() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_SPHERICAL_MEASURE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByPlanarMeasure() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_PLANAR_MEASURE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByMaxEigenvalue() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_MAX_EIGENVALUE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByMidEigenvalue() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_MID_EIGENVALUE);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByMinEigenvalue() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_MIN_EIGENVALUE);
}

void vtkDiffusionTensorGlyph::ColorGlyphsByOrientation() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_COLOR_ORIENTATION);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByRelativeAnisotropy() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_RELATIVE_ANISOTROPY);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByFractionalAnisotropy() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_FRACTIONAL_ANISOTROPY);
}
void vtkDiffusionTensorGlyph::ColorGlyphsByTrace() {
  this->ColorGlyphsBy(vtkDiffusionTensorMathematics::VTK_TENS_TRACE);
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

// TO DO: make input mask a point data object or scalars

int vtkDiffusionTensorGlyph::RequestData(
                                         vtkInformation *vtkNotUsed(request),
                                         vtkInformationVector **inputVector,
                                         vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkDataSet *input = vtkDataSet::SafeDownCast(
                                               inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *source = vtkPolyData::SafeDownCast(
                                                  sourceInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
                                                  outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDataArray *inTensors;
  vtkDataArray *inScalars;
  vtkIdType numPts, numSourcePts, numSourceCells, inPtId, i;
  int j;
  vtkPoints *sourcePts;
  vtkDataArray *sourceNormals;
  vtkCellArray *sourceCells, *cells;  
  vtkPoints *newPts;
  vtkFloatArray *newScalars=NULL;
  vtkFloatArray *newNormals=NULL;
  double x[3], x2[3], s;
  vtkTransform *trans;
  vtkCell *cell;
  vtkIdList *cellPts;
  int npts;
  vtkIdType *pts;
  vtkIdType cellId;
  vtkIdType subIncr;
  int numDirs, dir, eigen_dir, symmetric_dir;
  vtkMatrix4x4 *matrix;
  double *m[3], w[3], *v[3];
  double m0[3], m1[3], m2[3];
  double v0[3], v1[3], v2[3];
  double xv[3], yv[3], zv[3];
  double maxScale;
  vtkPointData *pd, *outPD;

  // Keeps track of the number of points added to the output polydata so far.
  // this replaces variable ptIncr in superclass vtkTensorGlyph.
  vtkIdType ptOffset = 0;

  // coordinate systems for DTI
  vtkTransform *userVolumeTransform = NULL;
  // masking of glyphs
  vtkDataArray *inMask;
  // glyph timing
  clock_t tStart=0;
  tStart = clock();

  // use simpler 3x3 array, not 9D as in vtkTensorGlyph class
  double tensor[3][3];

  // the number of eigenvectors to glyph * if there are two glyphs per vector
  numDirs = (this->ThreeGlyphs?3:1)*(this->Symmetric+1);
  
  pts = new vtkIdType[source->GetMaxCellSize()];
  trans = vtkTransform::New();
  matrix = vtkMatrix4x4::New();
  
  // set up working matrices
  m[0] = m0; m[1] = m1; m[2] = m2; 
  v[0] = v0; v[1] = v1; v[2] = v2; 

  vtkDebugMacro(<<"Generating tensor glyphs");

  pd = input->GetPointData();
  outPD = output->GetPointData();
  inTensors = pd->GetTensors();
  inScalars = pd->GetScalars();
  numPts = input->GetNumberOfPoints();

  if ( !inTensors || numPts < 1 )
    {
    vtkErrorMacro(<<"No data to glyph!");
    return 1;
    }

  //
  // Allocate storage for output PolyData
  //
  sourcePts = source->GetPoints();
  numSourcePts = sourcePts->GetNumberOfPoints();
  numSourceCells = source->GetNumberOfCells();

  newPts = vtkPoints::New();
  // Allocate as if we will glyph every point
  // If some are masked/skipped for Resolution this will be fixed later with Squeeze
  // TO DO allocate less for lower resolution
  newPts->Allocate(numDirs*numPts*numSourcePts);

  // Setting up for calls to PolyData::InsertNextCell()
  if ( (sourceCells=source->GetVerts())->GetNumberOfCells() > 0 )
    {
    cells = vtkCellArray::New();
    cells->Allocate(numDirs*numPts*sourceCells->GetSize());
    output->SetVerts(cells);
    cells->Delete();
    }
  if ( (sourceCells=this->GetSource()->GetLines())->GetNumberOfCells() > 0 )
    {
    cells = vtkCellArray::New();
    cells->Allocate(numDirs*numPts*sourceCells->GetSize());
    output->SetLines(cells);
    cells->Delete();
    }
  if ( (sourceCells=this->GetSource()->GetPolys())->GetNumberOfCells() > 0 )
    {
    cells = vtkCellArray::New();
    cells->Allocate(numDirs*numPts*sourceCells->GetSize());
    output->SetPolys(cells);
    cells->Delete();
    }
  if ( (sourceCells=this->GetSource()->GetStrips())->GetNumberOfCells() > 0 )
    {
    cells = vtkCellArray::New();
    cells->Allocate(numDirs*numPts*sourceCells->GetSize());
    output->SetStrips(cells);
    cells->Delete();
    }


  // Get point data, decide how to allocate scalars
  pd = this->GetSource()->GetPointData();

  // generate scalars if eigenvalues are chosen or if scalars exist.
  if (this->ColorGlyphs && 
      ((this->ColorMode == COLOR_BY_EIGENVALUES) || 
       (inScalars && (this->ColorMode == COLOR_BY_SCALARS)) ) )
    {
    newScalars = vtkFloatArray::New();
    newScalars->Allocate(numDirs*numPts*numSourcePts);
    }
  else
    {
    // only copy scalar data through
    // (superclass does this but why? if user has not asked for ColorGlyphs)
    outPD->CopyAllOff();
    outPD->CopyScalarsOn();
    outPD->CopyAllocate(pd,numDirs*numPts*numSourcePts);
    }
  if ( (sourceNormals = pd->GetNormals()) )
    {
    newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->Allocate(numDirs*3*numPts*numSourcePts);
    }

  // Don't copy all topology here as in superclass because
  // we are not necessarily outputting a glyph for every point.

  // Figure out if we are masking some of the glyphs
  inMask = NULL;
  
  if (this->MaskGlyphs)
    {
    if (this->Mask != NULL)
      {
      inMask = this->Mask->GetPointData()->GetScalars();
      }
    else 
      {
      vtkErrorMacro("User has not set input mask, but has requested MaskGlyphs");
      }
    }

  // Figure out if we are transforming output point locations
  if (this->VolumePositionMatrix)
    {
    // user transform to move points
    userVolumeTransform = vtkTransform::New();
    
    userVolumeTransform->SetMatrix(this->VolumePositionMatrix);
    userVolumeTransform->PreMultiply();
    }

  vtkDebugMacro(<<"Generating tensor glyphs: TRAVERSE POINTS");

  vtkErrorMacro("Scalar coloring (" <<  this->ColorMode << ")  ["<< vtkTensorGlyph::COLOR_BY_EIGENVALUES << "] is evals. Scalar Invariant (" << this->ScalarInvariant << ")") ;

  //
  // Traverse all Input points, transforming glyph in this->Source by tensor, 
  // and outputting it at each point.  (Input points are not all used, only
  // those not masked and included by this->Resolution.)
  //
  trans->PreMultiply();

  for (inPtId=0; inPtId < numPts; inPtId += this->Resolution)
    {

    // progress notification
    if ( ! (inPtId % 10000) ) 
      {
      this->UpdateProgress ((vtkFloatingPointType)inPtId/numPts);

      vtkDebugMacro(<<"Generating diffusion tensor glyphs: PROGRESS" << (vtkFloatingPointType)inPtId/numPts);
      if (this->GetAbortExecute())
        {
        break;
        }
      }



    inTensors->GetTuple(inPtId, (double *)tensor);

    // Decide whether this tensor will be glyphed:
    // Threshold by trace ( must be > 0)
    double trace = vtkDiffusionTensorMathematics::Trace(tensor);
      
    // Only display this glyph if either:
    // a) we are masking and the mask is 1 at this location.
    // b) the trace is positive and we are not masking (default).   
    if (( ( inMask != NULL ) && inMask->GetTuple1( inPtId ) ) || ( !this->MaskGlyphs && trace > 0 )) 
      {
      // copy topology of output glyph for this point
      for (cellId=0; cellId < numSourceCells; cellId++)
        {
        cell = this->GetSource()->GetCell(cellId);
        cellPts = cell->GetPointIds();
        npts = cellPts->GetNumberOfIds();
        for (dir=0; dir < numDirs; dir++)
          {
          // This variable may be removed, but that 
          // will not improve readability
          //subIncr = ptIncr + dir*numSourcePts;

          // Add offset calculated from all non-masked points added to output so far
          subIncr = ptOffset + dir*numSourcePts;

          for (i=0; i < npts; i++)
            {
            pts[i] = cellPts->GetId(i) + subIncr;
            }
          output->InsertNextCell(cell->GetCellType(),npts,pts);
          }
        }

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
      if ( inScalars && this->ColorGlyphs && ( this->ColorMode == vtkTensorGlyph::COLOR_BY_SCALARS ) ) 
        {
        // Copy point data from source
        s = inScalars->GetComponent(inPtId, 0);
        }

      // Output scalar invariants if requested 
      else if ( this->ColorGlyphs && ( this->ColorMode == vtkTensorGlyph::COLOR_BY_EIGENVALUES ) ) 
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
            if (this->TensorRotationMatrix)
              {
              vtkTransform *rotate = vtkTransform::New();
              rotate->SetMatrix(this->TensorRotationMatrix);
              rotate->TransformPoint(v_maj,v_maj);
              rotate->Delete();
              }
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

      // Use the square root of the eigenvalues for scaling
      // for DTI 
      w[0] = sqrt( w[0] );
      w[1] = sqrt( w[1] );
      w[2] = sqrt( w[2] );

      // compute scale factors (this modifies eigenvalues so 
      // scalar invariants were computed already above)
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

      // normalization is postponed

      // make sure scale is okay (non-zero) and scale data 
      // this scale checking is from superclass code
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

      // Now do the real work for each "direction"
      // This is a loop over each eigenvector allowing
      // a separate glyph for each (or two loops per eigenvector
      // allowing two symmetric glyphs for each)

      for (dir=0; dir < numDirs; dir++) 
        {
        eigen_dir = dir%(this->ThreeGlyphs?3:1);
        symmetric_dir = dir/(this->ThreeGlyphs?3:1);

        // Remove previous scales ...
        trans->Identity();



        // Actually output the scalar invariant calculated above
        if ( newScalars != NULL )
          {
          for (i=0; i < numSourcePts; i++) 
            {

            newScalars->InsertTuple(ptOffset+i, &s);
            }        
          }
        else
          {
          for (i=0; i < numSourcePts; i++) 
            {
            // TO DO: why does superclass have this if no scalar output?
            // in this case it appears copy scalars is on (above in
            // scalar allocation section).
            outPD->CopyData(pd,i,ptOffset+i);
            }
          }

        // translate Source to Input point
        input->GetPoint(inPtId, x);
        
        // If we have a user-specified matrix modifying the output point locations
        if ( userVolumeTransform != NULL )
          {
          userVolumeTransform->TransformPoint(x,x2);
          trans->Translate(x2[0], x2[1], x2[2]);
          }  
        else
          {
          trans->Translate(x[0], x[1], x[2]);
          }

        
        // If we have a user-specified matrix rotating each tensor
        if (this->TensorRotationMatrix)
          {
          trans->Concatenate(this->TensorRotationMatrix);
          }

        // normalized eigenvectors rotate object for eigen direction 0
        matrix->Element[0][0] = xv[0];
        matrix->Element[0][1] = yv[0];
        matrix->Element[0][2] = zv[0];
        matrix->Element[1][0] = xv[1];
        matrix->Element[1][1] = yv[1];
        matrix->Element[1][2] = zv[1];
        matrix->Element[2][0] = xv[2];
        matrix->Element[2][1] = yv[2];
        matrix->Element[2][2] = zv[2];
        trans->Concatenate(matrix);

        if (eigen_dir == 1) 
          {
          trans->RotateZ(90.0);
          }

        if (eigen_dir == 2)
          {
          trans->RotateY(-90.0);
          }

        if (this->ThreeGlyphs) 
          {
          trans->Scale(w[eigen_dir], this->ScaleFactor, this->ScaleFactor);
          }
        else
          {

          trans->Scale(w[0], w[1], w[2]);

          }

        // Mirror second set to the symmetric position
        if (symmetric_dir == 1)
          {
          trans->Scale(-1.,1.,1.);
          }

        // if the eigenvalue is negative, shift to reverse direction.
        // The && is there to ensure that we do not change the 
        // old behaviour of vtkTensorGlyphs (which only used one dir), 
        // in case there is an oriented glyph, e.g. an arrow.
        if (w[eigen_dir] < 0 && numDirs > 1) 
          {
          trans->Translate(-this->Length, 0., 0.);
          }

        // multiply points (and normals if available) by resulting
        // matrix.
        // This also appends them to the output "new" data.
        trans->TransformPoints(sourcePts,newPts); 

        // Apply the transformation to a series of points, 
        // and append the results to outPts.
        if ( newNormals )
          {
          trans->TransformNormals(sourceNormals,newNormals);
          }
        

        // Keep track of the number of points output so far.
        ptOffset += numSourcePts;

        } // end for number of dirs
 
      } // end if mask is 1 OR trace is ok (so tensor was glyphed)

    } // end loop over input points



  vtkDebugMacro(<<"Generated " << numPts <<" tensor glyphs");

  //
  // Update output and release memory
  //
  delete [] pts;

  output->SetPoints(newPts);
  newPts->Delete();

  if ( newScalars )
    {
    int idx = outPD->AddArray(newScalars);
    outPD->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
    newScalars->Delete();
    }

  if ( newNormals )
    {
    outPD->SetNormals(newNormals);
    newNormals->Delete();
    }

  output->Squeeze();
  trans->Delete();
  matrix->Delete();

  if ( userVolumeTransform )
    {
    userVolumeTransform->Delete();
    }

  vtkDebugMacro("glyph time: " << clock() - tStart );


  return 1;

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


