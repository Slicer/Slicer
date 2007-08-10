/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxAbaqusFileWriter.h,v $
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


#ifndef __vtkMimxAbaqusFileWriter_h
#define __vtkMimxAbaqusFileWriter_h

#include "vtkBoundingBox.h"

#include "vtkDataSet.h"
#include "vtkDataSetWriter.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"


class VTK_BOUNDINGBOX_EXPORT vtkMimxAbaqusFileWriter : public vtkDataSetWriter
{
public:
  
  static vtkMimxAbaqusFileWriter *New();
  vtkTypeRevisionMacro(vtkMimxAbaqusFileWriter,vtkDataSetWriter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkSetMacro( Modulus, double );
  vtkSetMacro( PoissonsRatio, double );
  vtkSetMacro( StartingNodeNumber, int );
  vtkSetMacro( StartingElementNumber, int);
protected:
  
  vtkMimxAbaqusFileWriter();
  ~vtkMimxAbaqusFileWriter();

  void UnstGridExecute( vtkUnstructuredGrid * );
  void PolyDataExecute( vtkPolyData * );
  void StructuredGridExecute(vtkStructuredGrid *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int StartingNodeNumber;
  int StartingElementNumber;

private:
  
  double Modulus;
  double PoissonsRatio;

  vtkMimxAbaqusFileWriter(const vtkMimxAbaqusFileWriter&);  // Not implemented.
  void operator=(const vtkMimxAbaqusFileWriter&);  // Not implemented.

};

#endif
