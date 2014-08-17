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

class vtkAbstractTransform;
class vtkMRMLTransformNode;

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

  /// Returns true if the filename indicates that it is an image file
  /// (then it is assumed to be a grid transform and the displacement
  /// field is read/written with an image reader/writer class)
  virtual bool IsImageFile(const std::string &filename);

  /// The method calls SetAndObserveTransformFromParent or SetAndObserveTransformToParent, depending on
  /// which one will result in storing a transform that is not inverted (Inverse flag is false).
  /// For example, if it the transform's Inverse flag is false then it is saved by using SetAndObserveTransformFromParent as is;
  /// if transform's Inverse flag is true, then the transform is inverted (so that its Inverse flag becomes false)
  /// and the resulting transform is saved in SetAndObserveTransformToParent.
  /// It makes the displayed transform information more intuitive.
  virtual void SetAndObserveTransformFromParentAutoInvert(vtkMRMLTransformNode* transformNode, vtkAbstractTransform *transform);

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// This method is specialized for ITKv3 tfm files as generated
  /// by legacy versions of BRAINSFit (see slicer issue #3788).
  /// This generates the vtkOrientedBSplineTransform with an
  /// additive bulk transform (as in ITKv3 bspline transform).
  virtual int ReadFromITKv3BSplineTransformFile(vtkMRMLNode *refNode);

  /// This method uses ITK's transform reading infrastucture to
  /// read simple or composite transforms.  The composite transform can
  /// contain nested transforms in any order coming in theory
  /// from any file type and it is read into a vtkGeneralTransform.
  /// In practice, as of Slicer 4.4, only HDF5 (.h5)
  /// files can support composite transforms and the only composite
  /// transform in general use is one which includes a linear component
  /// and a bspline component. (see slicer issue #3788).
  virtual int ReadFromTransformFile(vtkMRMLNode *refNode);

  /// Read displacement field transform from a 3-component scalar image.
  /// The 3 scalar components in the file specify the displacement
  /// in LPS coordinate system. When the transform is read into Slicer, the
  /// displacement vectors are converted to RAS coordinate system.
  virtual int ReadFromImageFile(vtkMRMLNode *refNode);

  /// Write data from a referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

  /// Writes simple or composite transform to an ITK transform file.
  /// Supports writing of legacy ITKv3 BSpline transform with additive bulk component.
  virtual int WriteToTransformFile(vtkMRMLNode* refNode);

  /// Write displacement field transform from a 3-component scalar image.
  /// The 3 scalar components in Slicer specify the displacement
  /// in RAS coordinate system. When the transform is written to file then the
  /// displacement vectors are converted to LPS coordinate system.
  virtual int WriteToImageFile(vtkMRMLNode* refNode);

};

#endif
