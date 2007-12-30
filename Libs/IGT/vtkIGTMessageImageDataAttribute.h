/*=auto=========================================================================

Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: $
Date:      $Date: $
Version:   $Revision: $

=========================================================================auto=*/

#ifndef IGTMESSAGEIMAGEDATAATTRIBUTE_H
#define IGTMESSAGEIMAGEDATAATTRIBUTE_H

#include "vtkObject.h"
#include "vtkIGTWin32Header.h" 

#include "vtkIGTMessageAttributeBase.h"
#include "vtkImageData.h"

class VTK_IGT_EXPORT vtkIGTMessageImageDataAttribute : public vtkIGTMessageAttributeBase {
public:

  static vtkIGTMessageImageDataAttribute *New();
  vtkTypeRevisionMacro(vtkIGTMessageImageDataAttribute, vtkIGTMessageAttributeBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkIGTMessageImageDataAttribute();
  ~vtkIGTMessageImageDataAttribute();

  /*
  //BTX
  template <typename T> 
  //virtual int SetData(T* ptr);
  int SetAttribute(T* ptr);

  template <typename T>
  //virtual int GetData(T* ptr);
  int GetAttribute(T* ptr);
  //ETX
  */
  virtual int SetAttribute(void* ptr);
  virtual int GetAttribute(void* ptr);

  int SetAttribute(vtkImageData* ptr);
  int GetAttribute(vtkImageData* ptr);


  virtual void ClearAttribute();

private:

  virtual int Alloc();
  virtual int Free();

  vtkImageData* data;

};


#endif // IGTMESSAGEIMAGEDATAATTRIBUTE_H
