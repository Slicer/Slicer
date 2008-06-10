/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTumorGrowthNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLTumorGrowthNode_h
#define __vtkMRMLTumorGrowthNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkImageData.h"
#include "vtkTumorGrowth.h"

class vtkImageData;
class vtkImageThreshold;
class vtkImageSumOverVoxels;

class VTK_TUMORGROWTH_EXPORT vtkMRMLTumorGrowthNode : public vtkMRMLNode
{
  public:
  static vtkMRMLTumorGrowthNode *New();
  vtkTypeMacro(vtkMRMLTumorGrowthNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create instance of a GAD node.
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
  void SetROIMin(int index, int val) {this->ROIMin[index] = val ; }
  int GetROIMin(int index) {return this->ROIMin[index]; } 

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

protected:
  vtkMRMLTumorGrowthNode();
  ~vtkMRMLTumorGrowthNode();
  vtkMRMLTumorGrowthNode(const vtkMRMLTumorGrowthNode&);
  void operator=(const vtkMRMLTumorGrowthNode&);

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

  char* Analysis_Intensity_Ref;
  char* Analysis_Deformable_Ref;

  char* Grid_Ref;

  char* WorkingDir;

  //BTX
  vtkstd::vector<int>  ROIMin; 
  vtkstd::vector<int>  ROIMax; 
  //ETX 
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
  
};

#endif

