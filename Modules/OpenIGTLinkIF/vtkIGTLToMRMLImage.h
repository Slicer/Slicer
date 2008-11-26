/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkIGTLToMRMLImage_h
#define __vtkIGTLToMRMLImage_h

#include "vtkObject.h"
#include "vtkOpenIGTLinkIFWin32Header.h" 
#include "vtkMRMLNode.h"
#include "vtkIGTLToMRMLBase.h"

#include "igtlTransformMessage.h"

class VTK_OPENIGTLINKIF_EXPORT vtkIGTLToMRMLImage : public vtkObject
{
 public:

  static vtkIGTLToMRMLImage *New();
  vtkTypeRevisionMacro(vtkIGTLToMRMLImage,vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char*  GetIGTLName() { return "IMAGE"; };
  virtual const char*  GetMRMLName() { return "Volume"; };
  virtual vtkMRMLNode* GetNewNode(const char* name);
  virtual vtkIntArray* GetNodeEvents();
  //BTX
  virtual int          IGTLToMRML(igtl::MessageBase::Pointer buffer, vtkMRMLNode* node);
  //ETX
  virtual int          MRMLToIGTL(int event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg);


 protected:
  vtkIGTLToMRMLImage();
  ~vtkIGTLToMRMLImage();

 protected:
  //BTX
  igtl::TransformMessage::Pointer OutTransformMsg;
  //ETX
  
};


#endif //__vtkIGTLToMRMLImage_h
