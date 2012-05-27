/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLFiberBundleStorageNode - MRML node for fiberBundle storage on disk.
///
/// The storage node has methods to read/write vtkPolyData to/from disk.

#ifndef __vtkMRMLFiberBundleStorageNode_h
#define __vtkMRMLFiberBundleStorageNode_h

// MRML includes
#include "vtkMRMLModelStorageNode.h"

// Tractography includes
#include "vtkSlicerTractographyDisplayModuleMRMLExport.h"

class VTK_SLICER_TRACTOGRAPHYDISPLAY_MODULE_MRML_EXPORT vtkMRMLFiberBundleStorageNode : public vtkMRMLModelStorageNode
{
  public:
  static vtkMRMLFiberBundleStorageNode *New();
  vtkTypeMacro(vtkMRMLFiberBundleStorageNode,vtkMRMLModelStorageNode);
  //void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "FiberBundleStorage";};

  ///
  /// Check to see if this storage node can handle the file type in the input
  /// string. If input string is null, check URI, then check FileName.
  /// Subclasses should implement this method.
  virtual int SupportedFileType(const char *fileName);

  ///
  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  ///
  /// Return a default file extension for writting
  virtual const char* GetDefaultWriteFileExtension()
    {
    return "vtk";
    };

protected:
  vtkMRMLFiberBundleStorageNode(){};
  ~vtkMRMLFiberBundleStorageNode(){};
  vtkMRMLFiberBundleStorageNode(const vtkMRMLFiberBundleStorageNode&);
  void operator=(const vtkMRMLFiberBundleStorageNode&);

};

#endif

