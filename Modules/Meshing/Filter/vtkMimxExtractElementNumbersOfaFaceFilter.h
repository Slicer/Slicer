/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractElementNumbersOfaFaceFilter.h,v $
Language:  C++
Date:      $Date: 2007/11/06 22:43:57 $
Version:   $Revision: 1.2 $

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

// .NAME vtkMimxExtractElementNumbersOfaFaceFilter - extracts cells where scalar value in cell satisfies threshold criterion
// .SECTION Description
// vtkMimxExtractElementNumbersOfaFaceFilter implements laplacian smoothing algorithm in 3D 
// solid grids. Currently it only implements smoothing to only to unstructured 
// topologically correct solid mesh.

#ifndef __vtkMimxExtractElementNumbersOfaFaceFilter_h
#define __vtkMimxExtractElementNumbersOfaFaceFilter_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkIntArray.h"
//#include "vtkIdList.h"
#include "vtkMimxFilterWin32Header.h"

class vtkUnstructuredGrid;

class VTK_MIMXFILTER_EXPORT vtkMimxExtractElementNumbersOfaFaceFilter : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkMimxExtractElementNumbersOfaFaceFilter *New();
  vtkTypeRevisionMacro(vtkMimxExtractElementNumbersOfaFaceFilter,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
        
  vtkSetObjectMacro (VertexList, vtkIntArray);
  //vtkGetObjectMacro (CellIdList, vtkIn);
  void SetBoundingBox(vtkUnstructuredGrid *);
  void SetFeMesh(vtkUnstructuredGrid *);
  vtkSetMacro(ElementSetName, const char*);
protected:
  vtkMimxExtractElementNumbersOfaFaceFilter();
  ~vtkMimxExtractElementNumbersOfaFaceFilter();
  vtkIntArray *VertexList ;
  vtkIntArray *CellIdList; 
  

  void GetFace(int ,int [], int);

  void GetFace0(int [] , int );
  void GetFace1(int [] , int );
  void GetFace2(int [] , int );
  void GetFace3(int [] , int );
  void GetFace4(int [] , int );
  void GetFace5(int [] , int );


  // Usual data generation method
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  virtual int FillInputPortInformation(int port, vtkInformation *info);
  
  const char *ElementSetName;
  
private:
  vtkMimxExtractElementNumbersOfaFaceFilter(const vtkMimxExtractElementNumbersOfaFaceFilter&);  // Not implemented.
  void operator=(const vtkMimxExtractElementNumbersOfaFaceFilter&);  // Not implemented.
};

#endif
