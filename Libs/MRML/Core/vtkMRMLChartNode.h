/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLChartNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLChartNode_h
#define __vtkMRMLChartNode_h

#include "vtkMRMLNode.h"

class vtkDataObject;
class vtkStringArray;
class DoubleArrayIDMap;
class ChartPropertyMap;

#include <string>

/// \brief MRML node for referencing a collection of data to plot.
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

  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override
    {return "Chart";}

  ///
  /// Method to propagate events generated in mrml
  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData ) override;

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
  /// \li  "type" - Line, Bar, Scatter, Box
  /// \li  "title" - title displayed on the chart
  /// \li  "showTitle" - show title "on" or "off"
  /// \li  "xAxisLabel" - label displayed on the x-axis
  /// \li  "xAxisPad" - width of the x-axis padding, which is a factor multiplied by the
  ///                   data range on the axis to give the axis range so that data points
  ///                   don't fall on the edges of the axis.
  /// \li  "showXAxisLabel" - show x-axis label "on" or "off"
  /// \li  "xAxisType" - type of data on x-axis, "quantitative", "categorical", "date"
  /// \li  "yAxisLabel" - label displayed on the y-axis
  /// \li  "yAxisPad" - height of the y-axis padding, which is a factor multiplied by the
  ///                   data range on the axis to give the axis range so that data points
  ///                   don't fall on the edges of the axis.
  /// \li  "showYAxisLabel" - show y-axis label "on" or "off"
  /// \li  "yAxisType" - type of data on y-axis, "quantitative", "categorical"
  /// \li  "showGrid" - show grid "on" or "off"
  /// \li  "showLegend" - show legend "on" or "off"
  /// \li  "lookupTable" - MRMLID of a ColorNode to use to color series
  ///
  /// Array level properties (can be assigned to "default" to apply to
  ///   entire chart)
  ///
  /// \li  "showLines" - show lines "on" or "off"
  /// \li  "showMarkers" - show markers "on" or "off"
  /// \li  "size" - marker size is an integer larger than 0 and smaller than 2^32 - 1
  /// \li  "linePattern" - line pattern can be "solid", "dashed", "dotted",
  ///                      "dashed-dotted"
  /// \li  "lineWidth" - line width is an integer larger than 0 and smaller than 2^32 - 1
  /// \li  "color" - color to use for the array lines and points (<code>\#RRGGBB</code>)
  /// \li  "lookupTable" - MRMLID of a ColorNode to use to color individual
  ///         bars in bar chart (useful with categorical data)
  ///
  /// When using DoubleArrayNodes, dates are specified as floats
  /// representing the number of seconds since January 1, 1970 UTC.
  /// Fractional seconds are permitted.
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

  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void SetSceneReferences() override;

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  ///
  /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override;

 protected:
  //----------------------------------------------------------------
  /// Constructor and destroctor
  //----------------------------------------------------------------
  vtkMRMLChartNode();
  ~vtkMRMLChartNode() override;
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

