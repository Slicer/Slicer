/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLDisplayNode - a supercalss for other storage nodes
// .SECTION Description
// a supercalss for other storage nodes like volume and model

#ifndef __vtkMRMLDisplayNode_h
#define __vtkMRMLDisplayNode_h


#include "vtkPolyData.h"
#include "vtkImageData.h"

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"


class VTK_MRML_EXPORT vtkMRMLDisplayNode : public vtkMRMLNode
{
  public:
  static vtkMRMLDisplayNode *New(){return NULL;};
  vtkTypeMacro(vtkMRMLDisplayNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance() = 0;
  
  // Description:
  // Gets PlyData converted from the real data in the node
  virtual vtkPolyData* GetPolyData() {return NULL;};
  
  // Description:
  // Gets ImageData converted from the real data in the node
  virtual vtkImageData* GetImageData() {return NULL;};

  // Description:
  // Update the pipeline based on this node attributes
  virtual void UpdatePolyDataPipeline() {};
 
  // Description:
  // Update the pipeline based on this node attributes
  virtual void UpdateImageDataPipeline() {};
 
  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts)
    {
    Superclass::ReadXMLAttributes(atts);
    };
  
  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent)
    {
    Superclass::WriteXML(of, indent);
    };

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node)
    {
    Superclass::Copy(node);
    };


  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName() = 0;


  // Description:
  // Propagate Progress Event generated in ReadData
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
    {
    Superclass::ProcessMRMLEvents(caller, event, callData);
    };


protected:
  vtkMRMLDisplayNode() {};
  ~vtkMRMLDisplayNode(){};
  vtkMRMLDisplayNode(const vtkMRMLDisplayNode&);
  void operator=(const vtkMRMLDisplayNode&);

};

#endif

