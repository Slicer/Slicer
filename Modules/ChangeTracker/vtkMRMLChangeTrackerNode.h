/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLChangeTrackerNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLChangeTrackerNode_h
#define __vtkMRMLChangeTrackerNode_h

#include "vtkMRMLNode.h"
#include "vtkChangeTracker.h" // EXPORT definitions

#define REGCHOICE_ALIGNED  1
#define REGCHOICE_RESAMPLE 2
#define REGCHOICE_REGISTER 3
        
#define RESCHOICE_NONE     1
#define RESCHOICE_LEGACY   2
#define RESCHOICE_ISO      3

class VTK_CHANGETRACKER_EXPORT vtkMRMLChangeTrackerNode : public vtkMRMLNode
{
  public:
  static vtkMRMLChangeTrackerNode *New();
  vtkTypeMacro(vtkMRMLChangeTrackerNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create instance of a node.
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "TGParameters";};

  // ------------------------------
  // -- First Step 
  // ------------------------------
  
  // Description:
  // Get/Set input volume MRML Id
  vtkGetStringMacro(Scan1_Ref);
  vtkSetStringMacro(Scan1_Ref);

  vtkSetStringMacro(WorkingDir);
  vtkGetStringMacro(WorkingDir);

  // ------------------------------
  // -- Second Step 
  // ------------------------------
  
  // Description:
  // Get/Set for SegmenterClass
  void SetROIMin(int index, int val) { this->ROIMin[index] = val;  }
  int GetROIMin(int index) { return this->ROIMin[index]; } 

  void SetROIMax(int index, int val) {this->ROIMax[index] = val ; }
  int GetROIMax(int index) {return this->ROIMax[index]; } 

  // Description:
  // Update the stored reference to another node in the scene
  // virtual void UpdateReferenceID(const char *oldID, const char *newID);

  vtkGetMacro(SuperSampled_Spacing,double);
  vtkSetMacro(SuperSampled_Spacing,double);

  vtkGetMacro(SuperSampled_VoxelVolume,double);
  vtkSetMacro(SuperSampled_VoxelVolume,double);

  vtkGetMacro(SuperSampled_RatioNewOldSpacing,double);
  vtkSetMacro(SuperSampled_RatioNewOldSpacing,double);

  vtkGetMacro(Scan1_VoxelVolume,double);
  vtkSetMacro(Scan1_VoxelVolume,double);

  // Description:
  // Result transfered to second step 
  vtkGetStringMacro(Scan1_SuperSampleRef);
  vtkSetStringMacro(Scan1_SuperSampleRef);

  // ------------------------------
  // -- Third Step 
  // ------------------------------
  vtkGetMacro(SegmentThresholdMin,double);
  vtkSetMacro(SegmentThresholdMin,double);

  vtkGetMacro(SegmentThresholdMax,double);
  vtkSetMacro(SegmentThresholdMax,double);

  vtkGetStringMacro(Scan1_SegmentRef);
  vtkSetStringMacro(Scan1_SegmentRef);

  // ------------------------------
  // -- Fourth Step 
  // ------------------------------

  // Description:
  // Get/Set output volume MRML Id
  vtkGetStringMacro(Scan2_Ref);
  vtkSetStringMacro(Scan2_Ref);

  vtkGetStringMacro(Scan2_GlobalRef);
  vtkSetStringMacro(Scan2_GlobalRef);

  vtkGetStringMacro(Scan2_LocalRef);
  vtkSetStringMacro(Scan2_LocalRef);

  vtkSetStringMacro(Scan2_SuperSampleRef);
  vtkGetStringMacro(Scan2_SuperSampleRef);

  vtkSetStringMacro(Scan2_NormedRef);
  vtkGetStringMacro(Scan2_NormedRef);

  vtkSetStringMacro(Scan1_ThreshRef);
  vtkGetStringMacro(Scan1_ThreshRef);

  vtkSetStringMacro(Scan2_ThreshRef);
  vtkGetStringMacro(Scan2_ThreshRef);

  vtkSetStringMacro(Grid_Ref);
  vtkGetStringMacro(Grid_Ref);


  vtkGetMacro(Analysis_Intensity_Flag,int);
  vtkSetMacro(Analysis_Intensity_Flag,int);

  vtkGetMacro(Analysis_Intensity_Sensitivity,double);
  vtkSetMacro(Analysis_Intensity_Sensitivity,double);

  vtkSetStringMacro(Analysis_Intensity_Ref);
  vtkGetStringMacro(Analysis_Intensity_Ref);

  vtkGetMacro(Analysis_Deformable_Flag,int);
  vtkSetMacro(Analysis_Deformable_Flag,int);

  vtkGetMacro(Analysis_Deformable_JacobianGrowth,double);
  vtkSetMacro(Analysis_Deformable_JacobianGrowth,double);

  vtkGetMacro(Analysis_Deformable_SegmentationGrowth,double);
  vtkSetMacro(Analysis_Deformable_SegmentationGrowth,double);

  vtkSetStringMacro(Analysis_Deformable_Ref);
  vtkGetStringMacro(Analysis_Deformable_Ref);

  vtkBooleanMacro(UseITK, bool);
  vtkGetMacro(UseITK, bool);
  vtkSetMacro(UseITK, bool);

  vtkGetStringMacro(Scan2_RegisteredRef);
  vtkSetStringMacro(Scan2_RegisteredRef);

  vtkBooleanMacro(Scan2_RegisteredReady, bool);
  vtkSetMacro(Scan2_RegisteredReady, bool);
  vtkGetMacro(Scan2_RegisteredReady, bool);

  vtkSetMacro(RegistrationChoice, int);
  vtkGetMacro(RegistrationChoice, int);

  vtkBooleanMacro(ROIRegistration, bool);
  vtkSetMacro(ROIRegistration, bool);
  vtkGetMacro(ROIRegistration, bool);

  vtkSetStringMacro(Scan2_TransformRef);
  vtkGetStringMacro(Scan2_TransformRef);

  vtkGetMacro(ResampleChoice, int);
  vtkSetMacro(ResampleChoice, int);
  
  vtkGetMacro(ResampleConst, double);
  vtkSetMacro(ResampleConst, double);

  vtkGetStringMacro(Scan1_InputSegmentRef);
  vtkSetStringMacro(Scan1_InputSegmentRef);
  vtkGetStringMacro(Scan1_InputSegmentSuperSampleRef);
  vtkSetStringMacro(Scan1_InputSegmentSuperSampleRef);

  vtkGetStringMacro(Scan2_InputSegmentRef);
  vtkSetStringMacro(Scan2_InputSegmentRef);
  vtkGetStringMacro(Scan2_InputSegmentSuperSampleRef);
  vtkSetStringMacro(Scan2_InputSegmentSuperSampleRef);

  vtkGetStringMacro(ROI_Ref);
  vtkSetStringMacro(ROI_Ref);

protected:
  vtkMRMLChangeTrackerNode();
  ~vtkMRMLChangeTrackerNode();
  vtkMRMLChangeTrackerNode(const vtkMRMLChangeTrackerNode&);
  void operator=(const vtkMRMLChangeTrackerNode&);

  // AF: this is a bit confusing, since "Ref" is actually a pointer to 
  // the "ID" of a MRML node
  char* Scan1_Ref;
  char* Scan1_SuperSampleRef;
  char* Scan1_SegmentRef;
  char* Scan1_ThreshRef;
  char* Scan2_Ref;
  char* Scan2_GlobalRef;
  char* Scan2_LocalRef;
  char* Scan2_SuperSampleRef;
  char* Scan2_NormedRef;
  char* Scan2_ThreshRef;
  char* ROI_Ref;

  // keep pointers to the input segmentations, if available
  char* Scan1_InputSegmentRef;
  char* Scan1_InputSegmentSuperSampleRef;
  char* Scan2_InputSegmentRef;
  char* Scan2_InputSegmentSuperSampleRef;

  char* Analysis_Intensity_Ref;
  char* Analysis_Deformable_Ref;

  char* Grid_Ref;

  char* WorkingDir;

  // AF: keep the reference to the ITK registration result (will be done
  // asynchronously)
  char* Scan2_RegisteredRef;

  vtkstd::vector<int>  ROIMin; 
  vtkstd::vector<int>  ROIMax; 
  double SuperSampled_Spacing;
  double SuperSampled_VoxelVolume;
  double SuperSampled_RatioNewOldSpacing;
  double Scan1_VoxelVolume;

  double SegmentThresholdMin;
  double SegmentThresholdMax;

  int    Analysis_Intensity_Flag;
  double Analysis_Intensity_Sensitivity;  

  int    Analysis_Deformable_Flag;
  double Analysis_Deformable_JacobianGrowth;
  double Analysis_Deformable_SegmentationGrowth;

  // AF: setting this will result in using ITK functionality for registration
  // and such. Alternatively, Slicer2-inherited functionality will be used
  // (Slicer2 stuff will need to stay, since we need to be able to validate 
  // previous results)
  // AF: TODO: these variables should NOT be in MRML node -- move this to
  // Logic
  bool UseITK;
  bool Scan2_RegisteredReady;
  int RegistrationChoice;
  bool ROIRegistration;
  int ResampleChoice;
  double ResampleConst;

  char* Scan2_TransformRef;
};

#endif

