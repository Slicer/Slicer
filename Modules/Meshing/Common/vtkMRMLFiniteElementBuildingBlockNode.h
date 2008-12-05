/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiniteElementBuildingBlockNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLFiniteElementBuildingBlockNode_h
#define __vtkMRMLFiniteElementBuildingBlockNode_h

//#include "vtkMRML.h"
#include "vtkMRMLUnstructuredGridNode.h"

// keep a pointer to the Mimx actor object because we are storing a direct reference
// to the actor so the application code doesn't have to be rearranged with all attributes
// added to the node before the commit is done.  If the same actor is used, in the local and MRML
// lists, then we know that all the values will be good in the actor.  Use the names in the actors
// instead of the separate fields for matching / searching. 

// searching the MRML-backed lists is needed because as surface,bblock, and mesh objects are created,
// they are stored in separate lists, according to their MRML type.  However, the application believes
// there is only one polymorphic list, so when objects are returned or removed by ID, a search is required
// to find the matching object. 

#include  "vtkMimxUnstructuredGridActor.h"
#include "vtkMimxCommonWin32Header.h"

class VTK_MIMXCOMMON_EXPORT vtkMRMLFiniteElementBuildingBlockNode : public vtkMRMLUnstructuredGridNode
{
  public:
  static vtkMRMLFiniteElementBuildingBlockNode *New();
  vtkTypeMacro(vtkMRMLFiniteElementBuildingBlockNode,vtkMRMLUnstructuredGridNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLFiniteElementBuildingBlockNode* CreateNodeInstance();

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
  virtual const char* GetNodeTagName() {return "FiniteElementBuildingBlock";};
  
  // Description:
  // Set pointer to the Mimx actor that contains the state variables
  void SetMimxUnstructuredGridActor(vtkMimxUnstructuredGridActor* ptr) {actor = ptr;}
  vtkMimxUnstructuredGridActor* GetMimxUnstructuredGridActor(void) {return actor;}

  // don't use VTK macros  because the values are stored in an actor instance
  void   SetDataType(int value) {this->actor->SetDataType(value);}   
  int    GetDataType(void)      {return this->actor->GetDataType();} 
  // *** svoid   SetFileName(char* value) {this->actor->SetFileName(value);}   
  char*  GetFileName(void)      {return this->actor->GetFileName();} 
  void   SetFilePath(char* value) {this->actor->SetFilePath(value);}   
  char*  GetFilePath(void)      {return this->actor->GetFilePath();} 
 
protected:
  vtkMRMLFiniteElementBuildingBlockNode();
  ~vtkMRMLFiniteElementBuildingBlockNode();
  vtkMRMLFiniteElementBuildingBlockNode(const vtkMRMLFiniteElementBuildingBlockNode&);
  void operator=(const vtkMRMLFiniteElementBuildingBlockNode&);

 
  vtkMimxUnstructuredGridActor *actor;
};

#endif

