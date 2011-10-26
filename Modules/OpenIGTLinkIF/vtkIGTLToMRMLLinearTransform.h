/*==========================================================================

  Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkIGTLToMRMLLinearTransform_h
#define __vtkIGTLToMRMLLinearTransform_h

#include "vtkObject.h"
#include "vtkOpenIGTLinkIFWin32Header.h" 
#include "vtkMRMLNode.h"
#include "vtkIGTLToMRMLBase.h"

#include "igtlTransformMessage.h"

class VTK_OPENIGTLINKIF_EXPORT vtkIGTLToMRMLLinearTransform : public vtkIGTLToMRMLBase
{
 public:

  static vtkIGTLToMRMLLinearTransform *New();
  vtkTypeRevisionMacro(vtkIGTLToMRMLLinearTransform,vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char*  GetIGTLName() { return "TRANSFORM"; };
  virtual const char*  GetMRMLName() { return "LinearTransform"; };
  virtual vtkIntArray* GetNodeEvents();
  virtual vtkMRMLNode* CreateNewNode(vtkMRMLScene* scene, const char* name);

  virtual int          IGTLToMRML(igtl::MessageBase::Pointer buffer, vtkMRMLNode* node);
  virtual int          MRMLToIGTL(unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg);


 protected:
  vtkIGTLToMRMLLinearTransform();
  ~vtkIGTLToMRMLLinearTransform();

 protected:
  igtl::TransformMessage::Pointer OutTransformMsg;
  
};


#endif //__vtkIGTLToMRMLLinearTransform_h
