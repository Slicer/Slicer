/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumesLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// STD includes
#include <algorithm>

// Volumes includes
#include "vtkSlicerVolumesLogic.h"

// MRML logic includes
#include "vtkMRMLColorLogic.h"
#include "vtkDataIOManagerLogic.h"
#include "vtkMRMLRemoteIOLogic.h"

// MRML nodes includes
#include "vtkCacheManager.h"
#include "vtkDataIOManager.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLLabelMapVolumeNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkErrorSink.h>
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkImageThreshold.h>
#include <vtkMathUtilities.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtksys/SystemTools.hxx>
#include <vtkVersion.h>
#include <vtkWeakPointer.h>
#include <vtkImageReslice.h>
#include <vtkTransform.h>

/// CTK includes
/// to avoid CTK includes which pull in a dependency on Qt, rehome some CTK
/// core utility methods here in the anonymous namespace until they get ported
/// to VTK

//----------------------------------------------------------------------------
namespace
{

/// Return a "smart" number of decimals needed to display (in a gui) a floating
/// number. 16 is the max that can be returned, -1 for NaN numbers. When the
/// number of decimals is not obvious, it defaults to defaultDecimals if it is
/// different from -1, 16 otherwise.
int significantDecimals(double value, int defaultDecimals = -1)
{
  if (value == 0.
      || fabs(value) == std::numeric_limits<double>::infinity())
    {
    return 0;
    }
  if (value != value) // is NaN
    {
    return -1;
    }
  std::string number;
  std::stringstream numberStream;
  numberStream << std::setprecision(16);
  numberStream << std::fixed << value;
  number = numberStream.str();
  size_t decimalPos = number.find_last_of('.');
  std::string fractional = number.substr(decimalPos + 1);
  if (fractional.length() != 16)
    {
    return -1;
    }
  char previous = ' ';
  int previousRepeat=0;
  bool only0s = true;
  bool isUnit = value > -1. && value < 1.;
  for (size_t i = 0; i < fractional.length(); ++i)
    {
    char digit = fractional.at(i);
    if (digit != '0')
      {
      only0s = false;
      }
    // Has the digit been repeated too many times ?
    if (digit == previous && previousRepeat == 2 &&
        !only0s)
      {
      if (digit == '0' || digit == '9')
        {
        return i - previousRepeat;
        }
      return i;
      }
    // Last digit
    if (i == fractional.length() - 1)
      {
      // If we are here, that means that the right number of significant
      // decimals for the number has not been figured out yet.
      if (previousRepeat > 2 && !(only0s && isUnit) )
        {
        return i - previousRepeat;
        }
      // If defaultDecimals has been provided, just use it.
      if (defaultDecimals >= 0)
        {
        return defaultDecimals;
        }
      return fractional.length();
      }
    // get ready for next
    if (previous != digit)
      {
      previous = digit;
      previousRepeat = 1;
      }
    else
      {
      ++previousRepeat;
      }
    }
  return -1;
//  return fractional.length();
};

/// Return the order of magnitude of a number or numeric_limits<int>::min() if
/// the order of magnitude can't be computed (e.g. 0, inf, Nan, denorm)...
int orderOfMagnitude(double value)
{
  value = fabs(value);
  if (value == 0.
      || value == std::numeric_limits<double>::infinity()
      || value != value // is NaN
      || value < std::numeric_limits<double>::epsilon() // is tool small to compute
  )
    {
    return std::numeric_limits<int>::min();
    }
  double magnitude = 1.00000000000000001;
  int magnitudeOrder = 0;

  int magnitudeStep = 1;
  double magnitudeFactor = 10;

  if (value < 1.)
    {
    magnitudeOrder = -1;
    magnitudeStep = -1;
    magnitudeFactor = 0.1;
    }

  double epsilon = std::numeric_limits<double>::epsilon();
  while ( (magnitudeStep > 0 && value >= magnitude) ||
          (magnitudeStep < 0 && value < magnitude - epsilon))
    {
    magnitude *= magnitudeFactor;
    magnitudeOrder += magnitudeStep;
    }
  // we went 1 order too far, so decrement it
  return magnitudeOrder - magnitudeStep;
};

} // end of anonymous namespace

//----------------------------------------------------------------------------
// vtkSlicerVolumesLogic methods

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerVolumesLogic);

//----------------------------------------------------------------------------
namespace
{

//----------------------------------------------------------------------------
ArchetypeVolumeNodeSet DiffusionWeightedVolumeNodeSetFactory(std::string& volumeName, vtkMRMLScene* scene, int options)
{
  ArchetypeVolumeNodeSet nodeSet(scene);

  // set up the dwi node's support nodes
  vtkMRMLDiffusionWeightedVolumeDisplayNode* dwdisplayNode =
      vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLDiffusionWeightedVolumeDisplayNode"));

  vtkMRMLDiffusionWeightedVolumeNode* dwiNode =
      vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLDiffusionWeightedVolumeNode", volumeName));
  dwiNode->SetAndObserveDisplayNodeID(dwdisplayNode->GetID());

  vtkMRMLNRRDStorageNode* storageNode =
      vtkMRMLNRRDStorageNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLNRRDStorageNode"));
  storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  dwiNode->SetAndObserveStorageNodeID(storageNode->GetID());

  nodeSet.StorageNode = storageNode;
  nodeSet.DisplayNode = dwdisplayNode;
  nodeSet.Node = dwiNode;

  return nodeSet;
}

//----------------------------------------------------------------------------
ArchetypeVolumeNodeSet DiffusionTensorVolumeNodeSetFactory(std::string& volumeName, vtkMRMLScene* scene, int options)
{
  ArchetypeVolumeNodeSet nodeSet(scene);

  // set up the tensor node's support nodes
  vtkMRMLDiffusionTensorVolumeDisplayNode* dtdisplayNode =
      vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLDiffusionTensorVolumeDisplayNode"));
  // jvm - are these the default settings anyway?
  int wasModifying = dtdisplayNode->StartModify();
  dtdisplayNode->SetWindow(0);
  dtdisplayNode->SetLevel(0);
  dtdisplayNode->SetUpperThreshold(0);
  dtdisplayNode->SetLowerThreshold(0);
  dtdisplayNode->SetAutoWindowLevel(1);
  dtdisplayNode->EndModify(wasModifying);

  vtkMRMLDiffusionTensorVolumeNode* tensorNode =
      vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLDiffusionTensorVolumeNode", volumeName));
  tensorNode->SetAndObserveDisplayNodeID(dtdisplayNode->GetID());

  vtkMRMLVolumeArchetypeStorageNode* storageNode =
      vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLVolumeArchetypeStorageNode"));
  wasModifying = storageNode->StartModify();
  storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  storageNode->SetUseOrientationFromFile(!((options & vtkSlicerVolumesLogic::DiscardOrientation) != 0));
  storageNode->SetSingleFile(options & vtkSlicerVolumesLogic::SingleFile);
  storageNode->EndModify(wasModifying);

  tensorNode->SetAndObserveStorageNodeID(storageNode->GetID());

  nodeSet.StorageNode = storageNode;
  nodeSet.DisplayNode = dtdisplayNode;
  nodeSet.Node = tensorNode;

  return nodeSet;
}

//----------------------------------------------------------------------------
ArchetypeVolumeNodeSet NRRDVectorVolumeNodeSetFactory(std::string& volumeName, vtkMRMLScene* scene, int options)
{
  ArchetypeVolumeNodeSet nodeSet(scene);

  // set up the vector node's support nodes
  vtkMRMLVectorVolumeDisplayNode* vdisplayNode =
      vtkMRMLVectorVolumeDisplayNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLVectorVolumeDisplayNode"));

  vtkMRMLVectorVolumeNode* vectorNode =
      vtkMRMLVectorVolumeNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode", volumeName));
  vectorNode->SetAndObserveDisplayNodeID(vdisplayNode->GetID());

  vtkMRMLNRRDStorageNode* storageNode =
      vtkMRMLNRRDStorageNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLNRRDStorageNode"));
  storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  vectorNode->SetAndObserveStorageNodeID(storageNode->GetID());

  nodeSet.StorageNode = storageNode;
  nodeSet.DisplayNode = vdisplayNode;
  nodeSet.Node = vectorNode;

  return nodeSet;
}

//----------------------------------------------------------------------------
ArchetypeVolumeNodeSet ArchetypeVectorVolumeNodeSetFactory(std::string& volumeName, vtkMRMLScene* scene, int options)
{
  ArchetypeVolumeNodeSet nodeSet(scene);

  // set up the vector node's support nodes
  vtkMRMLVectorVolumeDisplayNode* vdisplayNode =
      vtkMRMLVectorVolumeDisplayNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLVectorVolumeDisplayNode"));

  vtkMRMLVectorVolumeNode* vectorNode =
      vtkMRMLVectorVolumeNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode", volumeName));
  vectorNode->SetAndObserveDisplayNodeID(vdisplayNode->GetID());

  vtkMRMLVolumeArchetypeStorageNode* storageNode =
      vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLVolumeArchetypeStorageNode"));
  int wasModifying = storageNode->StartModify();
  storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  storageNode->SetUseOrientationFromFile(!((options & vtkSlicerVolumesLogic::DiscardOrientation) != 0));
  storageNode->SetSingleFile(options & vtkSlicerVolumesLogic::SingleFile);
  storageNode->EndModify(wasModifying);
  vectorNode->SetAndObserveStorageNodeID(storageNode->GetID());

  nodeSet.StorageNode = storageNode;
  nodeSet.DisplayNode = vdisplayNode;
  nodeSet.Node = vectorNode;

  return nodeSet;
}

//----------------------------------------------------------------------------
ArchetypeVolumeNodeSet LabelMapVolumeNodeSetFactory(std::string& volumeName, vtkMRMLScene* scene, int options)
{
  ArchetypeVolumeNodeSet nodeSet(scene);

  // set up the scalar node's support nodes
  vtkMRMLLabelMapVolumeDisplayNode* lmdisplayNode =
      vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLLabelMapVolumeDisplayNode"));

  vtkMRMLLabelMapVolumeNode* scalarNode =
      vtkMRMLLabelMapVolumeNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLLabelMapVolumeNode", volumeName));
  scalarNode->SetAndObserveDisplayNodeID(lmdisplayNode->GetID());

  vtkMRMLVolumeArchetypeStorageNode* storageNode =
      vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLVolumeArchetypeStorageNode"));
  int wasModifying = storageNode->StartModify();
  storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  storageNode->SetUseOrientationFromFile(!((options & vtkSlicerVolumesLogic::DiscardOrientation) != 0));
  storageNode->SetSingleFile(options & vtkSlicerVolumesLogic::SingleFile);
  storageNode->EndModify(wasModifying);
  scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());

  nodeSet.StorageNode = storageNode;
  nodeSet.DisplayNode = lmdisplayNode;
  nodeSet.Node = scalarNode;

  nodeSet.LabelMap = true;

  return nodeSet;
}

//----------------------------------------------------------------------------
ArchetypeVolumeNodeSet ScalarVolumeNodeSetFactory(std::string& volumeName, vtkMRMLScene* scene, int options)
{
  ArchetypeVolumeNodeSet nodeSet(scene);

  // set up the scalar node's support nodes
  vtkMRMLScalarVolumeDisplayNode* sdisplayNode =
      vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLScalarVolumeDisplayNode"));

  vtkMRMLScalarVolumeNode* scalarNode =
      vtkMRMLScalarVolumeNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLScalarVolumeNode", volumeName));
  scalarNode->SetAndObserveDisplayNodeID(sdisplayNode->GetID());

  vtkMRMLVolumeArchetypeStorageNode* storageNode =
      vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(
        nodeSet.Scene->AddNewNodeByClass("vtkMRMLVolumeArchetypeStorageNode"));
  int wasModifying = storageNode->StartModify();
  storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  storageNode->SetUseOrientationFromFile(!((options & vtkSlicerVolumesLogic::DiscardOrientation) != 0));
  storageNode->SetSingleFile(options & vtkSlicerVolumesLogic::SingleFile);
  storageNode->EndModify(wasModifying);
  scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());

  nodeSet.StorageNode = storageNode;
  nodeSet.DisplayNode = sdisplayNode;
  nodeSet.Node = scalarNode;

  return nodeSet;
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic::vtkSlicerVolumesLogic()
{
  // register the default factories for nodesets. this is done in a specific order
  this->RegisterArchetypeVolumeNodeSetFactory( DiffusionWeightedVolumeNodeSetFactory );
  this->RegisterArchetypeVolumeNodeSetFactory( DiffusionTensorVolumeNodeSetFactory );
  this->RegisterArchetypeVolumeNodeSetFactory( NRRDVectorVolumeNodeSetFactory );
  this->RegisterArchetypeVolumeNodeSetFactory( ArchetypeVectorVolumeNodeSetFactory );
  this->RegisterArchetypeVolumeNodeSetFactory( LabelMapVolumeNodeSetFactory );
  this->RegisterArchetypeVolumeNodeSetFactory( ScalarVolumeNodeSetFactory );

  this->CompareVolumeGeometryEpsilon = 0.000001;
  this->CompareVolumeGeometryPrecision = 6;
}

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic::~vtkSlicerVolumesLogic() = default;

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::ProcessMRMLNodesEvents(vtkObject *vtkNotUsed(caller),
                                            unsigned long event,
                                            void *callData)
{
  if (event ==  vtkCommand::ProgressEvent)
    {
    this->InvokeEvent ( vtkCommand::ProgressEvent,callData );
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::SetColorLogic(vtkMRMLColorLogic *colorLogic)
{
  if (this->ColorLogic == colorLogic)
    {
    return;
    }
  this->ColorLogic = colorLogic;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLColorLogic* vtkSlicerVolumesLogic::GetColorLogic()const
{
  return this->ColorLogic;
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::SetActiveVolumeNode(vtkMRMLVolumeNode *activeNode)
{
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, activeNode);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::GetActiveVolumeNode()const
{
  return this->ActiveVolumeNode;
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic
::SetAndObserveColorToDisplayNode(vtkMRMLDisplayNode * displayNode,
                                  int labelMap, const char* filename)
{
  vtkMRMLColorLogic * colorLogic = this->GetColorLogic();
  if (colorLogic == nullptr)
    {
    return;
    }
  if (labelMap)
    {
    if (displayNode->GetColorNodeID() == nullptr)
      {
      // only set default color node ID if it was not set already (by default volume node stored in the scene)
      displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultLabelMapColorNodeID());
      }
    }
  else if (displayNode->GetColorNodeID() == nullptr)
    {
    // only set default color node ID if it was not set already (by default volume node stored in the scene)
    displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::InitializeStorageNode(
  vtkMRMLStorageNode * storageNode, const char * filename, vtkStringArray *fileList, vtkMRMLScene * mrmlScene)
{
  bool useURI = false;

  if (mrmlScene == nullptr)
    {
    mrmlScene = this->GetMRMLScene();
    }

  if (mrmlScene && mrmlScene->GetCacheManager())
    {
    useURI = mrmlScene->GetCacheManager()->IsRemoteReference(filename);
    }
  if (useURI)
    {
    vtkDebugMacro("AddArchetypeVolume: input filename '" << filename << "' is a URI");
    // need to set the scene on the storage node so that it can look for file handlers
    storageNode->SetURI(filename);
    storageNode->SetScene(mrmlScene);
    if (fileList != nullptr)
      {
      // it's a list of uris
      int numURIs = fileList->GetNumberOfValues();
      vtkDebugMacro("Have a list of " << numURIs << " uris that go along with the archetype");
      vtkStdString thisURI;
      storageNode->ResetURIList();
      for (int n = 0; n < numURIs; n++)
        {
        thisURI = fileList->GetValue(n);
        storageNode->AddURI(thisURI);
        }
      }
    }
  else
    {
    storageNode->SetFileName(filename);
    if (fileList != nullptr)
      {
      int numFiles = fileList->GetNumberOfValues();
      vtkDebugMacro("Have a list of " << numFiles << " files that go along with the archetype");
      vtkStdString thisFileName;
      storageNode->ResetFileNameList();
      for (int n = 0; n < numFiles; n++)
        {
        thisFileName = fileList->GetValue(n);
        //vtkDebugMacro("\tfile " << n << " =  " << thisFileName);
        storageNode->AddFileName(thisFileName);
        }
      }
    }
  storageNode->AddObserver(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddArchetypeVolume(
    const char* filename, const char* volname,
    int loadingOptions, vtkStringArray *fileList)
{
  return this->AddArchetypeVolume(this->VolumeRegistry, filename, volname, loadingOptions, fileList);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerVolumesLogic::AddArchetypeScalarVolume(
    const char* filename, const char* volname, int loadingOptions, vtkStringArray *fileList)
{
  NodeSetFactoryRegistry nodeSetFactoryRegistry;
  nodeSetFactoryRegistry.push_back(&ScalarVolumeNodeSetFactory);
  return vtkMRMLScalarVolumeNode::SafeDownCast(this->AddArchetypeVolume(nodeSetFactoryRegistry, filename, volname, loadingOptions, fileList));
}

//----------------------------------------------------------------------------
// int loadingOptions is bit-coded as following:
// bit 0: label map
// bit 1: centered
// bit 2: loading single file
// bit 3: auto calculate window/level
// bit 4: discard image orientation
// higher bits are reserved for future use
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddArchetypeVolume (
    const NodeSetFactoryRegistry& volumeRegistry,
    const char* filename, const char* volname, int loadingOptions,
    vtkStringArray *fileList)
{
  if (this->GetMRMLScene() == nullptr)
    {
    vtkErrorMacro("AddArchetypeVolume: Failed to add volume - MRMLScene is null");
    return nullptr;
    }

  bool labelMap = false;
  if ( loadingOptions & 1 )    // labelMap is true
    {
    labelMap = true;
    }

  vtkSmartPointer<vtkMRMLVolumeNode> volumeNode;
  vtkSmartPointer<vtkMRMLVolumeDisplayNode> displayNode;
  vtkSmartPointer<vtkMRMLStorageNode> storageNode;

  // Compute volume name
  std::string volumeName = volname != nullptr ? volname : vtksys::SystemTools::GetFilenameName(filename);
  volumeName = this->GetMRMLScene()->GetUniqueNameByString(volumeName.c_str());

  vtkNew<vtkErrorSink> errorSink;

  // set up a mini scene to avoid adding and removing nodes from the main scene
  vtkNew<vtkMRMLScene> testScene;
  // associate default nodes with mini scene
  this->GetMRMLScene()->CopyDefaultNodesToScene(testScene.GetPointer());
  // set it up for remote io, the constructor creates a cache and data io manager
  vtkSmartPointer<vtkMRMLRemoteIOLogic> remoteIOLogic;
  remoteIOLogic = vtkSmartPointer<vtkMRMLRemoteIOLogic>::New();
  if (this->GetMRMLScene()->GetCacheManager())
    {
    // update the temp remote cache dir from the main one
    remoteIOLogic->GetCacheManager()->SetRemoteCacheDirectory(this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory());
    }
  // set up the data io manager logic to handle remote downloads
  vtkSmartPointer<vtkDataIOManagerLogic> dataIOManagerLogic;
  dataIOManagerLogic = vtkSmartPointer<vtkDataIOManagerLogic>::New();
  dataIOManagerLogic->SetMRMLApplicationLogic(this->GetApplicationLogic());
  dataIOManagerLogic->SetAndObserveDataIOManager(
    remoteIOLogic->GetDataIOManager());

  // and link up everything for the test scene
  this->GetApplicationLogic()->SetMRMLSceneDataIO(testScene.GetPointer(),
                                                  remoteIOLogic, dataIOManagerLogic);

  // Run through the factory list and test each factory until success
  for (NodeSetFactoryRegistry::const_iterator fit = volumeRegistry.begin();
       fit != volumeRegistry.end(); ++fit)
    {
    ArchetypeVolumeNodeSet nodeSet( (*fit)(volumeName, testScene.GetPointer(), loadingOptions) );

    // if the labelMap flags for reader and factory are consistent
    // (both true or both false)
    if (labelMap == nodeSet.LabelMap)
      {

      // connect the observers
      errorSink->SetObservedObject(nodeSet.StorageNode);
      nodeSet.StorageNode->AddObserver(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());

      this->InitializeStorageNode(nodeSet.StorageNode, filename, fileList, testScene.GetPointer());

      vtkDebugMacro("Attempt to read file as a volume of type "
                    << nodeSet.Node->GetNodeTagName() << " using "
                    << nodeSet.Node->GetClassName() << " [filename = " << filename << "]");
      bool success = nodeSet.StorageNode->ReadData(nodeSet.Node);

      // disconnect the observers
      errorSink->SetObservedObject(nullptr);
      nodeSet.StorageNode->RemoveObservers(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());

      if (success)
        {
        displayNode = nodeSet.DisplayNode;
        volumeNode =  nodeSet.Node;
        storageNode = nodeSet.StorageNode;
        vtkDebugMacro(<< "File successfully read as " << nodeSet.Node->GetNodeTagName()
                      << " [filename = " << filename << "]");
        break;
        }
      }

    //
    // Wasn't the right factory, so we need to clean up
    //

    // clean up the scene
    nodeSet.Node->SetAndObserveDisplayNodeID(nullptr);
    nodeSet.Node->SetAndObserveStorageNodeID(nullptr);
    testScene->RemoveNode(nodeSet.DisplayNode);
    testScene->RemoveNode(nodeSet.StorageNode);
    testScene->RemoveNode(nodeSet.Node);
    }

  // display any errors
  if (volumeNode == nullptr)
    {
    errorSink->DisplayMessages();
    }


  bool modified = false;
  if (volumeNode != nullptr)
    {
    // move the nodes from the test scene to the main one, removing from the
    // test scene first to avoid missing ID/reference errors and to fix a
    // problem found in testing an extension where the RAS to IJK matrix
    /// was reset to identity.
    testScene->RemoveNode(displayNode);
    testScene->RemoveNode(storageNode);
    testScene->RemoveNode(volumeNode);
    this->GetMRMLScene()->AddNode(displayNode);
    this->GetMRMLScene()->AddNode(storageNode);
    this->GetMRMLScene()->AddNode(volumeNode);
    volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
    volumeNode->SetAndObserveStorageNodeID(storageNode->GetID());

    this->SetAndObserveColorToDisplayNode(displayNode, labelMap, filename);

    vtkDebugMacro("Name vol node "<<volumeNode->GetClassName());
    vtkDebugMacro("Display node "<<displayNode->GetClassName());

    this->SetActiveVolumeNode(volumeNode);

    modified = true;
    }

  // clean up the test scene
  remoteIOLogic->RemoveDataIOFromScene();
  if (testScene->GetCacheManager())
    {
    testScene->SetCacheManager(nullptr);
    }
  if (testScene->GetDataIOManager())
    {
    testScene->SetDataIOManager(nullptr);
    }

  if (modified)
    {
    this->Modified();
    }
  return volumeNode;
}

//----------------------------------------------------------------------------
int vtkSlicerVolumesLogic::SaveArchetypeVolume (const char* filename, vtkMRMLVolumeNode *volumeNode)
{
  if (volumeNode == nullptr || filename == nullptr)
    {
    return 0;
    }

  vtkMRMLNRRDStorageNode *storageNode1 = nullptr;
  vtkMRMLVolumeArchetypeStorageNode *storageNode2 = nullptr;
  vtkMRMLStorageNode *storageNode = nullptr;
  vtkMRMLStorageNode *snode = volumeNode->GetStorageNode();

  if (snode != nullptr)
    {
    storageNode2 = vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(snode);
    storageNode1 = vtkMRMLNRRDStorageNode::SafeDownCast(snode);
    }

  bool useURI = false;
  if (this->GetMRMLScene() &&
      this->GetMRMLScene()->GetCacheManager())
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    }

  // Use NRRD writer if we are dealing with DWI, DTI or vector volumes

  if (volumeNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") ||
//      volumeNode->IsA("vtkMRMLDiffusionTensorVolumeNode") ||
      volumeNode->IsA("vtkMRMLVectorVolumeNode"))
    {

    if (storageNode1 == nullptr)
      {
      storageNode1 = vtkMRMLNRRDStorageNode::New();
      storageNode1->SetScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode1);
      volumeNode->SetAndObserveStorageNodeID(storageNode1->GetID());
      storageNode1->Delete();
      }
    if (useURI)
      {
      storageNode1->SetURI(filename);
      }
    else
      {
      storageNode1->SetFileName(filename);
      }
    storageNode = storageNode1;
    }
  else
    {
    if (storageNode2 == nullptr)
      {
      storageNode2 = vtkMRMLVolumeArchetypeStorageNode::New();
      storageNode2->SetScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode2);
      volumeNode->SetAndObserveStorageNodeID(storageNode2->GetID());
      storageNode2->Delete();
      }

    if (useURI)
      {
      storageNode2->SetURI(filename);
      }
    else
      {
      storageNode2->SetFileName(filename);
      }
    storageNode = storageNode2;
    }

  int res = storageNode->WriteData(volumeNode);
  return res;
}

//----------------------------------------------------------------------------
vtkMRMLLabelMapVolumeNode* vtkSlicerVolumesLogic
::CreateAndAddLabelVolume(vtkMRMLVolumeNode *volumeNode, const char *name)
{
  return this->CreateAndAddLabelVolume(this->GetMRMLScene(), volumeNode, name);
}

//----------------------------------------------------------------------------
vtkMRMLLabelMapVolumeNode *
vtkSlicerVolumesLogic::CreateAndAddLabelVolume(vtkMRMLScene *scene,
                                               vtkMRMLVolumeNode *volumeNode,
                                               const char *name)
{
  if (scene == nullptr || volumeNode == nullptr || name == nullptr)
    {
    return nullptr;
    }

  // Create a volume node as copy of source volume
  vtkNew<vtkMRMLLabelMapVolumeNode> labelNode;
  std::string uname = scene->GetUniqueNameByString(name);
  labelNode->SetName(uname.c_str());
  scene->AddNode(labelNode.GetPointer());

  this->CreateLabelVolumeFromVolume(scene, labelNode.GetPointer(), volumeNode);

  // Make an image data of the same size and shape as the input volume, but filled with zeros
  vtkSlicerVolumesLogic::ClearVolumeImageData(labelNode.GetPointer());

  return labelNode.GetPointer();
}

//----------------------------------------------------------------------------
vtkMRMLLabelMapVolumeNode* vtkSlicerVolumesLogic
::CreateLabelVolume(vtkMRMLVolumeNode *volumeNode,
                    const char *name)
{
  vtkWarningMacro("Deprecated, please use CreateAndAddLabelVolume instead");
  return this->CreateAndAddLabelVolume(volumeNode, name);
}

//----------------------------------------------------------------------------
vtkMRMLLabelMapVolumeNode* vtkSlicerVolumesLogic
::CreateLabelVolume(vtkMRMLScene* scene,
                    vtkMRMLVolumeNode *volumeNode,
                    const char *name)
{
  vtkWarningMacro("Deprecated, please use CreateAndAddLabelVolume instead");
  return this->CreateAndAddLabelVolume(scene, volumeNode, name);
}

//----------------------------------------------------------------------------
vtkMRMLLabelMapVolumeNode *
vtkSlicerVolumesLogic::FillLabelVolumeFromTemplate(vtkMRMLLabelMapVolumeNode *labelNode,
                                                   vtkMRMLVolumeNode *templateNode)
{
  return Self::FillLabelVolumeFromTemplate(this->GetMRMLScene(), labelNode, templateNode);
}

//----------------------------------------------------------------------------
vtkMRMLLabelMapVolumeNode *
vtkSlicerVolumesLogic::FillLabelVolumeFromTemplate(vtkMRMLScene *scene,
                                                   vtkMRMLLabelMapVolumeNode *labelNode,
                                                   vtkMRMLVolumeNode *templateNode)
{
  this->CreateLabelVolumeFromVolume(scene, labelNode, templateNode);

  // Make an image data of the same size and shape as the input volume, but filled with zeros
  vtkSlicerVolumesLogic::ClearVolumeImageData(labelNode);

  return labelNode;
}

//----------------------------------------------------------------------------
vtkMRMLLabelMapVolumeNode*
vtkSlicerVolumesLogic::CreateLabelVolumeFromVolume(vtkMRMLScene *scene,
                                                   vtkMRMLLabelMapVolumeNode *outputVolume,
                                                   vtkMRMLVolumeNode *inputVolume)
{
  if (scene == nullptr || outputVolume == nullptr || inputVolume == nullptr)
    {
    return nullptr;
    }

  // Associate labelmap with the source volume
  if (inputVolume->GetID())
    {
    outputVolume->SetNodeReferenceID("AssociatedNodeID", inputVolume->GetID());
    }

  // Copy and set image data of the input volume to the label volume
  vtkNew<vtkImageData> imageData;
  imageData->DeepCopy(inputVolume->GetImageData());
  outputVolume->SetAndObserveImageData(imageData.GetPointer());

  vtkNew<vtkMatrix4x4> ijkToRas;
  inputVolume->GetIJKToRASMatrix(ijkToRas.GetPointer());
  outputVolume->SetIJKToRASMatrix(ijkToRas.GetPointer());

  outputVolume->SetAndObserveTransformNodeID(inputVolume->GetTransformNodeID());

  // Create a display node if the label node does not have one
  vtkMRMLLabelMapVolumeDisplayNode* displayNode =
    vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(outputVolume->GetDisplayNode());
  if (displayNode == nullptr)
    {
    displayNode = vtkMRMLLabelMapVolumeDisplayNode::New();
    scene->AddNode(displayNode);
    displayNode->Delete();
    // Set the display node to have a label map lookup table
    this->SetAndObserveColorToDisplayNode(displayNode,
      /* labelMap = */ 1, /* filename= */ nullptr);
    outputVolume->SetAndObserveDisplayNodeID(displayNode->GetID());
    }

  return outputVolume;
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerVolumesLogic::CreateScalarVolumeFromVolume(
  vtkMRMLScene *scene, vtkMRMLScalarVolumeNode *outputVolume, vtkMRMLVolumeNode *inputVolume)
{
  if ( scene == nullptr || outputVolume == nullptr
    || inputVolume == nullptr || inputVolume->GetImageData() == nullptr )
    {
    return nullptr;
    }

  // Associate labelmap with the source volume
  if (inputVolume->GetID())
    {
    outputVolume->SetNodeReferenceID("AssociatedNodeID", inputVolume->GetID());
    }

  // Copy and set image data of the input volume to the label volume
  vtkNew<vtkImageData> imageData;
  imageData->DeepCopy(inputVolume->GetImageData());
  outputVolume->SetAndObserveImageData(imageData.GetPointer());

  vtkNew<vtkMatrix4x4> ijkToRas;
  inputVolume->GetIJKToRASMatrix(ijkToRas.GetPointer());
  outputVolume->SetIJKToRASMatrix(ijkToRas.GetPointer());

  outputVolume->SetAndObserveTransformNodeID(inputVolume->GetTransformNodeID());

  // Create a display node if the output node does not have one
  outputVolume->CreateDefaultDisplayNodes();

  return outputVolume;
}

//----------------------------------------------------------------------------
void
vtkSlicerVolumesLogic::ClearVolumeImageData(vtkMRMLVolumeNode *volumeNode)
{
  if (volumeNode == nullptr)
    {
    return;
    }

  // Make an image data of the same size and shape as the input volume, but filled with zeros
  vtkNew<vtkImageThreshold> thresh;
  thresh->ReplaceInOn();
  thresh->ReplaceOutOn();
  thresh->SetInValue(0);
  thresh->SetOutValue(0);
  thresh->SetOutputScalarType(VTK_SHORT);

  vtkNew<vtkImageData> imageData;
  thresh->SetInputData(volumeNode->GetImageData());
  thresh->Update();
  imageData->DeepCopy(thresh->GetOutput());
  volumeNode->SetAndObserveImageData(imageData.GetPointer());
}

//----------------------------------------------------------------------------
std::string
vtkSlicerVolumesLogic::CheckForLabelVolumeValidity(vtkMRMLScalarVolumeNode *volumeNode,
                                                   vtkMRMLLabelMapVolumeNode *labelNode)
{
  std::stringstream warnings;
  warnings << "";
  if (!volumeNode || !labelNode)
    {
    if (!volumeNode)
      {
      warnings << "Null volume node pointer\n";
      }
    if (!labelNode)
      {
      warnings << "Null label volume node pointer\n";
      }
    }
  else
    {
    if (vtkMRMLLabelMapVolumeNode::SafeDownCast(labelNode)==nullptr)
      {
      warnings << "Label node is not of type vtkMRMLLabelMapVolumeNode\n";
      }
    else
      {
      warnings << this->CompareVolumeGeometry(volumeNode, labelNode);
      }
    }
  return (warnings.str());
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::SetCompareVolumeGeometryEpsilon(double epsilon)
{
  vtkDebugMacro("vtkSlicerVolumesLogic setting "
                << " CompareVolumeGeometryEpsilon to " << epsilon);

  double positiveEpsilon = epsilon;
  // check for negative values
  if (positiveEpsilon < 0.0)
    {
    positiveEpsilon = fabs(epsilon);
    }

  if (this->CompareVolumeGeometryEpsilon != positiveEpsilon)
    {
    this->CompareVolumeGeometryEpsilon = positiveEpsilon;

    // now set the precision
    this->CompareVolumeGeometryPrecision = significantDecimals(this->CompareVolumeGeometryEpsilon);

    this->Modified();
    }
}

//----------------------------------------------------------------------------
std::string
vtkSlicerVolumesLogic::CompareVolumeGeometry(vtkMRMLScalarVolumeNode *volumeNode1,
                                             vtkMRMLScalarVolumeNode *volumeNode2)
{
  std::stringstream warnings;
  if (!volumeNode1 || !volumeNode2)
    {
    if (!volumeNode1)
      {
      warnings << "Null first volume node pointer\n";
      }
    else
      {
      warnings << "Null second volume node pointer\n";
      }
    }
  else
    {
    vtkImageData *volumeImage1 = volumeNode1->GetImageData();
    vtkImageData *volumeImage2  = volumeNode2->GetImageData();
    if (!volumeImage1 || !volumeImage2)
      {
      if (!volumeImage1)
        {
        warnings << "Null first image data pointer\n";
        }
      if (!volumeImage2)
        {
        warnings << "Null second image data pointer\n";
        }
      }
    else
      {

      // warning if one ID is set and not the other,
      // or if both are set but have different strings
      const char *transformID1, *transformID2;
      transformID1 = volumeNode1->GetTransformNodeID();
      transformID2 = volumeNode2->GetTransformNodeID();
      if ( transformID1 && transformID2 && !strcmp(transformID1,transformID2) )
        {
        warnings << "Transform mismatch\n";
        }
      else if ( transformID1 != transformID2 )
        {
        warnings << "Transform mismatch\n";
        }

      int row, column;
      double volumeValue1, volumeValue2;
      // set the floating point precision to match the precision of the espilon
      // used for the fuzzy compare (need fixed so precision applies to right
      // of the decimal point see https://github.com/Slicer/Slicer/issues/3776).
      warnings << std::fixed;
      warnings << std::setprecision(this->GetCompareVolumeGeometryPrecision());
      // sanity check versus the volume spacings
      double spacing1[3], spacing2[3];
      volumeNode1->GetSpacing(spacing1);
      volumeNode2->GetSpacing(spacing2);
      double minSpacing = spacing1[0];
      for (int i = 1; i < 3; ++i)
        {
        if (spacing1[i] < minSpacing)
          {
          minSpacing = spacing1[i];
          }
        }
      for (int i = 0; i < 3; ++i)
        {
        if (spacing2[i] < minSpacing)
          {
          minSpacing = spacing2[i];
          }
        }
      // in general the defaults assume that an epsilon of 1e-6 works with a min
      // spacing of 1mm, check that the epsilon is scaled appropriately for the
      // minimum spacing for these two volumes
      double logDiff = orderOfMagnitude(minSpacing) - orderOfMagnitude(this->CompareVolumeGeometryEpsilon);
      vtkDebugMacro("diff in order of mag between min spacing and epsilon = " << logDiff);
      if (logDiff < 3.0 || logDiff > 10.0)
        {
        warnings << "(Minimum spacing for volumes of " << minSpacing << " mismatched with epsilon " << this->CompareVolumeGeometryEpsilon << ",\ngeometry comparison may not be useful.\nTry resetting the Volumes module logic compare volume geometry epsilon variable.)\n";
        }
      for (row = 0; row < 3; row++)
        {
        volumeValue1 = volumeImage1->GetDimensions()[row];
        volumeValue2 = volumeImage2->GetDimensions()[row];

        if (volumeValue1 != volumeValue2)
          {
          warnings << "Dimension mismatch at row [" << row << "] (" << volumeValue1 << " != " << volumeValue2 << ")\n";
          }

        volumeValue1 = volumeImage1->GetSpacing()[row];
        volumeValue2 = volumeImage2->GetSpacing()[row];
        if (volumeValue1 != volumeValue2)
          {
          warnings << "Spacing mismatch at row [" << row << "] (" << volumeValue1 << " != " << volumeValue2 << ")\n";
          }

        volumeValue1 = volumeImage1->GetOrigin()[row];
        volumeValue2 = volumeImage2->GetOrigin()[row];
        if (volumeValue1 != volumeValue2)
          {
          warnings << "Origin mismatch at row [" << row << "] (" << volumeValue1 << " != " << volumeValue2 << ")\n";
          }
        }

      vtkMatrix4x4 *volumeIJKToRAS1 = vtkMatrix4x4::New();
      vtkMatrix4x4 *volumeIJKToRAS2 = vtkMatrix4x4::New();
      volumeNode1->GetIJKToRASMatrix(volumeIJKToRAS1);
      volumeNode2->GetIJKToRASMatrix(volumeIJKToRAS2);
      for (row = 0; row < 4; row++)
        {
        for (column = 0; column < 4; column++)
          {
          volumeValue1 = volumeIJKToRAS1->GetElement(row,column);
          volumeValue2 = volumeIJKToRAS2->GetElement(row,column);
          if (!vtkMathUtilities::FuzzyCompare<double>(volumeValue1,
                                                      volumeValue2,
                                                      this->CompareVolumeGeometryEpsilon))
            {
            warnings << "IJKToRAS mismatch at [" << row << ", " << column << "] ("
                     << volumeValue1 << " != " << volumeValue2 << ")\n";
            }
          }
        }
      volumeIJKToRAS1->Delete();
      volumeIJKToRAS2->Delete();
      }
    }
  return (warnings.str());
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*
vtkSlicerVolumesLogic::CloneVolume(vtkMRMLVolumeNode *volumeNode, const char *name)
{
  return Self::CloneVolume(this->GetMRMLScene(), volumeNode, name);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*
vtkSlicerVolumesLogic::
CloneVolume(vtkMRMLScene *scene, vtkMRMLVolumeNode *volumeNode, const char *name, bool cloneImageData/*=true*/)
{
  return vtkMRMLScalarVolumeNode::SafeDownCast(vtkSlicerVolumesLogic::CloneVolumeGeneric(scene, volumeNode, name, cloneImageData));
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode*
vtkSlicerVolumesLogic::
CloneVolumeGeneric (vtkMRMLScene *scene, vtkMRMLVolumeNode *volumeNode, const char *name, bool cloneImageData/*=true*/)
{
  if ( scene == nullptr || volumeNode == nullptr )
    {
    vtkGenericWarningMacro("vtkSlicerVolumesLogic::CloneVolumeGeneric failed: invalid scene or input volume node");
    return nullptr;
    }

  // clone the volume node
  vtkSmartPointer<vtkMRMLVolumeNode> clonedVolumeNode;
  clonedVolumeNode.TakeReference(vtkMRMLVolumeNode::SafeDownCast(scene->CreateNodeByClass(volumeNode->GetClassName())));
  if ( !clonedVolumeNode.GetPointer() )
    {
    vtkErrorWithObjectMacro(volumeNode, "Could not clone volume");
    return nullptr;
    }
  clonedVolumeNode->CopyWithScene(volumeNode);

  // remove storage nodes
  clonedVolumeNode->SetAndObserveStorageNodeID(nullptr);

  // remove display nodes (but not the first one)
  while (clonedVolumeNode->GetNumberOfDisplayNodes() > 1)
    {
    clonedVolumeNode->RemoveNthDisplayNodeID(1); // always remove at index 1 since they will shift
    }

  // clone the 1st display node if possible
  vtkMRMLDisplayNode* originalDisplayNode = volumeNode->GetDisplayNode();
  vtkSmartPointer<vtkMRMLDisplayNode> clonedDisplayNode;
  if (originalDisplayNode)
    {
    clonedDisplayNode.TakeReference((vtkMRMLDisplayNode*)scene->CreateNodeByClass(originalDisplayNode->GetClassName()));
    }
  if (clonedDisplayNode.GetPointer())
    {
    clonedDisplayNode->CopyWithScene(originalDisplayNode);
    scene->AddNode(clonedDisplayNode);
    clonedVolumeNode->SetAndObserveDisplayNodeID(clonedDisplayNode->GetID());
    }
  else
    {
    clonedVolumeNode->SetAndObserveDisplayNodeID(nullptr);
    }

  // update name
  std::string uname = scene->GetUniqueNameByString(name);
  clonedVolumeNode->SetName(uname.c_str());

  if (cloneImageData)
    {
    // copy over the volume's data
    if (volumeNode->GetImageData())
      {
      vtkNew<vtkImageData> clonedVolumeData;
      clonedVolumeData->DeepCopy(volumeNode->GetImageData());
      clonedVolumeNode->SetAndObserveImageData( clonedVolumeData.GetPointer() );
      }
    else
      {
      vtkErrorWithObjectMacro(scene, "CloneVolume: The ImageData of VolumeNode with ID "
                              << volumeNode->GetID() << " is null !");
      }
    }
  else
    {
    clonedVolumeNode->SetAndObserveImageData(nullptr);
    }

  // add the cloned volume to the scene
  scene->AddNode(clonedVolumeNode.GetPointer());

  return clonedVolumeNode.GetPointer();
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*
vtkSlicerVolumesLogic::
CloneVolumeWithoutImageData(vtkMRMLScene *scene, vtkMRMLVolumeNode *volumeNode, const char *name)
{
  return vtkSlicerVolumesLogic::CloneVolume(scene, volumeNode, name, /*cloneImageData:*/ false );
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerVolumesLogic:             " << this->GetClassName() << "\n";

  os << indent << "ActiveVolumeNode: " <<
    (this->ActiveVolumeNode ? this->ActiveVolumeNode->GetName() : "(none)") << "\n";
  os << indent << "CompareVolumeGeometryEpsilon: "
     << this->CompareVolumeGeometryEpsilon << "\n";
  os << indent << "CompareVolumeGeometryPrecision: "
     << this->CompareVolumeGeometryPrecision << "\n";
}

//----------------------------------------------------------------------------
int vtkSlicerVolumesLogic::IsFreeSurferVolume (const char* filename)
{
  if (filename == nullptr)
    {
    return 0;
    }

  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(filename);
  if (extension == std::string(".mgz") ||
      extension == std::string(".mgh") ||
      extension == std::string(".mgh.gz"))
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//-------------------------------------------------------------------------
void vtkSlicerVolumesLogic::ComputeTkRegVox2RASMatrix ( vtkMRMLVolumeNode *VNode,
                                                                       vtkMatrix4x4 *M )
{
    double dC, dS, dR;
    double Nc, Ns, Nr;
    int dim[3];

    if (!VNode)
      {
      vtkErrorMacro("ComputeTkRegVox2RASMatrix: input volume node is null");
      return;
      }
    if (!M)
      {
      vtkErrorMacro("ComputeTkRegVox2RASMatrix: input matrix is null");
      return;
      }
    double *spacing = VNode->GetSpacing();
    dC = spacing[0];
    dR = spacing[1];
    dS = spacing[2];

    if (VNode->GetImageData() == nullptr)
      {
      vtkErrorMacro("ComputeTkRegVox2RASMatrix: input volume's image data is null");
      return;
      }
    VNode->GetImageData()->GetDimensions(dim);
    Nc = dim[0] * dC;
    Nr = dim[1] * dR;
    Ns = dim[2] * dS;

    M->Zero();
    M->SetElement ( 0, 0, -dC );
    M->SetElement ( 0, 3, Nc/2.0 );
    M->SetElement ( 1, 2, dS );
    M->SetElement ( 1, 3, -Ns/2.0 );
    M->SetElement ( 2, 1, -dR );
    M->SetElement ( 2, 3, Nr/2.0 );
    M->SetElement ( 3, 3, 1.0 );
}

//-------------------------------------------------------------------------
void vtkSlicerVolumesLogic::CenterVolume(vtkMRMLVolumeNode* volumeNode)
{
  if (!volumeNode || !volumeNode->GetImageData())
    {
    return;
    }
  double origin[3];
  this->GetVolumeCenteredOrigin(volumeNode, origin);
  volumeNode->SetOrigin(origin);
}

//------------------------------------------------------------------------------
void vtkSlicerVolumesLogic
::GetVolumeCenteredOrigin(vtkMRMLVolumeNode* volumeNode, double* origin)
{
  // WARNING: this code is duplicated in qMRMLVolumeInfoWidget !
  origin[0] = 0.;
  origin[1] = 0.;
  origin[2] = 0.;

  vtkImageData *imageData = volumeNode ? volumeNode->GetImageData() : nullptr;
  if (!imageData)
    {
    return;
    }

  int *dims = imageData->GetDimensions();
  double dimsH[4];
  dimsH[0] = dims[0] - 1;
  dimsH[1] = dims[1] - 1;
  dimsH[2] = dims[2] - 1;
  dimsH[3] = 0.;

  vtkNew<vtkMatrix4x4> ijkToRAS;
  volumeNode->GetIJKToRASMatrix(ijkToRAS.GetPointer());
  double rasCorner[4];
  ijkToRAS->MultiplyPoint(dimsH, rasCorner);

  origin[0] = -0.5 * rasCorner[0];
  origin[1] = -0.5 * rasCorner[1];
  origin[2] = -0.5 * rasCorner[2];
}

//-------------------------------------------------------------------------
void vtkSlicerVolumesLogic::TranslateFreeSurferRegistrationMatrixIntoSlicerRASToRASMatrix( vtkMRMLVolumeNode *V1Node,
                                                                       vtkMRMLVolumeNode *V2Node,
                                                                       vtkMatrix4x4 *FSRegistrationMatrix,
                                                                       vtkMatrix4x4 *RAS2RASMatrix)
{
  if  ( V1Node  && V2Node && FSRegistrationMatrix  && RAS2RASMatrix )
    {
    RAS2RASMatrix->Zero();

    //
    // Looking for RASv1_To_RASv2:
    //
    //---
    //
    // In Slicer:
    // [ IJKv1_To_IJKv2] = [ RAS_To_IJKv2 ]  [ RASv1_To_RASv2 ] [ IJK_To_RASv1 ] [i,j,k]transpose
    //
    // In FreeSurfer:
    // [ IJKv1_To_IJKv2] = [FStkRegVox_To_RASv2 ]inverse [ FSRegistrationMatrix] [FStkRegVox_To_RASv1 ] [ i,j,k] transpose
    //
    //----
    //
    // So:
    // [FStkRegVox_To_RASv2 ] inverse [ FSRegistrationMatrix] [FStkRegVox_To_RASv1 ] =
    // [ RAS_To_IJKv2 ]  [ RASv1_To_RASv2 ] [ IJKv1_2_RAS ]
    //
    //---
    //
    // Below use this shorthand:
    //
    // S = FStkRegVox_To_RASv2
    // T = FStkRegVox_To_RASv1
    // N = RAS_To_IJKv2
    // M = IJK_To_RASv1
    // R = FSRegistrationMatrix
    // [Sinv]  [R]  [T] = [N]  [RASv1_To_RASv2]  [M];
    //
    // So this is what we'll compute and use in Slicer instead
    // of the FreeSurfer register.dat matrix:
    //
    // [Ninv]  [Sinv]  [R]  [T]  [Minv]  = RASv1_To_RASv2
    //
    // I think we need orientation in FreeSurfer: nothing in the tkRegVox2RAS
    // handles scanOrder. The tkRegVox2RAS = IJKToRAS matrix for a coronal
    // volume. But for an Axial volume, these two matrices are different.
    // How do we compute the correct orientation for FreeSurfer Data?

    vtkNew<vtkMatrix4x4> T;
    vtkNew<vtkMatrix4x4> S;
    vtkNew<vtkMatrix4x4> Sinv;
    vtkNew<vtkMatrix4x4> M;
    vtkNew<vtkMatrix4x4> Minv;
    vtkNew<vtkMatrix4x4> N;
    vtkNew<vtkMatrix4x4> Ninv;

    //--
    // compute FreeSurfer tkRegVox2RAS for V1 volume
    //--
    ComputeTkRegVox2RASMatrix(V1Node, T.GetPointer());

    //--
    // compute FreeSurfer tkRegVox2RAS for V2 volume
    //--
    ComputeTkRegVox2RASMatrix(V2Node, S.GetPointer());

    // Probably a faster way to do these things?
    vtkMatrix4x4::Invert(S.GetPointer(), Sinv.GetPointer());
    V1Node->GetIJKToRASMatrix(M.GetPointer());
    V2Node->GetRASToIJKMatrix(N.GetPointer());
    vtkMatrix4x4::Invert(M.GetPointer(), Minv.GetPointer());
    vtkMatrix4x4::Invert(N.GetPointer(), Ninv.GetPointer());

    //    [Ninv]  [Sinv]  [R]  [T]  [Minv]
    vtkMatrix4x4::Multiply4x4(T.GetPointer(), Minv.GetPointer(), RAS2RASMatrix );
    vtkMatrix4x4::Multiply4x4(FSRegistrationMatrix, RAS2RASMatrix, RAS2RASMatrix );
    vtkMatrix4x4::Multiply4x4(Sinv.GetPointer(), RAS2RASMatrix, RAS2RASMatrix );
    vtkMatrix4x4::Multiply4x4(Ninv.GetPointer(), RAS2RASMatrix, RAS2RASMatrix );
    }
}


// Add a class to the list of registry of volume types.
// The default storage nodes for these volume types will be tested in
// order of front to back.
void
vtkSlicerVolumesLogic
::RegisterArchetypeVolumeNodeSetFactory(ArchetypeVolumeNodeSetFactory factory)
{
  NodeSetFactoryRegistry::iterator
    rit = std::find(this->VolumeRegistry.begin(), this->VolumeRegistry.end(), factory);

  if (rit == this->VolumeRegistry.end())
    {
    this->VolumeRegistry.push_back(factory);
    }
}


void
vtkSlicerVolumesLogic
::PreRegisterArchetypeVolumeNodeSetFactory(ArchetypeVolumeNodeSetFactory factory)
{
  NodeSetFactoryRegistry::iterator
    rit = std::find(this->VolumeRegistry.begin(), this->VolumeRegistry.end(), factory);

  if (rit == this->VolumeRegistry.end())
    {
    this->VolumeRegistry.push_front(factory);
    }
  else
    {
    this->VolumeRegistry.erase(rit);
    this->VolumeRegistry.push_front(factory);
    }
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*
vtkSlicerVolumesLogic
::ResampleVolumeToReferenceVolume(vtkMRMLVolumeNode* inputVolumeNode,
                                  vtkMRMLVolumeNode* referenceVolumeNode)
{
  int dimensions[3] = {0, 0, 0};

  // Make sure inputs are initialized
  if (!inputVolumeNode || !referenceVolumeNode ||
      !inputVolumeNode->GetImageData() || !referenceVolumeNode->GetImageData())
    {
    return nullptr;
    }
  vtkMRMLScene* scene = inputVolumeNode->GetScene();
  if (!scene)
    {
    return nullptr;
    }

  // Clone the input volume without setting the imageData
  vtkMRMLScalarVolumeNode* outputVolumeNode = Self::CloneVolumeWithoutImageData(scene,
                                                                                inputVolumeNode,
                                                                                inputVolumeNode->GetName());

  vtkSmartPointer<vtkGeneralTransform> outputVolumeResliceTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  outputVolumeResliceTransform->Identity();
  outputVolumeResliceTransform->PostMultiply();

  vtkSmartPointer<vtkMatrix4x4> inputVolumeIJK2RASMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  inputVolumeNode->GetIJKToRASMatrix(inputVolumeIJK2RASMatrix);
  outputVolumeResliceTransform->Concatenate(inputVolumeIJK2RASMatrix);

  vtkSmartPointer<vtkMRMLTransformNode> inputVolumeNodeTransformNode = vtkMRMLTransformNode::SafeDownCast(
    scene->GetNodeByID(inputVolumeNode->GetTransformNodeID()));
  if (inputVolumeNodeTransformNode.GetPointer() != nullptr)
    {
    vtkSmartPointer<vtkGeneralTransform> inputVolumeRAS2RAS = vtkSmartPointer<vtkGeneralTransform>::New();
    inputVolumeNodeTransformNode->GetTransformToWorld(inputVolumeRAS2RAS);
    outputVolumeResliceTransform->Concatenate(inputVolumeRAS2RAS);
    }

  vtkSmartPointer<vtkMRMLTransformNode> referenceVolumeNodeTransformNode = vtkMRMLTransformNode::SafeDownCast(
    scene->GetNodeByID(referenceVolumeNode->GetTransformNodeID()));
  if (referenceVolumeNodeTransformNode.GetPointer() != nullptr &&
      inputVolumeNodeTransformNode.GetPointer() != nullptr)
    {
    vtkSmartPointer<vtkGeneralTransform> ras2referenceVolumeRAS = vtkSmartPointer<vtkGeneralTransform>::New();
    inputVolumeNodeTransformNode->GetTransformFromWorld(ras2referenceVolumeRAS);
    outputVolumeResliceTransform->Concatenate(ras2referenceVolumeRAS);
    }

  vtkSmartPointer<vtkMatrix4x4> referenceVolumeRAS2IJKMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  referenceVolumeNode->GetRASToIJKMatrix(referenceVolumeRAS2IJKMatrix);
  outputVolumeResliceTransform->Concatenate(referenceVolumeRAS2IJKMatrix);
  outputVolumeResliceTransform->Inverse();

  vtkSmartPointer<vtkImageReslice> resliceFilter = vtkSmartPointer<vtkImageReslice>::New();
  resliceFilter->SetInputData(inputVolumeNode->GetImageData());
  resliceFilter->SetOutputOrigin(0, 0, 0);
  resliceFilter->SetOutputSpacing(1, 1, 1);
  referenceVolumeNode->GetImageData()->GetDimensions(dimensions);
  resliceFilter->SetOutputExtent(0, dimensions[0]-1, 0, dimensions[1]-1, 0, dimensions[2]-1);

  // vtkImageReslice works faster if the input is a linear transform, so try to convert it
  // to a linear transform
  vtkSmartPointer<vtkTransform> linearResliceTransform = vtkSmartPointer<vtkTransform>::New();
  if (vtkMRMLTransformNode::IsGeneralTransformLinear(outputVolumeResliceTransform, linearResliceTransform))
    {
    resliceFilter->SetResliceTransform(linearResliceTransform);
    }
  else
    {
    resliceFilter->SetResliceTransform(outputVolumeResliceTransform);
    }
  // check for a label map and adjust interpolation mode
  if (inputVolumeNode->IsA("vtkMRMLLabelMapVolumeNode"))
    {
    resliceFilter->SetInterpolationModeToNearestNeighbor();
    }
  else
    {
    resliceFilter->SetInterpolationModeToLinear();
    }
  resliceFilter->Update();

  outputVolumeNode->CopyOrientation(referenceVolumeNode);
  outputVolumeNode->SetAndObserveImageData(resliceFilter->GetOutput());

  return outputVolumeNode;
}
