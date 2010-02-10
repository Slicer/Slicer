/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCurveAnalysisNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLArrayPlotNode_h
#define __vtkMRMLArrayPlotNode_h

#include <string>
#include <vector>

#include "vtkMRML.h"
#include "vtkMRMLPlotNode.h"

#include "vtkObject.h"

#include "vtkDataObject.h"
#include "vtkMRMLDoubleArrayNode.h"

class VTK_MRML_EXPORT vtkMRMLArrayPlotNode : public vtkMRMLPlotNode
{

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

  static vtkMRMLArrayPlotNode *New();
  vtkTypeMacro(vtkMRMLArrayPlotNode,vtkMRMLPlotNode);
  
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
    {return "ArrayPlot";};

  /// 
  /// Method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //----------------------------------------------------------------
  /// Get and Set Methods
  //----------------------------------------------------------------

  //vtkSetObjectMacro ( Array, vtkMRMLDoubleArrayNode );
  void SetAndObserveArray( vtkMRMLDoubleArrayNode* node );
  vtkGetObjectMacro ( Array, vtkMRMLDoubleArrayNode );
  
  vtkSetMacro ( ErrorBar, int );
  vtkGetMacro ( ErrorBar, int );

  //----------------------------------------------------------------
  /// Method for Plotting (called from friend classes)
  //----------------------------------------------------------------
 protected:
  /// 
  /// Get minimum and muximum X values.
  /// Returns 0 if the Y range cannot be determined.
  virtual int GetXRange(double* xrange);

  /// 
  /// Get minimum and muximum Y values.
  /// Returns 0 if the Y range cannot be determined.
  virtual int GetYRange(double* yrange);
  virtual int GetYRange(const double* xrange, double* yrange)
    { return Superclass::GetYRange(xrange, yrange); }

  /// 
  /// Get draw object (this funciton is called by vtkMRMLXYPlotManagerNode)
  virtual vtkDataObject* GetDrawObject(double* xrange, double* yrange);

  //----------------------------------------------------------------
  /// Subroutines for drawing
  //----------------------------------------------------------------
  vtkDoubleArray* CreatePlotDataWithErrorBar(vtkDoubleArray* srcData, double* xrange, double* yrange);

 protected:

  /// 
  /// Get draw object (this funciton is called by vtkMRMLXYPlotManagerNode)
  virtual vtkDataObject* GetDrawObject() {return NULL; };

  //----------------------------------------------------------------
  /// Constructor and destroctor
  //----------------------------------------------------------------
 protected:
  vtkMRMLArrayPlotNode();
  ~vtkMRMLArrayPlotNode();
  vtkMRMLArrayPlotNode(const vtkMRMLArrayPlotNode&);
  void operator=(const vtkMRMLArrayPlotNode&);


 protected:
  //----------------------------------------------------------------
  /// Data
  //----------------------------------------------------------------
  
  vtkMRMLDoubleArrayNode* Array;
  int ErrorBar;

};



#endif

