/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeRenderingDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLVolumeRenderingDisplayNode_h
#define __vtkMRMLVolumeRenderingDisplayNode_h

// Volume Rendering includes
#include "vtkSlicerVolumeRenderingModuleMRMLExport.h"

// MRML includes
#include "vtkMRMLDisplayNode.h"
class vtkIntArray;
class vtkMRMLAnnotationROINode;
class vtkMRMLMarkupsROINode;
class vtkMRMLShaderPropertyNode;
class vtkMRMLViewNode;
class vtkMRMLVolumeNode;
class vtkMRMLVolumePropertyNode;

/// \ingroup Slicer_QtModules_VolumeRendering
/// \name vtkMRMLVolumeRenderingDisplayNode
/// \brief Abstract MRML node for storing information for Volume Rendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLVolumeRenderingDisplayNode
  : public vtkMRMLDisplayNode
{
public:
  vtkTypeMacro(vtkMRMLVolumeRenderingDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  const char* GetVolumeNodeID();
  vtkMRMLVolumeNode* GetVolumeNode();

  const char* GetVolumePropertyNodeID();
  void SetAndObserveVolumePropertyNodeID(const char *volumePropertyNodeID);
  vtkMRMLVolumePropertyNode* GetVolumePropertyNode();

  const char* GetShaderPropertyNodeID();
  void SetAndObserveShaderPropertyNodeID(const char *shaderPropertyNodeID);
  vtkMRMLShaderPropertyNode* GetShaderPropertyNode();
  vtkMRMLShaderPropertyNode* GetOrCreateShaderPropertyNode( vtkMRMLScene * mrmlScene );

  const char* GetROINodeID();
  void SetAndObserveROINodeID(const char *roiNodeID);
  vtkMRMLDisplayableNode* GetROINode();
  /// Deprecated. Use GetROINode() instead for retrieving the markups ROI node.
  vtkMRMLAnnotationROINode* GetAnnotationROINode() { return nullptr; };
  vtkMRMLMarkupsROINode* GetMarkupsROINode();

  vtkMRMLViewNode* GetFirstViewNode();

  double GetSampleDistance();

  vtkSetMacro(CroppingEnabled,int);
  vtkGetMacro(CroppingEnabled,int);
  vtkBooleanMacro(CroppingEnabled,int);

  vtkSetVector2Macro(Threshold, double);
  vtkGetVectorMacro(Threshold, double, 2);

  vtkGetMacro(FollowVolumeDisplayNode, int);
  vtkSetMacro(FollowVolumeDisplayNode, int);

  vtkGetMacro(IgnoreVolumeDisplayNodeThreshold, int);
  vtkSetMacro(IgnoreVolumeDisplayNodeThreshold, int);

  vtkGetMacro(UseSingleVolumeProperty, int);
  vtkSetMacro(UseSingleVolumeProperty, int);

  vtkSetVector2Macro(WindowLevel, double);
  vtkGetVectorMacro(WindowLevel, double, 2);

protected:
  vtkMRMLVolumeRenderingDisplayNode();
  ~vtkMRMLVolumeRenderingDisplayNode() override;
  vtkMRMLVolumeRenderingDisplayNode(const vtkMRMLVolumeRenderingDisplayNode&);
  void operator=(const vtkMRMLVolumeRenderingDisplayNode&);

  void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData) override;

  static const char* VolumePropertyNodeReferenceRole;
  static const char* VolumePropertyNodeReferenceMRMLAttributeName;
  static const char* ROINodeReferenceRole;
  static const char* ROINodeReferenceMRMLAttributeName;
  static const char* ShaderPropertyNodeReferenceRole;
  static const char* ShaderPropertyNodeReferenceMRMLAttributeName;

protected:
  /// Flag indicating whether cropping is enabled
  int CroppingEnabled;

  double Threshold[2];

  /// Follow window/level and thresholding setting in volume display node
  int FollowVolumeDisplayNode;
  int IgnoreVolumeDisplayNodeThreshold;

  int UseSingleVolumeProperty;

  /// Volume window & level
  double WindowLevel[2];
};

#endif
