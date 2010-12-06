/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLProceduralColorNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.0 $

=========================================================================auto=*/
///  vtkMRMLProceduralColorNode - MRML node to represent procedurally
/// defined color information.
/// 
/// Procedural nodes define methods that are used to map colours to scalar
/// values. Usually they will incorporate a custom subclass of a
/// vtkLookupTable, or a vtkColorTransferFunction.

#ifndef __vtkMRMLProceduralColorNode_h
#define __vtkMRMLProceduralColorNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLColorNode.h"


class vtkColorTransferFunction;
class vtkMRMLStorageNode;

class VTK_MRML_EXPORT vtkMRMLProceduralColorNode : public vtkMRMLColorNode
{
public:
  static vtkMRMLProceduralColorNode *New();
  vtkTypeMacro(vtkMRMLProceduralColorNode,vtkMRMLColorNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);
  
  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "ProceduralColor";};

  /// 
  /// 
  virtual void UpdateScene(vtkMRMLScene *scene);

  /// 
  /// Get/Set for Type. In SetType, set up the custom colour options for this
  /// set of colours
  virtual void SetType(int type);

  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  /// The list of valid procedural types
  //enum
  //{
  ///  
  //};

  /// DisplayModifiedEvent is generated when display node parameters is changed
  enum
    {
      DisplayModifiedEvent = 20000
    };

  /// 
  /// Get the color transfer function for this node
  vtkGetObjectMacro(ColorTransferFunction, vtkColorTransferFunction);

  /// 
  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode()
    {
    return Superclass::CreateDefaultStorageNode();
    };

  /// 
  /// set up some names, going from the points defined in the transfer function
  bool SetNameFromColor(int index);
  
  virtual int GetNumberOfColors();
  virtual bool GetColor(int entry, double* color);

protected:
  vtkMRMLProceduralColorNode();
  ~vtkMRMLProceduralColorNode();
  vtkMRMLProceduralColorNode(const vtkMRMLProceduralColorNode&);
  void operator=(const vtkMRMLProceduralColorNode&);
  
  /// 
  /// a color transfer function built up by calls to AddRGBPoint and Build
  vtkColorTransferFunction *ColorTransferFunction;
};

#endif
