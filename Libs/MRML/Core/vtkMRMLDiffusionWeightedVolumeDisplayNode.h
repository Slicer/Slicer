/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDiffusionWeightedVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLDiffusionWeightedVolumeDisplayNode_h
#define __vtkMRMLDiffusionWeightedVolumeDisplayNode_h

// MRML includes
#include "vtkMRMLScalarVolumeDisplayNode.h"

// VTK includes
class vtkAlgorithmOutput;
class vtkImageData;
class vtkImageExtractComponents;

/// \brief MRML node for representing a volume (image stack).
///
/// Volume nodes describe data sets that can be thought of as stacks of 2D
/// images that form a 3D volume.  Volume nodes describe where the images
/// are stored on disk, how to render the data (window and level), and how
/// to read the files.  This information is extracted from the image
/// headers (if they exist) at the time the MRML file is generated.
/// Consequently, MRML files isolate MRML browsers from understanding how
/// to read the myriad of file formats for medical data.
class VTK_MRML_EXPORT vtkMRMLDiffusionWeightedVolumeDisplayNode : public vtkMRMLScalarVolumeDisplayNode
{
  public:
  static vtkMRMLDiffusionWeightedVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLDiffusionWeightedVolumeDisplayNode,vtkMRMLScalarVolumeDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "DiffusionWeightedVolumeDisplay";}

  ///
  /// Get the pipeline input
  vtkAlgorithmOutput* GetInputImageDataConnection() override;


  void UpdateImageDataPipeline() override;

  //--------------------------------------------------------------------------
  /// Display Information
  //--------------------------------------------------------------------------

  ///
  /// Set/Get interpolate reformated slices
  vtkGetMacro(DiffusionComponent, int);
  vtkSetMacro(DiffusionComponent, int);

protected:
  vtkMRMLDiffusionWeightedVolumeDisplayNode();
  ~vtkMRMLDiffusionWeightedVolumeDisplayNode() override;
  vtkMRMLDiffusionWeightedVolumeDisplayNode(const vtkMRMLDiffusionWeightedVolumeDisplayNode&);
  void operator=(const vtkMRMLDiffusionWeightedVolumeDisplayNode&);

  ///
  /// Set the input of the pipeline
  void SetInputToImageDataPipeline(vtkAlgorithmOutput *imageDataConnection) override;

  vtkAlgorithmOutput* GetScalarImageDataConnection() override;

  /// This property holds the current diffusion component used for display.
  int DiffusionComponent;
  vtkImageExtractComponents *ExtractComponent;
};

#endif

