/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

// .NAME vtkITKImageToImageFilter - Abstract base class for connecting ITK and VTK
// .SECTION Description
// vtkITKImageToImageFilter provides a 

#ifndef __vtkITKImageWriter_h
#define __vtkITKImageWriter_h

#include "vtkProcessObject.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkMatrix4x4.h"

#include "vtkITK.h"

class VTK_ITK_EXPORT vtkITKImageWriter : public vtkProcessObject
{
public:

  static vtkITKImageWriter *New();
  vtkTypeRevisionMacro(vtkITKImageWriter,vtkProcessObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Specify file name for the image file. You should specify either
  // a FileName or a FilePrefix. Use FilePrefix if the data is stored 
  // in multiple files.
  void SetFileName(const char *);

  char *GetFileName() {
    return FileName;
  }

  // Description:
  // use compression if possible
  vtkGetMacro (UseCompression, int);
  vtkSetMacro (UseCompression, int);

  // Description:
  // Set/Get the input object from the image pipeline.
  void SetInput(vtkImageData *input);

  vtkImageData *GetInput();

  // Description:
  // The main interface which triggers the writer to start.
  void Write();

  // Set orienation matrix
  void SetRasToIJKMatrix( vtkMatrix4x4* mat) {
    RasToIJKMatrix = mat;
  }

protected:
  vtkITKImageWriter();
  ~vtkITKImageWriter();

  char *FileName;
  vtkMatrix4x4* RasToIJKMatrix;
  int UseCompression;

private:
  vtkITKImageWriter(const vtkITKImageWriter&);  // Not implemented.
  void operator=(const vtkITKImageWriter&);  // Not implemented.
};

//vtkStandardNewMacro(vtkITKImageWriter)

#endif





