#ifndef __vtkEMSegmentLogic_h
#define __vtkEMSegmentLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkEMSegment.h"
#include "vtkEMSegmentMRMLManager.h"

class vtkImageEMLocalSegmenter;
class vtkImageEMLocalGenericClass;
class vtkImageEMLocalSuperClass;
class vtkImageEMLocalClass;
class vtkKWApplication;

class vtkGridTransform;
class vtkTransform;
class vtkImageData;
class vtkMatrix4x4;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentLogic : 
  public vtkSlicerModuleLogic
{
public:
  static vtkEMSegmentLogic *New();
  vtkTypeMacro(vtkEMSegmentLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Name of the Module
  /// This is used to construct the proc invocations
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  /// Actions
  virtual void      SaveTemplateNow();
  virtual bool      SaveIntermediateResults();

  /// Old Pipeline
  virtual bool      StartPreprocessing();
  virtual bool      StartPreprocessingInitializeInputData();
  virtual bool      StartPreprocessingTargetIntensityNormalization();
  virtual bool      StartPreprocessingTargetToTargetRegistration();
  virtual bool      StartPreprocessingAtlasToTargetRegistration();
  virtual void      StartSegmentation();

  /// New Pipeline
  virtual int       SourceTclFile(vtkKWApplication*app,const char *tclFile);
  virtual int       SourceTaskFiles(vtkKWApplication* app);
  virtual int       SourcePreprocessingTclFiles(vtkKWApplication* app); 
  virtual void      StartSegmentationWithoutPreprocessing();
  int               ComputeIntensityDistributionsFromSpatialPrior(vtkKWApplication* app);


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

  /// Register all the nodes used by this module with the current MRML scene.
  virtual void RegisterMRMLNodesWithScene()
      { 
      this->MRMLManager->RegisterMRMLNodesWithScene(); 
      }

  virtual void SetAndObserveMRMLScene(vtkMRMLScene* scene)
      {
      Superclass::SetAndObserveMRMLScene(scene);
      this->MRMLManager->SetMRMLScene(scene);
      }

  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event,
                                   void *callData )
      { 
      this->MRMLManager->ProcessMRMLEvents(caller, event, callData); 
      }

  ///
  /// Special testing functions
  virtual void      PopulateTestingData();
  virtual void      SpecialTestingFunction();

  /// Events to observe
  virtual vtkIntArray* NewObservableEvents();

  void StartPreprocessingResampleToTarget(vtkMRMLVolumeNode* movingVolumeNode, vtkMRMLVolumeNode* fixedVolumeNode, vtkMRMLVolumeNode* outputVolumeNode);

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


private:
  vtkEMSegmentLogic();
  ~vtkEMSegmentLogic();

  vtkEMSegmentLogic(const vtkEMSegmentLogic&); // Not implemented
  void operator=(const vtkEMSegmentLogic&);    // Not implemented

  /// The mrml manager is created in the constructor
  vtkSetObjectMacro(MRMLManager, vtkEMSegmentMRMLManager);

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

  void UpdateIntensityDistributionAuto(vtkKWApplication* app, vtkIdType nodeID);

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
