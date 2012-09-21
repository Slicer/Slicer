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

class vtkIntArray;

/// \ingroup Slicer_QtModules_VolumeRendering
/// \name vtkMRMLVolumeRenderingDisplayNode
/// \brief Abstract MRML node for storing information for Volume Rendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLVolumeRenderingDisplayNode
  : public vtkMRMLDisplayNode
{
public:
  static vtkMRMLVolumeRenderingDisplayNode *New();
  vtkTypeMacro(vtkMRMLVolumeRenderingDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "VolumeRendering";};

  /// Mark the volume, ROI and volume property nodes as references.
  virtual void SetSceneReferences();

  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  /// Observe the reference transform node
  virtual void UpdateScene(vtkMRMLScene *scene);

  /// the ID of a MRMLVolumeNode
  vtkGetStringMacro (VolumeNodeID);
  void SetAndObserveVolumeNodeID(const char *volumeNodeID);

  /// Associated transform MRML node
  vtkMRMLVolumeNode* GetVolumeNode();

  /// the ID of a parameter MRMLVolumePropertyNode
  vtkGetStringMacro (VolumePropertyNodeID);
  void SetAndObserveVolumePropertyNodeID(const char *volumePropertyNodeID);

  /// Associated transform MRML node
  vtkMRMLVolumePropertyNode* GetVolumePropertyNode();

  /// the ID of a parameter MRMLROINode
  vtkGetStringMacro (ROINodeID);
  void SetAndObserveROINodeID(const char *roiNodeID);

  /// Associated transform MRML node
  vtkMRMLAnnotationROINode* GetROINode();

  /// Is cropping enabled?
  vtkSetMacro(CroppingEnabled,int);
  vtkGetMacro(CroppingEnabled,int);
  vtkBooleanMacro(CroppingEnabled,int);

  //vtkSetMacro(UseThreshold,int);
  //vtkGetMacro(UseThreshold,int);
  //vtkBooleanMacro(UseThreshold,int);

  /// Estimated Sample Distance
  vtkSetMacro(EstimatedSampleDistance,double);
  vtkGetMacro(EstimatedSampleDistance,double);

  /// Expected FPS
  vtkSetMacro(ExpectedFPS,double);
  vtkGetMacro(ExpectedFPS,double);

  vtkSetMacro(PerformanceControl,int);
  vtkGetMacro(PerformanceControl,int);

  vtkGetMacro (GPUMemorySize, int);
  vtkSetMacro (GPUMemorySize, int);

  enum RayCastType
  {
    Composite = 0, // composite with directional lighting (default)
    CompositeEdgeColoring, // composite with fake lighting (edge coloring, faster)
    MaximumIntensityProjection,
    MinimumIntensityProjection,
    GradiantMagnitudeOpacityModulation,
    IllustrativeContextPreservingExploration
  };

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

  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData);

  vtkIntArray* ObservedEvents;

  char *VolumeNodeID;
  virtual void SetVolumeNodeID(const char* arg);
  vtkMRMLVolumeNode* VolumeNode;

  char *VolumePropertyNodeID;
  virtual void SetVolumePropertyNodeID(const char* arg);
  vtkMRMLVolumePropertyNode* VolumePropertyNode;

  char *ROINodeID;
  virtual void SetROINodeID(const char* arg);
  vtkMRMLAnnotationROINode* ROINode;

  int CroppingEnabled;

  double  EstimatedSampleDistance;
  double  ExpectedFPS;

  /// Tracking GPU memory size (in Mo), not saved into scene file
  /// because different machines may have different GPU memory
  /// values.
  /// A value of 0 indicates to use the default value in the settings
  ///
  int GPUMemorySize;

  double Threshold[2];

  //int UseThreshold;

  /// follow window/level and thresholding setting in volume display node
  int FollowVolumeDisplayNode;
  int IgnoreVolumeDisplayNodeThreshold;

  int UseSingleVolumeProperty;

  /// Volume window & level
  double WindowLevel[2];

  /// Performance Control method
  /// 0: Adaptive
  /// 1: Maximum Quality
  /// 2: Fixed Framerate
  int PerformanceControl;
};

#endif

