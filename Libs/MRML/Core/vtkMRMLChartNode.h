/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLChartNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLChartNode - MRML node for referencing a collection of data
///  to plot
/// 
//

#ifndef __vtkMRMLChartNode_h
#define __vtkMRMLChartNode_h

#include "vtkMRMLNode.h"

class vtkDataObject;
class vtkStringArray;
class DoubleArrayIDMap;
class ChartPropertyMap;

#include <string>

class VTK_MRML_EXPORT vtkMRMLChartNode : public vtkMRMLNode
{
 public:
  //----------------------------------------------------------------
  /// Constants
  //----------------------------------------------------------------



  //----------------------------------------------------------------
  /// Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLChartNode *New();
  vtkTypeMacro(vtkMRMLChartNode,vtkMRMLNode);
  
  void PrintSelf(ostream& os, vtkIndent indent);

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
  virtual const char* GetNodeTagName()
    {return "Chart";};

  /// 
  /// Method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  //----------------------------------------------------------------
  /// Access methods
  //----------------------------------------------------------------

  ///
  /// Add a double array to the chart. Parameter "name" is used for
  /// referencing the array when setting properties for plotting the
  /// array or for removing the array from the chart.
  void AddArray(const char *name, const char * id);
  
  ///
  /// Remove a double array from the chart
  void RemoveArray(const char *name);

  ///
  /// Remove all the arrays
  void ClearArrays();

  ///
  /// Get the array id referenced by a particular name
  const char *GetArray(const char *name);

  ///
  /// Get the list of array names
  vtkStringArray* GetArrayNames();

  ///
  /// Get the list of array ids
  vtkStringArray* GetArrays();

  ///
  /// Set/Get a property for a specific array to control how it will
  /// appear in the chart. If the array name is "default", then the property
  /// is either a property of the entire chart or a default property
  /// for the arrays (which can be overridden by properties assigned
  /// to specific arrays).  Available properties are:
  /// 
  /// Chart level properties
  ///
  ///   "type" - Line, Bar, Scatter
  ///   "title" - title displayed on the chart
  ///   "showTitle" - show title "on" or "off"
  ///   "xAxisLabel" - label displayed on the x-axis
  ///   "showXAxisLabel" - show x-axis label "on" or "off"
  ///   "xAxisType" - type of data on x-axis, "quantitative", "categorical", "date"
  ///   "yAxisLabel" - label displayed on the y-axis
  ///   "showYAxisLabel" - show y-axis label "on" or "off"
  ///   "yAxisType" - type of data on y-axis, "quantitative", "categorical", "date"
  ///   "showGrid" - show grid "on" or "off"
  ///   "showLegend" - show legend "on" or "off"
  ///   "lookupTable" - MRMLID of a ColorNode to use to color series
  ///
  /// Array level properties (can be assigned to "default" to apply to
  ///   entire chart)
  ///
  ///   "showLines" - show lines "on" or "off"
  ///   "showMarkers" - show markers "on" or "off"
  ///   "color" - color to use for the array lines and points (#RRGGBB)
  ///   "lookupTable" - MRMLID of a ColorNode to use to color individual 
  ///         bars in bar chart (useful with categorical data)
  /// 
  void SetProperty(const char *arrname, const char *property, const char *value);
  const char* GetProperty(const char *arrname, const char *property);

  ///
  /// Remove a property for an array
  void ClearProperty(const char *arrname, const char *property);

  ///
  /// Remove all the properties for an array
  void ClearProperties(const char *arrname);

  ///
  /// Remove all the properties for all the arrays
  void ClearProperties();

  /// 
  /// Updates this node if it depends on other nodes 
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  /// 
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

 protected:
  //----------------------------------------------------------------
  /// Constructor and destroctor
  //----------------------------------------------------------------
  vtkMRMLChartNode();
  ~vtkMRMLChartNode();
  vtkMRMLChartNode(const vtkMRMLChartNode&);
  void operator=(const vtkMRMLChartNode&);


 protected:
  //----------------------------------------------------------------
  /// Data
  //----------------------------------------------------------------
  DoubleArrayIDMap *DoubleArrayIDs;
  vtkStringArray *Arrays;        // only valid when GetArrays() is called
  vtkStringArray *ArrayNames;    // only valid when GetArrayNames() is called
  ChartPropertyMap *Properties;
};



#endif

