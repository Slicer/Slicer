#ifndef __vtkEMSegmentLogic_h
#define __vtkEMSegmentLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkEMSegment.h"
#include "vtkEMSegmentMRMLManager.h"

#include <string>
#include <map>

class vtkImageEMLocalSegmenter;
class vtkImageEMLocalGenericClass;
class vtkImageEMLocalSuperClass;
class vtkImageEMLocalClass;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentLogic : 
  public vtkSlicerModuleLogic
{
public:
  static vtkEMSegmentLogic *New();
  vtkTypeMacro(vtkEMSegmentLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: The name of the Module---this is used to construct
  // the proc invocations
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  //
  // actions
  //
  virtual void      SaveTemplateNow();
  virtual bool      SaveIntermediateResults();

  virtual void      StartPreprocessing();
  virtual void      StartPreprocessingInitializeInputData();
  virtual void      StartPreprocessingTargetIntensityNormalization();
  virtual void      StartPreprocessingTargetToTargetRegistration();
  virtual void      StartPreprocessingAtlasToTargetRegistration();

  virtual void      StartSegmentation();

  // Used within StartSegmentation to copy data from the MRMLManager
  // to the segmenter algorithm.  Possibly useful for research
  // purposes.
  virtual void      CopyDataToSegmenter(vtkImageEMLocalSegmenter* segmenter);

  //
  // progress bar related functions: not currently used, likely to
  // change
  vtkGetStringMacro(ProgressCurrentAction);
  vtkGetMacro(ProgressGlobalFractionCompleted, double);
  vtkGetMacro(ProgressCurrentFractionCompleted, double);

  //
  // MRML Related Methods.  The collection of MRML nodes for the
  // EMSegmenter is complicated.  Therefore, the management of these
  // nodes are delagated to the vtkEMSegmentMRMLManager class.
  vtkGetObjectMacro(MRMLManager, vtkEMSegmentMRMLManager);

  //
  // Register all the nodes used by this module with the current MRML
  // scene.
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

  //
  // special testing functions
  virtual void      PopulateTestingData();
  virtual void      SpecialTestingFunction();

private:
  vtkEMSegmentLogic();
  ~vtkEMSegmentLogic();
  vtkEMSegmentLogic(const vtkEMSegmentLogic&);
  void operator=(const vtkEMSegmentLogic&);

  // the mrml manager is created in the constructor
  vtkSetObjectMacro(MRMLManager, vtkEMSegmentMRMLManager);

  // utility---should probably go to general slicer lib at some point
  static void SlicerImageReslice(vtkMRMLVolumeNode* inputVolumeNode,
                                 vtkMRMLVolumeNode* outputVolumeNode,
                                 vtkMRMLVolumeNode* outputVolumeGeometryNode,
                                 vtkTransform* outputRASToInputRASTransform,
                                 double backgroundLevel);
  //BTX
  template <class T>
  static T GuessRegistrationBackgroundLevel(vtkImageData* imageData);
  //ETX

  static void PrintImageInfo(vtkImageData* image);

  // copy data from MRML to algorithm
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

  //
  // convienience methods for translating enums between algorithm and
  // this module
  virtual int
    ConvertGUIEnumToAlgorithmEnumStoppingConditionType(int guiEnumValue);
  virtual int
    ConvertGUIEnumToAlgorithmEnumInterpolationType(int guiEnumValue);

  // not currently used
  vtkSetStringMacro(ProgressCurrentAction);
  vtkSetMacro(ProgressGlobalFractionCompleted, double);
  vtkSetMacro(ProgressCurrentFractionCompleted, double);

  //
  // because the mrml nodes are very complicated for this module, we
  // delegate the handeling of them to a MRML manager
  vtkEMSegmentMRMLManager* MRMLManager;

  char *ModuleName;

  //
  // information related to progress bars: this mechanism is not
  // currently implemented and might me best implemented elsewhere
  char*  ProgressCurrentAction;
  double ProgressGlobalFractionCompleted;
  double ProgressCurrentFractionCompleted;
};

#endif
