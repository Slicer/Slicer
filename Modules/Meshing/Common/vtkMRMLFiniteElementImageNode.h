/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiniteElementImageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLFiniteElementImageNode_h
#define __vtkMRMLFiniteElementImageNode_h

#include "vtkMimxCommonWin32Header.h"
#include "vtkMRML.h"
#include "vtkMRMLScalarVolumeNode.h"
//#include "vtkMRMLStorageNode.h"

// keep a pointer to the Mimx actor object because we are storing a direct reference
// to the actor so the application code doesn't have to be rearranged with all attributes
// added to the node before the commit is done.  If the same actor is used, in the local and MRML
// lists, then we know that all the values will be good in the actor.  Use the names in the actors
// instead of the separate fields for matching / searching. 

// searching the MRML-backed lists is needed because as surface,bblock, and mesh objects are created,
// they are stored in separate lists, according to their MRML type.  However, the application believes
// there is only one polymorphic list, so when objects are returned or removed by ID, a search is required
// to find the matching object. 

// since we are including a reference to this object in the MRML node, its definition is included here
#include "vtkMimxImageActor.h"

class VTK_MIMXCOMMON_EXPORT vtkMRMLFiniteElementImageNode : public vtkMRMLScalarVolumeNode
{
  public:
  static vtkMRMLFiniteElementImageNode *New();
  vtkTypeMacro(vtkMRMLFiniteElementImageNode,vtkMRMLScalarVolumeNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLScalarVolumeNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "FiniteElementImage";};
  
  // Description:

  // Set pointer to the Mimx actor that contains the state variables
   void SetMimxImageActor(vtkMimxImageActor* ptr) {actor = ptr;}
   vtkMimxImageActor* GetMimxImageActor(void) {return actor;}
  
  void SetDataType(int value) {this->actor->SetDataType(value);}   
  int  GetDataType(void)      {return this->actor->GetDataType();} 

  char*  GetFileName(void)      {return this->actor->GetFileName();} 

  void   SetFilePath(char* value) {this->actor->SetFilePath(value);}   
  char*  GetFilePath(void)      {return this->actor->GetFilePath();} 
  
  void SetSavedVisibilityState(bool state) {this->savedVisabilityState = state;}
  bool GetSavedVisibilityState(void) {return this->savedVisabilityState;}


  protected:
  vtkMRMLFiniteElementImageNode();
  ~vtkMRMLFiniteElementImageNode();
  vtkMRMLFiniteElementImageNode(const vtkMRMLFiniteElementImageNode&);
  void operator=(const vtkMRMLFiniteElementImageNode&);

  vtkMimxImageActor* actor;
  
  // remember whether this node was on or off when IA_FEMesh was onscreen.  The state is stored here
  // during module exit and checked during module entry. 
  bool savedVisabilityState;
};

#endif

