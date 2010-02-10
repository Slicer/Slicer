/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLClipModelsNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
///  vtkMRMLClipModelsNode - MRML node to represent three clipping planes.
/// 
/// The vtkMRMLClipModelsNode MRML node stores 
/// the direction of clipping for each of the three clipping planes.
/// It also stored  the type of combined clipping operation as either intersection or union

#ifndef __vtkMRMLClipModelsNode_h
#define __vtkMRMLClipModelsNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

class VTK_MRML_EXPORT vtkMRMLClipModelsNode : public vtkMRMLNode
{
public:
  static vtkMRMLClipModelsNode *New();
  vtkTypeMacro(vtkMRMLClipModelsNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "ClipModels";};

  /// 
  /// Indicates the type of clipping
  /// "Intersection" or "Union"
  vtkGetMacro(ClipType, int);
  vtkSetMacro(ClipType, int);

  //BTX
  enum
    {
      ClipIntersection = 0,
      ClipUnion = 1
    };
  //ETX

  /// 
  /// Indicates if the Red slice clipping is Off, 
  /// Positive space, or Negative space
  vtkGetMacro(RedSliceClipState, int);
  vtkSetMacro(RedSliceClipState, int);

  /// 
  /// Indicates if the Yellow slice clipping is Off, 
  /// Positive space, or Negative space
  vtkGetMacro(YellowSliceClipState, int);
  vtkSetMacro(YellowSliceClipState, int);

  /// 
  /// Indicates if the Green slice clipping is Off, 
  /// Positive space, or Negative space
  vtkGetMacro(GreenSliceClipState, int);
  vtkSetMacro(GreenSliceClipState, int);

  //BTX
  enum
    {
      ClipOff = 0,
      ClipPositiveSpace = 1,
      ClipNegativeSpace = 2,
    };
  //ETX
  
protected:
  vtkMRMLClipModelsNode();
  ~vtkMRMLClipModelsNode();
  vtkMRMLClipModelsNode(const vtkMRMLClipModelsNode&);
  void operator=(const vtkMRMLClipModelsNode&);

  int ClipType;

  int RedSliceClipState;
  int YellowSliceClipState;
  int GreenSliceClipState;


};

#endif
