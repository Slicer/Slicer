/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCurveAnalysisNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLDoubleArrayNode_h
#define __vtkMRMLDoubleArrayNode_h

#include <string>
#include <vector>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"


#include "vtkObject.h"
#include "vtkFourDAnalysisWin32Header.h"

#include "vtkDoubleArray.h"

class VTK_FourDAnalysis_EXPORT vtkMRMLDoubleArrayNode : public vtkMRMLNode
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

  static vtkMRMLDoubleArrayNode *New();
  vtkTypeMacro(vtkMRMLDoubleArrayNode,vtkMRMLNode);
  
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "DoubleArray";};

  // Description:
  // Method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //----------------------------------------------------------------
  // Get and Set Macros
  //----------------------------------------------------------------
  vtkSetObjectMacro ( Array, vtkDoubleArray );
  vtkGetObjectMacro ( Array, vtkDoubleArray );

  //----------------------------------------------------------------
  // Access methods
  //----------------------------------------------------------------

  // Description:
  // Set / change the size of value data (number of points in time dimension)
  void SetSize(unsigned int n);

  // Description:
  // Get the size of value data (number of points in time dimension)
  unsigned int GetSize();

  // Description:
  // Get Y value by X. If X is between two data points, it interpolates the value
  // by using the method specified by 'interp'.
  // NOTE: TO BE IMPLEMENTED. 
  double GetYValue(double x, int interp=INTERP_LINEAR);

  // Description:
  // Get X and Y values at the data point specified by 'index'
  int GetXYValue(int index, double* x, double* y);

  // Description:
  // Get X and Y values with error (or standard deviation) of Y,
  // at the data point specified by 'index'
  int GetXYValue(int index, double* x, double* y, double*  yerr);

  // Description:
  // Set X and Y values at he data point specified by 'index'
  int SetXYValue(int index, double x, double y);

  // Description:
  // Set X and Y values with error (or standard deviation) of Y,
  // at the data point specified by 'index'
  int SetXYValue(int index, double x, double y, double yerr);

  // Description:
  // Add a set of X and Y values at the end of the array
  int AddXYValue(double x, double y);

  // Description:
  // Add a set of X and Y values with error (or standard deviation) of Y,
  // at the end of the array
  int AddXYValue(double x, double y, double yerr);

  // Description:
  // Search min and maximum value of X and Y in the array. The result is stored in 'range'.
  // (range[0]: minimum value, range[1]: maximum value)
  // if fIncludeError=1 is specified, the range takes account of errors.
  void GetRange(double* rangeX, double* rangeY, int fIncludeError=1);

  // Description:
  // Search min and maximum value of X in the array. The result is stored in 'range'.
  // (range[0]: minimum value, range[1]: maximum value)
  void GetXRange(double* range);

  // Description:
  // Search min and maximum value of Y in the array. The result is stored in 'range'.
  // (range[0]: minimum value, range[1]: maximum value)
  // if fIncludeError=1 is specified, the range takes account of errors.
  void GetYRange(double* range, int fIncludeError=1);


  //----------------------------------------------------------------
  // Constructor and destroctor
  //----------------------------------------------------------------
 protected:
  vtkMRMLDoubleArrayNode();
  ~vtkMRMLDoubleArrayNode();
  vtkMRMLDoubleArrayNode(const vtkMRMLDoubleArrayNode&);
  void operator=(const vtkMRMLDoubleArrayNode&);


 protected:
  //----------------------------------------------------------------
  // Data
  //----------------------------------------------------------------

  vtkDoubleArray* Array;

  //BTX
  std::vector< std::string > Unit;
  //ETX
  

};

#endif

