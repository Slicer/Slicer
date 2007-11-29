/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkAbaqusFileWriter.h,v $

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkAbaqusFileWriter_h
#define __vtkAbaqusFileWriter_h

#include "vtkDataSet.h"
#include "vtkDataSetWriter.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMeshIOWin32Header.h"


class VTK_MIMXIO_EXPORT vtkAbaqusFileWriter : public vtkDataSetWriter
{
public:
  
  static vtkAbaqusFileWriter *New();
  vtkTypeRevisionMacro(vtkAbaqusFileWriter,vtkDataSetWriter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkSetMacro( Modulus, double );
  vtkSetMacro( PoissonsRatio, double );
  vtkSetMacro( StartingNodeNumber, int );
  vtkSetMacro( StartingElementNumber, int);
  void SetAbaqusHeader( char * );
  void SetAbaqusHeader( std::string );
  
protected:
  
  vtkAbaqusFileWriter();
  ~vtkAbaqusFileWriter();

  void UnstGridExecute( vtkUnstructuredGrid * );
  void PolyDataExecute( vtkPolyData * );
  void StructuredGridExecute(vtkStructuredGrid *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int StartingNodeNumber;
  int StartingElementNumber;
  

private:
  
  double Modulus;
  double PoissonsRatio;
  std::string  Title;
  std::string  AbaqusHeader;

  vtkAbaqusFileWriter(const vtkAbaqusFileWriter&);  // Not implemented.
  void operator=(const vtkAbaqusFileWriter&);  // Not implemented.

};

#endif
