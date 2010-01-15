/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxAbaqusFileWriter.h,v $
Language:  C++
Date:      $Date: 2008/08/09 20:35:08 $
Version:   $Revision: 1.18 $

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

#include "vtkDataSet.h"
#include "vtkDataSetWriter.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include  "vtkFloatArray.h"
#include "vtkMimxIOWin32Header.h"

class VTK_MIMXIO_EXPORT vtkMimxAbaqusFileWriter : public vtkDataSetWriter
{
public:
  
  static vtkMimxAbaqusFileWriter *New();
  vtkTypeRevisionMacro(vtkMimxAbaqusFileWriter,vtkDataSetWriter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void SetFileName( const std::string & fieldStr)
    { this->vtkDataSetWriter::SetFileName( fieldStr.c_str() ); }
  //BTX
  using vtkDataWriter::SetFileName;
  //ETX
  void SetNodeElementFileName( const std::string & fieldStr ) {NodeElementFileName = fieldStr;} 
  void SetHeaderInformation( const std::string & fieldStr ) {HeaderInformation = fieldStr;} 
  void SetUserName( const std::string & fieldStr) {UserName = fieldStr;} 
  
  //Get/Set the Precision used for writing material properties
  vtkGetMacro (Precision, int);
  vtkSetMacro (Precision, int);
  
protected:
  
  vtkMimxAbaqusFileWriter();
  ~vtkMimxAbaqusFileWriter();

  void WriteHeader( ostream& os );
  void WriteHeadingSection( ostream& os );
  void WriteNodeElementHeader( ostream& os );
  void WriteNodes( ostream& os, vtkUnstructuredGrid *grid );
  void WriteNodeSets(ostream& os, vtkUnstructuredGrid *grid );
  void WriteHexElements( ostream& os, vtkUnstructuredGrid *grid );
  void WriteTetElements( ostream& os, vtkUnstructuredGrid *grid );
  void WriteQuadElements( ostream& os, vtkUnstructuredGrid *grid );
  void WriteElementSets( ostream& os, vtkUnstructuredGrid *grid );
  void WriteMaterialHeader( ostream& os );
  void WriteMaterialProperties( ostream& os, ostream& osi, vtkUnstructuredGrid *grid);
  void WriteBoundaryConditions( ostream& os, vtkUnstructuredGrid *grid );
  void WriteFooter( ostream& os );
  void WriteRepeatingMaterialProperties(vtkIntArray *ElementIds, vtkDoubleArray *MatProp, 
          vtkUnstructuredGrid *grid, ostream& os, const char* ElementSetName, double PoissonsRatio) ;
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  void ConcatenateStrings(const char*, const char*, 
          const char*, const char*, const char*, char*);
  void PrintVariablesOfInterest(vtkStdString &variable, ostream &os);
private:
  
  vtkMimxAbaqusFileWriter(const vtkMimxAbaqusFileWriter&);  // Not implemented.
  void operator=(const vtkMimxAbaqusFileWriter&);  // Not implemented.
  
  // Filenames for the output files
  std::string NodeElementFileName;
  std::string UserName;
  
  // Descriptive fields for the mesh
  std::string HeaderInformation;
  
  int Precision;
};

#endif
