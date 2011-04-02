/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerEMSegmentLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerEMSegmentLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerEMSegmentLogic_h
#define __vtkSlicerEMSegmentLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include <vtkMRML.h>

// STD includes
#include <cstdlib>

#include "vtkSlicerEMSegmentModuleLogicExport.h"

class vtkEMSegmentMRMLManager;
class vtkMRMLVolumeNode;
class vtkImageEMLocalSegmenter;
class vtkImageEMLocalSuperClass;
class vtkImageEMLocalClass;
class vtkImageEMLocalGenericClass;
class vtkTransform;
class vtkGridTransform;
class vtkImageData;
class vtkMatrix4x4;

/// \ingroup Slicer_QtModules_EMSegment
class VTK_SLICER_EMSEGMENT_MODULE_LOGIC_EXPORT vtkSlicerEMSegmentLogic :
  public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerEMSegmentLogic *New();
  vtkTypeRevisionMacro(vtkSlicerEMSegmentLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Name of the Module
  /// This is used to construct the proc invocations
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  /// Actions
  virtual void      SaveTemplateNow();
  virtual bool      SaveIntermediateResults();

  /// Old Pipeline
//  virtual bool      StartPreprocessing();
//  virtual bool      StartPreprocessingInitializeInputData();
//  virtual bool      StartPreprocessingTargetIntensityNormalization();
//  virtual bool      StartPreprocessingTargetToTargetRegistration();
//  virtual bool      StartPreprocessingAtlasToTargetRegistration();
//  virtual void      StartSegmentation();

  /// New Pipeline
//  virtual int       SourceTclFile(vtkKWApplication*app,const char *tclFile);
//  virtual int       SourceTaskFiles(vtkKWApplication* app);
//  virtual int       SourcePreprocessingTclFiles(vtkKWApplication* app);
  virtual void      StartSegmentationWithoutPreprocessing();
//  int               ComputeIntensityDistributionsFromSpatialPrior(vtkKWApplication* app);


  //BTX
  std::string DefineTclTaskFullPathName(const char* TclFileName);
  std::string GetTclTaskDirectory();
  std::string GetTclGeneralDirectory();
  std::string DefineTclTasksFileFromMRML();
  //ETX

  /// Used within StartSegmentation to copy data from the MRMLManager
  /// to the segmenter algorithm.  Possibly useful for research purposes.
  virtual void      CopyDataToSegmenter(vtkImageEMLocalSegmenter* segmenter);

  /// Progress bar related functions: not currently used, likely to
  /// change
  vtkGetStringMacro(ProgressCurrentAction);
  vtkGetMacro(ProgressGlobalFractionCompleted, double);
  vtkGetMacro(ProgressCurrentFractionCompleted, double);

  /// MRML Related Methods.  The collection of MRML nodes for the
  /// EMSegmenter is complicated.  Therefore, the management of these
  /// nodes are delagated to the vtkEMSegmentMRMLManager class.
  vtkGetObjectMacro(MRMLManager, vtkEMSegmentMRMLManager);

  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  ///
  /// Special testing functions
  virtual void      PopulateTestingData();
  virtual void      SpecialTestingFunction();

  void StartPreprocessingResampleToTarget(vtkMRMLVolumeNode* movingVolumeNode,
                                          vtkMRMLVolumeNode* fixedVolumeNode,
                                          vtkMRMLVolumeNode* outputVolumeNode);

  static void TransferIJKToRAS(vtkMRMLVolumeNode* volumeNode, int ijk[3], double ras[3]);
  static void TransferRASToIJK(vtkMRMLVolumeNode* volumeNode, double ras[3], int ijk[3]);


  double GuessRegistrationBackgroundLevel(vtkMRMLVolumeNode* volumeNode);

  static void SlicerRigidRegister(vtkMRMLVolumeNode* fixedVolumeNode,
                                  vtkMRMLVolumeNode* movingVolumeNode,
                                  vtkMRMLVolumeNode* outputVolumeNode,
                                  vtkTransform* fixedRASToMovingRASTransform,
                                  int imageMatchType,
                                  int iterpolationType,
                                  double backgroundLevel);

  static void
  SlicerBSplineRegister(vtkMRMLVolumeNode* fixedVolumeNode,
                        vtkMRMLVolumeNode* movingVolumeNode,
                        vtkMRMLVolumeNode* outputVolumeNode,
                        vtkGridTransform* fixedRASToMovingRASTransform,
                        vtkTransform* fixedRASToMovingRASAffineTransform,
                        int imageMatchType,
                        int iterpolationType,
                        double backgroundLevel);

  static void
  SlicerImageResliceWithGrid(vtkMRMLVolumeNode* inputVolumeNode,
                             vtkMRMLVolumeNode* outputVolumeNode,
                             vtkMRMLVolumeNode* outputVolumeGeometryNode,
                             vtkGridTransform* outputRASToInputRASTransform,
                             int iterpolationType,
                             double backgroundLevel);


  /// Utility --- should probably go to general slicer lib at some point
  static void SlicerImageReslice(vtkMRMLVolumeNode* inputVolumeNode,
                                 vtkMRMLVolumeNode* outputVolumeNode,
                                 vtkMRMLVolumeNode* outputVolumeGeometryNode,
                                 vtkTransform* outputRASToInputRASTransform,
                                 int iterpolationType,
                                 double backgroundLevel);

  void PrintText(char *TEXT);

  void DefineValidSegmentationBoundary();


protected:

  vtkSlicerEMSegmentLogic();
  virtual ~vtkSlicerEMSegmentLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  virtual void RegisterNodes();

private:

  vtkSlicerEMSegmentLogic(const vtkSlicerEMSegmentLogic&); // Not implemented
  void operator=(const vtkSlicerEMSegmentLogic&);          // Not implemented

  //BTX
  template <class T>
  static T GuessRegistrationBackgroundLevel(vtkImageData* imageData);
  //ETX

  static void
  ComposeGridTransform(vtkGridTransform* inGrid,
                       vtkMatrix4x4*     preMultiply,
                       vtkMatrix4x4*     postMultiply,
                       vtkGridTransform* outGrid);

  /// Convenience method for determining if two volumes have same geometry
  static bool IsVolumeGeometryEqual(vtkMRMLVolumeNode* lhs,
                                    vtkMRMLVolumeNode* rhs);

  static void PrintImageInfo(vtkMRMLVolumeNode* volumeNode);
  static void PrintImageInfo(vtkImageData* image);

  /// Copy data from MRML to algorithm
  virtual void CopyAtlasDataToSegmenter(vtkImageEMLocalSegmenter* segmenter);
  virtual void CopyTargetDataToSegmenter(vtkImageEMLocalSegmenter* segmenter);
  virtual void CopyGlobalDataToSegmenter(vtkImageEMLocalSegmenter* segmenter);
  virtual void CopyTreeDataToSegmenter(vtkImageEMLocalSuperClass* node,
                                       vtkIdType nodeID);
  virtual void CopyTreeGenericDataToSegmenter(vtkImageEMLocalGenericClass*
                                              node,
                                              vtkIdType nodeID);
  virtual void CopyTreeParentDataToSegmenter(vtkImageEMLocalSuperClass* node,
                                             vtkIdType nodeID);
  virtual void CopyTreeLeafDataToSegmenter(vtkImageEMLocalClass* node,
                                           vtkIdType nodeID);

  /// Convienience methods for translating enums between algorithm and this module
  virtual int ConvertGUIEnumToAlgorithmEnumStoppingConditionType(int guiEnumValue);
  virtual int ConvertGUIEnumToAlgorithmEnumInterpolationType(int guiEnumValue);

  /// Not currently used
  vtkSetStringMacro(ProgressCurrentAction);
  vtkSetMacro(ProgressGlobalFractionCompleted, double);
  vtkSetMacro(ProgressCurrentFractionCompleted, double);

  //void UpdateIntensityDistributionAuto(vtkKWApplication* app, vtkIdType nodeID);

  ///
  /// Since the mrml nodes are very complicated for this module, we
  /// delegate the handling of them to a MRML manager
  vtkEMSegmentMRMLManager* MRMLManager;

  char *ModuleName;

  ///
  /// Information related to progress bars: this mechanism is not
  /// currently implemented and might me best implemented elsewhere
  char*  ProgressCurrentAction;
  double ProgressGlobalFractionCompleted;
  double ProgressCurrentFractionCompleted;
};

#endif

