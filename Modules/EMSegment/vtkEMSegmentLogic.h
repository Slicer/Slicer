#ifndef __vtkEMSegmentLogic_h
#define __vtkEMSegmentLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkEMSegment.h"

class vtkMRMLEMSNode;
class vtkMRMLEMSGlobalParametersNode;
class vtkMRMLEMSSegmenterNode;
class vtkMRMLEMSTemplateNode;
class vtkMRMLEMSTreeNode;

class vtkImageEMLocalSegmenter;
class vtkImageEMLocalSuperClass;
class vtkImageEMLocalGenericClass;
class vtkImageEMLocalClass;

#include <string>
#include <map>

class VTK_EMSEGMENT_EXPORT vtkEMSegmentLogic : 
  public vtkSlicerModuleLogic
{
public:
  static vtkEMSegmentLogic *New();
  vtkTypeMacro(vtkEMSegmentLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event,
                                   void *callData );

  // Description: Get/Set MRML node storing parameter values
  vtkGetObjectMacro(Node, 
                    vtkMRMLEMSNode);
  virtual void SetAndObserveNode(vtkMRMLEMSNode *n);

  // Description: The name of the Module---this is used to construct
  // the proc invocations
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  //
  // functions for getting and setting the current template builder
  // node (i.e. the set of parameters to edit and use)
  //
  virtual int         GetNumberOfParameterSets();
  virtual const char* GetNthParameterSetName(int n);
  // this functions creates a full set of MRML nodes for this module,
  // populates the nodes with default values, adds the nodes to the
  // MRML scene.
  virtual void        CreateAndObserveNewParameterSet();
  // temporary backwards compatibility
  virtual void        CreateAndObserveNewNodeSet() 
  { this->CreateAndObserveNewParameterSet(); }
  virtual void        SetLoadedParameterSetIndex(int i);

  //
  // functions for manipulating the tree structure
  //
  virtual vtkIdType GetTreeRootNodeID();  
  virtual int       GetTreeNodeIsLeaf(vtkIdType nodeID);
  virtual int       GetTreeNodeNumberOfChildren(vtkIdType nodeID);
  virtual vtkIdType GetTreeNodeChildNodeID(vtkIdType parentNodeID, 
                                           int childIndex);
  virtual vtkIdType GetTreeNodeParentNodeID(vtkIdType childNodeID);
  virtual void      SetTreeNodeParentNodeID(vtkIdType childNodeID, 
                                    vtkIdType newParentNodeID);
  virtual vtkIdType AddTreeNode(vtkIdType parentNodeID);
  virtual void      RemoveTreeNode(vtkIdType removedNodeID);

  //
  // functions for accessing tree-node parameters
  //

  // Step 1
  virtual const char* GetTreeNodeLabel(vtkIdType id);
  virtual void        SetTreeNodeLabel(vtkIdType id, const char* label);

  virtual const char* GetTreeNodeName(vtkIdType id);
  virtual void        SetTreeNodeName(vtkIdType id, const char* label);

  virtual void        GetTreeNodeColor(vtkIdType nodeID, double rgb[3]);
  virtual void        SetTreeNodeColor(vtkIdType nodeID, double rgb[3]);

  // Step 2 see below (volume access)

  // Step 3 does not depend on tree structure

  // Step 4
  
  //BTX
  enum
  {
    DistributionSpecificationManual = 0,
    DistributionSpecificationManuallySample,
    DistributionSpecificationAutoSample
  };
  //ETX
  virtual int   GetTreeNodeDistributionSpecificationMethod(vtkIdType nodeID);
  virtual void  SetTreeNodeDistributionSpecificationMethod(vtkIdType nodeID, 
                                                           int method);

  virtual double   GetTreeNodeDistributionLogMean(vtkIdType nodeID, 
                                                  int volumeNumber);
  virtual void     SetTreeNodeDistributionLogMean(vtkIdType nodeID, 
                                                  int volumeNumber, 
                                                  double value);

  virtual double   GetTreeNodeDistributionLogCovariance(vtkIdType nodeID, 
                                                        int rowIndex,
                                                        int columnIndex);
  virtual void     SetTreeNodeDistributionLogCovariance(vtkIdType nodeID, 
                                                        int rowIndex, 
                                                        int columnIndex,
                                                        double value);

  virtual int      GetTreeNodeDistributionNumberOfSamples(vtkIdType nodeID);

  // send RAS coordinates
  virtual int   AddTreeNodeDistributionSamplePoint(vtkIdType nodeID, 
                                                   double xyz[3]);
  virtual void  RemoveTreeNodeDistributionSamplePoint(vtkIdType nodeID, 
                                                      int samplePointNumber);
  virtual void  RemoveAllTreeNodeDistributionSamplePoints(vtkIdType nodeID);
  virtual void  GetTreeNodeDistributionSamplePoint(vtkIdType nodeID, 
                                                   int tupleNumber, 
                                                   double xyz[3]);

  virtual double 
  GetTreeNodeDistributionSampleIntensityValue(vtkIdType nodeID, 
                                              int tupleNumber, 
                                              vtkIdType volumeID);
  // Step 5

  virtual int      GetTreeNodePrintWeight(vtkIdType nodeID);
  virtual void     SetTreeNodePrintWeight(vtkIdType nodeID, int shouldPrint);

  virtual int      GetTreeNodePrintQuality(vtkIdType nodeID);
  virtual void     SetTreeNodePrintQuality(vtkIdType nodeID, int shouldPrint);

  virtual int      GetTreeNodeIntensityLabel(vtkIdType nodeID);
  virtual void     SetTreeNodeIntensityLabel(vtkIdType nodeID, int label);

  virtual int      GetTreeNodePrintFrequency(vtkIdType nodeID);
  virtual void     SetTreeNodePrintFrequency(vtkIdType nodeID, 
                                             int shouldPrint);

  virtual int      GetTreeNodePrintLabelMap(vtkIdType nodeID);
  virtual void     SetTreeNodePrintLabelMap(vtkIdType nodeID, int shouldPrint);

  virtual int      GetTreeNodePrintEMLabelMapConvergence(vtkIdType nodeID);
  virtual void     SetTreeNodePrintEMLabelMapConvergence(vtkIdType nodeID, 
                                                         int shouldPrint);
  
  virtual int      GetTreeNodePrintEMWeightsConvergence(vtkIdType nodeID);
  virtual void     SetTreeNodePrintEMWeightsConvergence(vtkIdType nodeID, 
                                                        int shouldPrint);

  virtual int      GetTreeNodePrintMFALabelMapConvergence(vtkIdType nodeID);
  virtual void     SetTreeNodePrintMFALabelMapConvergence(vtkIdType nodeID, 
                                                          int shouldPrint);

  virtual int      GetTreeNodePrintMFAWeightsConvergence(vtkIdType nodeID);
  virtual void     SetTreeNodePrintMFAWeightsConvergence(vtkIdType nodeID, 
                                                         int shouldPrint);

  virtual int      GetTreeNodeGenerateBackgroundProbability(vtkIdType nodeID);
  virtual void     SetTreeNodeGenerateBackgroundProbability(vtkIdType nodeID, 
                                                            int value);

  virtual int      GetTreeNodeExcludeFromIncompleteEStep(vtkIdType nodeID);
  virtual void     SetTreeNodeExcludeFromIncompleteEStep(vtkIdType nodeID, 
                                                         int shouldExclude);

  virtual double   GetTreeNodeAlpha(vtkIdType nodeID);
  virtual void     SetTreeNodeAlpha(vtkIdType nodeID, double value);
  
  virtual int      GetTreeNodePrintBias(vtkIdType nodeID);
  virtual void     SetTreeNodePrintBias(vtkIdType nodeID, int shouldPrint);

  virtual int      GetTreeNodeBiasCalculationMaxIterations(vtkIdType nodeID);
  virtual void     SetTreeNodeBiasCalculationMaxIterations(vtkIdType nodeID, 
                                                           int value);

  virtual int      GetTreeNodeSmoothingKernelWidth(vtkIdType nodeID);
  virtual void     SetTreeNodeSmoothingKernelWidth(vtkIdType nodeID, 
                                                   int value);

  virtual double   GetTreeNodeSmoothingKernelSigma(vtkIdType nodeID);
  virtual void     SetTreeNodeSmoothingKernelSigma(vtkIdType nodeID, 
                                                   double value);

  virtual double   GetTreeNodeClassProbability(vtkIdType nodeID);
  virtual void     SetTreeNodeClassProbability(vtkIdType nodeID, double value);

  virtual double   GetTreeNodeSpatialPriorWeight(vtkIdType nodeID);
  virtual void     SetTreeNodeSpatialPriorWeight(vtkIdType nodeID, 
                                                 double value);

  virtual double   GetTreeNodeInputChannelWeight(vtkIdType nodeID, 
                                                 int volumeNumber);
  virtual void     SetTreeNodeInputChannelWeight(vtkIdType nodeID, 
                                                 int volumeNumber, 
                                                 double value);

  virtual int      GetTreeNodeStoppingConditionEMType(vtkIdType nodeID);
  virtual void     SetTreeNodeStoppingConditionEMType(vtkIdType nodeID, 
                                                      int conditionType);
  
  virtual double   GetTreeNodeStoppingConditionEMValue(vtkIdType nodeID);
  virtual void     SetTreeNodeStoppingConditionEMValue(vtkIdType nodeID, 
                                                       double value);
  
  virtual int      GetTreeNodeStoppingConditionEMIterations(vtkIdType nodeID);
  virtual void     SetTreeNodeStoppingConditionEMIterations(vtkIdType nodeID,
                                                            int iterations);

  //BTX
  enum
  {
    StoppingConditionIterations,
    StoppingConditionLabelMapMeasure,
    StoppingConditionWeightsMeasure
  };
  //ETX
  virtual int      GetTreeNodeStoppingConditionMFAType(vtkIdType nodeID);
  virtual void     SetTreeNodeStoppingConditionMFAType(vtkIdType nodeID, 
                                                       int conditionType);
  
  virtual double   GetTreeNodeStoppingConditionMFAValue(vtkIdType nodeID);
  virtual void     SetTreeNodeStoppingConditionMFAValue(vtkIdType nodeID, 
                                                        double value);
  
  virtual int      GetTreeNodeStoppingConditionMFAIterations(vtkIdType nodeID);
  virtual void     SetTreeNodeStoppingConditionMFAIterations(vtkIdType nodeID,
                                                             int iterations);

  // Step 6 does not depend on tree structure

  // Step 7 does not depend on tree structure

  //
  // functions for accessing volumes
  //
  virtual int       GetVolumeNumberOfChoices();
  virtual vtkIdType GetVolumeNthID(int n);
  virtual const char* GetVolumeName(vtkIdType volumeID);

  // spatial prior volumes
  virtual vtkIdType GetTreeNodeSpatialPriorVolumeID(vtkIdType nodeID);
  virtual void      SetTreeNodeSpatialPriorVolumeID(vtkIdType nodeID, 
                                                    vtkIdType volumeID);

  // target volumes
  virtual int       GetTargetNumberOfSelectedVolumes();
  // index in [0, #selected volumes)
  virtual vtkIdType GetTargetSelectedVolumeNthID(int n); 
  virtual void      AddTargetSelectedVolume(vtkIdType volumeID);
  virtual void      RemoveTargetSelectedVolume(vtkIdType volumeID);

  //
  // registration parameters
  //
  virtual int       GetRegistrationAffineType();
  virtual void      SetRegistrationAffineType(int affineType);

  virtual int       GetRegistrationDeformableType();
  virtual void      SetRegistrationDeformableType(int deformableType);

  //BTX
  enum
  {
    InterpolationLinear,
    InterpolationNearestNeighbor,
    // !!!todo!!! there is no corresponding definition in the algorithm!
    InterpolationCubic
  };
  //ETX
  virtual int       GetRegistrationInterpolationType();
  virtual void      SetRegistrationInterpolationType(int interpolationType);

  virtual vtkIdType GetRegistrationAtlasVolumeID();
  virtual void      SetRegistrationAtlasVolumeID(vtkIdType volumeID);

  virtual vtkIdType GetRegistrationTargetVolumeID();
  virtual void      SetRegistrationTargetVolumeID(vtkIdType volumeID);

  //
  // save parameters
  //
  virtual const char*  GetSaveWorkingDirectory();
  virtual void         SetSaveWorkingDirectory(const char* directory);

  virtual const char*  GetSaveTemplateFilename();
  virtual void         SetSaveTemplateFilename(const char* file);

  virtual int          GetSaveTemplateAfterSegmentation();
  virtual void         SetSaveTemplateAfterSegmentation(int shouldSave);

  virtual int          GetSaveIntermediateResults();
  virtual void         SetSaveIntermediateResults(int shouldSaveResults);

  virtual int          GetSaveSurfaceModels();
  virtual void         SetSaveSurfaceModels(int shouldSaveModels);

  virtual const char*  GetOutputVolumeMRMLID();
  virtual void         SetOutputVolumeMRMLID(const char* mrmlID);

  //
  // actions
  //
  virtual void      SaveTemplateNow();
  virtual void      StartSegmentation();

  //
  // progress bar related functions: not currently used, likely to
  // change
  //
  vtkGetStringMacro(ProgressCurrentAction);
  vtkGetMacro(ProgressGlobalFractionCompleted, double);
  vtkGetMacro(ProgressCurrentFractionCompleted, double);

  //
  // miscellaneous
  //
  virtual int       GetEnableMultithreading();
  virtual void      SetEnableMultithreading(int isEnabled);

  virtual int       GetAtlasNumberOfTrainingSamples();

  virtual void      GetSegmentationBoundaryMin(int minPoint[3]);
  virtual void      SetSegmentationBoundaryMin(int minPoint[3]);

  virtual void      GetSegmentationBoundaryMax(int maxPoint[3]);
  virtual void      SetSegmentationBoundaryMax(int maxPoint[3]);

  virtual int       CheckMRMLNodeStructure();

  //
  // this functions registers all of the MRML nodes needed by this
  // class with the MRML scene
  //
  virtual void      RegisterMRMLNodesWithScene();

  //
  // special testing functions
  //
  virtual void      PopulateTestingData();
  virtual void      SpecialTestingFunction();

  // Return if we have a global parameters node
  virtual int HasGlobalParametersNode();

  virtual void PrintTree(vtkIdType rootID, vtkIndent indent);

private:
  vtkEMSegmentLogic();
  ~vtkEMSegmentLogic();
  vtkEMSegmentLogic(const vtkEMSegmentLogic&);
  void operator=(const vtkEMSegmentLogic&);

  vtkSetStringMacro(ProgressCurrentAction);
  vtkSetMacro(ProgressGlobalFractionCompleted, double);
  vtkSetMacro(ProgressCurrentFractionCompleted, double);

  //
  // convienince functions for managing MRML nodes
  //
  virtual vtkMRMLEMSSegmenterNode*        GetSegmenterNode();
  virtual vtkMRMLEMSTemplateNode*         GetTemplateNode();
  virtual vtkMRMLEMSGlobalParametersNode* GetGlobalParametersNode();
  virtual vtkMRMLEMSTreeNode*             GetTreeRootNode();
  virtual vtkMRMLEMSTreeNode*             GetTreeNode(vtkIdType);

  virtual vtkIdType                       AddNewTreeNode();
  virtual vtkIdType                       GetNewVTKNodeID();

  virtual void           RemoveTreeNodeParametersNodes(vtkIdType nodeID);

  virtual vtkMRMLVolumeNode* GetVolumeNode(vtkIdType);

  virtual void           PropogateChangeInNumberOfSelectedTargetImages();
  virtual void           UpdateMeanAndCovarianceFromSample(vtkIdType nodeID);

  //BTX
  virtual void           GetListOfTreeNodeIDs(vtkIdType rootNodeID, 
                                              vtkstd::vector<vtkIdType>& list);
  //ETX

  //
  // convienience functions for managing ID mapping (mrml id <-> vtkIdType)
  //
  virtual vtkIdType          MapMRMLNodeIDToVTKNodeID(const char* MRMLNodeID);
  virtual const char*        MapVTKNodeIDToMRMLNodeID(vtkIdType vtkID);

  virtual void               IDMapInsertPair(vtkIdType vtkID, 
                                             const char* MRMLNodeID);
  virtual void               IDMapRemovePair(vtkIdType vtkID);
  virtual void               IDMapRemovePair(const char* MRMLNodeID);

  virtual int                IDMapContainsMRMLNodeID(const char* MRMLNodeID);
  virtual int                IDMapContainsVTKNodeID(vtkIdType id);

  virtual void               UpdateMapsFromMRML();

  //
  // convienince methods for running algorithm
  //
  virtual void CopyDataToSegmenter(vtkImageEMLocalSegmenter* segmenter);
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
  // convienience methods for translating enums
  //
  virtual int 
  ConvertGUIEnumToAlgorithmEnumStoppingConditionType(int guiEnumValue);
  virtual int 
  ConvertAlgorithmEnumToGUIEnumStoppingConditionType(int algEnumValue);
  virtual int 
  ConvertGUIEnumToAlgorithmEnumInterpolationType(int guiEnumValue);
  virtual int 
  ConvertAlgorithmEnumToGUIEnumInterpolationType(int algEnumValue);
  
  //
  // parameters node that is currently under consideration
  //
  vtkMRMLEMSNode* Node;
  
  char *ModuleName;

  //
  // information related to progress bars: this mechanism is not
  // currently implemented and might me best implemented elsewhere
  //
  char*  ProgressCurrentAction;
  double ProgressGlobalFractionCompleted;
  double ProgressCurrentFractionCompleted;

  // global switch to hide EM segment parameters from MRML tree
  // editors
  bool   HideNodesFromEditors;

  //
  // The api of this class exposes vtkIdType ids for tree nodes and
  // volumes.  This esentially hides the mrml ids from client code and
  // insulates the client from changes in the slicer mrml id
  // mechanism.
  //

  vtkIdType NextVTKNodeID;
  //BTX
  typedef vtksys_stl::map<vtkIdType, vtksys_stl::string>  VTKToMRMLMapType;
  VTKToMRMLMapType                                VTKNodeIDToMRMLNodeIDMap;
  typedef vtksys_stl::map<vtksys_stl::string, vtkIdType>  MRMLToVTKMapType;
  MRMLToVTKMapType                                MRMLNodeIDToVTKNodeIDMap;
  //ETX

  //
  // Information related to anatomical tree nodes that is not stored
  // in mrml nodes.  This is the distribution specification method and
  // possibly a list of points from which to sample the images to
  // determine intensity distributions.  This might better be stored
  // in the mrml scene?
  //
  //BTX
  typedef vtksys_stl::vector<vtksys_stl::vector<double> > SamplePointsList;
  struct TreeInfo {
    int                DistributionSpecificationMethod;
    SamplePointsList   SamplePoints;

    TreeInfo() {
      DistributionSpecificationMethod = 
        vtkEMSegmentLogic::DistributionSpecificationManual;
    }
  };
  typedef vtksys_stl::map<vtkIdType, TreeInfo>        TreeInfoMapType;
  TreeInfoMapType                                     TreeInfoMap;  
  //ETX
};

#endif

