/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkIGTLToMRMLBase_h
#define __vtkIGTLToMRMLBase_h

#include "vtkObject.h"
#include "vtkOpenIGTLinkIFWin32Header.h" 
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "igtlMessageBase.h"

class VTK_OPENIGTLINKIF_EXPORT vtkIGTLToMRMLBase : public vtkObject
{
 public:

  static vtkIGTLToMRMLBase *New();
  vtkTypeRevisionMacro(vtkIGTLToMRMLBase,vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char*  GetIGTLName() { return NULL; };
  virtual const char*  GetMRMLName() { return NULL; };
  virtual vtkIntArray* GetNodeEvents() { return NULL; };
  virtual vtkMRMLNode* CreateNewNode(vtkMRMLScene* scene, const char* name)  { return NULL; };

  //BTX
  virtual int          IGTLToMRML(igtl::MessageBase::Pointer buffer, vtkMRMLNode* node) { return 0; };
  //ETX
  virtual int          MRMLToIGTL(unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg) { return 0; };


 protected:
  vtkIGTLToMRMLBase();
  ~vtkIGTLToMRMLBase();

 protected:
  
};


#endif //__vtkIGTLToMRMLBase_h
