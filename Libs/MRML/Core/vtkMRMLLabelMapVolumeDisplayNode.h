/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLLabelMapVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLLabelMapVolumeDisplayNode_h
#define __vtkMRMLLabelMapVolumeDisplayNode_h

#include "vtkMRMLVolumeDisplayNode.h"

class vtkImageAlgorithm;
class vtkImageMapToColors;

/// \brief MRML node for representing a volume display attributes.
///
/// vtkMRMLLabelMapVolumeDisplayNode nodes describe how volume is displayed.
class VTK_MRML_EXPORT vtkMRMLLabelMapVolumeDisplayNode : public vtkMRMLVolumeDisplayNode
{
  public:
  static vtkMRMLLabelMapVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLLabelMapVolumeDisplayNode,vtkMRMLVolumeDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "LabelMapVolumeDisplay";}

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentDefaultMacro(vtkMRMLLabelMapVolumeDisplayNode);

  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;
  ///
  /// set default labels colormap
  void SetDefaultColorMap() override;

  /// Set the pipeline input
  void SetInputImageDataConnection(vtkAlgorithmOutput *imageDataConnection) override;

  /// Get the pipeline input
  vtkImageData* GetInputImageData() override;

  /// Gets the pipeline output
  vtkAlgorithmOutput* GetOutputImageDataConnection() override;

  void UpdateImageDataPipeline() override;

protected:
  vtkMRMLLabelMapVolumeDisplayNode();
  ~vtkMRMLLabelMapVolumeDisplayNode() override;
  vtkMRMLLabelMapVolumeDisplayNode(const vtkMRMLLabelMapVolumeDisplayNode&);
  void operator=(const vtkMRMLLabelMapVolumeDisplayNode&);

  vtkImageMapToColors *MapToColors;

};

#endif
