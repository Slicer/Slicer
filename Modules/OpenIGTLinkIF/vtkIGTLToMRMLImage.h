/*==========================================================================

  Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkIGTLToMRMLImage_h
#define __vtkIGTLToMRMLImage_h

#include "vtkObject.h"
#include "vtkOpenIGTLinkIFWin32Header.h" 
#include "vtkMRMLNode.h"
#include "vtkIGTLToMRMLBase.h"

#include "igtlImageMessage.h"

class vtkMRMLVolumeNode;

class VTK_OPENIGTLINKIF_EXPORT vtkIGTLToMRMLImage : public vtkIGTLToMRMLBase
{
 public:

  static vtkIGTLToMRMLImage *New();
  vtkTypeRevisionMacro(vtkIGTLToMRMLImage,vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char*  GetIGTLName() { return "IMAGE"; };
  virtual const char*  GetMRMLName() { return "Volume"; };
  virtual vtkIntArray* GetNodeEvents();
  virtual vtkMRMLNode* CreateNewNode(vtkMRMLScene* scene, const char* name);

  //BTX
  virtual int          IGTLToMRML(igtl::MessageBase::Pointer buffer, vtkMRMLNode* node);
  //ETX
  virtual int          MRMLToIGTL(unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg);


 protected:
  vtkIGTLToMRMLImage();
  ~vtkIGTLToMRMLImage();

  void CenterImage(vtkMRMLVolumeNode *volumeNode);

 protected:
  //BTX
  igtl::ImageMessage::Pointer OutImageMessage;
  igtl::GetImageMessage::Pointer GetImageMessage;
  //ETX
  
};


#endif //__vtkIGTLToMRMLImage_h
