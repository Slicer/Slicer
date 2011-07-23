/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCurveAnalysisNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLXYPlotManagerNode_h
#define __vtkMRMLXYPlotManagerNode_h

// MRML includes
#include "vtkMRMLNode.h"
class vtkMRMLPlotNode;

// VTK includes
class vtkCollection;
class vtkIntArray;

// STD includes
#include <string>

class VTK_MRML_EXPORT vtkMRMLXYPlotManagerNode : public vtkMRMLNode
{

 public:

  //----------------------------------------------------------------
  /// Constants
  //----------------------------------------------------------------

  /// Events
  enum {
    UpdateGraphEvent = 11900
  };

  /// Interpolation method
  enum {
    INTERP_LINEAR = 0
  };
  enum {
    TYPE_CURVE = 0,
    TYPE_VERTICAL_LINE,
    TYPE_HORIZONTAL_LINE
  };

  //----------------------------------------------------------------
  /// Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLXYPlotManagerNode *New();
  vtkTypeMacro(vtkMRMLXYPlotManagerNode,vtkMRMLNode);
  
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Set node attributes.
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object.
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() { return "XYPlot"; };

  /// 
  /// Method to propagate events generated in mrml.
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //----------------------------------------------------------------
  /// PlotNode management
  //----------------------------------------------------------------
  
  /// 
  /// Add DoiubleArrayNode to the plotting list. Returns array ID.
  int AddPlotNode(vtkMRMLPlotNode* node);

  /// 
  /// Remove DoubleArrayNode from the plotting list.
  void RemovePlotNode(int id);

  /// 
  /// Remove DoubleArrayNode from the plotting list.
  void RemovePlotNodeByNodeID(const char* nodeID);

  /// 
  /// Remove all DoubleArrayNode from the plotting list.
  void ClearPlotNodes();

  /// 
  /// Get number of arrays on the list
  unsigned int GetNumberOfPlotNodes();

  /// 
  /// Get list of IDs
  vtkIntArray* GetPlotNodeIDList();

  /// 
  /// Get n-th vtkMRMLDoubleArrayNode on the list
  vtkMRMLPlotNode* GetPlotNode(int id);

  /// 
  /// Get list of plot node objects.
  /// If 'tag' is specified, GetPlotNode() returns a list of nodes, which have the specified tag name.
  vtkCollection* GetPlotNodes(const char* tag=NULL);


  //----------------------------------------------------------------
  /// Methods to change property of plot objects
  //----------------------------------------------------------------

  /// 
  /// Set visibility of the all curves (i == 0 : off; i == 1 : on)
  void SetVisibilityAll(int i);

  /// 
  /// Set visibility of the all curves (i == 0 : off; i == 1 : on)
  void SetErrorBarAll(int i);


  //----------------------------------------------------------------
  /// Plot graph
  //----------------------------------------------------------------

  ///   
  /// Invoke UpdateGraphEvent to force refreshing the graph
  void Refresh();

  //----------------------------------------------------------------
  /// Graph properties
  //----------------------------------------------------------------

  /// 
  /// Set title of graph.
  void SetTitle(const char* str)
  {
    this->Title = str;
    this->Modified();
  };

  /// 
  /// Get title of graph.
  const char* GetTitle()
  {
    return this->Title.c_str();
  };

  /// 
  /// Set label for X-axis
  void SetXLabel(const char* str)
  {
    this->XLabel = str;
    this->Modified();
  };

  /// 
  /// Get label for X-axis
  const char* GetXLabel()
  {
    return this->XLabel.c_str();
  };

  /// 
  /// Set label for Y-axis
  void SetYLabel(const char* str)
  {
    this->YLabel = str;
    this->Modified();
  };
  
  /// 
  /// Get label for Y-axis
  const char* GetYLabel()
  {
    return this->YLabel.c_str();
  };

  /// 
  /// Set/Get automatic range setting flag
  vtkSetMacro ( AutoXRange, int );
  vtkGetMacro ( AutoXRange, int );
  vtkSetMacro ( AutoYRange, int );
  vtkGetMacro ( AutoYRange, int );

  /// 
  /// Set X range
  void SetXRange(double* range)
  {
    if (range[0] <= range[1])
      {
        this->XRange[0] = range[0];
        this->XRange[1] = range[1];
        this->Modified();
      }
  };

  /// 
  /// Set Y range
  void SetYRange(double* range)
  {
    if (range[0] <= range[1])
      {
        this->YRange[0] = range[0];
        this->YRange[1] = range[1];
        this->Modified();
      }
  };

  /// 
  /// Get X range
  void GetXRange(double* range)
  {
    range[0] = this->YRange[0];
    range[1] = this->YRange[1];
  };
 
  /// 
  /// Get Y range
  void GetYRange(double* range)
  {
    range[0] = this->YRange[0];
    range[1] = this->YRange[1];
  };

  /// 
  /// Set / get color of axes
  vtkSetVector3Macro( AxesColor, double );
  vtkGetVector3Macro( AxesColor, double );

  /// 
  /// Set / get color of background
  void SetBackgroundColor(double r, double g, double b)
  {
    this->BackgroundColor[0] = r;
    this->BackgroundColor[1] = g;
    this->BackgroundColor[2] = b;
    this->Modified();
  };

  /// 
  /// Get color of background
  void GetBackgroundColor(double* r, double* g, double* b)
  {
    *r = this->BackgroundColor[0];
    *g = this->BackgroundColor[1];
    *b = this->BackgroundColor[2];
  };

  
 protected:
  //----------------------------------------------------------------
  /// Constructor and destroctor
  //----------------------------------------------------------------
  
  vtkMRMLXYPlotManagerNode();
  ~vtkMRMLXYPlotManagerNode();
  vtkMRMLXYPlotManagerNode(const vtkMRMLXYPlotManagerNode&);
  void operator=(const vtkMRMLXYPlotManagerNode&);


 protected:
  //----------------------------------------------------------------
  /// Data
  //----------------------------------------------------------------

  //BTX
  std::string Title;             /// Plotting graph title
  std::string XLabel;            /// Label for x-axis
  std::string YLabel;            /// Label for y-axis
  std::map< int, vtkMRMLPlotNode* > Data;  /// map for plotting data
  //ETX

  int LastArrayID;

  int    AutoXRange;  /// Flag for automatic range adjust (default: 0 = off)
  int    AutoYRange;  /// Flag for automatic range adjust (default: 0 = off)
  double XRange[2];
  double YRange[2];

  double AxesColor[3];
  double BackgroundColor[3];
  
};

#endif

