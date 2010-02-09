/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCurveAnalysisNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLPlotNode_h
#define __vtkMRMLPlotNode_h

#include <string>
#include <vector>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkObject.h"

#include "vtkDataObject.h"

class vtkSlicerXYPlotWidget;
class vtkMRMLXYPlotManagerNode;


class VTK_MRML_EXPORT vtkMRMLPlotNode : public vtkMRMLNode
{

  //----------------------------------------------------------------
  /// Friend classes
  //----------------------------------------------------------------
  /// 
  /// These clases are defined as a friend calls of vtkMRMLPlotNode
  /// to call protected the function: GetDrawObject();
  //BTX
  friend class vtkSlicerXYPlotWidget;
  friend class vtkMRMLXYPlotManagerNode;
  //ETX

 public:
  //----------------------------------------------------------------
  /// Constants
  //----------------------------------------------------------------

  /// Interpolation method
  //BTX
  enum {
    INTERP_LINEAR = 0
  };
  //ETX

  //----------------------------------------------------------------
  /// Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLPlotNode *New();
  vtkTypeMacro(vtkMRMLPlotNode,vtkMRMLNode);
  
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
    {return "Plot";};

  /// 
  /// Method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //----------------------------------------------------------------
  /// Get and Set Macros
  //----------------------------------------------------------------
  vtkSetMacro ( Visible, int );
  vtkGetMacro ( Visible, int );

  /// 
  /// Set line color by SetColor(double, double, double) or SetColor(Double* c[3])
  vtkSetVector3Macro( Color, double );
  vtkGetVector3Macro( Color, double );
  
  //----------------------------------------------------------------
  /// Access methods
  //----------------------------------------------------------------

  /// 
  /// Get Y range for given X range.
  /// Returns 0 if it cannot determin the x range.
  virtual int GetYRange(const double* vtkNotUsed(xrange), double* vtkNotUsed(yrange)) { return 0; };

  /// 
  /// Set legend of the object for the graph.
  void SetLegend(const char* legend)
  {
    this->Legend = legend;
  };


  /// 
  /// Get legend of the object for the graph.
  const char* GetLegend()
  {
    return this->Legend.c_str();
  };


  //----------------------------------------------------------------
  /// Methods for Plotting (called from friend classes)
  //----------------------------------------------------------------
 protected:
  /// 
  /// Get minimum and muximum X values.
  /// Returns 0 if the Y range cannot be determined.
  virtual int GetXRange(double* vtkNotUsed(xrange)) {return 0; };

  /// 
  /// Get minimum and muximum Y values.
  /// Returns 0 if the Y range cannot be determined.
  virtual int GetYRange(double* vtkNotUsed(yrange)) {return 0;};

  /// 
  /// Get draw object (this funciton is called by vtkMRMLXYPlotManagerNode)
  virtual vtkDataObject* GetDrawObject(double* vtkNotUsed(xrange), double* vtkNotUsed(yrange))
  {
    return 0;
  };


  //----------------------------------------------------------------
  /// Constructor and destroctor
  //----------------------------------------------------------------
 protected:
  vtkMRMLPlotNode();
  ~vtkMRMLPlotNode();
  vtkMRMLPlotNode(const vtkMRMLPlotNode&);
  void operator=(const vtkMRMLPlotNode&);


 protected:
  //----------------------------------------------------------------
  /// Data
  //----------------------------------------------------------------
  int Visible;
  double Color[3];
  //BTX
  std::string Legend;
  //ETX

};



#endif

