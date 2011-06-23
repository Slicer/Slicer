/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiniteElementMeshNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLFiniteElementMeshNode_h
#define __vtkMRMLFiniteElementMeshNode_h

//#include "vtkMRML.h"
#include "vtkMRMLUnstructuredGridNode.h"

#include "vtkMimxMeshActor.h"
#include "vtkMimxCommonWin32Header.h"

class VTK_MIMXCOMMON_EXPORT vtkMRMLFiniteElementMeshNode : public vtkMRMLUnstructuredGridNode
{
  public:
  static vtkMRMLFiniteElementMeshNode *New();
  vtkTypeMacro(vtkMRMLFiniteElementMeshNode,vtkMRMLUnstructuredGridNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLFiniteElementMeshNode* CreateNodeInstance();

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
  virtual const char* GetNodeTagName() {return "FiniteElementMesh";};

  // Description:
  // Set pointer to the Mimx actor that contains the state variables
  vtkSetObjectMacro(MimxMeshActor, vtkMimxMeshActor);
  vtkGetObjectMacro(MimxMeshActor, vtkMimxMeshActor);

   // don't use VTK macros  because the values are stored in an actor instance
   void   SetDataType(int value) {this->MimxMeshActor->SetDataType(value);}   
   int    GetDataType(void)      {return this->MimxMeshActor->GetDataType();} 
 //***  void   SetFileName(char* value) {this->MimxMeshActor->SetFileName(value);}   
   char*  GetFileName(void)      {return this->MimxMeshActor->GetFileName();} 
   void   SetFilePath(char* value) {this->MimxMeshActor->SetFilePath(value);}   
   char*  GetFilePath(void)      {return this->MimxMeshActor->GetFilePath();} 
   
   void SetSavedVisibilityState(bool state) {this->savedVisibilityState = state;}
   bool GetSavedVisibilityState(void) {return this->savedVisibilityState;}  
  
protected:
  vtkMRMLFiniteElementMeshNode();
  ~vtkMRMLFiniteElementMeshNode();
  vtkMRMLFiniteElementMeshNode(const vtkMRMLFiniteElementMeshNode&);
  void operator=(const vtkMRMLFiniteElementMeshNode&);

  // store the state inside an actor, so it can share with the local list
  vtkMimxMeshActor* MimxMeshActor;
  
  // remember whether this node was on or off when IA_FEMesh was onscreen.  The state is stored here
  // during module exit and checked during module entry. 
  bool savedVisibilityState;
};

#endif

