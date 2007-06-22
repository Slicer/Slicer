/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkPreciseHyperStreamlinePoints.cxx,v $
  Date:      $Date: 2006/06/29 13:46:25 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
#include "vtkPreciseHyperStreamlinePoints.h"
#include "vtkObjectFactory.h"
// is there any problem with including a cxx file?
// this is done for class vtkPreciseHyperPoint which is defined here
// otherwise we cannot access the points calculated by superclass
//#include "vtkPreciseHyperStreamline.cxx"

//------------------------------------------------------------------------------

#include "vtkPreciseHyperArray.h"
#include "vtkPreciseHyperPoint.h"

/*
class vtkPreciseHyperPoint { //;prevent man page generation
public:
    vtkPreciseHyperPoint(); // method sets up storage
    vtkPreciseHyperPoint &operator=(const vtkPreciseHyperPoint& hp); //for resizing
    
    float   X[3];    // position 
    vtkIdType     CellId;  // cell
    int     SubId; // cell sub id
    float   P[3];    // parametric coords in cell 
    float   W[3];    // eigenvalues (sorted in decreasing value)
    float   *V[3];   // pointers to eigenvectors (also sorted)
    float   V0[3];   // storage for eigenvectors
    float   V1[3];
    float   V2[3];
    float   S;       // scalar value 
    float   D;       // distance travelled so far 
};
//ETX

class vtkPreciseHyperArray { //;prevent man page generation
public:
  vtkPreciseHyperArray();
  ~vtkPreciseHyperArray()
    {
      if (this->Array)
        {
        delete [] this->Array;
        }
    };
  vtkIdType GetNumberOfPoints() {return this->MaxId + 1;};
  vtkPreciseHyperPoint *GetPreciseHyperPoint(vtkIdType i) {return this->Array + i;};
  vtkPreciseHyperPoint *InsertNextPreciseHyperPoint() 
    {
    if ( ++this->MaxId >= this->Size )
      {
      this->Resize(this->MaxId);
      }
    return this->Array + this->MaxId;
    }
  vtkPreciseHyperPoint *Resize(vtkIdType sz); //reallocates data
  void Reset() {this->MaxId = -1;};

  vtkPreciseHyperPoint *Array;  // pointer to data
  vtkIdType MaxId;             // maximum index inserted thus far
  vtkIdType Size;              // allocated size of data
  vtkIdType Extend;            // grow array by this amount
  float Direction;       // integration direction
};
*/

vtkStandardNewMacro(vtkPreciseHyperStreamlinePoints);

//------------------------------------------------------------------------------
vtkPreciseHyperStreamlinePoints::vtkPreciseHyperStreamlinePoints()
{
  this->PreciseHyperStreamline0 = vtkPoints::New();
  this->PreciseHyperStreamline1 = vtkPoints::New();
  this->PreciseHyperStreamlines[0] = this->PreciseHyperStreamline0;
  this->PreciseHyperStreamlines[1] = this->PreciseHyperStreamline1;
}

//------------------------------------------------------------------------------
vtkPreciseHyperStreamlinePoints::~vtkPreciseHyperStreamlinePoints()
{
  this->PreciseHyperStreamline0->Delete();
  this->PreciseHyperStreamline1->Delete();
  this->PreciseHyperStreamlines[0] = this->PreciseHyperStreamlines[1] = NULL;
}

//------------------------------------------------------------------------------
void vtkPreciseHyperStreamlinePoints::Execute()
{
  vtkPreciseHyperPoint *sPtr;
  vtkIdType i, npts;
  int ptId;
  vtkIdType numIntPts;

  vtkDebugMacro(<<"Calling superclass execute");
  
  // default superclass behavior
  vtkPreciseHyperStreamline::Execute();

  vtkDebugMacro(<<"Grabbing superclass output points.");

  // just grab points of output to make them available to user
  // more info in BuildTube code in superclass
  //
  // Loop over all hyperstreamlines generating points
  //
  for (ptId=0; ptId < this->NumberOfStreamers; ptId++)
    {
      // if no points give up
      if ( (numIntPts=this->Streamers[ptId].GetNumberOfPoints()) < 1 )
        {
          continue;
        }

      // get first point
      i = 0;
      sPtr=this->Streamers[ptId].GetPreciseHyperPoint(i);

      // loop through all points
      for ( npts=0; i < numIntPts && sPtr->CellId >= 0;
            i++, sPtr=this->Streamers[ptId].GetPreciseHyperPoint(i) )
        {
          //for (j=0; j<3; j++) // grab point's coordinates
          //{
          //cout << sPtr->X[j] << " ";
          //}
          //cout << endl;

          this->PreciseHyperStreamlines[ptId]->InsertPoint(i,sPtr->X);
          npts++;
        } //while
    } //for this hyperstreamline

  vtkDebugMacro(<<"Done Grabbing superclass output points.");
}

//------------------------------------------------------------------------------
void vtkPreciseHyperStreamlinePoints::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPreciseHyperStreamline::PrintSelf(os,indent);
}
