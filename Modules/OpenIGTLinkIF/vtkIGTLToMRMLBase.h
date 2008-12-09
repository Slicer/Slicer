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
#include <vector>
#include <string>

class VTK_OPENIGTLINKIF_EXPORT vtkIGTLToMRMLBase : public vtkObject
{

 public:

  // IGTL to MRML Converter types (returned values from GetConverterType())
  // NOTE: if you want to define a child class that can handle multiple types
  // of OpenIGTLink messages, override GetConverterType() method to return
  // TYPE_MULTI_IGTL_NAME.
  //BTX
  enum {
    TYPE_NORMAL,            // supports only single IGTL message type (default)
    TYPE_MULTI_IGTL_NAMES,  // supports multiple IGTL message names (device types)
  };
  //ETX

 public:

  static vtkIGTLToMRMLBase *New();
  vtkTypeRevisionMacro(vtkIGTLToMRMLBase,vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int          GetConverterType() { return TYPE_NORMAL; };
  virtual const char*  GetIGTLName()      { return NULL;};
  virtual const char*  GetMRMLName()      { return NULL;};
  virtual vtkIntArray* GetNodeEvents()    { return NULL; };
  virtual vtkMRMLNode* CreateNewNode(vtkMRMLScene* scene, const char* name)  { return NULL; };

  // for TYPE_MULTI_IGTL_NAMES
  int                  GetNumberOfIGTLNames()   { return this->IGTLNames.size(); };
  const char*          GetIGTLName(int index)   { return this->IGTLNames[index].c_str(); };

  //BTX
  virtual int          IGTLToMRML(igtl::MessageBase::Pointer buffer, vtkMRMLNode* node) { return 0; };
  //ETX
  virtual int          MRMLToIGTL(unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg) { return 0; };


 protected:
  vtkIGTLToMRMLBase();
  ~vtkIGTLToMRMLBase();

 protected:

  //BTX
  // list of IGTL names (used only when the class supports multiple IGTL names)
  std::vector<std::string>  IGTLNames;
  //ETX

  
};


#endif //__vtkIGTLToMRMLBase_h
