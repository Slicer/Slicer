/*=auto=========================================================================

Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: $
Date:      $Date: $
Version:   $Revision: $

=========================================================================auto=*/

#ifndef IGTMESSAGEGENERICATTRIBUTE_H
#define IGTMESSAGEGENERICATTRIBUTE_H

#include <string>
#include <vector>

#include "vtkIGTWin32Header.h" 
#include "vtkObject.h"
#include "vtkIGTMessageAttributeBase.h"

//BTX
template <class T>
//ETX
class VTK_IGT_EXPORT vtkIGTMessageGenericAttribute : public vtkIGTMessageAttributeBase {

public:

  static vtkIGTMessageGenericAttribute *New();
  vtkTypeRevisionMacro(vtkIGTMessageGenericAttribute,vtkIGTMessageAttributeBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkIGTMessageGenericAttribute();
  ~vtkIGTMessageGenericAttribute();

  /*
  //BTX
  template <typename T2> 
  //virtual int SetAttribute(T2* ptr);
  int SetAttribute(T2* ptr);

  template <typename T2>
  //virtual int GetAttribute(T2* ptr);
  int GetAttribute(T2* ptr);
  //ETX
  */
  virtual int SetAttribute(void* ptr);
  virtual int GetAttribute(void* ptr);

  int SetAttribute(T* ptr);
  int GetAttribute(T* ptr);

  virtual void ClearAttribute();

private:

  virtual int Alloc();
  virtual int Free();

  size_t size;
  T data;

};

//#ifdef VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION
#include "vtkIGTMessageGenericAttribute.txx"
//#endif 


#endif //IGTMESSAGEGENERICATTRIBUTE_H
