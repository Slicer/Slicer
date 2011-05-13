#ifndef __vtkMRMLVolumeRenderingDisplayableManager_h
#define __vtkMRMLVolumeRenderingDisplayableManager_h

#include "vtkSlicerModuleLogic.h"
#include "vtkVolumeMapper.h"
#include "vtkVolume.h"

#include <vtkMRMLAbstractThreeDViewDisplayableManager.h>

#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkMRMLVolumeRenderingScenarioNode.h"
#include "vtkSlicerVolumeRenderingLogic.h"

#include "qSlicerVolumeRenderingModuleExport.h"

class vtkSlicerVolumeTextureMapper3D;
class vtkSlicerFixedPointVolumeRayCastMapper;
class vtkSlicerGPURayCastVolumeMapper;
class vtkSlicerGPURayCastMultiVolumeMapper;
class vtkGPUVolumeRayCastMapper;

class vtkTimerLog;
class vtkMatrix4x4;
class vtkPlanes;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT vtkMRMLVolumeRenderingDisplayableManager  :
  public vtkMRMLAbstractThreeDViewDisplayableManager
{
public:
  static vtkMRMLVolumeRenderingDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLVolumeRenderingDisplayableManager, vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  //virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
    //                               void *callData ){};


  void Reset();

  void SetGUICallbackCommand(vtkCommand* callback);

  virtual void Create();
  
  // Description:
  // Get, Set and Observe DisplayNode
  vtkGetObjectMacro (DisplayNode, vtkMRMLVolumeRenderingDisplayNode);
  void SetAndObserveDisplayNode(vtkMRMLVolumeRenderingDisplayNode* vspNode);


  // Description:
  // setup mapper based on current parameters node
  // return values:
  // -1: requested mapper not supported
  //  0: invalid input parameter
  //  1: success
  int SetupMapperFromParametersNode(vtkMRMLVolumeRenderingDisplayNode* vspNode);


  // Description:
  // Update MRML events
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                  unsigned long /*event*/,
                                  void * /*callData*/ );

  virtual void UpdateFromMRML();

  // Description:
  // Get Volume Actor
  vtkVolume* GetVolumeActor(){return this->Volume;}

  void SetupHistograms(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  //vtkKWHistogramSet* GetHistogramSet(){return this->Histograms;}

  void SetupHistogramsFg(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  //vtkKWHistogramSet* GetHistogramSetFg(){return this->HistogramsFg;}

  void SetExpectedFPS(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  void SetGPUMemorySize(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  void SetCPURaycastParameters(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  void SetGPURaycastParameters(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  void SetGPURaycastIIParameters(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  void SetGPURaycast3Parameters(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  void SetROI(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  void CreateVolumePropertyGPURaycastII(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  void UpdateVolumePropertyGPURaycastII(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  //void CreateVolumePropertyGPURaycast3(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  //void UpdateVolumePropertyGPURaycast3(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  void TransformModified(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  void SetVolumeVisibility(int isVisible);

  /* return values:
   * 0: cpu ray cast not used
   * 1: success
   */
  int SetupVolumeRenderingInteractive(vtkMRMLVolumeRenderingDisplayNode* vspNode, int buttonDown);

  /* return values:
   * 0: mapper not supported
   * 1: mapper supported
   */
  int IsCurrentMapperSupported(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  virtual void OnMRMLSceneAboutToBeClosedEvent();
  virtual void OnMRMLSceneClosedEvent();
  virtual void OnMRMLSceneImportedEvent();
  virtual void OnMRMLSceneRestoredEvent();
  virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node);

  void RemoveMRMLObservers();

  void UpdateDisplayNodeObservers();

  void RemoveDisplayNodeObservers();

protected:
  vtkMRMLVolumeRenderingDisplayableManager();
  ~vtkMRMLVolumeRenderingDisplayableManager();
  vtkMRMLVolumeRenderingDisplayableManager(const vtkMRMLVolumeRenderingDisplayableManager&);
  void operator=(const vtkMRMLVolumeRenderingDisplayableManager&);

  //virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);

  //virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node);

  void OnCreate();

  static bool First;

  vtkSlicerVolumeRenderingLogic *VolumeRenderingLogic;
  // Description:
  // The hardware accelerated texture mapper.
  vtkSlicerVolumeTextureMapper3D *MapperTexture;

  // Description:
  // The hardware accelerated gpu ray cast mapper.
  vtkSlicerGPURayCastVolumeMapper *MapperGPURaycast;

  // Description:
  // The hardware accelerated multi-volume gpu ray cast mapper.
  vtkSlicerGPURayCastMultiVolumeMapper *MapperGPURaycastII;

  // Description:
  // The software accelerated software mapper
  vtkSlicerFixedPointVolumeRayCastMapper *MapperRaycast;

  // Description:
  // The gpu ray cast mapper.
  vtkGPUVolumeRayCastMapper *MapperGPURaycast3;

  // Description:
  // Actor used for Volume Rendering
  vtkVolume *Volume;

  // Description:
  // internal histogram instance (bg)
  //vtkKWHistogramSet *Histograms;

  // Description:
  // internal histogram instance (fg)
  //vtkKWHistogramSet *HistogramsFg;

  /// Holders for MRML callbacks
  //vtkCallbackCommand *MRMLCallback;


  vtkVolumeProperty *VolumePropertyGPURaycastII;

  //vtkVolumeProperty *VolumePropertyGPURaycast3;

  vtkMRMLVolumeRenderingDisplayNode*    DisplayNode;

  int SceneIsLoadingFlag;
  int ProcessingMRMLFlag;
  int UpdatingFromMRML;

  std::map<std::string, vtkMRMLVolumeRenderingDisplayNode *>      DisplayNodes;


protected:
  void OnScenarioNodeModified();
  void OnVolumeRenderingDisplayNodeModified();


  void ComputeInternalVolumeSize(int index);
  void CalculateMatrix(vtkMRMLVolumeRenderingDisplayNode *vspNode, vtkMatrix4x4 *output);
  void EstimateSampleDistance(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  void RemoveVolumeFromViewers();
  void AddVolumeToViewers();
  void InitializePipelineFromDisplayNode();
  int ValidateDisplayNode(vtkMRMLVolumeRenderingDisplayNode* vspNode);

};

#endif
