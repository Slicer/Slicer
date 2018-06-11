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
class vtkMRMLAnnotationROINode;
class vtkMRMLVolumeNode;
class vtkMRMLVolumePropertyNode;
class vtkMRMLViewNode;

class vtkIntArray;

/// \ingroup Slicer_QtModules_VolumeRendering
/// \name vtkMRMLVolumeRenderingDisplayNode
/// \brief Abstract MRML node for storing information for Volume Rendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLVolumeRenderingDisplayNode
  : public vtkMRMLDisplayNode
{
public:
  vtkTypeMacro(vtkMRMLVolumeRenderingDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts) VTK_OVERRIDE;

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent) VTK_OVERRIDE;

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node) VTK_OVERRIDE;

  const char* GetVolumeNodeID();
  void SetAndObserveVolumeNodeID(const char *volumeNodeID);
  vtkMRMLVolumeNode* GetVolumeNode();

  const char* GetVolumePropertyNodeID();
  void SetAndObserveVolumePropertyNodeID(const char *volumePropertyNodeID);
  vtkMRMLVolumePropertyNode* GetVolumePropertyNode();

  const char* GetROINodeID();
  void SetAndObserveROINodeID(const char *roiNodeID);
  vtkMRMLAnnotationROINode* GetROINode();

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
  ~vtkMRMLVolumeRenderingDisplayNode();
  vtkMRMLVolumeRenderingDisplayNode(const vtkMRMLVolumeRenderingDisplayNode&);
  void operator=(const vtkMRMLVolumeRenderingDisplayNode&);

  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData) VTK_OVERRIDE;

  static const char* VolumeNodeReferenceRole;
  static const char* VolumeNodeReferenceMRMLAttributeName;
  static const char* VolumePropertyNodeReferenceRole;
  static const char* VolumePropertyNodeReferenceMRMLAttributeName;
  static const char* ROINodeReferenceRole;
  static const char* ROINodeReferenceMRMLAttributeName;

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

