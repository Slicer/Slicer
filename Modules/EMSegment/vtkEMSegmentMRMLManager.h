#ifndef __vtkEMSegmentMRMLManager_h
#define __vtkEMSegmentMRMLManager_h

#include "vtkSlicerModuleLogic.h"
#include "vtkEMSegment.h"
#include <vtkSetGet.h>

class vtkMRMLEMSNode;
class vtkMRMLEMSGlobalParametersNode;
class vtkMRMLEMSSegmenterNode;
class vtkMRMLEMSTemplateNode;
class vtkMRMLEMSTargetNode;
class vtkMRMLEMSAtlasNode;
class vtkMRMLEMSTreeNode;
class vtkMRMLEMSTreeParametersNode;
class vtkMRMLEMSTreeParametersParentNode;
//class vtkMRMLEMSTreeParametersLeafNode;
class vtkMRMLEMSWorkingDataNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLVolumeNode;

// need enum values
#include "MRML/vtkMRMLEMSTreeParametersLeafNode.h"
#include "MRML/vtkMRMLEMSClassInteractionMatrixNode.h"

class vtkMRMLScene;

#include <vtksys/stl/string>
#include <vtksys/stl/map>
#include <vtksys/stl/vector>

class VTK_EMSEGMENT_EXPORT vtkEMSegmentMRMLManager : 
  public vtkObject
{
public:
  static vtkEMSegmentMRMLManager *New();
  vtkTypeMacro(vtkEMSegmentMRMLManager,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // the current mrml scene
  vtkSetObjectMacro(MRMLScene, vtkMRMLScene);
  vtkGetObjectMacro(MRMLScene, vtkMRMLScene);

  // Get/Set MRML node storing parameter values
  virtual void SetNode(vtkMRMLEMSNode*);
  vtkGetObjectMacro(Node, vtkMRMLEMSNode);

  // this will be be passed along by the logic node 
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event,
                                   void *callData );

  // 
  // copy all nodes relating to the EMSegmenter into newScene
  virtual bool PackageAndWriteData(const char* packageDirectoryName);

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

  virtual int      GetTreeNodeIntensityLabel(vtkIdType nodeID);
  virtual void     SetTreeNodeIntensityLabel(vtkIdType nodeID, int label);

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
    DistributionSpecificationManual = 
    vtkMRMLEMSTreeParametersLeafNode::DistributionSpecificationManual,
    DistributionSpecificationManuallySample = 
    vtkMRMLEMSTreeParametersLeafNode::DistributionSpecificationManuallySample,
    DistributionSpecificationAutoSample =
    vtkMRMLEMSTreeParametersLeafNode::DistributionSpecificationAutoSample
    };
  //ETX
  virtual int   GetTreeNodeDistributionSpecificationMethod(vtkIdType nodeID);
  virtual void  SetTreeNodeDistributionSpecificationMethod(vtkIdType nodeID, 
                                                           int method);
  virtual void  ChangeTreeNodeDistributionsFromManualSamplingToManual();

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

  virtual void     UpdateIntensityDistributions();

  // Step 5

  virtual int      GetTreeNodePrintWeight(vtkIdType nodeID);
  virtual void     SetTreeNodePrintWeight(vtkIdType nodeID, int shouldPrint);

  virtual int      GetTreeNodePrintQuality(vtkIdType nodeID);
  virtual void     SetTreeNodePrintQuality(vtkIdType nodeID, int shouldPrint);

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

  //BTX
  enum 
    {
      DirectionWest  = vtkMRMLEMSClassInteractionMatrixNode::DirectionWest, 
      DirectionNorth = vtkMRMLEMSClassInteractionMatrixNode::DirectionNorth,
      DirectionUp    = vtkMRMLEMSClassInteractionMatrixNode::DirectionUp, 
      DirectionEast  = vtkMRMLEMSClassInteractionMatrixNode::DirectionEast, 
      DirectionSouth = vtkMRMLEMSClassInteractionMatrixNode::DirectionSouth, 
      DirectionDown  = vtkMRMLEMSClassInteractionMatrixNode::DirectionDown
    };
  //ETX
  virtual double   GetTreeNodeClassInteraction(vtkIdType nodeID, 
                                               int direction,
                                               int row, int column);
  virtual void     SetTreeNodeClassInteraction(vtkIdType nodeID, 
                                               int direction,
                                               int row, int column,
                                               double value);

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
    StoppingConditionIterations = 0,
    StoppingConditionLabelMapMeasure = 1,
    StoppingConditionWeightsMeasure = 2
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
  virtual int         GetTargetNumberOfSelectedVolumes();
  // index in [0, #selected volumes)
  virtual vtkIdType   GetTargetSelectedVolumeNthID(int n); 
  virtual const char* GetTargetSelectedVolumeNthMRMLID(int n); 
  //BTX
  virtual void
    ResetTargetSelectedVolumes(const std::vector<vtkIdType>& volumeID);
  //ETX
  virtual void        AddTargetSelectedVolume(vtkIdType volumeID);
  virtual void        AddTargetSelectedVolumeByMRMLID(char* mrmlID);
  virtual void        RemoveTargetSelectedVolume(vtkIdType volumeID);
  virtual void        MoveNthTargetSelectedVolume(int fromIndex,
                                                  int toIndex);
  virtual void        MoveTargetSelectedVolume(vtkIdType volumeID,
                                               int toIndex);

  //
  // target volume normalization
  virtual void  
    SetNthTargetVolumeIntensityNormalizationToDefaultT1SPGR(int n);
  virtual void  
    SetTargetVolumeIntensityNormalizationToDefaultT1SPGR(vtkIdType volumeID);
  virtual void  
    SetNthTargetVolumeIntensityNormalizationToDefaultT2(int n);
  virtual void  
    SetTargetVolumeIntensityNormalizationToDefaultT2(vtkIdType volumeID);
  virtual void  
    SetNthTargetVolumeIntensityNormalizationToDefaultT2_2(int n);
  virtual void  
    SetTargetVolumeIntensityNormalizationToDefaultT2_2(vtkIdType volumeID);

  virtual void 
    SetNthTargetVolumeIntensityNormalizationNormValue(int n, double d);
  virtual void 
    SetTargetVolumeIntensityNormalizationNormValue(vtkIdType volumeID, 
                                                   double d);
  virtual double
    GetNthTargetVolumeIntensityNormalizationNormValue(int n);
  virtual double
    GetTargetVolumeIntensityNormalizationNormValue(vtkIdType volumeID);

  virtual void 
    SetNthTargetVolumeIntensityNormalizationNormType(int n, int i);
  virtual void 
    SetTargetVolumeIntensityNormalizationNormType(vtkIdType volumeID, int i);
  virtual int
    GetNthTargetVolumeIntensityNormalizationNormType(int n);
  virtual int
    GetTargetVolumeIntensityNormalizationNormType(vtkIdType volumeID);

  virtual void 
    SetNthTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(int n, int i);
  virtual void 
    SetTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(vtkIdType volumeID, int i);
  virtual int 
    GetNthTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(int n);
  virtual int 
    GetTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(vtkIdType volumeID);

  virtual void 
    SetNthTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(int n, int i);
  virtual void 
    SetTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(vtkIdType volumeID, int i);
  virtual int 
    GetNthTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(int n);
  virtual int 
    GetTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(vtkIdType volumeID);

  virtual void 
    SetNthTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(int n, float f);
  virtual void 
    SetTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(vtkIdType volumeID, float f);
  virtual float
    GetNthTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(int n);
  virtual float
    GetTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(vtkIdType volumeID);

  virtual void 
    SetNthTargetVolumeIntensityNormalizationPrintInfo(int n, int i);
  virtual void 
    SetTargetVolumeIntensityNormalizationPrintInfo(vtkIdType volumeID, int i);
  virtual int 
    GetNthTargetVolumeIntensityNormalizationPrintInfo(int n);
  virtual int 
    GetTargetVolumeIntensityNormalizationPrintInfo(vtkIdType volumeID);

  virtual void 
    SetNthTargetVolumeIntensityNormalizationEnabled(int n, int i);
  virtual void 
    SetTargetVolumeIntensityNormalizationEnabled(vtkIdType volumeID, int i);
  virtual int 
    GetNthTargetVolumeIntensityNormalizationEnabled(int n);
  virtual int 
    GetTargetVolumeIntensityNormalizationEnabled(vtkIdType volumeID);

  //
  // registration parameters
  //

  //BTX
  enum
    {
    AtlasToTargetAffineRegistrationOff          = 0,
    AtlasToTargetAffineRegistrationCenters      = 1,
    AtlasToTargetAffineRegistrationRigidMMI     = 2,
    AtlasToTargetAffineRegistrationRigidNCC     = 3,
    AtlasToTargetAffineRegistrationRigidMMIFast = 4,
    AtlasToTargetAffineRegistrationRigidNCCFast = 5,
    AtlasToTargetAffineRegistrationRigidMMISlow = 6,
    AtlasToTargetAffineRegistrationRigidNCCSlow = 7
    };
  //ETX
  virtual int       GetRegistrationAffineType();
  virtual void      SetRegistrationAffineType(int affineType);

  //BTX
  enum
    {
    AtlasToTargetDeformableRegistrationOff            = 0,
    AtlasToTargetDeformableRegistrationBSplineMMI     = 1,
    AtlasToTargetDeformableRegistrationBSplineNCC     = 2,
    AtlasToTargetDeformableRegistrationBSplineMMIFast = 3,
    AtlasToTargetDeformableRegistrationBSplineNCCFast = 4,
    AtlasToTargetDeformableRegistrationBSplineMMISlow = 5,
    AtlasToTargetDeformableRegistrationBSplineNCCSlow = 6
    };
  //ETX
  virtual int       GetRegistrationDeformableType();
  virtual void      SetRegistrationDeformableType(int deformableType);

  virtual int       GetEnableTargetToTargetRegistration();
  virtual void      SetEnableTargetToTargetRegistration(int enable);

  //BTX
  enum
    {
    InterpolationLinear = 0,
    InterpolationNearestNeighbor = 1,
    // !!!todo!!! there is no corresponding definition in the algorithm!
    InterpolationCubic = 2
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
  virtual void         SetOutputVolumeID(vtkIdType volumeID);

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

  // Return if we have a global parameters node
  virtual int HasGlobalParametersNode();

  virtual void PrintTree();
  virtual void PrintTree(vtkIdType rootID, vtkIndent indent);

  virtual void PrintVolumeInfo();

  //
  // convienince functions for managing MRML nodes
  //
  virtual vtkMRMLEMSTemplateNode*         GetTemplateNode();
  virtual vtkMRMLEMSTargetNode*           GetTargetNode();
  virtual vtkMRMLEMSAtlasNode*            GetAtlasNode();
  virtual vtkMRMLScalarVolumeNode*        GetOutputVolumeNode();
  virtual vtkMRMLEMSGlobalParametersNode* GetGlobalParametersNode();
  virtual vtkMRMLEMSTreeNode*             GetTreeRootNode();
  virtual vtkMRMLEMSTreeNode*             GetTreeNode(vtkIdType);
  virtual vtkMRMLEMSTreeParametersNode*   GetTreeParametersNode(vtkIdType);  
  virtual vtkMRMLEMSTreeParametersLeafNode* 
    GetTreeParametersLeafNode(vtkIdType);  
  virtual vtkMRMLEMSTreeParametersParentNode* 
    GetTreeParametersParentNode(vtkIdType);  
  virtual vtkMRMLEMSClassInteractionMatrixNode* 
    GetTreeClassInteractionNode(vtkIdType);  
  virtual vtkMRMLEMSNode*                 GetEMSNode();
  virtual vtkMRMLEMSSegmenterNode*        GetSegmenterNode();
  virtual vtkMRMLVolumeNode*              GetVolumeNode(vtkIdType);
  virtual vtkMRMLEMSWorkingDataNode*      GetWorkingDataNode();

  virtual vtkMRMLEMSTargetNode* CloneTargetNode(vtkMRMLEMSTargetNode* target,
                                                const char* name);

  virtual vtkMRMLEMSAtlasNode*  CloneAtlasNode(vtkMRMLEMSAtlasNode* target,
                                               const char* name);

private:
  vtkEMSegmentMRMLManager();
  ~vtkEMSegmentMRMLManager();
  vtkEMSegmentMRMLManager(const vtkEMSegmentMRMLManager&);
  void operator=(const vtkEMSegmentMRMLManager&);

  virtual vtkIdType                       AddNewTreeNode();
  virtual vtkIdType                       GetNewVTKNodeID();

  virtual void           RemoveTreeNodeParametersNodes(vtkIdType nodeID);

  virtual void           PropogateAdditionOfSelectedTargetImage();
  virtual void           PropogateRemovalOfSelectedTargetImage(int index);
  virtual void           PropogateMovementOfSelectedTargetImage(int fromIndex,
                                                                int toIndex);

  //
  // functions for packaging and writing intermediate results
  virtual void CopyEMRelatedNodesToMRMLScene(vtkMRMLScene* newScene);
  virtual void CreatePackageFilenames(vtkMRMLScene* scene, 
                                      const char* packageDirectoryName);
  virtual bool CreatePackageDirectories(const char* packageDirectoryName);
  virtual bool WritePackagedScene(vtkMRMLScene* scene);

  // Update intensity statistics for a particular tissue type.
  virtual void      UpdateIntensityDistributionFromSample(vtkIdType nodeID);

  //BTX
  virtual void           GetListOfTreeNodeIDs(vtkIdType rootNodeID, 
                                              vtkstd::vector<vtkIdType>& list);
  //ETX

  //
  // convienience functions for managing ID mapping (mrml id <-> vtkIdType)
  //
  virtual vtkIdType    MapMRMLNodeIDToVTKNodeID(const char* MRMLNodeID);
  virtual const char*  MapVTKNodeIDToMRMLNodeID(vtkIdType vtkID);

  virtual void         IDMapInsertPair(vtkIdType vtkID, 
                                       const char* MRMLNodeID);
  virtual void         IDMapRemovePair(vtkIdType vtkID);
  virtual void         IDMapRemovePair(const char* MRMLNodeID);

  virtual int          IDMapContainsMRMLNodeID(const char* MRMLNodeID);
  virtual int          IDMapContainsVTKNodeID(vtkIdType id);

  virtual void         UpdateMapsFromMRML();

  virtual int          GetTargetVolumeIndex(vtkIdType vtkID);

  // the current mrml scene
  vtkMRMLScene*   MRMLScene;

  //
  // parameters node that is currently under consideration
  //
  vtkMRMLEMSNode* Node;
  
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
};

#endif
