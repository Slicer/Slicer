/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLChartNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLChartViewNode - MRML node to represent chart view parameters.
///   
///  ChartViewNodes are associated one to one with a ChartWidget.
///
#ifndef __vtkMRMLChartViewNode_h
#define __vtkMRMLChartViewNode_h

#include "vtkMRMLViewNode.h"

class VTK_MRML_EXPORT vtkMRMLChartViewNode : public vtkMRMLViewNode
{
public:
  static vtkMRMLChartViewNode *New();
  vtkTypeMacro(vtkMRMLChartViewNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName();

  ///
  /// Set the Chart node id displayed in this Chart View
  void SetChartNodeID(const char *);

  ///
  /// Get the Chart node id displayed in this Chart View
  vtkGetStringMacro(ChartNodeID);

protected:
  vtkMRMLChartViewNode();
  ~vtkMRMLChartViewNode();
  vtkMRMLChartViewNode(const vtkMRMLChartViewNode&);
  void operator=(const vtkMRMLChartViewNode&);

  char* ChartNodeID;
};

#endif
