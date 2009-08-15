/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCurveAnalysisNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLXYPlotNode_h
#define __vtkMRMLXYPlotNode_h

#include <string>
#include <vector>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"


#include "vtkObject.h"
#include "vtkFourDAnalysisWin32Header.h"

#include "vtkMRMLDoubleArrayNode.h"


class VTK_FourDAnalysis_EXPORT vtkMRMLXYPlotNode : public vtkMRMLNode
{

 public:

  //----------------------------------------------------------------
  // Constants
  //----------------------------------------------------------------

  // Interpolation method
  //BTX
  enum {
    INTERP_LINEAR = 0,
  };
  //ETX

  //----------------------------------------------------------------
  // Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLXYPlotNode *New();
  vtkTypeMacro(vtkMRMLXYPlotNode,vtkMRMLNode);
  
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes.
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object.
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "XYPlot";};

  // Description:
  // Method to propagate events generated in mrml.
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //----------------------------------------------------------------
  // Get and Set Macros
  //----------------------------------------------------------------
  //vtkSetObjectMacro ( Array, vtkDoubleArray );
  //vtkGetObjectMacro ( Array, vtkDoubleArray );

  //----------------------------------------------------------------
  // Curve Data / properties
  //----------------------------------------------------------------
  
  // Description:
  // Add DoiubleArrayNode to the plotting list.
  void AddArrayNode(vtkMRMLDoubleArrayNode* node);

  // Description:
  // Remove DoubleArrayNode from the plotting list.
  void RemoveArrayNode(vtkMRMLDoubleArrayNode* node);

  // Description:
  // Get number of arrays on the list
  unsigned int GetNumberOfArrays();

  // Description:
  // Get n-th vtkMRMLDoubleArrayNode on the list
  vtkMRMLDoubleArrayNode* GetArrayNode(unsigned int n);

  // Description:
  // Set line color
  void SetColor(unsigned int n, double r, double g, double b);
  
  // Description:
  // Get line color
  void GetColor(unsigned int n, double* r, double* g, double* b);
  
  // Description:
  // Set line name (this method updates the array node's name as well)
  void SetLineName(unsigned int n, const char* str);

  // Description:
  // Get line name (same as the array node's name)
  const char* GetLineName(unsigned int n);
  
  // Description:
  // Set visibility of the specified curve (i == 0 : off; i == 1 : on)
  void SetVisibility(unsigned int n, int i);

  // Description:
  // Set visibility of the specified curve (0 : off; 1 : on)
  int GetVisibility(unsigned int n);

  // Description:
  // Set visibility of the all curves (i == 0 : off; i == 1 : on)
  void SetVisibilityAll(int i);

  // Description:
  // Set error bar plot for the specified curve(i == 0 : off; i == 1 : on)
  void SetErrorBarPlot(unsigned int n, int i);

  // Description:
  // Get error bar plot for the specivied curve(0 : off; 1 : on)
  int GetErrorBarPlot(unsigned int n);

  // Description:
  // Set visibility of the all curves (i == 0 : off; i == 1 : on)
  void SetErrorBarPlotAll(int i);

  //----------------------------------------------------------------
  // Vertical and horizontal lines
  //----------------------------------------------------------------
  
  //// Description:
  //// Add DoiubleArrayNode to the plotting list.
  //void AddVerticalLine(double position);
  //
  //// Description:
  //// Remove DoubleArrayNode from the plotting list.
  //void ClearVerticalLines(int i);
  //
  //// Description:
  //// Get number of arrays on the list
  //unsigned int GetNumberOfHorizontalLines();
  //
  //// Description:
  //// Add DoiubleArrayNode to the plotting list.
  //void AddHorizontalLine(double position);
  //
  //// Description:
  //// Remove DoubleArrayNode from the plotting list.
  //void ClearHorizontalLines(int i);
  //
  //// Description:
  //// Get number of arrays on the list
  //unsigned int GetNumberOfHorizontalLines();

  

  //----------------------------------------------------------------
  // Graph properties
  //----------------------------------------------------------------

  // Description:
  // Set title of graph.
  void SetTitle(const char* str);

  // Description:
  // Get title of graph.
  const char* GetTitle();

  // Description:
  // Set label for X-axis
  void SetXLabel(const char* str);

  // Description:
  // Get label for X-axis
  const char* GetXLabel();

  // Description:
  // Set label for Y-axis
  void SetYLabel(const char* str);
  
  // Description:
  // Get label for Y-axis
  const char* GetYLabel();

  // Description:
  // Set/Get automatic range setting flag
  vtkSetMacro ( AutoXRange, int );
  vtkGetMacro ( AutoXRange, int );
  vtkSetMacro ( AutoYRange, int );
  vtkGetMacro ( AutoYRange, int );

  // Description:
  // Set X range
  void SetXRange(double* range);

  // Description:
  // Set Y range
  void SetYRange(double* range);

  // Description:
  // Get X range
  void GetXRange(double* range);

  // Description:
  // Get Y range
  void GetYRange(double* range);



 protected:
  //----------------------------------------------------------------
  // Constructor and destroctor
  //----------------------------------------------------------------
  
  vtkMRMLXYPlotNode();
  ~vtkMRMLXYPlotNode();
  vtkMRMLXYPlotNode(const vtkMRMLXYPlotNode&);
  void operator=(const vtkMRMLXYPlotNode&);


 protected:
  //----------------------------------------------------------------
  // Data
  //----------------------------------------------------------------

  //BTX
  typedef struct {
    double                  lineColor[3];
    int                     visibility;
    int                     errorBar;
    vtkMRMLDoubleArrayNode* arrayNode;
  } PlotDataType;

  typedef struct {
    double                  lineColor[3];
    int                     visibility;
    int                     errorBar;
    double                  position;
  } LineDataType;

  std::string Title;             // Plotting graph title
  std::string XLabel;            // Label for x-axis
  std::string YLabel;            // Label for y-axis

  std::vector< PlotDataType > Data;  // list for plotting
  //std::vector< LineDataType > VerticalLines;
  //std::vector< LineDataType > HorizontalLines;
  //ETX

  int    AutoXRange;  // Flag for automatic range adjust (default: 0 = off)
  int    AutoYRange;  // Flag for automatic range adjust (default: 0 = off)
  double XRange[2];
  double YRange[2];
  
};

#endif

