/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLChartNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLChartViewNode_h
#define __vtkMRMLChartViewNode_h

#include "vtkMRMLAbstractViewNode.h"

/// \brief MRML node to represent chart view parameters.
///
/// ChartViewNodes are associated one to one with a ChartWidget.
class VTK_MRML_EXPORT vtkMRMLChartViewNode : public vtkMRMLAbstractViewNode
{
public:
  static vtkMRMLChartViewNode *New();
  vtkTypeMacro(vtkMRMLChartViewNode, vtkMRMLAbstractViewNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

   vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;


  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override;

  ///
  /// Set the Chart node id displayed in this Chart View
  void SetChartNodeID(const char *);

  ///
  /// Get the Chart node id displayed in this Chart View
  vtkGetStringMacro(ChartNodeID);

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  ///
  /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override;

  ///
  /// Events
  enum
  {
    ChartNodeChangedEvent = 16000
  };

protected:
  vtkMRMLChartViewNode();
  ~vtkMRMLChartViewNode() override;
  vtkMRMLChartViewNode(const vtkMRMLChartViewNode&);
  void operator=(const vtkMRMLChartViewNode&);

  char* ChartNodeID;
};

#endif
