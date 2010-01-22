/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCurveAnalysisNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLOrthogonalLinePlotNode_h
#define __vtkMRMLOrthogonalLinePlotNode_h

#include <string>
#include <vector>

#include "vtkMRML.h"
#include "vtkMRMLPlotNode.h"

#include "vtkObject.h"

#include "vtkDataObject.h"

class VTK_MRML_EXPORT vtkMRMLOrthogonalLinePlotNode : public vtkMRMLPlotNode
{

 public:

  //----------------------------------------------------------------
  /// Constants
  //----------------------------------------------------------------

  /// Line direction
  //BTX
  enum {
    VERTICAL = 0,
    HORIZONTAL
  };
  //ETX


  //----------------------------------------------------------------
  /// Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLOrthogonalLinePlotNode *New();
  vtkTypeMacro(vtkMRMLOrthogonalLinePlotNode, vtkMRMLPlotNode);
  
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
    {return "OrthogonalLinePlot";};

  /// 
  /// Method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );


  //----------------------------------------------------------------
  /// Get and Set Macros
  //----------------------------------------------------------------
 public:
  vtkSetVector2Macro ( Point, double );
  vtkGetVector2Macro ( Point, double );

  vtkSetMacro ( Direction, int );
  vtkGetMacro ( Direction, int );
  

  //----------------------------------------------------------------
  /// Access methods
  //----------------------------------------------------------------
 public:

  //----------------------------------------------------------------
  /// Methods for plotting (called from friend classes)
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
  /// Constructor and destroctor
  //----------------------------------------------------------------
 protected:
  vtkMRMLOrthogonalLinePlotNode();
  ~vtkMRMLOrthogonalLinePlotNode();
  vtkMRMLOrthogonalLinePlotNode(const vtkMRMLOrthogonalLinePlotNode&);
  void operator=(const vtkMRMLOrthogonalLinePlotNode&);


 protected:
  //----------------------------------------------------------------
  /// Data
  //----------------------------------------------------------------
  
  double Point[2];   /// A point on the line
  int Direction;

};



#endif

