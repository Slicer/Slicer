/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLNode.h,v $
  Date:      $Date: 2006/02/11 17:20:11 $
  Version:   $Revision: 1.9 $

=========================================================================auto=*/
// .NAME vtkMRMLNode - Abstract Superclass for all specific types of MRML nodes.
// .SECTION Description
// This node encapsulates the functionality common to all types of MRML nodes.
// This includes member variables for ID, Description, and Options,
// as well as member functions to Copy() and Write().

#ifndef __vtkMRMLNode_h
#define __vtkMRMLNode_h

#include "vtkObject.h"

class VTK_EXPORT vtkMRMLNode : public vtkObject
{
public:
  static vtkMRMLNode *New();
  vtkTypeMacro(vtkMRMLNode,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Create instance of the default node. Like New only virtual
  // NOTE: Subclasses should implement this method
  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  // Description:
  // Set node attributes
  // NOTE: Subclasses should implement this method
  // NOTE: Call this method in the subclass impementation
  virtual void ReadXMLAttributes(const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  // NOTE: Subclasses should implement this method
  // NOTE: Call this method in the subclass impementation
  virtual void WriteXML(ostream& of, int indent);
  
  // Read data for the node
  // NOTE: Subclasses should implement this method
  virtual void ReadData() = 0;
  
  // Write data for the node
  // NOTE: Subclasses should implement this method
  virtual void WriteData() = 0;

  // Description:
  // Copy everything from another node of the same type.
  // NOTE: Subclasses should implement this method
  // NOTE: Call this method in the subclass impementation
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  // NOTE: Subclasses should implement this method
  virtual char* GetNodeTagName() = 0;
  
  // Description:
  // Set/Get a numerical ID for the calling program to use to keep track
  // of its various node objects.
  vtkSetMacro(ID, unsigned int);
  vtkGetMacro(ID, unsigned int);
  
  // Description:
  // Text description of this node, to be set by the user
  vtkSetStringMacro(Description);
  vtkGetStringMacro(Description);

  // Description:
  // Root directory of MRML scene
  vtkSetStringMacro(SceneRootDir);
  vtkGetStringMacro(SceneRootDir);
  
  // Description:
  // Name of this node, to be set by the user
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);
  
  
  // Description:
  // Name of space in which this node lives
  vtkSetStringMacro(SpaceName);
  vtkGetStringMacro(SpaceName);
  
  // Description:
  // Node's effect on indentation when displaying the
  // contents of a MRML file. (0, +1, -1)
  vtkGetMacro(Indent, int);
  
protected:
  
  vtkMRMLNode();
  // critical to have a virtual destructor!
  virtual ~vtkMRMLNode();
  vtkMRMLNode(const vtkMRMLNode&);
  void operator=(const vtkMRMLNode&);
  
  vtkSetMacro(Indent, int);
  
  unsigned int ID;
  char *Description;
  char *SceneRootDir;
  char *Name;
  char *SpaceName;
  int Indent;
};

#endif



