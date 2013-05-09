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
#include <vtkMRMLSliceLinkLogic.h>
#include <vtkMRMLModelHierarchyLogic.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLSceneViewNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkNew.h>
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
vtkCxxRevisionMacro(vtkMRMLApplicationLogic, "$Revision$");
vtkStandardNewMacro(vtkMRMLApplicationLogic);

//----------------------------------------------------------------------------
class vtkMRMLApplicationLogic::vtkInternal
{
public:
  vtkInternal();
  ~vtkInternal();

  vtkSmartPointer<vtkMRMLSelectionNode>    SelectionNode;
  vtkSmartPointer<vtkMRMLInteractionNode>  InteractionNode;
  vtkSmartPointer<vtkCollection> SliceLogics;
  vtkSmartPointer<vtkMRMLSliceLinkLogic> SliceLinkLogic;
  vtkSmartPointer<vtkMRMLModelHierarchyLogic> ModelHierarchyLogic;
  vtkSmartPointer<vtkMRMLColorLogic> ColorLogic;

};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::vtkInternal::vtkInternal()
{
  this->SliceLinkLogic = vtkSmartPointer<vtkMRMLSliceLinkLogic>::New();
  this->ModelHierarchyLogic = vtkSmartPointer<vtkMRMLModelHierarchyLogic>::New();
  this->ColorLogic = vtkSmartPointer<vtkMRMLColorLogic>::New();
}

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::vtkInternal::~vtkInternal()
{
}

//----------------------------------------------------------------------------
// vtkMRMLApplicationLogic methods

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::vtkMRMLApplicationLogic()
{
  this->Internal = new vtkInternal;
  this->Internal->SliceLinkLogic->SetMRMLApplicationLogic(this);
  this->Internal->ModelHierarchyLogic->SetMRMLApplicationLogic(this);
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
vtkMRMLSelectionNode * vtkMRMLApplicationLogic::GetSelectionNode()const
{
  return this->Internal->SelectionNode;
}

//----------------------------------------------------------------------------
vtkMRMLInteractionNode * vtkMRMLApplicationLogic::GetInteractionNode()const
{
  return this->Internal->InteractionNode;
}

//----------------------------------------------------------------------------
vtkMRMLModelHierarchyLogic* vtkMRMLApplicationLogic::GetModelHierarchyLogic()const
{
  return this->Internal->ModelHierarchyLogic;
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
  this->Internal->SliceLogics = sliceLogics;
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic* vtkMRMLApplicationLogic::
GetSliceLogic(vtkMRMLSliceNode* sliceNode) const
{
  if(!sliceNode || !this->Internal->SliceLogics)
    {
    return 0;
    }

  vtkMRMLSliceLogic* logic = 0;
  vtkCollectionSimpleIterator it;
  vtkCollection* logics = this->Internal->SliceLogics;

  for (logics->InitTraversal(it);
      (logic=vtkMRMLSliceLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
    {
    if (logic->GetSliceNode() == sliceNode)
      {
      break;
      }

    logic = 0;
    }

  return logic;
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic* vtkMRMLApplicationLogic::
GetSliceLogicByLayoutName(const char* layoutName) const
{
  if(!layoutName || !this->Internal->SliceLogics)
    {
    return 0;
    }

  vtkMRMLSliceLogic* logic = 0;
  vtkCollectionSimpleIterator it;
  vtkCollection* logics = this->Internal->SliceLogics;

  for (logics->InitTraversal(it);
      (logic=vtkMRMLSliceLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
    {
    if (logic->GetSliceNode())
      {
      if ( !strcmp( logic->GetSliceNode()->GetLayoutName(), layoutName) )
        {
        break;
        }
      }

    logic = 0;
    }

  return logic;
}

//---------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetMRMLSceneInternal(vtkMRMLScene *newScene)
{
  vtkMRMLNode * selectionNode = 0;
  if (newScene)
    {
    // Selection Node
    selectionNode = newScene->GetNthNodeByClass(0, "vtkMRMLSelectionNode");
    if (!selectionNode)
      {
      selectionNode = newScene->AddNode(vtkNew<vtkMRMLSelectionNode>().GetPointer());
      }
    assert(vtkMRMLSelectionNode::SafeDownCast(selectionNode));
    }
  this->SetSelectionNode(vtkMRMLSelectionNode::SafeDownCast(selectionNode));

  vtkMRMLNode * interactionNode = 0;
  if (newScene)
    {
    // Interaction Node
    interactionNode = newScene->GetNthNodeByClass(0, "vtkMRMLInteractionNode");
    if (!interactionNode)
      {
      interactionNode = newScene->AddNode(vtkNew<vtkMRMLInteractionNode>().GetPointer());
      }
    assert(vtkMRMLInteractionNode::SafeDownCast(interactionNode));
    }
  this->SetInteractionNode(vtkMRMLInteractionNode::SafeDownCast(interactionNode));

  this->Superclass::SetMRMLSceneInternal(newScene);

  this->Internal->SliceLinkLogic->SetMRMLScene(newScene);
  this->Internal->ModelHierarchyLogic->SetMRMLScene(newScene);
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetSelectionNode(vtkMRMLSelectionNode *selectionNode)
{
  if (selectionNode == this->Internal->SelectionNode)
    {
    return;
    }
  this->Internal->SelectionNode = selectionNode;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetInteractionNode(vtkMRMLInteractionNode *interactionNode)
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
  if ( !this->Internal->SelectionNode || !this->GetMRMLScene() )
    {
    std::cout << this->Internal->SelectionNode << "    " << this->GetMRMLScene() << std::endl;
    return;
    }

  char *ID = this->Internal->SelectionNode->GetActiveVolumeID();
  char *secondID = this->Internal->SelectionNode->GetSecondaryVolumeID();
  char *labelID = this->Internal->SelectionNode->GetActiveLabelVolumeID();

  vtkMRMLSliceCompositeNode *cnode;
  const int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  for (int i = 0; i < nnodes; i++)
    {
    cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
      this->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
    if(!cnode->GetDoPropagateVolumeSelection())
      {
      continue;
      }
    cnode->SetBackgroundVolumeID( ID );
    cnode->SetForegroundVolumeID( secondID );
    cnode->SetLabelVolumeID( labelID );
    }

  if (fit)
    {
    this->FitSliceToAll();
    }
}


//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::FitSliceToAll()
{
  if (this->Internal->SliceLogics.GetPointer() == 0)
    {
    return;
    }
  vtkMRMLSliceLogic* sliceLogic = 0;
  vtkCollectionSimpleIterator it;
  for(this->Internal->SliceLogics->InitTraversal(it);
      (sliceLogic = vtkMRMLSliceLogic::SafeDownCast(
        this->Internal->SliceLogics->GetNextItemAsObject(it)));)
    {
    vtkMRMLSliceNode *sliceNode = sliceLogic->GetSliceNode();
    int *dims = sliceNode->GetDimensions();
    sliceLogic->FitSliceToAll(dims[0], dims[1]);
    sliceLogic->SnapSliceOffsetToIJK();
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLApplicationLogic::Zip(const char *zipFileName, const char *directoryToZip)
{
  // call function in vtkArchive
  return zip(zipFileName, directoryToZip);
}

//----------------------------------------------------------------------------
bool vtkMRMLApplicationLogic::Unzip(const char *zipFileName, const char *destinationDirectory)
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
bool vtkMRMLApplicationLogic::OpenSlicerDataBundle(const char *sdbFilePath, const char *temporaryDirectory)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("no scene");
    return NULL;
    }

  std::string mrmlFile = this->UnpackSlicerDataBundle(sdbFilePath, temporaryDirectory);

  if ( mrmlFile.empty() )
    {
    vtkErrorMacro("Could not unpack mrml scene");
    return false;
    }

  this->GetMRMLScene()->SetURL( mrmlFile.c_str() );
  int result = this->GetMRMLScene()->Connect();
  if ( result )
    {
    vtkErrorMacro("Could not connect to scene");
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
std::string vtkMRMLApplicationLogic::PercentEncode(std::string s)
{
  std::string validchars = "-_.,@#$%^&()[]{}<>+=";
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
bool vtkMRMLApplicationLogic::SaveSceneToSlicerDataBundleDirectory(const char *sdbDir, vtkImageData *screenShot)
{

  //
  // first, confirm the arguments are valid and create directories if needed
  // then, save all paths and filenames in the current scene
  //  and replace them with paths to the sdbDir
  // then create a scene view of the contents of the data bundle
  // then save the scene
  // -- replace the original paths
  // -- remove the scene view
  //
  // at the end, the scene should be restored to its original state
  // except that some storables will have default storage nodes
  //

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("no scene to bundle!");
    return false;
    }
  if (!sdbDir)
    {
    vtkErrorMacro("no directory given!");
    return false;
    }

  // if the path to the directory is not absolute, return
  if (!vtksys::SystemTools::FileIsFullPath(sdbDir))
    {
    vtkErrorMacro("given directory is not a full path: " << sdbDir);
    return false;
    }
  // is it a directory?
  if (!vtksys::SystemTools::FileIsDirectory(sdbDir))
    {
    vtkErrorMacro("given directory name is not actually a directory, try again!" << sdbDir);
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
      vtkErrorMacro("Error removing SDB scene directory " << rootDir.c_str() << ", cannot make a fresh archive.");
      return false;
      }
    }
  // create the SDB directory
  if (!vtksys::SystemTools::FileExists(rootDir.c_str(), false))
    {
    if (!vtksys::SystemTools::MakeDirectory(rootDir.c_str()))
      {
      vtkErrorMacro("Unable to make temporary directory " << rootDir);
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
      vtkErrorMacro("Unable to make data directory " << dataDir);
      return false;
      }
    }

  //
  // start changing the scene - don't return from below here
  // until scene has been restored to original state
  //
  this->GetMRMLScene()->SetRootDirectory(rootDir.c_str());
  this->GetMRMLScene()->SetURL(urlStr.c_str());

  // change all storage nodes and file names to be unique in the new directory
  // write the new data as we go; save old values
  std::map<vtkMRMLStorageNode*, std::string> origStorageNodeDirs;
  std::map<vtkMRMLStorageNode*, std::string> origStorageNodeFileNames;

  int numNodes = this->GetMRMLScene()->GetNumberOfNodes();
  for (int i = 0; i < numNodes; ++i)
    {
    vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNthNode(i);
    if (!mrmlNode)
      {
      vtkErrorMacro("unable to get " << i << "th node from scene with " << numNodes << " nodes");
      continue;
      }
    if (mrmlNode->IsA("vtkMRMLSceneViewNode"))
      {
      vtkMRMLSceneViewNode *sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(mrmlNode);
      sceneViewNode->SetSceneViewRootDir(this->GetMRMLScene()->GetRootDirectory());
      }
    if (mrmlNode->IsA("vtkMRMLStorableNode"))
      {
      // adjust the file paths
      vtkMRMLStorableNode *storableNode = vtkMRMLStorableNode::SafeDownCast(mrmlNode);
      if (storableNode && storableNode->GetSaveWithScene())
        {
        vtkMRMLStorageNode *storageNode = storableNode->GetStorageNode();
        if (!storageNode)
          {
          // have to create one!
          vtkWarningMacro("creating a new storage node for " << storableNode->GetID());
          storageNode = storableNode->CreateDefaultStorageNode();
          if (storageNode)
            {
            std::string fileBaseName = this->PercentEncode(std::string(storableNode->GetName()));
            std::string extension = storageNode->GetDefaultWriteFileExtension();
            std::string storageFileName = fileBaseName + std::string(".") + extension;
            vtkDebugMacro("new file name = " << storageFileName.c_str());
            storageNode->SetFileName(storageFileName.c_str());
            this->GetMRMLScene()->AddNode(storageNode);
            storableNode->SetAndObserveStorageNodeID(storageNode->GetID());
            storageNode->Delete();
            storageNode = storableNode->GetStorageNode();
            }
          else
            {
            vtkErrorMacro("cannot make a new storage node for storable node " << storableNode->GetID());
            }
          }
        if (storageNode)
          {
          // save the old values
          std::string fileName(storageNode->GetFileName());
          origStorageNodeFileNames[storageNode] = fileName;

          std::vector<std::string> pathComponents;
          vtksys::SystemTools::SplitPath(fileName.c_str(), pathComponents);
          pathComponents.pop_back();
          origStorageNodeDirs[storageNode] = vtksys::SystemTools::JoinPath(pathComponents);

          storageNode->SetDataDirectory(dataDir.c_str());
          vtkDebugMacro("set data directory to "
            << dataDir.c_str() << ", storable node " << storableNode->GetID()
            << " file name is now: " << storageNode->GetFileName());
          // deal with existing files
          if (vtksys::SystemTools::FileExists(storageNode->GetFileName(), true))
            {
            vtkWarningMacro("file " << storageNode->GetFileName() << " already exists, renaming!");
            std::string baseName = vtksys::SystemTools::GetFilenameWithoutExtension(storageNode->GetFileName());
            std::string ext = vtksys::SystemTools::GetFilenameExtension(storageNode->GetFileName());
            bool uniqueName = false;
            std::string uniqueFileName;
            int v = 1;
            while (!uniqueName)
              {
              std::stringstream ss;
              ss << v;
              uniqueFileName = baseName + ss.str() + ext;
              if (vtksys::SystemTools::FileExists(uniqueFileName.c_str()) == 0)
                {
                uniqueName = true;
                }
              else
                {
                ++v;
                }
              }
            vtkDebugMacro("found unique file name " << uniqueFileName.c_str());
            storageNode->SetFileName(uniqueFileName.c_str());
            }
          storageNode->WriteData(storableNode);
          }
        }
      }
    }

  //
  // create a scene view, using the snapshot passed in if any
  //
  vtkMRMLSceneViewNode * newSceneViewNode = vtkMRMLSceneViewNode::New();
  newSceneViewNode->SetScene(this->GetMRMLScene());
  newSceneViewNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("Slicer Data Bundle Scene View"));
  newSceneViewNode->SetSceneViewDescription("Scene at MRML file save point");
  // save the scene view
  newSceneViewNode->StoreScene();
  this->GetMRMLScene()->AddNode(newSceneViewNode);

  vtkMRMLStorageNode *sceneViewStorageNode = 0;
  if (screenShot)
    {
    // assumes has been passed a screen shot of the full layout
    newSceneViewNode->SetScreenShotType(4);
    newSceneViewNode->SetScreenShot(screenShot);
    // create a storage node
    vtkMRMLStorageNode *sceneViewStorageNode = newSceneViewNode->CreateDefaultStorageNode();
    // set the file name from the node name, using a relative path, it will go
    // at the same level as the  .mrml file
    std::string sceneViewFileName = std::string(newSceneViewNode->GetName()) + std::string(".png");
    sceneViewStorageNode->SetFileName(sceneViewFileName.c_str());
    this->GetMRMLScene()->AddNode(sceneViewStorageNode);
    newSceneViewNode->SetAndObserveStorageNodeID(sceneViewStorageNode->GetID());
    // force a write
    sceneViewStorageNode->WriteData(newSceneViewNode);
    }

  // write the scene to disk, changes paths to relative
  vtkDebugMacro("calling commit on the scene, to url " << this->GetMRMLScene()->GetURL());
  this->GetMRMLScene()->Commit();

  //
  // Now, restore the state of the scene
  //

  // clean up scene views
  this->GetMRMLScene()->RemoveNode(newSceneViewNode);
  newSceneViewNode->Delete();
  if (sceneViewStorageNode)
    {
    this->GetMRMLScene()->RemoveNode(sceneViewStorageNode);
    sceneViewStorageNode->Delete();
    }

  // reset the storage paths
  numNodes = this->GetMRMLScene()->GetNumberOfNodes();
  for (int i = 0; i < numNodes; ++i)
    {
    vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNthNode(i);
    if (!mrmlNode)
      {
      vtkErrorMacro("unable to get " << i << "th node from scene with " << numNodes << " nodes");
      continue;
      }
    if (mrmlNode->IsA("vtkMRMLSceneViewNode"))
      {
      vtkMRMLSceneViewNode *sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(mrmlNode);
      sceneViewNode->SetSceneViewRootDir(origRootDirectory.c_str());
      }
    if (mrmlNode->IsA("vtkMRMLStorableNode"))
      {
      vtkMRMLStorableNode *storableNode = vtkMRMLStorableNode::SafeDownCast(mrmlNode);
      vtkMRMLStorageNode *storageNode = storableNode->GetStorageNode();
      if (storageNode && origStorageNodeFileNames.find( storageNode ) != origStorageNodeFileNames.end() )
        {
        storageNode->SetFileName(origStorageNodeFileNames[storageNode].c_str());
        }
      if (storageNode && origStorageNodeDirs.find( storageNode ) != origStorageNodeDirs.end() )
        {
        storageNode->SetDataDirectory(origStorageNodeDirs[storageNode].c_str());
        }
      }
    }

  this->GetMRMLScene()->SetURL(origURL.c_str());
  this->GetMRMLScene()->SetRootDirectory(origRootDirectory.c_str());

  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLApplicationLogic::LoadDefaultParameterSets(vtkMRMLScene *scene,
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
//   if (this->UserColorFilePaths != NULL)
//     {
//     vtkDebugMacro("\nFindColorFiles: got user color file paths = " << this->UserColorFilePaths);
//     // parse out the list, breaking at delimiter strings
// #ifdef WIN32
//     const char *delim = ";";
// #else
//     const char *delim = ":";
// #endif
//     char *ptr = strtok(this->UserColorFilePaths, delim);
//     while (ptr != NULL)
//       {
//       std::string dir = std::string(ptr);
//       vtkDebugMacro("\nFindColorFiles: Adding user dir " << dir.c_str() << " to the directories to check");
//       DirectoriesToCheck.push_back(dir);
//       ptr = strtok(NULL, delim);
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
    DIR *dp;
    struct dirent *dirp;
    if ((dp  = opendir(dirString.c_str())) == NULL)
      {
      vtkGenericWarningMacro("Error(" << errno << ") opening " << dirString.c_str());
      }
    else
      {
      while ((dirp = readdir(dp)) != NULL)
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
  , Caller(0)
  , EventID(vtkCommand::ModifiedEvent)
  , CallData(0)
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
