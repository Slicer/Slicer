/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLMultiVolumeRenderingDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLMultiVolumeRenderingDisplayNode - MRML node for storing volume rendering display parameters
// .SECTION Description
// This node stores the display information about the currently selected volume
//
//

#ifndef __vtkMRMLMultiVolumeRenderingDisplayNode_h
#define __vtkMRMLMultiVolumeRenderingDisplayNode_h

// Multi-Volume Rendering includes
#include "vtkSlicerMultiVolumeRenderingModuleMRMLExport.h"

// MRML includes
#include "vtkMRMLDisplayNode.h"
class vtkMRMLAnnotationROINode;
class vtkMRMLVolumeNode;
class vtkMRMLVolumePropertyNode;

class vtkIntArray;

/// \ingroup Slicer_QtModules_VolumeRendering
class VTK_SLICER_MULTIVOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLMultiVolumeRenderingDisplayNode
  : public vtkMRMLDisplayNode
{
  public:
  static vtkMRMLMultiVolumeRenderingDisplayNode *New();
  vtkTypeMacro(vtkMRMLMultiVolumeRenderingDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "MultiVolumeRenderingParameters";};

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Observe the reference transform node
  virtual void UpdateScene(vtkMRMLScene *scene);

  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData);

  // Description:
  // the ID of a MRMLVolumeNode (bg volume)
  vtkGetStringMacro (BgVolumeNodeID);
  void SetAndObserveBgVolumeNodeID(const char *volumeNodeID);

  // Description:
  // the ID of a MRMLVolumeNode (fg volume)
  vtkGetStringMacro (FgVolumeNodeID);
  void SetAndObserveFgVolumeNodeID(const char *volumeNodeID);

  // Description:
  // the ID of a MRMLVolumeNode (fg volume)
  vtkGetStringMacro (LabelmapVolumeNodeID);
  void SetAndObserveLabelmapVolumeNodeID(const char *volumeNodeID);

  // Description:
  // Associated transform MRML node (bg volume)
  vtkMRMLVolumeNode* GetBgVolumeNode();

  // Description:
  // Associated transform MRML node (fg volume)
  vtkMRMLVolumeNode* GetFgVolumeNode();

  // Description:
  // Associated transform MRML node (labelmap volume)
  vtkMRMLVolumeNode* GetLabelmapVolumeNode();

  // Description:
  // the ID of a parameter MRMLVolumePropertyNode (bg volume)
  vtkGetStringMacro (BgVolumePropertyNodeID);
  void SetAndObserveBgVolumePropertyNodeID(const char *volumePropertyNodeID);

  // Description:
  // the ID of a parameter MRMLVolumePropertyNode (fg volume)
  vtkGetStringMacro (FgVolumePropertyNodeID);
  void SetAndObserveFgVolumePropertyNodeID(const char *volumePropertyNodeID);

  // Description:
  // Associated property MRML node (bg volume)
  vtkMRMLVolumePropertyNode* GetBgVolumePropertyNode();

  // Description:
  // Associated property MRML node (fg volume)
  vtkMRMLVolumePropertyNode* GetFgVolumePropertyNode();

  // Description:
  // the ID of a parameter MRMLROINode (bg volume)
  vtkGetStringMacro (BgROINodeID);
  void SetAndObserveBgROINodeID(const char *rOINodeID);

  // Description:
  // the ID of a parameter MRMLROINode (fg volume)
  vtkGetStringMacro (FgROINodeID);
  void SetAndObserveFgROINodeID(const char *rOINodeID);

  // Description:
  // the ID of a parameter MRMLROINode (labelmap volume)
  vtkGetStringMacro (LabelmapROINodeID);
  void SetAndObserveLabelmapROINodeID(const char *rOINodeID);

  // Description:
  // Associated ROI MRML node
  vtkMRMLAnnotationROINode* GetBgROINode();
  vtkMRMLAnnotationROINode* GetFgROINode();
  vtkMRMLAnnotationROINode* GetLabelmapROINode();

  // Description:
  // Is cropping enabled?
  vtkSetMacro(BgCroppingEnabled,int);
  vtkGetMacro(BgCroppingEnabled,int);
  vtkBooleanMacro(BgCroppingEnabled,int);

  // Description:
  // Is cropping enabled?
  vtkSetMacro(FgCroppingEnabled,int);
  vtkGetMacro(FgCroppingEnabled,int);
  vtkBooleanMacro(FgCroppingEnabled,int);

  // Description:
  // Is cropping enabled?
  vtkSetMacro(LabelmapCroppingEnabled,int);
  vtkGetMacro(LabelmapCroppingEnabled,int);
  vtkBooleanMacro(LabelmapCroppingEnabled,int);

  // Description:
  // Estimated Sample Distance
  vtkSetMacro(EstimatedSampleDistance,double);
  vtkGetMacro(EstimatedSampleDistance,double);

  // Description:
  // Expected FPS
  vtkSetMacro(ExpectedFPS,double);
  vtkGetMacro(ExpectedFPS,double);

  vtkSetMacro(PerformanceControl,int);
  vtkGetMacro(PerformanceControl,int);

  enum MultiVolumeRenderingModeType
  {
    Linked = 0,
    Independent
  };

  vtkSetMacro(MultiVolumeRenderingMode, int);
  vtkGetMacro(MultiVolumeRenderingMode, int);

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

  vtkGetMacro (BgRaycastTechnique, int);
  vtkSetMacro (BgRaycastTechnique, int);

  vtkGetMacro (FgRaycastTechnique, int);
  vtkSetMacro (FgRaycastTechnique, int);

  enum FusionType
  {
    AlphaBlendingOR = 0, // (default)
    AlphaBlendingAND,
    AlphaBlendingNOT
  };

  vtkGetMacro (MultiVolumeFusionMethod, int);
  vtkSetMacro (MultiVolumeFusionMethod, int);

  vtkGetMacro (BgFgRatio, float);
  vtkSetMacro (BgFgRatio, float);

  vtkGetMacro(BgFollowVolumeDisplayNode, int);
  vtkSetMacro(BgFollowVolumeDisplayNode, int);

  vtkGetMacro(FgFollowVolumeDisplayNode, int);
  vtkSetMacro(FgFollowVolumeDisplayNode, int);

  vtkGetMacro(BgIgnoreVolumeDisplayNodeThreshold, int);
  vtkSetMacro(BgIgnoreVolumeDisplayNodeThreshold, int);

  vtkGetMacro(FgIgnoreVolumeDisplayNodeThreshold, int);
  vtkSetMacro(FgIgnoreVolumeDisplayNodeThreshold, int);

  vtkGetMacro(BgVisibility, bool);
  vtkSetMacro(BgVisibility, bool);

  vtkGetMacro(FgVisibility, bool);
  vtkSetMacro(FgVisibility, bool);

  vtkGetMacro(LabelmapVisibility, bool);
  vtkSetMacro(LabelmapVisibility, bool);

  //Description:
  //update display node visibility based on user settings for bg, fg and labelmap
  void UpdateVisibility();

protected:
  vtkMRMLMultiVolumeRenderingDisplayNode();
  ~vtkMRMLMultiVolumeRenderingDisplayNode();
  vtkMRMLMultiVolumeRenderingDisplayNode(const vtkMRMLMultiVolumeRenderingDisplayNode&);
  void operator=(const vtkMRMLMultiVolumeRenderingDisplayNode&);

  vtkIntArray* ObservedEvents;

  char *BgVolumeNodeID;
  virtual void SetBgVolumeNodeID(const char* arg);
  vtkMRMLVolumeNode* BgVolumeNode;

  char *FgVolumeNodeID;
  virtual void SetFgVolumeNodeID(const char* arg);
  vtkMRMLVolumeNode* FgVolumeNode;

  char *LabelmapVolumeNodeID;
  virtual void SetLabelmapVolumeNodeID(const char* arg);
  vtkMRMLVolumeNode* LabelmapVolumeNode;

  char *BgVolumePropertyNodeID;
  virtual void SetBgVolumePropertyNodeID(const char* arg);
  vtkMRMLVolumePropertyNode* BgVolumePropertyNode;

  char *FgVolumePropertyNodeID;
  virtual void SetFgVolumePropertyNodeID(const char* arg);
  vtkMRMLVolumePropertyNode* FgVolumePropertyNode;

  char *BgROINodeID;
  virtual void SetBgROINodeID(const char* arg);
  vtkMRMLAnnotationROINode* BgROINode;

  char *FgROINodeID;
  virtual void SetFgROINodeID(const char* arg);
  vtkMRMLAnnotationROINode* FgROINode;

  char *LabelmapROINodeID;
  virtual void SetLabelmapROINodeID(const char* arg);
  vtkMRMLAnnotationROINode* LabelmapROINode;

  int BgCroppingEnabled;
  int FgCroppingEnabled;
  int LabelmapCroppingEnabled;

  double  EstimatedSampleDistance;
  double  ExpectedFPS;

  /* Tracking GPU memory size (in Mo), not saved into scene file
   * because different machines may have different GPU memory
   * values.
   * A value of 0 indicates to use the default value in the settings
   */
  int GPUMemorySize;

  /* techniques in GPU ray cast
   * 0: composite with directional lighting (default)
   * 1: composite with fake lighting (edge coloring, faster)
   * 2: MIP
   * 3: MINIP
   * 4: Gradient Magnitude Opacity Modulation
   * 5: Illustrative Context Preserving Exploration
   * */
  int BgRaycastTechnique;
  int FgRaycastTechnique;

  /*
   * fusion method in GPU ray cast II
   * 0: Alpha Blending OR (default)
   * 1: Alpha Blending AND
   * 2: Alpha Blending NOT
   * */
  int MultiVolumeFusionMethod;

  int MultiVolumeRenderingMode;

  float BgFgRatio;

  //follow window/level and thresholding setting in volume display node
  int BgFollowVolumeDisplayNode;
  int BgIgnoreVolumeDisplayNodeThreshold;

  int FgFollowVolumeDisplayNode;
  int FgIgnoreVolumeDisplayNodeThreshold;

  /*
   * Performance Control method
   * 0: Adaptive
   * 1: Maximum Quality
   * 2: Fixed Framerate
   * */
  int PerformanceControl;

  bool BgVisibility;
  bool FgVisibility;
  bool LabelmapVisibility;
};

#endif

