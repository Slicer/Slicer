/*==========================================================================

  Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Modules/OpenIGTLinkIF/vtkIGTLToMRMLBase.h $
  Date:      $Date: 2010-01-28 19:02:08 -0500 (Thu, 28 Jan 2010) $
  Version:   $Revision: 11855 $

==========================================================================*/

#ifndef __vtkIGTLToMRMLBase_h
#define __vtkIGTLToMRMLBase_h

#include "vtkBrainlabModuleWin32Header.h"

#include "vtkObject.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "igtlMessageBase.h"
#include <vector>
#include <string>

class VTK_BRAINLABMODULE_EXPORT vtkIGTLToMRMLBase : public vtkObject
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
  virtual vtkMRMLNode* CreateNewNode(vtkMRMLScene* vtkNotUsed(scene), const char* vtkNotUsed(name))
    { return NULL; };

  // for TYPE_MULTI_IGTL_NAMES
  int                  GetNumberOfIGTLNames()   { return this->IGTLNames.size(); };
  const char*          GetIGTLName(int index)   { return this->IGTLNames[index].c_str(); };

  //BTX
  virtual int          IGTLToMRML(igtl::MessageBase::Pointer vtkNotUsed(buffer),
                                  vtkMRMLNode* vtkNotUsed(node)) { return 0; };
  //ETX
  virtual int          MRMLToIGTL(unsigned long vtkNotUsed(event), vtkMRMLNode* vtkNotUsed(mrmlNode),
                                  int* vtkNotUsed(size), void** vtkNotUsed(igtlMsg)){ return 0; };

  vtkGetMacro( CheckCRC, int );
  vtkSetMacro( CheckCRC, int );

 protected:
  vtkIGTLToMRMLBase();
  ~vtkIGTLToMRMLBase();

 protected:

  //BTX
  // list of IGTL names (used only when the class supports multiple IGTL names)
  std::vector<std::string>  IGTLNames;
  //ETX
  
  int CheckCRC;
  
};


#endif //__vtkIGTLToMRMLBase_h
