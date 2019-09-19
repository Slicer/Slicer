/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// vtkSlicer includes
#include "vtkArchive.h" // note: this is not a class

// MRMLLogic includes
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLColorLogic.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceLinkLogic.h"
#include "vtkMRMLViewLogic.h"
#include "vtkMRMLViewLinkLogic.h"

// MRML includes
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLPlotViewNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLStorableNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLSceneViewNode.h"
#include "vtkMRMLTableViewNode.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPNGWriter.h>
#include <vtkSmartPointer.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>
#include <vtksys/Glob.hxx>

// STD includes
#include <cassert>
#include <sstream>

// For LoadDefaultParameterSets
#ifdef WIN32
# include <windows.h>
#else
# include <dirent.h>
# include <errno.h>
#endif

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLApplicationLogic);

//----------------------------------------------------------------------------
class vtkMRMLApplicationLogic::vtkInternal
{
public:
  vtkInternal(vtkMRMLApplicationLogic* external);
  void PropagateVolumeSelection(int layer, int fit);
  ~vtkInternal();

  vtkMRMLApplicationLogic* External;
  vtkSmartPointer<vtkMRMLSelectionNode> SelectionNode;
  vtkSmartPointer<vtkMRMLInteractionNode> InteractionNode;
  vtkSmartPointer<vtkCollection> SliceLogics;
  vtkSmartPointer<vtkCollection> ViewLogics;
  vtkSmartPointer<vtkMRMLSliceLinkLogic> SliceLinkLogic;
  vtkSmartPointer<vtkMRMLViewLinkLogic> ViewLinkLogic;
  vtkSmartPointer<vtkMRMLColorLogic> ColorLogic;
  std::string TemporaryPath;

};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::vtkInternal::vtkInternal(vtkMRMLApplicationLogic* external)
{
  this->External = external;
  this->SliceLinkLogic = vtkSmartPointer<vtkMRMLSliceLinkLogic>::New();
  this->ViewLinkLogic = vtkSmartPointer<vtkMRMLViewLinkLogic>::New();
  this->ColorLogic = vtkSmartPointer<vtkMRMLColorLogic>::New();
}

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::vtkInternal::~vtkInternal()
= default;

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::vtkInternal::PropagateVolumeSelection(int layer, int fit)
{
  if ( !this->SelectionNode || !this->External->GetMRMLScene() )
    {
    return;
    }

  char* ID = this->SelectionNode->GetActiveVolumeID();
  char* secondID = this->SelectionNode->GetSecondaryVolumeID();
  char* labelID = this->SelectionNode->GetActiveLabelVolumeID();

  vtkMRMLSliceCompositeNode* cnode;
  const int nnodes = this->External->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");

  for (int i = 0; i < nnodes; i++)
    {
    cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
      this->External->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
    if(!cnode->GetDoPropagateVolumeSelection())
      {
      continue;
      }
    if (layer & vtkMRMLApplicationLogic::BackgroundLayer)
      {
      cnode->SetBackgroundVolumeID( ID );
      }
    if (layer & vtkMRMLApplicationLogic::ForegroundLayer)
      {
      cnode->SetForegroundVolumeID( secondID );
      }
    if (layer & vtkMRMLApplicationLogic::LabelLayer)
      {
      cnode->SetLabelVolumeID( labelID );
      }
    }
  if (fit)
    {
    this->External->FitSliceToAll(true);
    }
}
//----------------------------------------------------------------------------
// vtkMRMLApplicationLogic methods

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::vtkMRMLApplicationLogic()
{
  this->Internal = new vtkInternal(this);
  this->Internal->SliceLinkLogic->SetMRMLApplicationLogic(this);
  this->Internal->ViewLinkLogic->SetMRMLApplicationLogic(this);
  this->Internal->ColorLogic->SetMRMLApplicationLogic(this);
}

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::~vtkMRMLApplicationLogic()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkMRMLSelectionNode* vtkMRMLApplicationLogic::GetSelectionNode()const
{
  return this->Internal->SelectionNode;
}

//----------------------------------------------------------------------------
vtkMRMLInteractionNode* vtkMRMLApplicationLogic::GetInteractionNode()const
{
  return this->Internal->InteractionNode;
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetColorLogic(vtkMRMLColorLogic* colorLogic)
{
  this->Internal->ColorLogic = colorLogic;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLColorLogic* vtkMRMLApplicationLogic::GetColorLogic()const
{
  return this->Internal->ColorLogic;
}

//----------------------------------------------------------------------------
vtkCollection* vtkMRMLApplicationLogic::GetSliceLogics()const
{
  return this->Internal->SliceLogics;
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetSliceLogics(vtkCollection* sliceLogics)
{
  if (sliceLogics == this->Internal->SliceLogics)
    {
    return;
    }
  this->Internal->SliceLogics = sliceLogics;
  this->Modified();
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic* vtkMRMLApplicationLogic::
GetSliceLogic(vtkMRMLSliceNode* sliceNode) const
{
  if(!sliceNode || !this->Internal->SliceLogics)
    {
    return nullptr;
    }

  vtkMRMLSliceLogic* logic = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* logics = this->Internal->SliceLogics;

  for (logics->InitTraversal(it);
      (logic=vtkMRMLSliceLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
    {
    if (logic->GetSliceNode() == sliceNode)
      {
      break;
      }

    logic = nullptr;
    }

  return logic;
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic* vtkMRMLApplicationLogic::
GetSliceLogicByLayoutName(const char* layoutName) const
{
  if(!layoutName || !this->Internal->SliceLogics)
    {
    return nullptr;
    }

  vtkMRMLSliceLogic* logic = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* logics = this->Internal->SliceLogics;

  for (logics->InitTraversal(it);
      (logic=vtkMRMLSliceLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
    {
    if (logic->GetSliceNode())
      {
      if ( !strcmp( logic->GetSliceNode()->GetLayoutName(), layoutName) )
        {
        return logic;
        }
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic* vtkMRMLApplicationLogic::
GetSliceLogicByModelDisplayNode(vtkMRMLModelDisplayNode* displayNode) const
{
  if (!displayNode || !this->Internal->SliceLogics)
    {
    return nullptr;
    }

  vtkMRMLSliceLogic* logic = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* logics = this->Internal->SliceLogics;

  for (logics->InitTraversal(it);
    (logic = vtkMRMLSliceLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
    {
    if (logic->GetSliceModelDisplayNode() == displayNode)
      {
      return logic;
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetViewLogics(vtkCollection* viewLogics)
{
  if (viewLogics == this->Internal->ViewLogics)
    {
    return;
    }
  this->Internal->ViewLogics = viewLogics;
  this->Modified();
}

//---------------------------------------------------------------------------
vtkCollection* vtkMRMLApplicationLogic::GetViewLogics() const
{
  return this->Internal->ViewLogics;
}

//---------------------------------------------------------------------------
vtkMRMLViewLogic* vtkMRMLApplicationLogic::
GetViewLogic(vtkMRMLViewNode* viewNode) const
{
  if(!viewNode || !this->Internal->ViewLogics)
    {
    return nullptr;
    }

  vtkMRMLViewLogic* logic = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* logics = this->Internal->ViewLogics;

  for (logics->InitTraversal(it);
      (logic=vtkMRMLViewLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
    {
    if (logic->GetViewNode() == viewNode)
      {
      break;
      }

    logic = nullptr;
    }

  return logic;
}

//---------------------------------------------------------------------------
vtkMRMLViewLogic* vtkMRMLApplicationLogic::
GetViewLogicByLayoutName(const char* layoutName) const
{
  if(!layoutName || !this->Internal->ViewLogics)
    {
    return nullptr;
    }

  vtkMRMLViewLogic* logic = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* logics = this->Internal->ViewLogics;

  for (logics->InitTraversal(it);
      (logic=vtkMRMLViewLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
    {
    if (logic->GetViewNode())
      {
      if ( !strcmp( logic->GetViewNode()->GetLayoutName(), layoutName) )
        {
        return logic;
        }
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkMRMLNode* selectionNode = nullptr;
  if (newScene)
    {
    // Selection Node
    selectionNode = newScene->GetNodeByID("vtkMRMLSelectionNodeSingleton");
    if (!selectionNode)
      {
      selectionNode = newScene->AddNode(vtkNew<vtkMRMLSelectionNode>().GetPointer());
      }
    assert(vtkMRMLSelectionNode::SafeDownCast(selectionNode));
    }
  this->SetSelectionNode(vtkMRMLSelectionNode::SafeDownCast(selectionNode));

  vtkMRMLNode* interactionNode = nullptr;
  if (newScene)
    {
    // Interaction Node
    interactionNode = newScene->GetNodeByID("vtkMRMLInteractionNodeSingleton");
    if (!interactionNode)
      {
      interactionNode = newScene->AddNode(vtkNew<vtkMRMLInteractionNode>().GetPointer());
      }
    assert(vtkMRMLInteractionNode::SafeDownCast(interactionNode));
    }
  this->SetInteractionNode(vtkMRMLInteractionNode::SafeDownCast(interactionNode));

  // Add default slice orientation presets
  vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(newScene);

  this->Superclass::SetMRMLSceneInternal(newScene);

  this->Internal->SliceLinkLogic->SetMRMLScene(newScene);
  this->Internal->ViewLinkLogic->SetMRMLScene(newScene);
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetSelectionNode(vtkMRMLSelectionNode* selectionNode)
{
  if (selectionNode == this->Internal->SelectionNode)
    {
    return;
    }
  this->Internal->SelectionNode = selectionNode;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetInteractionNode(vtkMRMLInteractionNode* interactionNode)
{
  if (interactionNode == this->Internal->InteractionNode)
    {
    return;
    }
  this->Internal->InteractionNode = interactionNode;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::PropagateVolumeSelection(int fit)
{
  this->PropagateVolumeSelection(
        BackgroundLayer | ForegroundLayer | LabelLayer, fit);
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::PropagateBackgroundVolumeSelection(int fit)
{
  this->PropagateVolumeSelection(BackgroundLayer, fit);
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::PropagateForegroundVolumeSelection(int fit)
{
  this->PropagateVolumeSelection(ForegroundLayer, fit);
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::PropagateLabelVolumeSelection(int fit)
{
  this->PropagateVolumeSelection(LabelLayer, fit);
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::PropagateVolumeSelection(int layer, int fit)
{
  this->Internal->PropagateVolumeSelection(layer, fit);
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::PropagateTableSelection()
{
  if ( !this->Internal->SelectionNode || !this->GetMRMLScene() )
    {
    return;
    }

  char* tableId = this->Internal->SelectionNode->GetActiveTableID();

  const int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLTableViewNode");
  for (int i = 0; i < nnodes; i++)
    {
    vtkMRMLTableViewNode* tnode = vtkMRMLTableViewNode::SafeDownCast (
      this->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLTableViewNode" ) );
    if(!tnode->GetDoPropagateTableSelection())
      {
      continue;
      }
    tnode->SetTableNodeID( tableId );
  }
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::PropagatePlotChartSelection()
{
  if ( !this->Internal->SelectionNode || !this->GetMRMLScene() )
    {
    return;
    }

  char* PlotChartId = this->Internal->SelectionNode->GetActivePlotChartID();

  const int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLPlotViewNode");
  for (int i = 0; i < nnodes; i++)
    {
    vtkMRMLPlotViewNode* pnode = vtkMRMLPlotViewNode::SafeDownCast (
      this->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLPlotViewNode" ) );
    if(!pnode->GetDoPropagatePlotChartSelection())
      {
      continue;
      }
    pnode->SetPlotChartNodeID(PlotChartId);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::FitSliceToAll(bool onlyIfPropagateVolumeSelectionAllowed /* =false */)
{
  if (this->Internal->SliceLogics.GetPointer() == nullptr)
    {
    return;
    }
  vtkMRMLSliceLogic* sliceLogic = nullptr;
  vtkCollectionSimpleIterator it;
  for(this->Internal->SliceLogics->InitTraversal(it);
      (sliceLogic = vtkMRMLSliceLogic::SafeDownCast(
        this->Internal->SliceLogics->GetNextItemAsObject(it)));)
    {
    if (onlyIfPropagateVolumeSelectionAllowed)
      {
      vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
      if (sliceCompositeNode!=nullptr && !sliceCompositeNode->GetDoPropagateVolumeSelection())
        {
        // propagate volume selection is disabled, skip this slice
        continue;
        }
      }
    vtkMRMLSliceNode* sliceNode = sliceLogic->GetSliceNode();
    int* dims = sliceNode->GetDimensions();
    sliceLogic->FitSliceToAll(dims[0], dims[1]);
    sliceLogic->SnapSliceOffsetToIJK();
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLApplicationLogic::Zip(const char* zipFileName, const char* directoryToZip)
{
  // call function in vtkArchive
  return zip(zipFileName, directoryToZip);
}

//----------------------------------------------------------------------------
bool vtkMRMLApplicationLogic::Unzip(const char* zipFileName, const char* destinationDirectory)
{
  // call function in vtkArchive
  return unzip(zipFileName, destinationDirectory);
}

//----------------------------------------------------------------------------
std::string vtkMRMLApplicationLogic::UnpackSlicerDataBundle(const char *sdbFilePath, const char *temporaryDirectory)
{
  if ( !this->Unzip(sdbFilePath, temporaryDirectory) )
    {
    vtkErrorMacro("could not open bundle file");
    return "";
    }

  vtksys::Glob glob;
  glob.RecurseOn();
  glob.RecurseThroughSymlinksOff();
  std::string globPattern(temporaryDirectory);
  if ( !glob.FindFiles( globPattern + "/*.mrml" ) )
    {
    vtkErrorMacro("could not search archive");
    return "";
    }
  std::vector<std::string> files = glob.GetFiles();
  if ( files.size() <= 0 )
    {
    vtkErrorMacro("could not find mrml file in archive");
    return "";
    }

  return( files[0] );
}

//----------------------------------------------------------------------------
bool vtkMRMLApplicationLogic::OpenSlicerDataBundle(const char* sdbFilePath, const char* temporaryDirectory)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("no scene");
    return false;
    }

  std::string mrmlFile = this->UnpackSlicerDataBundle(sdbFilePath, temporaryDirectory);

  if ( mrmlFile.empty() )
    {
    vtkErrorMacro("Could not unpack mrml scene");
    return false;
    }

  this->GetMRMLScene()->SetURL( mrmlFile.c_str() );
  int success = this->GetMRMLScene()->Connect();
  if ( !success )
    {
    vtkErrorMacro("Could not connect to scene");
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
std::string vtkMRMLApplicationLogic::PercentEncode(std::string s)
{
  std::string validchars = " -_.,@#$%^&()[]{}<>+=";
  std::ostringstream result;

  for (size_t i = 0; i < s.size(); i++)
    {
    if ( (s[i] >= 'A' && s[i] <= 'z')
          ||
         (s[i] >= '0'&& s[i] <= '9')
          ||
         (validchars.find(s[i]) != std::string::npos) )
      {
        result << s[i];
      }
    else
      {
        result << '%' << std::hex << (unsigned short) s[i] << std::dec;
      }
    }
  return result.str();
}

//----------------------------------------------------------------------------
bool vtkMRMLApplicationLogic::SaveSceneToSlicerDataBundleDirectory(const char* sdbDir, vtkImageData* screenShot)
{

  // Overview:
  // - confirm the arguments are valid and create directories if needed
  // - save all current file storage paths in the scene
  // - replace all file storage folders by sdbDir/Data
  // - create a screenshot of the scene (for allowing preview of scene content without opening in Slicer)
  // - save the scene (mrml files and all storable nodes)
  // - revert all file storage paths to the original
  //
  // At the end, the scene should be restored to its original state
  // except that some storables will have default storage nodes.

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory failed: invalid scene");
    return false;
    }
  if (!sdbDir)
    {
    vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory failed: invalid subdirectory");
    return false;
    }

  // if the path to the directory is not absolute, return
  if (!vtksys::SystemTools::FileIsFullPath(sdbDir))
    {
    vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory failed: given directory is not a full path: " << sdbDir);
    return false;
    }
  // is it a directory?
  if (!vtksys::SystemTools::FileIsDirectory(sdbDir))
    {
    vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory failed: given directory name is not actually a directory" << sdbDir);
    return false;
    }
  std::string rootDir = std::string(sdbDir);
  vtkDebugMacro("Using root dir of " << rootDir);
  // need the components to build file names
  std::vector<std::string> rootPathComponents;
  vtksys::SystemTools::SplitPath(rootDir.c_str(), rootPathComponents);

  // remove the directory if it does exist
  if (vtksys::SystemTools::FileExists(rootDir.c_str(), false))
    {
    if (!vtksys::SystemTools::RemoveADirectory(rootDir.c_str()))
      {
      vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory failed: Error removing SDB scene directory " << rootDir.c_str() << ", cannot make a fresh archive.");
      return false;
      }
    }
  // create the SDB directory
  if (!vtksys::SystemTools::FileExists(rootDir.c_str(), false))
    {
    if (!vtksys::SystemTools::MakeDirectory(rootDir.c_str()))
      {
      vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory failed: Unable to make temporary directory " << rootDir);
      return false;
      }
    }

  //
  // now, replace paths with data bundle paths, saving the original values
  //

  // the root directory
  std::string origURL( this->GetMRMLScene()->GetURL() );
  std::string origRootDirectory( this->GetMRMLScene()->GetRootDirectory() );

  // the new url of the mrml scene
  std::string urlStr = vtksys::SystemTools::GetFilenameWithoutExtension(rootDir.c_str()) + std::string(".mrml");
  rootPathComponents.push_back(urlStr);
  urlStr =  vtksys::SystemTools::JoinPath(rootPathComponents);
  rootPathComponents.pop_back();
  vtkDebugMacro("set new scene url to " << this->GetMRMLScene()->GetURL());

  // the new data directory
  std::vector<std::string> pathComponents;
  vtksys::SystemTools::SplitPath(rootDir.c_str(), pathComponents);
  pathComponents.push_back("Data");
  std::string dataDir =  vtksys::SystemTools::JoinPath(pathComponents);
  vtkDebugMacro("using data dir of " << dataDir);

  // create the data dir
  if (!vtksys::SystemTools::FileExists(dataDir.c_str()))
    {
    if (!vtksys::SystemTools::MakeDirectory(dataDir.c_str()))
      {
      vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory failed: Unable to make data directory " << dataDir);
      return false;
      }
    }

  //
  // start changing the scene - don't return from below here
  // until scene has been restored to original state
  //
  this->GetMRMLScene()->SetRootDirectory(rootDir.c_str());
  this->GetMRMLScene()->SetURL(urlStr.c_str());

  if (screenShot)
    {
    this->SaveSceneScreenshot(screenShot);
    }

  // change all storage nodes and file names to be unique in the new directory
  // write the new data as we go; save old values
  this->OriginalStorageNodeFileNames.clear();

  std::map<std::string, vtkMRMLNode *> storableNodes;

  int numNodes = this->GetMRMLScene()->GetNumberOfNodes();
  for (int i = 0; i < numNodes; ++i)
    {
    vtkMRMLNode* mrmlNode = this->GetMRMLScene()->GetNthNode(i);
    if (!mrmlNode)
      {
      vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory: unable to get " << i << "th node from scene with " << numNodes << " nodes");
      continue;
      }
    if (mrmlNode->IsA("vtkMRMLStorableNode"))
      {
      // get all storable nodes in the main scene
      // and store them in the map by ID to avoid duplicates for the scene views
      vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(mrmlNode);

      this->SaveStorableNodeToSlicerDataBundleDirectory(storableNode, dataDir);

      storableNodes[std::string(storableNode->GetID())] = storableNode;
      }
    }
  // Update all storage nodes in all scene views.
  // Nodes that are not present in the main scene are actually saved to file, others just have their paths updated.
  for (int i = 0; i < numNodes; ++i)
    {
    vtkMRMLSceneViewNode* sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNthNode(i));
    if (!sceneViewNode)
      {
      continue;
      }
    sceneViewNode->SetSceneViewRootDir(this->GetMRMLScene()->GetRootDirectory());

    std::vector<vtkMRMLNode *> snodes;
    sceneViewNode->GetNodesByClass("vtkMRMLStorableNode", snodes);
    std::vector<vtkMRMLNode *>::iterator sit;
    for (sit = snodes.begin(); sit != snodes.end(); sit++)
      {
      vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(*sit);
      std::map<std::string, vtkMRMLNode *>::iterator storableNodeIt = storableNodes.find(std::string(storableNode->GetID()));
      if (storableNodeIt == storableNodes.end())
        {
        // this storable node has been deleted from the main scene: save it
        storableNode->SetAddToScene(1);
        storableNode->UpdateScene(this->GetMRMLScene());
        this->SaveStorableNodeToSlicerDataBundleDirectory(storableNode, dataDir);
        storableNodes[std::string(storableNode->GetID())] = storableNode;
        storableNode->SetAddToScene(0);
        }
      else
        {
        // this storable node is still in the main scene, just save and update the path
        vtkMRMLStorageNode* storageNodeInMainScene = vtkMRMLStorableNode::SafeDownCast(storableNodeIt->second)->GetStorageNode();
        vtkMRMLStorageNode* storageNode = storableNode->GetStorageNode();
        if (storageNode)
          {
          // Save original file names
          this->OriginalStorageNodeFileNames[storageNode].push_back(storageNode->GetFileName() ? storageNode->GetFileName() : "");
          for (int i = 0; i < storageNode->GetNumberOfFileNames(); ++i)
            {
            this->OriginalStorageNodeFileNames[storageNode].push_back(storageNode->GetNthFileName(i) ? storageNode->GetNthFileName(i) : "");
            }
          // Update file names (to match the file names in the main scene)
          if (storageNodeInMainScene)
            {
            storageNode->SetFileName(storageNodeInMainScene->GetFileName());
            storageNode->ResetFileNameList();
            for (int i = 0; i < storageNodeInMainScene->GetNumberOfFileNames(); ++i)
              {
              storageNode->AddFileName(storageNodeInMainScene->GetNthFileName(i));
              }
            }
          }
        }
      }
    }

  // write the scene to disk, changes paths to relative
  vtkDebugMacro("calling commit on the scene, to url " << this->GetMRMLScene()->GetURL());
  this->GetMRMLScene()->Commit();

  //
  // Now, restore the state of the scene
  //

  this->GetMRMLScene()->SetURL(origURL.c_str());
  this->GetMRMLScene()->SetRootDirectory(origRootDirectory.c_str());

  // reset the storage paths
  numNodes = this->GetMRMLScene()->GetNumberOfNodes();
  for (int i = 0; i < numNodes; ++i)
    {
    vtkMRMLNode* mrmlNode = this->GetMRMLScene()->GetNthNode(i);
    if (!mrmlNode)
      {
      vtkErrorMacro("unable to get " << i << "th node from scene with " << numNodes << " nodes");
      continue;
      }
    if (mrmlNode->IsA("vtkMRMLSceneViewNode"))
      {
      vtkMRMLSceneViewNode* sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(mrmlNode);
      sceneViewNode->GetScene()->SetURL(origURL.c_str());
      sceneViewNode->SetSceneViewRootDir(origRootDirectory.c_str());

      // get all additional storable nodes for all scene views
      std::vector<vtkMRMLNode *> snodes;
      sceneViewNode->GetNodesByClass("vtkMRMLStorableNode", snodes);
      std::vector<vtkMRMLNode *>::iterator sit;
      for (sit = snodes.begin(); sit != snodes.end(); sit++)
        {
        vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(*sit);
        vtkMRMLStorageNode* storageNode = storableNode->GetStorageNode();

        if (storageNode && this->OriginalStorageNodeFileNames.find( storageNode ) != this->OriginalStorageNodeFileNames.end() )
          {
          storageNode->ResetFileNameList();
          std::vector< std::string > &originalFileNames = this->OriginalStorageNodeFileNames[storageNode];
          for (size_t index = 0; index < originalFileNames.size(); index++)
            {
            if (index == 0)
              {
              storageNode->SetFileName(originalFileNames[index].c_str());
              }
            else
              {
              storageNode->AddFileName(originalFileNames[index].c_str());
              }
            }
          }
        }
      }
    if (mrmlNode->IsA("vtkMRMLStorableNode"))
      {
      vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(mrmlNode);
      vtkMRMLStorageNode* storageNode = storableNode->GetStorageNode();
      if (storageNode && this->OriginalStorageNodeFileNames.find( storageNode ) != this->OriginalStorageNodeFileNames.end() )
        {
        // std::cout << "Resetting filename on storage node " << storageNode->GetID() << " from " << storageNode->GetFileName() << " back to " << this->OriginalStorageNodeFileNames[storageNode][0].c_str() << "\n\tmodified since read = " << storableNode->GetModifiedSinceRead() << std::endl;

        storageNode->ResetFileNameList();
        std::vector< std::string > &originalFileNames = this->OriginalStorageNodeFileNames[storageNode];
        for (size_t index = 0; index < originalFileNames.size(); index++)
          {
          if (index == 0)
            {
            storageNode->SetFileName(originalFileNames[index].c_str());
            }
          else
            {
            storageNode->AddFileName(originalFileNames[index].c_str());
            }
          }
        }
      }
    }

  this->GetMRMLScene()->SetURL(origURL.c_str());
  this->GetMRMLScene()->SetRootDirectory(origRootDirectory.c_str());

  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SaveStorableNodeToSlicerDataBundleDirectory(vtkMRMLStorableNode* storableNode,
                                                                          std::string &dataDir)
{
  if (!storableNode || !storableNode->GetSaveWithScene())
    {
    return;
    }
  // adjust the file paths for storable nodes
  vtkMRMLStorageNode* storageNode = storableNode->GetStorageNode();
  if (!storageNode)
    {
    vtkDebugMacro("creating a new storage node for " << storableNode->GetID());
    storableNode->AddDefaultStorageNode();
    storageNode = storableNode->GetStorageNode();
    if (!storageNode)
      {
      // no need for storage node to store this node
      return;
      }
    }

  // Save the original storage filenames (absolute paths) into this->OriginalStorageNodeFileNames
  std::string fileName(storageNode->GetFileName()?storageNode->GetFileName():"");
  this->OriginalStorageNodeFileNames[storageNode].push_back(fileName);
  for (int i = 0; i < storageNode->GetNumberOfFileNames(); ++i)
    {
    this->OriginalStorageNodeFileNames[storageNode].push_back(storageNode->GetNthFileName(i) ? storageNode->GetNthFileName(i) : "");
    }

  // Clear out the additional file list since it's assumed that the default write format needs only a single file
  // (if more files are needed then storage node must generate appropriate additional file names based on the primary file name).
  storageNode->ResetFileNameList();

  // Update primary file name (set name from node name if empty, encode special characters, use default file extension)
  if (fileName.empty())
    {
    // Default storage node usually has empty file name (if Save dialog is not opened yet)
    // file name is encoded to handle : or / characters in the node names
    std::string fileBaseName = this->PercentEncode(std::string(storableNode->GetName()));
    std::string extension = storageNode->GetDefaultWriteFileExtension();
    std::string storageFileName = fileBaseName + std::string(".") + extension;
    vtkDebugMacro("new file name = " << storageFileName.c_str());
    storageNode->SetFileName(storageFileName.c_str());
    }
  else
    {
    // new file name is encoded to handle : or / characters in the node names
    std::string storageFileName = this->PercentEncode(vtksys::SystemTools::GetFilenameName(fileName));
    std::string defaultWriteExtension = std::string(".") + vtksys::SystemTools::LowerCase(storageNode->GetDefaultWriteFileExtension());
    std::string currentExtension = storageNode->GetSupportedFileExtension(storageFileName.c_str());
    if (defaultWriteExtension != currentExtension)
      {
      // for saving to MRB all nodes will be written in their default format
      storageFileName = storageNode->GetFileNameWithoutExtension(storageFileName.c_str()) + defaultWriteExtension;
      }
    vtkDebugMacro("updated file name = " << storageFileName.c_str());
    storageNode->SetFileName(storageFileName.c_str());
    }

  storageNode->SetDataDirectory(dataDir.c_str());
  vtkDebugMacro("Set data directory to " << dataDir.c_str() << ". Storable node " << storableNode->GetID()
    << " file name is now: " << storageNode->GetFileName());

  // Make sure the filename is unique (default filenames may be the same if for example there are multiple
  // nodes with the same name).
  std::string existingFileName = (storageNode->GetFileName() ? storageNode->GetFileName() : "");
  if (vtksys::SystemTools::FileExists(existingFileName, true))
    {
    std::string currentExtension = storageNode->GetSupportedFileExtension(existingFileName.c_str());
    std::string uniqueFileName = this->CreateUniqueFileName(existingFileName, currentExtension);
    vtkDebugMacro("file " << existingFileName << " already exists, use " << uniqueFileName << " filename instead");
    storageNode->SetFileName(uniqueFileName.c_str());
    }

  storageNode->WriteData(storableNode);
 }

//----------------------------------------------------------------------------
std::string vtkMRMLApplicationLogic::CreateUniqueFileName(const std::string &filename, const std::string& knownExtension)
{
  if (!vtksys::SystemTools::FileExists(filename.c_str()))
    {
    // filename is unique already
    return filename;
    }

  std::string extension = knownExtension;
  if (extension.empty())
    {
    // if there is no information about the file extension then we
    // assume it is the last extension
    extension = vtksys::SystemTools::GetFilenameLastExtension(filename);
    }

  std::string baseName = filename.substr(0, filename.size()-extension.size());

  // If there is a numeric suffix, separated by underscore (somefile_23)
  // then use the string before the separator (somefile) as basename and increment the suffix value.
  int suffix = 0;

  std::size_t filenameStartPosition1 = baseName.find_last_of("/");
  std::size_t filenameStartPosition2 = baseName.find_last_of("\\");
  std::size_t filenameStartPosition = 0;
  if (filenameStartPosition1 != std::string::npos && filenameStartPosition < filenameStartPosition1)
    {
    filenameStartPosition = filenameStartPosition1;
    }
  if (filenameStartPosition2 != std::string::npos && filenameStartPosition < filenameStartPosition2)
    {
    filenameStartPosition = filenameStartPosition2;
    }

  std::size_t separatorPosition = baseName.find_last_of("_");
  if (separatorPosition != std::string::npos && separatorPosition > filenameStartPosition)
    {
    std::string suffixStr = baseName.substr(separatorPosition + 1, baseName.size() - separatorPosition - 1);
    std::stringstream ss(suffixStr);
    if (!(ss >> suffix).fail())
      {
      // numeric suffix found successfully
      // remove the suffix from the base name
      baseName = baseName.substr(0, separatorPosition);
      }
    }

  std::string uniqueFilename;
  while (true)
    {
    ++suffix;
    std::stringstream ss;
    ss << baseName << "_" << suffix << extension;
    uniqueFilename = ss.str();
    if (!vtksys::SystemTools::FileExists(uniqueFilename))
      {
      // found unique filename
      break;
      }
    }
  return uniqueFilename;
}

//----------------------------------------------------------------------------
int vtkMRMLApplicationLogic::LoadDefaultParameterSets(vtkMRMLScene* scene,
                                                      const std::vector<std::string>& directories)
{

  // build up the vector
  std::vector<std::string> filesVector;
  std::vector<std::string> filesToLoad;
  //filesVector.push_back(""); // for relative path

// Didn't port this next block of code yet.  Would need to add a
//   UserParameterSetsPath to the object and some window
//
//   // add the list of dirs set from the application
//   if (this->UserColorFilePaths != nullptr)
//     {
//     vtkDebugMacro("\nFindColorFiles: got user color file paths = " << this->UserColorFilePaths);
//     // parse out the list, breaking at delimiter strings
// #ifdef WIN32
//     const char *delim = ";";
// #else
//     const char *delim = ":";
// #endif
//     char *ptr = strtok(this->UserColorFilePaths, delim);
//     while (ptr != nullptr)
//       {
//       std::string dir = std::string(ptr);
//       vtkDebugMacro("\nFindColorFiles: Adding user dir " << dir.c_str() << " to the directories to check");
//       DirectoriesToCheck.push_back(dir);
//       ptr = strtok(nullptr, delim);
//       }
//     } else { vtkDebugMacro("\nFindColorFiles: oops, the user color file paths aren't set!"); }


  // Get the list of parameter sets in these dir
  for (unsigned int d = 0; d < directories.size(); d++)
    {
    std::string dirString = directories[d];
    //vtkDebugMacro("\nLoadDefaultParameterSets: checking for parameter sets in dir " << d << " = " << dirString.c_str());

    filesVector.clear();
    filesVector.push_back(dirString);
    filesVector.push_back(std::string("/"));

#ifdef WIN32
    WIN32_FIND_DATA findData;
    HANDLE fileHandle;
    int flag = 1;
    std::string search ("*.*");
    dirString += "/";
    search = dirString + search;

    fileHandle = FindFirstFile(search.c_str(), &findData);
    if (fileHandle != INVALID_HANDLE_VALUE)
      {
      while (flag)
        {
        // add this file to the vector holding the base dir name so check the
        // file type using the full path
        filesVector.push_back(std::string(findData.cFileName));
#else
    DIR* dp;
    struct dirent* dirp;
    if ((dp  = opendir(dirString.c_str())) == nullptr)
      {
      vtkGenericWarningMacro("Error(" << errno << ") opening " << dirString.c_str());
      }
    else
      {
      while ((dirp = readdir(dp)) != nullptr)
        {
        // add this file to the vector holding the base dir name
        filesVector.push_back(std::string(dirp->d_name));
#endif

        std::string fileToCheck = vtksys::SystemTools::JoinPath(filesVector);
        int fileType = vtksys::SystemTools::DetectFileType(fileToCheck.c_str());
        if (fileType == vtksys::SystemTools::FileTypeText)
          {
          //vtkDebugMacro("\nAdding " << fileToCheck.c_str() << " to list of potential parameter sets. Type = " << fileType);
          filesToLoad.push_back(fileToCheck);
          }
        else
          {
          //vtkDebugMacro("\nSkipping potential parameter set " << fileToCheck.c_str() << ", file type = " << fileType);
          }
        // take this file off so that can build the next file name
        filesVector.pop_back();

#ifdef WIN32
        flag = FindNextFile(fileHandle, &findData);
        } // end of while flag
      FindClose(fileHandle);
      } // end of having a valid fileHandle
#else
        } // end of while loop over reading the directory entries
      closedir(dp);
      } // end of able to open dir
#endif

    } // end of looping over dirs

  // Save the URL and root directory of the scene so it can
  // be restored after loading presets
  std::string url = scene->GetURL();
  std::string rootdir = scene->GetRootDirectory();

  // Finally, load each of the parameter sets
  std::vector<std::string>::iterator fit;
  for (fit = filesToLoad.begin(); fit != filesToLoad.end(); ++fit)
    {
    scene->SetURL( (*fit).c_str() );
    scene->Import();
    }

  // restore URL and root dir
  scene->SetURL(url.c_str());
  scene->SetRootDirectory(rootdir.c_str());

  return static_cast<int>(filesToLoad.size());
}

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic
::InvokeRequest::InvokeRequest()
  : Delay (100)
  , Caller(nullptr)
  , EventID(vtkCommand::ModifiedEvent)
  , CallData(nullptr)
{
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic
::InvokeEventWithDelay(unsigned int delayInMs, vtkObject* caller, unsigned long eventID,
              void* callData)
{
  InvokeRequest request;
  request.Delay = delayInMs;
  request.Caller = caller;
  request.EventID = eventID;
  request.CallData = callData;
  this->InvokeEvent(vtkMRMLApplicationLogic::RequestInvokeEvent, &request);
}

//----------------------------------------------------------------------------
const char* vtkMRMLApplicationLogic::GetTemporaryPath()
{
  return this->Internal->TemporaryPath.c_str();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetTemporaryPath(const char* path)
{
    if (path == nullptr)
      {
      this->Internal->TemporaryPath.clear();
      }
    else if (this->Internal->TemporaryPath == std::string(path))
      {
      return;
      }
    else
      {
      this->Internal->TemporaryPath = std::string(path);
      }
    this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SaveSceneScreenshot(vtkImageData* screenshot)
{
  if (this->GetMRMLScene() == nullptr || this->GetMRMLScene()->GetURL() == nullptr)
  {
    vtkErrorMacro("vtkMRMLApplicationLogic::SaveSceneScreenshot failed: invalid scene or URL");
    return;
  }
  std::string screenshotFilePath = this->GetMRMLScene()->GetURL();

  // Write screenshot with the same name and folder as the mrml file but with .png extension

  // Strip file extension (.mrml)
  std::string::size_type dot_pos = screenshotFilePath.rfind('.');
  if (dot_pos != std::string::npos)
    {
    screenshotFilePath = screenshotFilePath.substr(0, dot_pos);
    }

  screenshotFilePath += std::string(".png");

  vtkNew<vtkPNGWriter> screenShotWriter;
  screenShotWriter->SetInputData(screenshot);
  screenShotWriter->SetFileName(screenshotFilePath.c_str());
  screenShotWriter->Write();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::PauseRender()
{
  // Observers in qSlicerCoreApplication listen for PauseRenderEvent and call pauseRender
  // to temporarily stop rendering in all views in the current layout
  this->InvokeEvent(PauseRenderEvent);
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::ResumeRender()
{
  // Observers in qSlicerCoreApplication listen for ResumeRenderEvent and call resumeRender
  // to resume rendering in all views in the current layout
  this->InvokeEvent(ResumeRenderEvent);
}
