/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeRenderingParametersNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLVolumeRenderingParametersNode - MRML node for storing a slice through RAS space
// .SECTION Description
// This node stores the information about the currently selected volume
//
//

#ifndef __vtkMRMLVolumeRenderingParametersNode_h
#define __vtkMRMLVolumeRenderingParametersNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLROINode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumePropertyNode.h"

#include "vtkVolumeRendering.h"

#include "vtkMatrix4x4.h"

class VTK_SLICERVOLUMERENDERING_EXPORT vtkMRMLVolumeRenderingParametersNode : public vtkMRMLNode
{
  public:
  static vtkMRMLVolumeRenderingParametersNode *New();
  vtkTypeMacro(vtkMRMLVolumeRenderingParametersNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "VolumeRenderingParameters";};

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
  // the ID of a MRMLVolumeNode
  vtkGetStringMacro (VolumeNodeID);
  void SetAndObserveVolumeNodeID(const char *volumeNodeID);

  // Description:
  // the ID of a MRMLVolumeNode (fg volume)
  vtkGetStringMacro (FgVolumeNodeID);
  void SetAndObserveFgVolumeNodeID(const char *volumeNodeID);

  // Description:
  // Associated transform MRML node
  vtkMRMLVolumeNode* GetVolumeNode();

  // Description:
  // Associated transform MRML node (fg volume)
  vtkMRMLVolumeNode* GetFgVolumeNode();

  // Description:
  // the ID of a parameter MRMLVolumePropertyNode
  vtkGetStringMacro (VolumePropertyNodeID);
  void SetAndObserveVolumePropertyNodeID(const char *volumePropertyNodeID);

  // Description:
  // the ID of a parameter MRMLVolumePropertyNode (fg volume)
  vtkGetStringMacro (FgVolumePropertyNodeID);
  void SetAndObserveFgVolumePropertyNodeID(const char *volumePropertyNodeID);

  // Description:
  // Associated transform MRML node
  vtkMRMLVolumePropertyNode* GetVolumePropertyNode();

  // Description:
  // Associated transform MRML node (fg volume)
  vtkMRMLVolumePropertyNode* GetFgVolumePropertyNode();

  // Description:
  // the ID of a parameter MRMLROINode
  vtkGetStringMacro (ROINodeID);
  void SetAndObserveROINodeID(const char *rOINodeID);

  // Description:
  // Associated transform MRML node
  vtkMRMLROINode* GetROINode();

  // Description:
  // Is cropping enabled?
  vtkSetMacro(CroppingEnabled,int);
  vtkGetMacro(CroppingEnabled,int);
  vtkBooleanMacro(CroppingEnabled,int);

  vtkSetMacro(UseThreshold,int);
  vtkGetMacro(UseThreshold,int);
  vtkBooleanMacro(UseThreshold,int);

  vtkSetMacro(UseFgThreshold,int);
  vtkGetMacro(UseFgThreshold,int);
  vtkBooleanMacro(UseFgThreshold,int);

  // Description:
  // Estimated Sample Distance
  vtkSetMacro(EstimatedSampleDistance,double);
  vtkGetMacro(EstimatedSampleDistance,double);

  // Description:
  // Expected FPS
  vtkSetMacro(ExpectedFPS,int);
  vtkGetMacro(ExpectedFPS,int);

  vtkSetMacro(PerformanceControl,int);
  vtkGetMacro(PerformanceControl,int);

  vtkGetMacro (CurrentVolumeMapper, int);
  vtkSetMacro (CurrentVolumeMapper, int);

  vtkGetMacro (GPUMemorySize, int);
  vtkSetMacro (GPUMemorySize, int);

  vtkGetMacro (CPURaycastMode, int);
  vtkSetMacro (CPURaycastMode, int);

  vtkGetMacro (DepthPeelingThreshold, float);
  vtkSetMacro (DepthPeelingThreshold, float);

  vtkGetMacro (DistanceColorBlending, float);
  vtkSetMacro (DistanceColorBlending, float);

  vtkGetMacro (ICPEScale, float);
  vtkSetMacro (ICPEScale, float);

  vtkGetMacro (ICPESmoothness, float);
  vtkSetMacro (ICPESmoothness, float);

  vtkGetMacro (GPURaycastTechnique, int);
  vtkSetMacro (GPURaycastTechnique, int);

  vtkGetMacro (GPURaycastTechniqueII, int);
  vtkSetMacro (GPURaycastTechniqueII, int);

  vtkGetMacro (GPURaycastTechniqueIIFg, int);
  vtkSetMacro (GPURaycastTechniqueIIFg, int);

  vtkGetMacro (GPURaycastIIFusion, int);
  vtkSetMacro (GPURaycastIIFusion, int);

  vtkGetMacro (GPURaycastTechnique3, int);
  vtkSetMacro (GPURaycastTechnique3, int);

  vtkSetVector2Macro(Threshold, double);
  vtkGetVectorMacro(Threshold, double, 2);

  vtkSetVector2Macro(ThresholdFg, double);
  vtkGetVectorMacro(ThresholdFg, double, 2);

  vtkGetMacro (GPURaycastIIBgFgRatio, float);
  vtkSetMacro (GPURaycastIIBgFgRatio, float);

  vtkGetMacro(FollowVolumeDisplayNode, int);
  vtkSetMacro(FollowVolumeDisplayNode, int);

  vtkGetMacro(UseSingleVolumeProperty, int);
  vtkSetMacro(UseSingleVolumeProperty, int);
  
  vtkSetVector2Macro(WindowLevel, double);
  vtkGetVectorMacro(WindowLevel, double, 2);

  vtkSetVector2Macro(WindowLevelFg, double);
  vtkGetVectorMacro(WindowLevelFg, double, 2);
  
protected:
  vtkMRMLVolumeRenderingParametersNode();
  ~vtkMRMLVolumeRenderingParametersNode();
  vtkMRMLVolumeRenderingParametersNode(const vtkMRMLVolumeRenderingParametersNode&);
  void operator=(const vtkMRMLVolumeRenderingParametersNode&);

  char *VolumeNodeID;
  vtkSetReferenceStringMacro(VolumeNodeID);
  vtkMRMLVolumeNode* VolumeNode;

  char *VolumePropertyNodeID;
  vtkSetReferenceStringMacro(VolumePropertyNodeID);
  vtkMRMLVolumePropertyNode* VolumePropertyNode;

  char *FgVolumeNodeID;
  vtkSetReferenceStringMacro(FgVolumeNodeID);
  vtkMRMLVolumeNode* FgVolumeNode;

  char *FgVolumePropertyNodeID;
  vtkSetReferenceStringMacro(FgVolumePropertyNodeID);
  vtkMRMLVolumePropertyNode* FgVolumePropertyNode;

  char *ROINodeID;
  vtkSetReferenceStringMacro(ROINodeID);
  vtkMRMLROINode* ROINode;

  int CroppingEnabled;

  double  EstimatedSampleDistance;
  int     ExpectedFPS;

  /* tracking which mapper to use, not saved into scene file
   * because different machines may or maybe not support the same mapper
   * values of CurrentVolumeMapper
   *-1: not initialized (default)
   * 0: VTK CPU ray cast
   * 1: VTK GPU ray cast
   * 2: VTK GPU texture mapping
   * 3: NCI GPU ray cast
   * 4: NCI GPU ray cast (multi-volume)
   */
  int CurrentVolumeMapper;

  /* tracking GPU memory size, not saved into scene file
   * because different machines may have different GPU memory
   * values
   * 0: 128M
   * 1: 256M (default)
   * 2: 512M
   * 3: 1.0G
   * 4: 1.5G
   * 5: 2.0G
   * 6: 3.0G
   * 7: 4.0G
   */
  int GPUMemorySize;

  /* possible values
   * 0: composite (default)
   * 1: MIP
   */
  int CPURaycastMode;

  float DepthPeelingThreshold;
  float DistanceColorBlending;

  float ICPEScale;
  float ICPESmoothness;

  /* techniques in GPU ray cast
   * 0: composite with directional lighting (default)
   * 1: composite with fake lighting (edge coloring, faster)
   * 2: MIP
   * 3: MINIP
   * 4: Gradient Magnitude Opacity Modulation
   * 5: Illustrative Context Preserving Exploration
   * */
  int GPURaycastTechnique;

  /* techniques in GPU ray cast II (bg)
   * 0: composite with directional lighting (default)
   * 1: composite with fake lighting (edge coloring, faster)
   * 2: MIP
   * 3: MINIP
   * 4: Gradient Magnitude Opacity Modulation
   * 5: Illustrative Context Preserving Exploration
   * */
  int GPURaycastTechniqueII;

  /* techniques in GPU ray cast II (fg)
   * 0: composite with directional lighting (default)
   * 1: composite with fake lighting (edge coloring, faster)
   * 2: MIP
   * 3: MINIP
   * 4: Gradient Magnitude Opacity Modulation
   * 5: Illustrative Context Preserving Exploration
   * */
  int GPURaycastTechniqueIIFg;

  /*
   * fusion method in GPU ray cast II
   * 0: Alpha Blending OR (default)
   * 1: Alpha Blending AND
   * 2: Alpha Blending NOT
   * */
  int GPURaycastIIFusion;

  /* techniques in GPU ray cast 3
   * 0: composite (default)
   * 2: MIP
   * 3: MINIP
   * */
  int GPURaycastTechnique3;

  double Threshold[2];
  double ThresholdFg[2];

  int UseThreshold;
  int UseFgThreshold;

  float GPURaycastIIBgFgRatio;

  //follow window/level and thresholding setting in volume display node
  int FollowVolumeDisplayNode;

  int UseSingleVolumeProperty;

  //bg volume window & level
  double WindowLevel[2];

  //fg volume window & level
  double WindowLevelFg[2];

  /*
   * Performance Control method
   * 0: Adaptive
   * 1: Maximum Quality
   * 2: Fixed Framerate
   * */
  int PerformanceControl;
};

#endif

