/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLProceduralColorNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.0 $

=========================================================================auto=*/
// .NAME vtkMRMLProceduralColorNode - MRML node to represent procedurally
// defined color information.
// .SECTION Description
// Procedural nodes define methods that are used to map colours to scalar
// values. Usually they will incorporate a custom subclass of a
// vtkLookupTable, or a vtkColorTransferFunction.

#ifndef __vtkMRMLProceduralColorNode_h
#define __vtkMRMLProceduralColorNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLColorNode.h"


class vtkColorTransferFunction;
class VTK_MRML_EXPORT vtkMRMLProceduralColorNode : public vtkMRMLColorNode
{
public:
  static vtkMRMLProceduralColorNode *New();
  vtkTypeMacro(vtkMRMLProceduralColorNode,vtkMRMLColorNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

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
  virtual const char* GetNodeTagName() {return "ProceduralColor";};

  // Description:
  // 
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Get/Set for Type. In SetType, set up the custom colour options for this
  // set of colours
  virtual void SetType(int type);

  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //BTX
  // Description:
  // The list of valid procedural types
  //enum
  //{
  //  
  //};
  //ETX

  //BTX
  // Description:
  // DisplayModifiedEvent is generated when display node parameters is changed
  enum
    {
      DisplayModifiedEvent = 20000,
    };
//ETX

  // Description:
  // Read in a text file with procedural definitions
  virtual int ReadFile ();

  // Description:
  // Get the color transfer function for this node
  vtkGetObjectMacro(ColorTransferFunction, vtkColorTransferFunction);

protected:
  vtkMRMLProceduralColorNode();
  ~vtkMRMLProceduralColorNode();
  vtkMRMLProceduralColorNode(const vtkMRMLProceduralColorNode&);
  void operator=(const vtkMRMLProceduralColorNode&);
  
  // Description:
  // a color transfer function built up by calls to AddRGBPoint and Build
  vtkColorTransferFunction *ColorTransferFunction;
};

#endif
