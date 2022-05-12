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
#include "vtkMRMLSliceDisplayNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLStorableNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLSceneViewNode.h"
#include "vtkMRMLTableViewNode.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>
#include <vtksys/Glob.hxx>

// STD includes
#include <cassert>
#include <map>
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
  std::map<std::string, vtkWeakPointer<vtkMRMLAbstractLogic> > ModuleLogicMap;
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
vtkMRMLApplicationLogic::vtkInternal::~vtkInternal() = default;

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::vtkInternal::PropagateVolumeSelection(int layer, int fit)
{
  if ( !this->SelectionNode || !this->External->GetMRMLScene() )
    {
    return;
    }

  const char* ID = this->SelectionNode->GetActiveVolumeID();
  const char* secondID = this->SelectionNode->GetSecondaryVolumeID();
  const char* labelID = this->SelectionNode->GetActiveLabelVolumeID();

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

  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::StartBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::StartCloseEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndCloseEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::StartImportEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndImportEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::StartRestoreEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndRestoreEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents.GetPointer());

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

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLInteractionNode::EditNodeEvent);
  events->InsertNextValue(vtkMRMLInteractionNode::ShowViewContextMenuEvent);
  vtkSetAndObserveMRMLNodeEventsMacro(this->Internal->InteractionNode, interactionNode, events);
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

  const char* tableId = this->Internal->SelectionNode->GetActiveTableID();

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

  const char* PlotChartId = this->Internal->SelectionNode->GetActivePlotChartID();

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
void vtkMRMLApplicationLogic::FitSliceToAll(bool onlyIfPropagateVolumeSelectionAllowed /* =false */, bool resetOrientation /* =true */)
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
    if (resetOrientation)
      {
      // Set to default orientation before rotation so that the view is snapped
      // closest to the default orientation of this slice view.
      sliceNode->SetOrientationToDefault();
      sliceLogic->RotateSliceToLowestVolumeAxes(false);
      }
    int* dims = sliceNode->GetDimensions();
    sliceLogic->FitSliceToAll(dims[0], dims[1]);
    sliceLogic->SnapSliceOffsetToIJK();
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLApplicationLogic::Zip(const char* zipFileName, const char* directoryToZip)
{
  return vtkArchive::Zip(zipFileName, directoryToZip);
}

//----------------------------------------------------------------------------
bool vtkMRMLApplicationLogic::Unzip(const char* zipFileName, const char* destinationDirectory)
{
  return vtkArchive::UnZip(zipFileName, destinationDirectory);
}

//----------------------------------------------------------------------------
std::string vtkMRMLApplicationLogic::UnpackSlicerDataBundle(const char *sdbFilePath, const char *temporaryDirectory)
{
  return vtkMRMLScene::UnpackSlicerDataBundle(sdbFilePath, temporaryDirectory);
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
  return vtkMRMLScene::PercentEncode(s);
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
  return this->GetMRMLScene()->SaveSceneToSlicerDataBundleDirectory(sdbDir, screenShot);
}

//----------------------------------------------------------------------------
std::string vtkMRMLApplicationLogic::CreateUniqueFileName(const std::string &filename, const std::string& knownExtension)
{
  return vtkMRMLScene::CreateUniqueFileName(filename, knownExtension);
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
    filesVector.emplace_back("/");

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
        filesVector.emplace_back(dirp->d_name);
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
::InvokeRequest::InvokeRequest() = default;

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
  if (this->GetMRMLScene() == nullptr)
    {
    vtkErrorMacro("vtkMRMLApplicationLogic::SaveSceneScreenshot failed: invalid scene or URL");
    return;
    }
  this->GetMRMLScene()->SaveSceneScreenshot(screenshot);
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

//---------------------------------------------------------------------------
void vtkMRMLApplicationLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  if (vtkMRMLInteractionNode::SafeDownCast(caller) && event == vtkMRMLInteractionNode::EditNodeEvent)
    {
    if (callData != nullptr)
      {
      vtkMRMLNode* nodeToBeEdited = reinterpret_cast<vtkMRMLNode*>(callData);
      this->EditNode(nodeToBeEdited);
      }
    }
  else if (vtkMRMLInteractionNode::SafeDownCast(caller) && event == vtkMRMLInteractionNode::ShowViewContextMenuEvent)
    {
    this->InvokeEvent(ShowViewContextMenuEvent, callData);
    }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::EditNode(vtkMRMLNode* node)
{
  // Observers in qSlicerCoreApplication listen for this event
  this->InvokeEvent(EditNodeEvent, node);
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetModuleLogic(const char* moduleName,
                                             vtkMRMLAbstractLogic* moduleLogic)
{
  if (!moduleName)
    {
    vtkErrorMacro("AddModuleLogic: invalid module name.");
    return;
    }
  if (moduleLogic)
    {
    this->Internal->ModuleLogicMap[moduleName] = moduleLogic;
    }
  else
    {
    // If no logic is provided, erase the module-logic association.
    this->Internal->ModuleLogicMap.erase(moduleName);
    }
}

//----------------------------------------------------------------------------
vtkMRMLAbstractLogic* vtkMRMLApplicationLogic::GetModuleLogic(const char* moduleName) const
{
  if (!moduleName)
    {
    vtkErrorMacro("GetModuleLogic: invalid module name");
    return nullptr;
    }
  //Check that the logic is registered.
  if (this->Internal->ModuleLogicMap.count(moduleName) == 0)
    {
    return nullptr;
    }
  return this->Internal->ModuleLogicMap[moduleName];
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::OnMRMLSceneStartBatchProcess()
{
  this->PauseRender();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::OnMRMLSceneEndBatchProcess()
{
  this->ResumeRender();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::OnMRMLSceneStartImport()
{
  this->PauseRender();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::OnMRMLSceneEndImport()
{
  this->ResumeRender();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::OnMRMLSceneStartRestore()
{
  this->PauseRender();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::OnMRMLSceneEndRestore()
{
  this->ResumeRender();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetIntersectingSlicesEnabled(
  vtkMRMLApplicationLogic::IntersectingSlicesOperation operation, bool enabled)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkWarningMacro("vtkMRMLApplicationLogic::SetIntersectingSlicesEnabled failed: invalid scene");
    return;
    }

  vtkSmartPointer<vtkCollection> sliceDisplayNodes =
    vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkMRMLSliceDisplayNode"));
  if (sliceDisplayNodes.GetPointer())
    {
    vtkMRMLSliceDisplayNode* sliceDisplayNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (sliceDisplayNodes->InitTraversal(it);
      (sliceDisplayNode = static_cast<vtkMRMLSliceDisplayNode*>(sliceDisplayNodes->GetNextItemAsObject(it)));)
      {
      switch (operation)
        {
        case vtkMRMLApplicationLogic::IntersectingSlicesVisibility:
          sliceDisplayNode->SetIntersectingSlicesVisibility(enabled);
          break;
        case vtkMRMLApplicationLogic::IntersectingSlicesInteractive:
          sliceDisplayNode->SetIntersectingSlicesInteractive(enabled);
          break;
        case vtkMRMLApplicationLogic::IntersectingSlicesTranslation:
          sliceDisplayNode->SetIntersectingSlicesTranslationEnabled(enabled);
          break;
        case vtkMRMLApplicationLogic::IntersectingSlicesRotation:
          sliceDisplayNode->SetIntersectingSlicesRotationEnabled(enabled);
          break;
        }
      }
    }

  // The vtkMRMLSliceIntersectionWidget should observe slice display node modifications
  // but as a workaround for now, trigger update by modifying all the slice nodes.
  vtkSmartPointer<vtkCollection> sliceNodes =
    vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkMRMLSliceNode"));
  if (sliceNodes.GetPointer())
    {
    vtkMRMLSliceNode* sliceNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (sliceNodes->InitTraversal(it);
      (sliceNode = static_cast<vtkMRMLSliceNode*>(sliceNodes->GetNextItemAsObject(it)));)
      {
      sliceNode->Modified();
      }
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLApplicationLogic::GetIntersectingSlicesEnabled(
  vtkMRMLApplicationLogic::IntersectingSlicesOperation operation)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkWarningMacro("vtkMRMLApplicationLogic::GetIntersectingSlicesEnabled failed: invalid scene");
    return false;
    }
  vtkMRMLSliceDisplayNode* sliceDisplayNode = vtkMRMLSliceDisplayNode::SafeDownCast(
    scene->GetFirstNodeByClass("vtkMRMLSliceDisplayNode"));
  if (!sliceDisplayNode)
    {
    // No slice display nodes are in the scene yet, use the scene default node instead.
    // Developers can set the default appearance of intersecting slices by modifying the
    // default slice display node.
    vtkSmartPointer<vtkMRMLSliceDisplayNode> defaultSliceDisplayNode =
      vtkMRMLSliceDisplayNode::SafeDownCast(scene->GetDefaultNodeByClass("vtkMRMLSliceDisplayNode"));
    if (!defaultSliceDisplayNode.GetPointer())
      {
      defaultSliceDisplayNode = vtkSmartPointer<vtkMRMLSliceDisplayNode>::New();
      scene->AddDefaultNode(defaultSliceDisplayNode);
      }
    sliceDisplayNode = defaultSliceDisplayNode;
    if (!sliceDisplayNode)
      {
      vtkErrorMacro("vtkMRMLApplicationLogic::GetIntersectingSlicesEnabled failed: cannot get slice display node");
      return false;
      }
    }

  switch (operation)
    {
    case vtkMRMLApplicationLogic::IntersectingSlicesVisibility:
      return sliceDisplayNode->GetIntersectingSlicesVisibility();
    case vtkMRMLApplicationLogic::IntersectingSlicesInteractive:
      return sliceDisplayNode->GetIntersectingSlicesInteractive();
    case vtkMRMLApplicationLogic::IntersectingSlicesTranslation:
      return sliceDisplayNode->GetIntersectingSlicesTranslationEnabled();
    case vtkMRMLApplicationLogic::IntersectingSlicesRotation:
      return sliceDisplayNode->GetIntersectingSlicesRotationEnabled();
    }

  return false;
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetIntersectingSlicesIntersectionMode(int mode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkWarningMacro("vtkMRMLApplicationLogic::SetIntersectingSlicesEnabled failed: invalid scene");
    return;
    }

  vtkSmartPointer<vtkCollection> sliceDisplayNodes =
    vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkMRMLSliceDisplayNode"));
  if (sliceDisplayNodes.GetPointer())
    {
    vtkMRMLSliceDisplayNode* sliceDisplayNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (sliceDisplayNodes->InitTraversal(it);
      (sliceDisplayNode = static_cast<vtkMRMLSliceDisplayNode*>(sliceDisplayNodes->GetNextItemAsObject(it)));)
      {
      sliceDisplayNode->SetIntersectingSlicesIntersectionMode(mode);
      }
    }

  // The vtkMRMLSliceIntersectionWidget should observe slice display node modifications
  // but as a workaround for now, trigger update by modifying all the slice nodes.
  vtkSmartPointer<vtkCollection> sliceNodes =
    vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkMRMLSliceNode"));
  if (sliceNodes.GetPointer())
  {
    vtkMRMLSliceNode* sliceNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (sliceNodes->InitTraversal(it);
      (sliceNode = static_cast<vtkMRMLSliceNode*>(sliceNodes->GetNextItemAsObject(it)));)
    {
      sliceNode->Modified();
    }
  }
}

//----------------------------------------------------------------------------
int vtkMRMLApplicationLogic::GetIntersectingSlicesIntersectionMode()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkWarningMacro("vtkMRMLApplicationLogic::GetIntersectingSlicesEnabled failed: invalid scene");
    return false;
    }
  vtkMRMLSliceDisplayNode* sliceDisplayNode = vtkMRMLSliceDisplayNode::SafeDownCast(
    scene->GetFirstNodeByClass("vtkMRMLSliceDisplayNode"));
  if (!sliceDisplayNode)
    {
    // No slice display nodes are in the scene yet, use the scene default node instead.
    // Developers can set the default appearance of intersecting slices by modifying the
    // default slice display node.
    vtkSmartPointer<vtkMRMLSliceDisplayNode> defaultSliceDisplayNode =
      vtkMRMLSliceDisplayNode::SafeDownCast(scene->GetDefaultNodeByClass("vtkMRMLSliceDisplayNode"));
    if (!defaultSliceDisplayNode.GetPointer())
      {
      defaultSliceDisplayNode = vtkSmartPointer<vtkMRMLSliceDisplayNode>::New();
      scene->AddDefaultNode(defaultSliceDisplayNode);
      }
    sliceDisplayNode = defaultSliceDisplayNode;
    if (!sliceDisplayNode)
      {
      vtkErrorMacro("vtkMRMLApplicationLogic::GetIntersectingSlicesEnabled failed: cannot get slice display node");
      return false;
      }
    }

  return sliceDisplayNode->GetIntersectingSlicesIntersectionMode();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetIntersectingSlicesLineThicknessMode(int mode)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkWarningMacro("vtkMRMLApplicationLogic::SetIntersectingSlicesEnabled failed: invalid scene");
    return;
    }

  vtkSmartPointer<vtkCollection> sliceDisplayNodes =
    vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkMRMLSliceDisplayNode"));
  if (sliceDisplayNodes.GetPointer())
    {
    vtkMRMLSliceDisplayNode* sliceDisplayNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (sliceDisplayNodes->InitTraversal(it);
      (sliceDisplayNode = static_cast<vtkMRMLSliceDisplayNode*>(sliceDisplayNodes->GetNextItemAsObject(it)));)
      {
      sliceDisplayNode->SetIntersectingSlicesLineThicknessMode(mode);
      }
    }

  // The vtkMRMLSliceIntersectionWidget should observe slice display node modifications
  // but as a workaround for now, trigger update by modifying all the slice nodes.
  vtkSmartPointer<vtkCollection> sliceNodes =
    vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkMRMLSliceNode"));
  if (sliceNodes.GetPointer())
    {
    vtkMRMLSliceNode* sliceNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (sliceNodes->InitTraversal(it);
      (sliceNode = static_cast<vtkMRMLSliceNode*>(sliceNodes->GetNextItemAsObject(it)));)
      {
      sliceNode->Modified();
      }
    }
}

//----------------------------------------------------------------------------
int vtkMRMLApplicationLogic::GetIntersectingSlicesLineThicknessMode()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkWarningMacro("vtkMRMLApplicationLogic::GetIntersectingSlicesEnabled failed: invalid scene");
    return false;
    }
  vtkMRMLSliceDisplayNode* sliceDisplayNode = vtkMRMLSliceDisplayNode::SafeDownCast(
    scene->GetFirstNodeByClass("vtkMRMLSliceDisplayNode"));
  if (!sliceDisplayNode)
    {
    // No slice display nodes are in the scene yet, use the scene default node instead.
    // Developers can set the default appearance of intersecting slices by modifying the
    // default slice display node.
    vtkSmartPointer<vtkMRMLSliceDisplayNode> defaultSliceDisplayNode =
      vtkMRMLSliceDisplayNode::SafeDownCast(scene->GetDefaultNodeByClass("vtkMRMLSliceDisplayNode"));
    if (!defaultSliceDisplayNode.GetPointer())
      {
      defaultSliceDisplayNode = vtkSmartPointer<vtkMRMLSliceDisplayNode>::New();
      scene->AddDefaultNode(defaultSliceDisplayNode);
      }
    sliceDisplayNode = defaultSliceDisplayNode;
    if (!sliceDisplayNode)
      {
      vtkErrorMacro("vtkMRMLApplicationLogic::GetIntersectingSlicesEnabled failed: cannot get slice display node");
      return false;
      }
    }

  return sliceDisplayNode->GetIntersectingSlicesLineThicknessMode();
}
