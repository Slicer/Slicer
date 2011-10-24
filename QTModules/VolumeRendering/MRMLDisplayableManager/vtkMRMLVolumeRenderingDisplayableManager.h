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
class vtkFixedPointVolumeRayCastMapper;
class vtkSlicerGPURayCastVolumeMapper;
class vtkSlicerGPURayCastMultiVolumeMapper;
class vtkGPUVolumeRayCastMapper;

class vtkIntArray;
class vtkMatrix4x4;
class vtkPlanes;
class vtkTimerLog;

#define VTKIS_VOLUME_PROPS 100

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT vtkMRMLVolumeRenderingDisplayableManager  :
  public vtkMRMLAbstractThreeDViewDisplayableManager
{
public:
  static vtkMRMLVolumeRenderingDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLVolumeRenderingDisplayableManager, vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  void Reset();

  void SetGUICallbackCommand(vtkCommand* callback);

  virtual void Create();
  
  // Description:
  // Get, Set and Observe DisplayNode
  //vtkGetObjectMacro (DisplayNode, vtkMRMLVolumeRenderingDisplayNode);
  //void SetAndObserveDisplayNode(vtkMRMLVolumeRenderingDisplayNode* vspNode);


  // Description:
  // setup mapper based on current parameters node
  // return values:
  // -1: requested mapper not supported
  //  0: invalid input parameter
  //  1: success
  int SetupMapperFromParametersNode(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  // Description:
  // Get Volume Actor
  vtkVolume* GetVolumeActor(){return this->Volume;}

  void SetupHistograms(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  //vtkKWHistogramSet* GetHistogramSet(){return this->Histograms;}

  void SetupHistogramsFg(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  //vtkKWHistogramSet* GetHistogramSetFg(){return this->HistogramsFg;}

  void SetExpectedFPS(double fps);
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

  //void SetupVolumeRenderingInteractive(vtkMRMLVolumeRenderingDisplayNode* vspNode, int buttonDown);

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

  void UpdateDisplayNodeList();

  void AddDisplayNode(vtkMRMLVolumeRenderingDisplayNode* dnode);
  void RemoveDisplayNode(vtkMRMLVolumeRenderingDisplayNode* dnode);
  void RemoveDisplayNodes();

  static int DefaultGPUMemorySize;

protected:
  vtkMRMLVolumeRenderingDisplayableManager();
  ~vtkMRMLVolumeRenderingDisplayableManager();
  vtkMRMLVolumeRenderingDisplayableManager(const vtkMRMLVolumeRenderingDisplayableManager&);
  void operator=(const vtkMRMLVolumeRenderingDisplayableManager&);


  // Description:
  // Don't support nested event processing
  // TODO: Probably a bad idea to not support nested calls
  virtual bool EnterMRMLNodesCallback()const;

  // Description:
  // Update MRML events
  virtual void ProcessMRMLNodesEvents(vtkObject * caller,
                                 unsigned long event,
                                 void * callData);

  virtual void OnInteractorStyleEvent(int eventId);

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
  vtkFixedPointVolumeRayCastMapper *MapperRaycast;

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

  vtkMRMLVolumeRenderingDisplayNode*    DisplayedNode;

  typedef std::map<std::string, vtkMRMLVolumeRenderingDisplayNode *> DisplayNodesType;
  DisplayNodesType      DisplayNodes;

  vtkIntArray* DisplayObservedEvents;
  // When interaction is >0, we are in interactive mode (low LOD)
  int Interaction;

protected:
  void OnScenarioNodeModified();
  void OnVolumeRenderingDisplayNodeModified(vtkMRMLVolumeRenderingDisplayNode* dnode);


  void ComputeInternalVolumeSize(int index);
  void CalculateMatrix(vtkMRMLVolumeRenderingDisplayNode *vspNode, vtkMatrix4x4 *output);
  void EstimateSampleDistance(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  /// Return true if the volume wasn't in the view.
  bool AddVolumeToView();
  void RemoveVolumeFromView();
  void RemoveVolumeFromView(vtkVolume* volume);
  void InitializePipelineFromDisplayNode(vtkMRMLVolumeRenderingDisplayNode* vspNode);
  int ValidateDisplayNode(vtkMRMLVolumeRenderingDisplayNode* vspNode);

};

#endif
