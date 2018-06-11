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

  /// Is cropping enabled?
  vtkSetMacro(CroppingEnabled,int);
  vtkGetMacro(CroppingEnabled,int);
  vtkBooleanMacro(CroppingEnabled,int);

  /// Estimated Sample Distance
  vtkSetMacro(EstimatedSampleDistance,double);
  vtkGetMacro(EstimatedSampleDistance,double);

  /// Expected FPS
  vtkSetMacro(ExpectedFPS,double);
  vtkGetMacro(ExpectedFPS,double);

  /// Quality used for PerformanceControl
  enum Quality
  {
    AdaptiveQuality = 0, ///< quality determined from desired update rate
    NormalQuality = 1,   ///< good image quality at reasonable speed
    MaximumQuality = 2,  ///< high image quality, rendering time is not considered
    Adaptative = 0       ///< deprecated (kept for backward compatibility only, same as AdaptiveQuality)
  };
  vtkSetMacro(PerformanceControl,int);
  vtkGetMacro(PerformanceControl,int);

  vtkGetMacro(GPUMemorySize, int);
  vtkSetMacro(GPUMemorySize, int);

  enum RayCastType
  {
    Composite = 0, // Composite with directional lighting (default)
    CompositeEdgeColoring, // Composite with fake lighting (edge coloring, faster) - Not used
    MaximumIntensityProjection,
    MinimumIntensityProjection,
    GradiantMagnitudeOpacityModulation, // Not used
    IllustrativeContextPreservingExploration // Not used
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

  vtkGetMacro(RaycastTechnique, int);
  vtkSetMacro(RaycastTechnique, int);

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

  double EstimatedSampleDistance;
  double ExpectedFPS;

  /// Tracking GPU memory size (in MB), not saved into scene file
  /// because different machines may have different GPU memory
  /// values.
  /// A value of 0 indicates to use the default value in the settings
  int GPUMemorySize;

  double Threshold[2];

  /// Follow window/level and thresholding setting in volume display node
  int FollowVolumeDisplayNode;
  int IgnoreVolumeDisplayNodeThreshold;

  int UseSingleVolumeProperty;

  /// Volume window & level
  double WindowLevel[2];

  /// Performance Control method
  /// 0: Adaptive
  /// 1: Maximum Quality
  /// 2: Fixed Framerate // unsupported yet
  int PerformanceControl;

  /// Techniques for ray cast
  /// 0: Composite with directional lighting (default)
  /// 1: Composite with fake lighting (edge coloring, faster) - Not used
  /// 2: MIP
  /// 3: MINIP
  /// 4: Gradient Magnitude Opacity Modulation - Not used
  /// 5: Illustrative Context Preserving Exploration - Not used
  int RaycastTechnique;
};

#endif

