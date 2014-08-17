/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLTransformStorageNode_h
#define __vtkMRMLTransformStorageNode_h

#include "vtkMRMLStorageNode.h"

class vtkImageData;

class vtkMRMLLinearTransformNode;
class vtkMRMLBSplineTransformNode;
class vtkMRMLGridTransformNode;

/// \brief MRML node for transform storage on disk.
///
/// Storage nodes has methods to read/write transforms to/from disk.
class VTK_MRML_EXPORT vtkMRMLTransformStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLTransformStorageNode *New();
  vtkTypeMacro(vtkMRMLTransformStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Get node XML tag name (like Storage, Transform)
  virtual const char* GetNodeTagName()  {return "TransformStorage";};

  ///
  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  ///
  /// Return a default file extension for writting
  virtual const char* GetDefaultWriteFileExtension();

  /// Support only transform nodes
  virtual bool CanReadInReferenceNode(vtkMRMLNode* refNode);

protected:
  vtkMRMLTransformStorageNode();
  ~vtkMRMLTransformStorageNode();
  vtkMRMLTransformStorageNode(const vtkMRMLTransformStorageNode&);
  void operator=(const vtkMRMLTransformStorageNode&);

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  virtual int ReadLinearTransform(vtkMRMLNode *refNode);

  /// This method is specialized for ITKv3 tfm files as generated
  /// by legacy versions of BRAINSFit (see slicer issue #3788).
  /// This generates the vtkOrientedBSplineTransform to match
  /// the unusual mathematics of the ITKv3 bspline transform.
  virtual int ReadBSplineTransform(vtkMRMLNode *refNode);

  /// This method uses ITK's transform reading infrastucture to
  /// read composite transforms.  The composite transform can
  /// contain nested transforms in any order coming in theory
  /// from any file type.  In practice, as of Slicer 4.4, only HDF5 (.h5)
  /// files can support composite transforms and the only composite
  /// transform in general use is one which includes a linear component
  /// and a bspline component.  Because this method is used to fill a
  /// single MRML transform node, it maps the linear+bpline itk composite
  /// transform into a single vtkGeneralTransform that can include
  /// the linear and nonlinear components.
  /// (see slicer issue #3788).
  virtual int ReadCompositeTransform(vtkMRMLNode *refNode);

  /// Read displacement field transform from a 3-component scalar image.
  /// The 3 scalar components in the file specify the displacement
  /// in LPS coordinate system. When the transform is read into Slicer, the
  /// displacement vectors are converted to RAS coordinate system.
  virtual int ReadGridTransform(vtkMRMLNode *refNode);

  /// Write data from a referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

  virtual int WriteLinearTransform(vtkMRMLLinearTransformNode *ln);
  virtual int WriteBSplineTransform(vtkMRMLBSplineTransformNode *bs);

  /// Write displacement field transform from a 3-component scalar image.
  /// The 3 scalar components in Slicer specify the displacement
  /// in RAS coordinate system. When the transform is written to file then the
  /// displacement vectors are converted to LPS coordinate system.
  virtual int WriteGridTransform(vtkMRMLGridTransformNode *gd);

};

#endif
