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
#include "vtkMRMLNode.h"

class VTK_OPENIGTLINKIF_EXPORT vtkIGTLToMRMLBase : public vtkObject
{
 public:

  static vtkIGTLToMRMLBase *New();
  vtkTypeRevisionMacro(vtkIGTLToMRMLBase,vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char*  GetIGTLName() { return NULL; };
  virtual const char*  GetMRMLName() { return NULL; };
  virtual vtkMRMLNode* GetNewNode()  { return NULL; };
  virtual void         UpdateNode(int event, vtkMRMLNode* node) {};
  virtual void         MRMLToIGTL(vtkMRMLNode* mrmlNode, int* size, void** igtlMsg) {};
  virtual vtkIntArray* GetNodeEvents() { return NULL; };

 protected:
  vtkIGTLToMRMLBase();
  ~vtkIGTLToMRMLBase();

 protected:
  
};


#endif //__vtkIGTLToMRMLBase_h
