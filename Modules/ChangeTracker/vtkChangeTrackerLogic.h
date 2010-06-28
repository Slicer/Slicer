#ifndef __vtkChangeTrackerLogic_h
#define __vtkChangeTrackerLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkChangeTracker.h"
#include "vtkMRMLChangeTrackerNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkTransform.h"

#include <string>

#define ERR_OTHER      -1
#define ERR_GLOBAL_REG -2
#define ERR_LOCAL_REG  -3

class vtkMRMLScene;
class vtkMRMLScalarNode;
class vtkMRMLVolumeNode;
class vtkSlicerApplication;
class vtkKWApplication;
class vtkImageMathematics;
class vtkImageIslandFilter;
class vtkImageMedian3D;
class vtkImageData;
class vtkImageThreshold;
class vtkImageSumOverVoxels;

class VTK_CHANGETRACKER_EXPORT vtkChangeTrackerLogic : 
  public vtkSlicerModuleLogic
{
public:
  static vtkChangeTrackerLogic *New();
  vtkTypeMacro(vtkChangeTrackerLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

 // TODO: do we need to observe MRML here?
 // pohl: I so not I follow example vtkGradnientAnisotrpoicDiffusionoFilterGUI
 // virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
 //                                  void *callData ){};
 // AF: yes, need to observe MRML events to detect completion of registration
  virtual void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData);



  // Description: The name of the Module---this is used to construct
  // the proc invocations
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  vtkGetObjectMacro (ChangeTrackerNode, vtkMRMLChangeTrackerNode);
  void SetAndObserveChangeTrackerNode(vtkMRMLChangeTrackerNode *n) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->ChangeTrackerNode, n);
    }


  //
  // progress bar related functions: not currently used, likely to
  // change
  vtkGetStringMacro(ProgressCurrentAction);
  vtkGetMacro(ProgressGlobalFractionCompleted, double);
  vtkGetMacro(ProgressCurrentFractionCompleted, double);

  void RegisterMRMLNodesWithScene(); 

  // special testing functions
  void DeleteSuperSample(int ScanNum);
  vtkMRMLScalarVolumeNode* CreateSuperSample(int ScanNum);
  double DefineSuperSampleSize(const double inputSpacing[3], const int ROIMin[3], const int ROIMax[3], double resampleConst, int resampleChoice); 

  int CheckROI(vtkMRMLVolumeNode* volumeNode);

  int CreateSuperSampleFct(vtkImageData *input, const int ROIMin[3], const int ROIMax[3], const double SuperSampleSpacing, vtkImageData *output, bool linearInterpolation=1);
  int CreateSuperSampleRASFct(vtkImageData *input, const double ROICenter[3], const double ROIRadius[3], const double SuperSampleSpacing, vtkImageData *output, 
    vtkMatrix4x4* inputRASToIJK, vtkMatrix4x4 *inputXform, vtkMatrix4x4* outputRASToIJK, bool linearInterpolation=1);
  vtkMRMLScalarVolumeNode* CreateVolumeNode(vtkMRMLVolumeNode *volumeNode, const char *name);

  // Main Growth Function 
  int AnalyzeGrowth(vtkSlicerApplication *application);
  void MeassureGrowth(int ThreshMin, int ThreshMax, double &Shrinkage, double &Growth,vtkImageData* segm = NULL);
  void MeassureGrowth(double &Shrinkage, double &Growth);
  void DeleteAnalyzeOutput(vtkSlicerApplication *app);

  vtkImageThreshold*    CreateAnalysis_Intensity_Scan1ByLower();
  vtkImageThreshold*    CreateAnalysis_Intensity_Scan1Range();
  vtkImageThreshold*    CreateAnalysis_Intensity_Scan2ByLower();
  vtkImageThreshold*    CreateAnalysis_Intensity_Scan2Range();

  vtkImageMathematics*  CreateAnalysis_Intensity_ScanSubtract();
  vtkImageMedian3D*     CreateAnalysis_Intensity_ScanSubtractSmooth();
  vtkImageData*         CreateAnalysis_Intensity_ROIGrowth();
  vtkImageData*         CreateAnalysis_Intensity_ROIShrink();

  vtkImageMathematics*  CreateAnalysis_Intensity_ROIGrowthInt();
  vtkImageMathematics*  CreateAnalysis_Intensity_ROIShrinkInt();
  vtkImageThreshold*    CreateAnalysis_Intensity_ROINegativeBin();
  vtkImageThreshold*    CreateAnalysis_Intensity_ROIPositiveBin();
  vtkImageMathematics*  CreateAnalysis_Intensity_ROIBinCombine();
  vtkImageIslandFilter* CreateAnalysis_Intensity_ROINegativeBinReal();
  vtkImageIslandFilter* CreateAnalysis_Intensity_ROIPositiveBinReal();

  vtkImageData   *GetAnalysis_Intensity_ROIBinCombine();

  vtkImageMathematics* CreateAnalysis_Intensity_ROIBinAdd();
  vtkImageThreshold* CreateAnalysis_Intensity_ROIBinDisplay();
  vtkImageData   *GetAnalysis_Intensity_ROIBinDisplay();

  vtkImageSumOverVoxels* CreateAnalysis_Intensity_ROIShrinkVolume();
  vtkImageSumOverVoxels* CreateAnalysis_Intensity_ROIGrowthVolume();

  vtkSetMacro(Analysis_Intensity_Mean,double);
  vtkGetMacro(Analysis_Intensity_Mean,double);
  vtkSetMacro(Analysis_Intensity_Variance,double);
  vtkGetMacro(Analysis_Intensity_Variance,double);
  vtkSetMacro(Analysis_Intensity_Threshold,double);
  vtkGetMacro(Analysis_Intensity_Threshold,double);

  vtkSetMacro(SaveVolumeFlag,int);
  vtkGetMacro(SaveVolumeFlag,int);

  void SaveVolume(vtkSlicerApplication *app, vtkMRMLVolumeNode *volNode);
  // Save the output ignoring SaveVolumeFlag
  void SaveVolumeForce(vtkSlicerApplication *app, vtkMRMLVolumeNode *volNode);

  void SaveVolumeFileName(vtkMRMLVolumeNode *volNode, char* FileName);

  vtkMRMLVolumeNode* LoadVolume(vtkSlicerApplication *app, char* fileName, int LabelMapFlag,const char* volumeName);
  void VolumeWriter(const char* fileName, vtkImageData *Output);

  void PrintResult(ostream& os, vtkSlicerApplication *app);

  void PrintText(char *TEXT);

  void SourceAnalyzeTclScripts(vtkKWApplication *app);

  // This is necessary so that we can call it from the command line module 
  static void DefinePreSegment(vtkImageData *INPUT, const int RANGE[2], vtkImageThreshold *OUTPUT);
  static void DefineSegment(vtkImageData *INPUT, vtkImageIslandFilter *OUTPUT);

  static void LinearResample (vtkMRMLVolumeNode* inputVolumeNode, vtkMRMLVolumeNode* outputVolumeNode, vtkMRMLVolumeNode* outputVolumeGeometryNode,
                 vtkTransform* outputRASToInputRASTransform, double backgroundLevel);


  // AF: do registration using "Linear registration" module
  // For now, use this for global alignment of the input scans. In the future,
  // use the same functionality for ROI
  int DoITKRegistration(vtkSlicerApplication *app);
  int DoITKROIRegistration(vtkSlicerApplication *app);
  int ResampleScan2(vtkSlicerApplication *app);

  char* GetInputScanName(int);

  void SetThresholdsFromSegmentation();
  void GetConnectivityMask(vtkImageData*,vtkImageData*,vtkImageData*,vtkImageData*);

private:
  vtkChangeTrackerLogic();
  ~vtkChangeTrackerLogic();
  vtkChangeTrackerLogic(const vtkChangeTrackerLogic&);
  void operator=(const vtkChangeTrackerLogic&);

  // not currently used
  vtkSetStringMacro(ProgressCurrentAction);
  vtkSetMacro(ProgressGlobalFractionCompleted, double);
  vtkSetMacro(ProgressCurrentFractionCompleted, double);

  //
  // because the mrml nodes are very complicated for this module, we
  // delegate the handeling of them to a MRML manager
  vtkMRMLChangeTrackerNode* ChangeTrackerNode;

  char *ModuleName;

  //
  // information related to progress bars: this mechanism is not
  // currently implemented and might me best implemented elsewhere
  char*  ProgressCurrentAction;
  double ProgressGlobalFractionCompleted;
  double ProgressCurrentFractionCompleted;

  //vtkTransform* GlobalTransform; 
  // vtkTransform* LocalTransform; 

  double Analysis_Intensity_Mean;
  double Analysis_Intensity_Variance;
  double Analysis_Intensity_Threshold;
  vtkImageThreshold*    Analysis_Intensity_Scan1ByLower;
  vtkImageThreshold*    Analysis_Intensity_Scan1Range;
  vtkImageThreshold*    Analysis_Intensity_Scan2ByLower;
  vtkImageThreshold*    Analysis_Intensity_Scan2Range;

  vtkImageMathematics*  Analysis_Intensity_ScanSubtract;
  vtkImageMedian3D*     Analysis_Intensity_ScanSubtractSmooth;
  vtkImageData*         Analysis_Intensity_ROIGrowth;
  vtkImageData*         Analysis_Intensity_ROIShrink;

  vtkImageMathematics   *Analysis_Intensity_ROIGrowthInt;
  vtkImageMathematics   *Analysis_Intensity_ROIShrinkInt;
  vtkImageThreshold     *Analysis_Intensity_ROINegativeBin;
  vtkImageThreshold     *Analysis_Intensity_ROIPositiveBin;
  vtkImageMathematics   *Analysis_Intensity_ROIBinCombine;
  vtkImageIslandFilter  *Analysis_Intensity_ROINegativeBinReal;
  vtkImageIslandFilter  *Analysis_Intensity_ROIPositiveBinReal;
  vtkImageMathematics   *Analysis_Intensity_ROIBinAdd;
  vtkImageThreshold     *Analysis_Intensity_ROIBinDisplay;
  vtkImageSumOverVoxels *Analysis_Intensity_ROIShrinkVolume;
  vtkImageSumOverVoxels *Analysis_Intensity_ROIGrowthVolume;

  int SaveVolumeFlag;

  // AF: store the pointer to the Scan2 registered volume for MRML event handling
  vtkMRMLScalarVolumeNode *Scan2_RegisteredVolume;
  vtkMRMLScalarVolumeNode *Scan2_SuperSampleRegisteredVolume;
};

#endif
