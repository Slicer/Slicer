/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLVolumeDisplayNode_h
#define __vtkMRMLVolumeDisplayNode_h

// MRML includes
#include "vtkMRMLDisplayNode.h"
class vtkMRMLScene;
class vtkMRMLVolumeNode;

// VTK includes
class vtkAlgorithmOutput;
class vtkImageData;
class vtkImageStencilData;

/// \brief MRML node for representing a volume display attributes.
///
/// vtkMRMLVolumeDisplayNode nodes describe how volume is displayed.
class VTK_MRML_EXPORT vtkMRMLVolumeDisplayNode : public vtkMRMLDisplayNode
{
public:
  vtkTypeMacro(vtkMRMLVolumeDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLVolumeDisplayNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override = 0;

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  ///
  /// Finds the storage node and read the data
  void UpdateScene(vtkMRMLScene *scene) override;

  ///
  /// Sets ImageData for background mask
  /// Must be reimplemented in deriving class if they need it.
  /// GetBackgroundImageStencilDataConnection() returns 0 if the background image data
  /// is not used.
  virtual void SetBackgroundImageStencilDataConnection(vtkAlgorithmOutput * imageDataConnection);
  virtual vtkAlgorithmOutput* GetBackgroundImageStencilDataConnection();
  virtual vtkImageStencilData* GetBackgroundImageStencilData();

  /// Returns the output of the pipeline if there is a not a null input.
  /// Gets ImageData converted from the real data in the node
  /// The image is the direct output of the pipeline, it might not be
  /// up-to-date. You can call Update() on the returned vtkImageData or use
  /// GetUpToDateImageData() instead.
  /// \sa GetUpToDateImageData()
  virtual vtkAlgorithmOutput* GetImageDataConnection();

  /// Gets ImageData and ensure it's up-to-date by calling Update() on the
  /// pipeline.
  /// Please note that it can be slow, depending on the filters in
  /// the pipeline and the dimension of the input data.

  /// Set the pipeline input.
  /// Filters can be applied to the input image data. The output image data
  /// is the one used by the mappers.
  /// It internally calls SetInputImageDataPipeline that can be reimplemented.
  virtual void SetInputImageDataConnection(vtkAlgorithmOutput *imageDataConnection);
  virtual vtkAlgorithmOutput* GetInputImageDataConnection();

  /// Gets the pipeline input. To be reimplemented in subclasses.
  virtual vtkImageData* GetInputImageData();

  /// Gets the pipeline output. To be reimplemented in subclasses.
  virtual vtkImageData* GetOutputImageData();
  virtual vtkAlgorithmOutput* GetOutputImageDataConnection();

  ///
  /// Update the pipeline based on this node attributes
  virtual void UpdateImageDataPipeline();

  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;
  ///
  /// set gray colormap or override in subclass
  virtual void SetDefaultColorMap();

  /// Search in the scene the volume node vtkMRMLVolumeDisplayNode is associated
  /// to
  vtkMRMLVolumeNode* GetVolumeNode();

protected:
  vtkMRMLVolumeDisplayNode();
  ~vtkMRMLVolumeDisplayNode() override;
  vtkMRMLVolumeDisplayNode(const vtkMRMLVolumeDisplayNode&);
  void operator=(const vtkMRMLVolumeDisplayNode&);

  virtual void SetInputToImageDataPipeline(vtkAlgorithmOutput *imageDataConnection);
};

#endif
