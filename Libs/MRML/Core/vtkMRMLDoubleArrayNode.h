/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
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

#include "vtkMRMLStorableNode.h"
class vtkDoubleArray;
class vtkMRMLStorageNode;

class VTK_MRML_EXPORT vtkMRMLDoubleArrayNode : public vtkMRMLStorableNode
{
public:
  //----------------------------------------------------------------
  /// Constants
  //----------------------------------------------------------------

  /// Interpolation method
  enum {
    INTERP_LINEAR = 0
  };

  //----------------------------------------------------------------
  /// Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLDoubleArrayNode *New();
  vtkTypeMacro(vtkMRMLDoubleArrayNode,vtkMRMLStorableNode);

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
    {return "DoubleArray";};

  ///
  /// Method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //----------------------------------------------------------------
  /// Get and Set Macros
  //----------------------------------------------------------------
  virtual void SetArray(vtkDoubleArray*);
  vtkGetObjectMacro ( Array, vtkDoubleArray );

  //----------------------------------------------------------------
  /// Access methods
  //----------------------------------------------------------------

  ///
  /// Set / change the size of value data (number of points in time dimension)
  void SetSize(unsigned int n);

  ///
  /// Get the size of value data (number of points in time dimension)
  unsigned int GetSize();

  ///
  /// Get Y value by X. If X is between two data points, it interpolates the value
  /// by using the method specified by 'interp'.
  /// NOTE: TO BE IMPLEMENTED.
  double GetYAxisValue(double x, int interp=INTERP_LINEAR);

  /// \bug Fix function GetXYAxisValue(int index, double* x, double* y);
  /// Get X and Y values at the data point specified by 'index'
  /// int GetXYAxisValue(int index, double* x, double* y);

  /// \bug Fix function GetXYAxisValue(int index, double* x, double* y, double*  yerr);
  /// Get X and Y values with error (or standard deviation) of Y,
  /// at the data point specified by 'index'
  /// int GetXYAxisValue(int index, double* x, double* y, double*  yerr);

  ///
  /// Set X and Y values at he data point specified by 'index'
  int SetXYValue(int index, double x, double y);

  ///
  /// Set X and Y values with error (or standard deviation) of Y,
  /// at the data point specified by 'index'
  int SetXYValue(int index, double x, double y, double yerr);

  ///
  /// Get X and Y values at he data point specified by 'index'
  int GetXYValue(int index, double* x, double* y);

  ///
  /// Get X and Y values with error (or standard deviation) of Y,
  /// at the data point specified by 'index'
  int GetXYValue(int index, double* x, double* y, double* yerr);


  ///
  /// Add a set of X and Y values at the end of the array
  int AddXYValue(double x, double y);

  ///
  /// Add a set of X and Y values with error (or standard deviation) of Y,
  /// at the end of the array
  int AddXYValue(double x, double y, double yerr);

  ///
  /// Search min and maximum value of X and Y in the array. The result is stored in 'range'.
  /// (range[0]: minimum value, range[1]: maximum value)
  /// if fIncludeError=1 is specified, the range takes account of errors.
  void GetRange(double* rangeX, double* rangeY, int fIncludeError=1);

  ///
  /// Search min and maximum value of X in the array. The result is stored in 'range'.
  /// (range[0]: minimum value, range[1]: maximum value)
  void GetXRange(double* range);

  ///
  /// Search min and maximum value of Y in the array. The result is stored in 'range'.
  /// (range[0]: minimum value, range[1]: maximum value)
  /// if fIncludeError=1 is specified, the range takes account of errors.
  void GetYRange(double* range, int fIncludeError=1);

  // Description:
  //Set labels
  //void SetLabel(std::vector< std::string > labels);
  typedef std::vector< std::string > LabelsVectorType;
  void SetLabels(const LabelsVectorType &labels);

  // Description:
  //Get labels
  const LabelsVectorType & GetLabels() const;

  ///
  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  //----------------------------------------------------------------
  /// Constructor and destroctor
  //----------------------------------------------------------------
 protected:
  vtkMRMLDoubleArrayNode();
  ~vtkMRMLDoubleArrayNode();
  vtkMRMLDoubleArrayNode(const vtkMRMLDoubleArrayNode&);
  void operator=(const vtkMRMLDoubleArrayNode&);


 protected:
  //----------------------------------------------------------------
  /// Data
  //----------------------------------------------------------------

  vtkDoubleArray* Array;

  std::vector< std::string > Unit;
  std::vector< std::string > Labels;


};

#endif

