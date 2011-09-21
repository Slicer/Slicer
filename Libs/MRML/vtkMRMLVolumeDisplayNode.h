/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLVolumeDisplayNode - MRML node for representing a volume display attributes
/// 
/// vtkMRMLVolumeDisplayNode nodes describe how volume is displayed.

#ifndef __vtkMRMLVolumeDisplayNode_h
#define __vtkMRMLVolumeDisplayNode_h

// MRML includes
#include "vtkMRMLDisplayNode.h"
class vtkMRMLScene;
class vtkMRMLVolumeNode;

// VTK includes
class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLVolumeDisplayNode : public vtkMRMLDisplayNode
{
public:
  vtkTypeMacro(vtkMRMLVolumeDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() = 0;

  /// 
  /// Updates this node if it depends on other nodes 
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  /// 
  /// Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  /// 
  /// Sets ImageData for background mask 
  virtual void SetBackgroundImageData(vtkImageData * vtkNotUsed(imageData)) {};

  ///
  /// Returns the output of the pipeline if there is a not a null input.
  virtual vtkImageData* GetImageData();

  ///
  /// Set the pipeline input. Internally calls ConnectInputImageDataToPipeline to be
  /// reimplemented.
  virtual void SetInputImageData(vtkImageData *imageData);

  ///
  /// Gets the pipeline input. To be reimplemented in subclasses.
  virtual vtkImageData* GetInputImageData();

  ///
  /// Gets the pipeline output. To be reimplemented in subclasses.
  virtual vtkImageData* GetOutputImageData();

  /// 
  /// Update the pipeline based on this node attributes
  virtual void UpdateImageDataPipeline();

  /// 
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
  /// 
  /// set gray colormap or override in subclass
  virtual void SetDefaultColorMap();

  ///
  /// Search in the scene the volume node vtkMRMLVolumeDisplayNode is associated
  /// to
  vtkMRMLVolumeNode* GetVolumeNode();

protected:
  vtkMRMLVolumeDisplayNode();
  ~vtkMRMLVolumeDisplayNode();
  vtkMRMLVolumeDisplayNode(const vtkMRMLVolumeDisplayNode&);
  void operator=(const vtkMRMLVolumeDisplayNode&);
  
  virtual void SetInputToImageDataPipeline(vtkImageData *imageData);
};

#endif

