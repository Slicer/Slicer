/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLMultiVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLMultiVolumeDisplayNode - MRML node for representing a volume (image stack).
/// 
/// Volume nodes describe data sets that can be thought of as stacks of 2D 
/// images that form a 3D volume.  Volume nodes describe where the images 
/// are stored on disk, how to render the data (window and level), and how 
/// to read the files.  This information is extracted from the image 
/// headers (if they exist) at the time the MRML file is generated.  
/// Consequently, MRML files isolate MRML browsers from understanding how 
/// to read the myriad of file formats for medical data. 

#ifndef __vtkMRMLMultiVolumeDisplayNode_h
#define __vtkMRMLMultiVolumeDisplayNode_h

// MultiVolumeExplorer includes
#include <vtkSlicerMultiVolumeExplorerModuleMRMLExport.h>

// MRML includes
#include "vtkMRMLScalarVolumeDisplayNode.h"

// VTK includes
class vtkAlgorithmOutput;
class vtkImageData;
class vtkImageExtractComponents;

class VTK_SLICER_MULTIVOLUMEEXPLORER_MODULE_MRML_EXPORT vtkMRMLMultiVolumeDisplayNode : public vtkMRMLScalarVolumeDisplayNode
{
  public:
  static vtkMRMLMultiVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLMultiVolumeDisplayNode,vtkMRMLScalarVolumeDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  virtual vtkMRMLNode* CreateNodeInstance() override;

  /// 
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts) override;

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent) override;

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node) override;

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() override {return "MultiVolumeDisplay";};

  /// 
  /// Get the pipeline input
#if (VTK_MAJOR_VERSION <= 5)
  virtual vtkImageData* GetInputImageData();
#else
  virtual vtkAlgorithmOutput* GetInputImageDataConnection() override;
#endif
  /// 
  /// Get the pipeline output
#if (VTK_MAJOR_VERSION <= 5)
  //BTX
  virtual vtkImageData* GetOutputImageData();
  //ETX
#else
  virtual vtkAlgorithmOutput* GetOutputImageDataConnection() override;
#endif
  virtual void UpdateImageDataPipeline() override;

  //--------------------------------------------------------------------------
  /// Display Information
  //--------------------------------------------------------------------------

  /// 
  /// Set/Get interpolate reformated slices
  vtkGetMacro(FrameComponent, int);
  vtkSetMacro(FrameComponent, int);
 
protected:
  vtkMRMLMultiVolumeDisplayNode();
  ~vtkMRMLMultiVolumeDisplayNode();
  vtkMRMLMultiVolumeDisplayNode(const vtkMRMLMultiVolumeDisplayNode&);
  void operator=(const vtkMRMLMultiVolumeDisplayNode&);

  /// 
  /// Set the input of the pipeline
#if (VTK_MAJOR_VERSION <= 5)
  virtual void SetInputToImageDataPipeline(vtkImageData *imageData);
#else
  virtual void SetInputToImageDataPipeline(vtkAlgorithmOutput *imageDataConnection) override;
#endif

  virtual vtkImageData* GetScalarImageData();

  int FrameComponent;

  vtkImageExtractComponents *ExtractComponent;


};

#endif

