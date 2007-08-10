/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxGenerateIntervertebralDisc.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.6 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// .NAME vtkMimxGenerateIntervertebralDisc - 
// .SECTION Caveats
// Filter takes in total 3 inputs, 1) Bottom vertebral body 2) Top vertebral body
// 3) and corresponding bounding box definition. User should make sure the two
// vertebral bodies used use the same mesh seeds and the corresponding cell number
// locations should be very similar with change only along the axis of vertebtal 
// body. The vertebral bodies should be oriented such that their axis lie along
// superior inferior axis.

#ifndef __vtkMimxGenerateIntervertebralDisc_h
#define __vtkMimxGenerateIntervertebralDisc_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridAlgorithm.h"

class vtkPolyData;
class vtkStructuredGrid;

class VTK_MIMXFILTER_EXPORT vtkMimxGenerateIntervertebralDisc : public vtkUnstructuredGridAlgorithm
{
public:

  static vtkMimxGenerateIntervertebralDisc *New();
  vtkTypeRevisionMacro(vtkMimxGenerateIntervertebralDisc,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkSetMacro(DivisionsAlongAxis, int);
  vtkSetMacro(OuterBulgeOffset, double);
  vtkSetMacro(InnerBulgeOffset, double);
  void SetLowerVertebralBody(vtkUnstructuredGrid* );
  void SetUpperVertebralBody(vtkUnstructuredGrid* );
  void SetBoundingBox(vtkUnstructuredGrid *);
  vtkGetMacro(DiscBoundingBox, vtkUnstructuredGrid*);
protected:
  vtkMimxGenerateIntervertebralDisc();
  ~vtkMimxGenerateIntervertebralDisc();
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  // number of divisions along the axis of the vertebral body
  // if it were assumed to be an irregular cylinder
  int DivisionsAlongAxis;
  // to define  the bulge observed in a typical intervertebral disc
  double OuterBulgeOffset;
  double InnerBulgeOffset;
  vtkUnstructuredGrid *DiscBoundingBox;
  void EdgeFromPoints(int BoxNum, int EdgeNum, vtkPolyData *Edge1, vtkPolyData *Edge2, 
          vtkPolyData *OutputEdge, int BeginOrEnd);
  double Center[3];
  vtkStructuredGrid *UpSGrid[9];
  vtkStructuredGrid *LowSGrid[9];
  vtkStructuredGrid *DiscSGrid[9];
  void GenerateStructuredGrid(int SGridNum);
  void MergeSGridToUGrid();
  private:
  vtkMimxGenerateIntervertebralDisc(const vtkMimxGenerateIntervertebralDisc&);  // Not implemented.
  void operator=(const vtkMimxGenerateIntervertebralDisc&);  // Not implemented.
};

#endif
