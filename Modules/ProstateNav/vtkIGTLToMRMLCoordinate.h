/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkIGTLToMRMLCoordinate_h
#define __vtkIGTLToMRMLCoordinate_h

#include "vtkObject.h"
#include "vtkProstateNavWin32Header.h"
#include "vtkMRMLNode.h"
#include "vtkIGTLToMRMLBase.h"

#include "igtlCoordinateMessage.h"

class VTK_PROSTATENAV_EXPORT vtkIGTLToMRMLCoordinate : public vtkIGTLToMRMLBase
{
 public:

  static vtkIGTLToMRMLCoordinate *New();
  vtkTypeRevisionMacro(vtkIGTLToMRMLCoordinate,vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char*  GetIGTLName() { return "COORDINATES"; };
  virtual const char*  GetMRMLName() { return "LinearTransform"; };
  virtual vtkIntArray* GetNodeEvents();
  virtual vtkMRMLNode* CreateNewNode(vtkMRMLScene* scene, const char* name);

  //BTX
  virtual int          IGTLToMRML(igtl::MessageBase::Pointer buffer, vtkMRMLNode* node);
  //ETX
  virtual int          MRMLToIGTL(unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg);


 protected:
  vtkIGTLToMRMLCoordinate();
  ~vtkIGTLToMRMLCoordinate();

 protected:
  //BTX
  igtl::CoordinateMessage::Pointer OutPositionMsg;
  //ETX
  
};


#endif //__vtkIGTLToMRMLCoordinate_h
