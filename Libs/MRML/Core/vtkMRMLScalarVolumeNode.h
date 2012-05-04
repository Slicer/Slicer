/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
///  vtkMRMLVolumeNode - MRML node for representing a volume (image stack).
/// 
/// Volume nodes describe data sets that can be thought of as stacks of 2D 
/// images that form a 3D volume. Volume nodes contain only the image data,
/// where it is store on disk and how to read the files is controlled by
/// the volume storage node, how to render the data (window and level) is
/// controlled by the volume display nodes. Image information is extracted 
/// from the image headers (if they exist) at the time the MRML file is
/// generated.
/// Consequently, MRML files isolate MRML browsers from understanding how 
/// to read the myriad of file formats for medical data. 
/// A scalar volume node can be a labelmap, which is typically the output of
/// a segmentation that labels each voxel according to its tissue type.
/// The alternative is a gray-level or color image

#ifndef __vtkMRMLScalarVolumeNode_h
#define __vtkMRMLScalarVolumeNode_h

// MRML includes
#include "vtkMRMLVolumeNode.h"
class vtkMRMLScalarVolumeDisplayNode;
class vtkMRMLVolumeArchetypeStorageNode;

// VTK includes
class vtkImageData;
class vtkImageAccumulateDiscrete;
class vtkImageBimodalAnalysis;

class VTK_MRML_EXPORT vtkMRMLScalarVolumeNode : public vtkMRMLVolumeNode
{
  public:
  static vtkMRMLScalarVolumeNode *New();
  vtkTypeMacro(vtkMRMLScalarVolumeNode,vtkMRMLVolumeNode);
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
  virtual const char* GetNodeTagName() {return "Volume";};

  ///
  /// Returns true (1) if the volume is a labelmap (1 label value per voxel
  /// to indicates the tissue type.
  /// \sa SetLabelMap
  int GetLabelMap();

  ///
  /// Set the volume as a labelmap. 
  /// If true (1), it sets the volume as a labelmap.
  /// \sa GetLabelMap(), LabelMapOn(), LabelMapOff()
  void SetLabelMap(int);

  ///
  /// Convenient method that sets the volume as labelmap
  /// \sa SetLabelMap, LabelMapOff()
  void LabelMapOn();

  ///
  /// Convenient method that unsets the volume a labelmap
  /// \sa SetLabelMap, LabelMapOff()
  void LabelMapOff();

  /// 
  /// Make a 'None' volume node with blank image data
  static void CreateNoneNode(vtkMRMLScene *scene);

  /// 
  /// Associated display MRML node
  virtual vtkMRMLScalarVolumeDisplayNode* GetScalarVolumeDisplayNode();

  /// 
  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

protected:
  vtkMRMLScalarVolumeNode();
  ~vtkMRMLScalarVolumeNode();
  vtkMRMLScalarVolumeNode(const vtkMRMLScalarVolumeNode&);
  void operator=(const vtkMRMLScalarVolumeNode&);

  /// 
  /// Used internally in CalculateScalarAutoLevels and CalculateStatisticsAutoLevels
  vtkImageAccumulateDiscrete *Accumulate;
  vtkImageBimodalAnalysis *Bimodal;

  int CalculatingAutoLevels;
};

#endif


 

