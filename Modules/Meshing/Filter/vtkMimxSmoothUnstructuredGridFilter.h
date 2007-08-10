/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSmoothUnstructuredGridFilter.h,v $
Language:  C++
Date:      $Date: 2007/07/12 14:15:21 $
Version:   $Revision: 1.8 $

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

// .NAME vtkMimxSmoothUnstructuredGridFilter - extracts cells where scalar value in cell satisfies threshold criterion
// .SECTION Description
// vtkMimxSmoothUnstructuredGridFilter implements laplacian smoothing algorithm in 3D 
// solid grids. Currently it only implements smoothing to only to unstructured 
// topologically correct solid mesh.

#ifndef __vtkMimxSmoothUnstructuredGridFilter_h
#define __vtkMimxSmoothUnstructuredGridFilter_h

#include "vtkFilter.h"

#include "vtkUnstructuredGridAlgorithm.h"

class vtkIntArray;
class vtkPolyData;
class vtkIdList;

class VTK_MIMXFILTER_EXPORT vtkMimxSmoothUnstructuredGridFilter : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkMimxSmoothUnstructuredGridFilter *New();
  vtkTypeRevisionMacro(vtkMimxSmoothUnstructuredGridFilter,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Number of smoothing iterations
  vtkSetMacro(NumberOfIterations,int);
  vtkGetMacro(NumberOfIterations,int);
  vtkSetMacro(ConstrainedSmoothing,int);
 // vtkSetMacro(Source,vtkPolyData*);
 // vtkGetMacro(Source,vtkPolyData*);
    void SetSource(vtkPolyData* polydata);
protected:
  vtkMimxSmoothUnstructuredGridFilter();
  ~vtkMimxSmoothUnstructuredGridFilter();

  // Usual data generation method
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  int    NumberOfIterations;
  void InsertConnectedNeighbors(vtkIdType pos, vtkIdList* CellIdList, vtkIdList* ConnectedNeighbors);
  vtkPolyData* Source;
  int ConstrainedSmoothing;
private:
  vtkMimxSmoothUnstructuredGridFilter(const vtkMimxSmoothUnstructuredGridFilter&);  // Not implemented.
  void operator=(const vtkMimxSmoothUnstructuredGridFilter&);  // Not implemented.
};

#endif
