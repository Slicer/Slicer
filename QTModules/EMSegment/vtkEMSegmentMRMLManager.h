#ifndef __vtkEMSegmentMRMLManager_h
#define __vtkEMSegmentMRMLManager_h

#include "vtkEMSegment.h"
#include <vtkSetGet.h>

#define ERROR_NODE_VTKID 0

class vtkMRMLEMSGlobalParametersNode;
class vtkMRMLEMSTemplateNode;
class vtkMRMLEMSAtlasNode;
class vtkMRMLEMSTreeNode;
class vtkMRMLEMSTreeParametersNode;
class vtkMRMLEMSTreeParametersParentNode;
class vtkMRMLEMSWorkingDataNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLVolumeNode;
class vtkMRMLEMSVolumeCollectionNode;
class vtkMRMLStorableNode;

// need enum values
#include "vtkMRMLEMSTreeParametersLeafNode.h"

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

  // Prints out important info about the current template / task 
  void PrintInfo(ostream& os);
  void PrintInfo();

  // Get/Set the current mrml scene
  vtkSetObjectMacro(MRMLScene, vtkMRMLScene);
  vtkGetObjectMacro(MRMLScene, vtkMRMLScene);

  // Get/Set MRML node storing parameter values
  virtual int SetNodeWithCheck(vtkMRMLEMSTemplateNode*);
  vtkGetObjectMacro(Node, vtkMRMLEMSTemplateNode);

  // this will be be passed along by the logic node 
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event,
                                   void *callData );

  int CreateTemplateFile();

  //
  // functions for getting and setting the current template builder
  // node (i.e. the set of parameters to edit and use)
  //
  virtual int         GetNumberOfParameterSets();
  virtual const char* GetNthParameterSetName(int n);
  virtual void SetNthParameterName(int n, const char* newName);

  // this functions creates a full set of MRML nodes for this module,
  // populates the nodes with default values, adds the nodes to the
  // MRML scene.
  virtual void        CreateAndObserveNewParameterSet();
  int         SetLoadedParameterSetIndex(int i);
  int         SetLoadedParameterSetIndex(vtkMRMLEMSTemplateNode* tNode);
  virtual int         CheckEMSTemplateVolumeNodes(vtkMRMLEMSTemplateNode* emsTemplateNode);

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
  int         GetTreeNodeIntensityLabel(vtkIdType nodeID);
  virtual void        SetTreeNodeIntensityLabel(vtkIdType nodeID, int label);

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
  virtual void  SetTreeNodeDistributionLogMean(vtkIdType nodeID, int volumeNumber, double value);

  virtual double   GetTreeNodeDistributionLogMeanWithCorrection(vtkIdType nodeID, int volumeNumber);

  virtual void     SetTreeNodeDistributionLogCovariance(vtkIdType nodeID, 
                                                        int rowIndex, 
                                                        int columnIndex,
                                                        double value);


  virtual double   GetTreeNodeDistributionLogCovarianceWithCorrection(vtkIdType nodeID, int rowIndex, int columnIndex);

  virtual bool     IsTreeNodeDistributionLogCovarianceWithCorrectionInvertableAndSemiDefinite(vtkIdType nodeID);

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
  virtual int       GetTreeNodeInteractionMatrices2DFlag(vtkIdType nodeID);
  virtual void    SetTreeNodeInteractionMatrices2DFlag(vtkIdType nodeID, int value);


  virtual int      GetTreeNodePrintWeight(vtkIdType nodeID);
  virtual void     SetTreeNodePrintWeight(vtkIdType nodeID, int shouldPrint);

  virtual void     PrintWeightOnForEntireTree(); 

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

  virtual double   GetBiasCorrectionType(vtkIdType nodeID);
  virtual void     SetBiasCorrectionType(vtkIdType nodeID,
                                         double value);

  virtual double   GetTreeNodeClassProbability(vtkIdType nodeID);
  virtual void     SetTreeNodeClassProbability(vtkIdType nodeID, double value);

  virtual double   GetTreeNodeChildrenSumClassProbability(vtkIdType nodeID);

  virtual double   GetTreeNodeSpatialPriorWeight(vtkIdType nodeID);
  virtual void     SetTreeNodeSpatialPriorWeight(vtkIdType nodeID, double value);

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
  // functions for checking tree node parameters
  //
  virtual vtkIdType GetTreeNodeFirstIDWithChildProbabilityError();

  //
  // functions for accessing volumes
  //
  virtual int       GetVolumeNumberOfChoices();
  virtual vtkIdType GetVolumeNthID(int n);
  virtual const char* GetVolumeName(vtkIdType volumeID);

  // spatial prior volumes
  virtual vtkIdType GetTreeNodeSpatialPriorVolumeID(vtkIdType nodeID);
  virtual void      SetTreeNodeSpatialPriorVolumeID(vtkIdType nodeID, vtkIdType volumeID);
  vtkMRMLVolumeNode* GetAlignedSpatialPriorFromTreeNodeID(vtkIdType nodeID);
  virtual void SetAlignedSpatialPrior(vtkIdType nodeID, vtkIdType volumeID);

  virtual vtkIdType GetTreeNodeSubParcellationVolumeID(vtkIdType nodeID);
  virtual void      SetTreeNodeSubParcellationVolumeID(vtkIdType nodeID, vtkIdType volumeID);
  vtkMRMLVolumeNode* GetAlignedSubParcellationFromTreeNodeID(vtkIdType nodeID);

  virtual void      SetEnableSubParcellation(int state);
  virtual int       GetEnableSubParcellation();

  virtual void      SetMinimumIslandSize(int value);
  virtual int       GetMinimumIslandSize();

  virtual int       GetIsland2DFlag() ;
  virtual void    SetIsland2DFlag(int) ;


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
  virtual void        RemoveTargetSelectedVolumeIndex(vtkIdType imageIndex);

  virtual void        MoveNthTargetSelectedVolume(int fromIndex,
                                                  int toIndex);
  virtual void        MoveTargetSelectedVolume(vtkIdType volumeID,
                                               int toIndex);

  virtual bool        DoTargetAndAtlasDataTypesMatch( vtkMRMLEMSVolumeCollectionNode* targetNode, vtkMRMLEMSAtlasNode* atlasNode ); 

  //
  // registration parameters
  //

  //BTX
  enum
    {
    RegistrationTest = -1,
    RegistrationOff  = 0,
    RegistrationFast = 1,
    RegistrationSlow = 2
    };
  //ETX
  virtual int       GetRegistrationAffineType();
  virtual void      SetRegistrationAffineType(int affineType);

  int GetRegistrationTypeFromString(const char* type);

  virtual int       GetRegistrationDeformableType();
  virtual void      SetRegistrationDeformableType(int deformableType);

  virtual int       GetEnableTargetToTargetRegistration();
  virtual void      SetEnableTargetToTargetRegistration(int enable);

  virtual const char*  GetColorNodeID();
  virtual void         SetColorNodeID(const char* colormap);

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
  int               GetInterpolationTypeFromString(const char* type);

  //BTX
  enum
    {
    CMTK = 0,
    BRAINS = 1,
    PLASTIMATCH = 2,
    DEMONS = 3,
    DRAMMS = 4,
    ANTS = 5,
    };
  //ETX
  virtual int       GetRegistrationPackageType();
  virtual void      SetRegistrationPackageType(int packageType);
  int               GetPackageTypeFromString(const char* type);

  virtual vtkIdType GetRegistrationAtlasVolumeID();
  virtual void      SetRegistrationAtlasVolumeID(vtkIdType volumeID);

  virtual vtkIdType GetRegistrationAtlasVolumeID(vtkIdType inputID);
  virtual void      SetRegistrationAtlasVolumeID(vtkIdType inputID, vtkIdType volumeID);
  virtual bool      ExistRegistrationAtlasVolumeKey(vtkIdType inputID);

  virtual void   SetTargetSelectedVolumeNthID(int n, vtkIdType newVolumeID); 
  virtual void SetTargetSelectedVolumeNthMRMLID(int n, const char* mrmlID); 

  virtual double   GetTreeNodeDistributionMeanWithCorrection(vtkIdType nodeID, int volumeNumber);
  virtual void     SetTreeNodeDistributionMeanWithCorrection(vtkIdType nodeID, int volumeNumber, double value);
  virtual void     ResetTreeNodeDistributionLogMeanCorrection(vtkIdType nodeID); 

  void ResetTreeNodeDistributionLogCovarianceCorrection(vtkIdType nodeID); 
  // Resets the LogCovariance of the entire tree
  void ResetLogCovarianceCorrectionOfAllNodes();
  void SetTreeNodeDistributionLogCovarianceWithCorrection(vtkIdType nodeID, int rowIndex, int columnIndex, double value);

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
  // miscellaneous
  //
  virtual int       GetEnableMultithreading();
  virtual void      SetEnableMultithreading(int isEnabled);

  virtual int       GetUpdateIntermediateData();
  virtual void      SetUpdateIntermediateData(int shouldUpdate);

  virtual int       GetAtlasNumberOfTrainingSamples();
  virtual void      ComputeAtlasNumberOfTrainingSamples();

  virtual void      GetSegmentationBoundaryMin(int minPoint[3]);
  virtual void      SetSegmentationBoundaryMin(int minPoint[3]);

  virtual void      GetSegmentationBoundaryMax(int maxPoint[3]);
  virtual void      SetSegmentationBoundaryMax(int maxPoint[3]);

  // If flag is set then only checks the template for nodes that are essential for GUI
  // If flag = 0 then checks the existence of all nodes essential for processing 
  virtual   int     CheckTemplateMRMLStructure(vtkMRMLEMSTemplateNode *emsTemp, int guiFlag);

  // checks the entire tree if all nodes are defined so we can start segmentation
  virtual int     CheckMRMLNodeStructureForProcessing()
  {
    return this->CheckTemplateMRMLStructure(this->Node,0);
  }

  // adds important nodes to the tree that are needed for using the gui 
  virtual void      CompleteTemplateMRMLStructureForGUI(vtkMRMLEMSTemplateNode *emsTemp);


 

  //
  // this functions registers all of the MRML nodes needed by this
  // class with the MRML scene
  //
  virtual void      RegisterMRMLNodesWithScene();

  // Return if we have a global parameters node
  virtual int HasGlobalParametersNode();

  virtual void PrintTree();
  virtual void PrintTree(vtkIdType rootID, vtkIndent indent);

  virtual void PrintVolumeInfo( vtkMRMLScene* mrmlScene );

  //
  // convenience functions for managing MRML nodes
  //
  virtual vtkMRMLEMSVolumeCollectionNode*   GetTargetInputNode();
  virtual vtkMRMLEMSAtlasNode*              GetAtlasInputNode();
  virtual vtkMRMLEMSVolumeCollectionNode*   GetSubParcellationInputNode();
  virtual vtkMRMLEMSAtlasNode*              GetAtlasAlignedNode();

  virtual vtkMRMLScalarVolumeNode*          GetOutputVolumeNode();

  virtual vtkMRMLEMSGlobalParametersNode*   GetGlobalParametersNode();
  virtual vtkMRMLEMSTreeNode*               GetTreeRootNode();
  virtual vtkMRMLEMSTreeNode*               GetTreeNode(vtkIdType);
  virtual bool TreeNodeExists(vtkIdType nodeID);
  virtual vtkMRMLEMSTreeParametersLeafNode* GetTreeParametersLeafNode(vtkIdType);  

  virtual vtkMRMLVolumeNode*                GetVolumeNode(vtkIdType);
  virtual vtkMRMLEMSWorkingDataNode*        GetWorkingDataNode();

  virtual vtkMRMLEMSVolumeCollectionNode*   CloneTargetNode(vtkMRMLEMSVolumeCollectionNode* target, const char* name)
  {
    return CloneVolumeCollectionNode(target, name);
  }
  virtual vtkMRMLEMSVolumeCollectionNode*   CloneSubParcellationNode(vtkMRMLEMSVolumeCollectionNode* target, const char* name)
  {
    return CloneVolumeCollectionNode(target, name);
  }

  virtual vtkMRMLEMSAtlasNode*  CloneAtlasNode(vtkMRMLEMSAtlasNode* target, const char* name);

  virtual void SynchronizeTargetNode(vtkMRMLEMSVolumeCollectionNode* templateNode, vtkMRMLEMSVolumeCollectionNode* changingNode, const char* name)
  {
       this->SynchronizeVolumeCollectionNode(templateNode, changingNode, name);
  }

  virtual void SynchronizeSubParcellationNode(vtkMRMLEMSVolumeCollectionNode* templateNode, vtkMRMLEMSVolumeCollectionNode* changingNode, const char* name)
  {
       this->SynchronizeVolumeCollectionNode(templateNode, changingNode, name);
  }

  virtual void SynchronizeAtlasNode(vtkMRMLEMSAtlasNode* templateNode, vtkMRMLEMSAtlasNode* changingNode, const char* name);

  //
  // convenience functions for managing ID mapping (mrml id <-> vtkIdType)
  //
  virtual const char*  MapVTKNodeIDToMRMLNodeID(vtkIdType vtkID);
  virtual vtkIdType    MapMRMLNodeIDToVTKNodeID(const char* MRMLNodeID);

  //BTX
  virtual void           GetListOfTreeNodeIDs(vtkIdType rootNodeID, 
                                              vtkstd::vector<vtkIdType>& list);
  //ETX

  virtual const char* GetTclTaskFilename();
  virtual void SetTclTaskFilename(const char* fileName);

  //BTX
  vtksys_stl::string TurnDefaultMRMLFileIntoTaskName(const char* fileName);
  vtksys_stl::string TurnDefaultTclFileIntoPreprocessingName(const char* fileName);
  //ETX

  virtual int          IDMapContainsMRMLNodeID(const char* MRMLNodeID);
  virtual int          IDMapContainsVTKNodeID(vtkIdType id);

  void  SetTreeNodeDistributionLogCovarianceOffDiagonal(vtkIdType nodeID, double value);

  virtual void CopyEMRelatedNodesToMRMLScene(vtkMRMLScene* newScene);

  virtual  void RemoveLegacyNodes();
 
  // Needed for Tcl  - if you set it as "" then it is interpreted as an id 
  void SetStorageNodeToNULL(vtkMRMLStorableNode* sNode) ;

//BTX
  void ImportMRMLFile(const char *mrmlFile,  vtksys_stl::string errMSG);  
//ETX
  // interface to ImportMRMLFile without the errMSG
  void ImportMRMLFile(const char *mrmlFile);

  //  referenceNode is only needed to create a volume node of the same type - Returns VTKNodeID 
  vtkMRMLScalarVolumeNode*  CreateVolumeScalarNode(vtkMRMLScalarVolumeNode*  referenceNode , const char *name); 
  vtkIdType CreateVolumeScalarNodeVolumeID(vtkMRMLScalarVolumeNode*  referenceNode , const char *name);
 
private:
  vtkEMSegmentMRMLManager();
  ~vtkEMSegmentMRMLManager();
  vtkEMSegmentMRMLManager(const vtkEMSegmentMRMLManager&);
  void operator=(const vtkEMSegmentMRMLManager&);

  // Should only be used within  this structure as it does not check 
  virtual void SetNode(vtkMRMLEMSTemplateNode*);

  virtual vtkIdType                       AddNewTreeNode();
  virtual vtkIdType                       GetNewVTKNodeID();

  virtual void           RemoveTreeNodeParametersNodes(vtkIdType nodeID);

  virtual void           PropogateAdditionOfSelectedTargetImage();
  virtual void           PropogateRemovalOfSelectedTargetImage(int index);
  virtual void           PropogateMovementOfSelectedTargetImage(int fromIndex,
                                                                int toIndex);

  // Update intensity statistics for a particular tissue type.
  virtual void      UpdateIntensityDistributionFromSample(vtkIdType nodeID);

  //BTX
  vtksys_stl::string TurnDefaultFileIntoName(vtksys_stl::string taskName);
 //ETX



  virtual void         IDMapInsertPair(vtkIdType vtkID, 
                                       const char* MRMLNodeID);
  virtual void         IDMapRemovePair(vtkIdType vtkID);
  virtual void         IDMapRemovePair(const char* MRMLNodeID);

  virtual void         UpdateMapsFromMRML();

  virtual int          GetTargetVolumeIndex(vtkIdType vtkID);

  // the current mrml scene
  vtkMRMLScene*   MRMLScene;

  //
  // parameters node that is currently under consideration
  //
  vtkMRMLEMSTemplateNode* Node;
  
  // global switch to hide EM segment parameters from MRML tree
  // editors
  bool   HideNodesFromEditors;

  //
  // The api of this class exposes vtkIdType ids for tree nodes and
  // volumes.  This essentially hides the mrml ids from client code and
  // insulates the client from changes in the slicer mrml id
  // mechanism.
  //

  vtkIdType NextVTKNodeID;

  //BTX
  typedef vtksys_stl::map<vtkIdType, vtksys_stl::string>  VTKToMRMLMapType;
  VTKToMRMLMapType                                VTKNodeIDToMRMLNodeIDMap;
  typedef vtksys_stl::map<vtksys_stl::string, vtkIdType>  MRMLToVTKMapType;
  MRMLToVTKMapType                                MRMLNodeIDToVTKNodeIDMap;

  vtkstd::vector<vtkstd::vector<double> > GetTreeNodeDistributionLogCovariance(vtkIdType nodeID);
  vtkstd::vector<vtkstd::vector<double> > GetTreeNodeDistributionLogCovarianceCorrection(vtkIdType nodeID);

   // Should Only be used in this function - bc only set through gui which calls DistributionMeanWithCorrection
  virtual double   GetTreeNodeDistributionLogMeanCorrection(vtkIdType nodeID, 
                                                  int volumeNumber);
  virtual void     SetTreeNodeDistributionLogMeanCorrection(vtkIdType nodeID, 
                                                  int volumeNumber, 
                                                  double value);

  // Functions should only call the corrected ones 
  virtual double   GetTreeNodeDistributionLogMean(vtkIdType nodeID, int volumeNumber);

  // Should Only be used in this function - bc only set through gui which calls DistributionLogCovarianceWithCorrection
  virtual double   GetTreeNodeDistributionLogCovarianceCorrection(vtkIdType nodeID, int rowIndex, int columnIndex);

  // virtual void     SetTreeNodeDistributionLogCovarianceCorrection(vtkIdType nodeID, vtkstd::vector<vtkstd::vector<double> > cov);

    virtual double   GetTreeNodeDistributionLogCovariance(vtkIdType nodeID, 
                                                        int rowIndex,
                                                        int columnIndex);
  int TreeNodeDistributionLogCovarianceCorrectionEnabled(vtkIdType nodeID);

  // Reset the correction of the node as well as subtree 
  void ResetLogCovarianceCorrectionsOfAllNodes(vtkIdType rootID);

  void TurnFromParentToLeafNode(vtkMRMLEMSTreeNode* treeNode) ;


  virtual vtkMRMLEMSVolumeCollectionNode*  CloneVolumeCollectionNode(vtkMRMLEMSVolumeCollectionNode* target, const char* name);
  virtual void SynchronizeVolumeCollectionNode(vtkMRMLEMSVolumeCollectionNode* templateNode, vtkMRMLEMSVolumeCollectionNode* changingNode, const char* name);

  //ETX

  virtual void         SetOutputVolumeID(vtkIdType volumeID);

  void PrintWeightOnForTree(vtkIdType rootID); 
};

#endif
