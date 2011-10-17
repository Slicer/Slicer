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

// MRMLLogic includes
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLSliceLogic.h"
#include <vtkMRMLSliceLinkLogic.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLSceneViewNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <cassert>
#include <sstream>

// LibArchive includes
#include <archive.h>
#include <archive_entry.h>

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
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::vtkInternal::vtkInternal()
{
  this->SliceLinkLogic = vtkSmartPointer<vtkMRMLSliceLinkLogic>::New();
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

////----------------------------------------------------------------------------
//void vtkMRMLApplicationLogic::ProcessMRMLEvents(vtkObject * vtkNotUsed(caller),
//                                                unsigned long vtkNotUsed(event),
//                                                void * vtkNotUsed(callData))
//{
//  //
//  // Look for a selection node in the scene:
//  // - we always use the first one in the scene
//  // - if it doesn't match the one we had, we switch
//  // - if there isn't one, we create one
//  // - we add it to the scene if needed
//  //
//  vtkMRMLSelectionNode *node =
//      vtkMRMLSelectionNode::SafeDownCast(
//          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));

//  // selection node
//  if (node == 0)
//    {
//    node = vtkMRMLSelectionNode::New();
//    this->SetSelectionNode(node);
//    node->Delete();
//    }
//  if ( this->Internal->SelectionNode != node )
//    {
//      this->SetSelectionNode(node);
//    }
//  if (this->GetMRMLScene()->GetNodeByID(this->Internal->SelectionNode->GetID()) == NULL)
//    {
//    this->SetMRMLScene(this->GetMRMLScene());
//    this->SetSelectionNode(vtkMRMLSelectionNode::SafeDownCast(
//        this->GetMRMLScene()->AddNode(this->Internal->SelectionNode)));
//    this->SetAndObserveMRMLScene(this->GetMRMLScene());
//    }


//  vtkMRMLInteractionNode *inode;
//  inode = vtkMRMLInteractionNode::SafeDownCast (
//          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));

//  // interaction node
//  if (inode == 0)
//    {
//    inode = vtkMRMLInteractionNode::New();
//    this->SetInteractionNode(inode);
//    inode->Delete();
//    }
//  if (this->Internal->InteractionNode != inode)
//    {
//    this->SetInteractionNode(inode);
//    }
//  if (this->GetMRMLScene()->GetNodeByID(this->Internal->InteractionNode->GetID()) == 0)
//    {
//    this->SetMRMLScene(this->GetMRMLScene());
//    this->SetInteractionNode(vtkMRMLInteractionNode::SafeDownCast(
//        this->GetMRMLScene()->AddNode(this->Internal->InteractionNode)));
//    this->SetAndObserveMRMLScene(this->GetMRMLScene());
//    }
//}

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

  if (fit) {
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
    }
}

//----------------------------------------------------------------------------
const char * vtkMRMLApplicationLogic::Zip(const char *zipFileName, const char *tempDir, vtkImageData *screenShot)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("Zip: no scene to zip!");
    return NULL;
    }
  if (!tempDir)
    {
    vtkErrorMacro("Zip: no temporary directory given!");
    return NULL;
    }

  vtkDebugMacro("Zip: zipFileName = " << zipFileName << ", temp dir = " << tempDir);

  vtksys_stl::string archiveFileName;
  vtksys_stl::string newName;
  if (!zipFileName)
    {
    // use the current URL as a base
    vtksys_stl::string url = this->GetMRMLScene()->GetURL();
    if (url != "")
      {
      // use the url without the file path or extension
      newName = vtksys::SystemTools::GetFilenameWithoutExtension(vtksys::SystemTools::GetFilenameName(url));
      }
    else
      {
      // use a generic name
      newName = "Slicer4Scene.zip";
      }
    }
  else
    {
    newName = vtksys_stl::string(zipFileName);
    }

  // does the name have a .zip extension?
  size_t pos = newName.find(".zip");
  if (pos == std::string::npos)
    {
    // append .zip to the end of the name
    newName += vtksys_stl::string(".zip");
    vtkWarningMacro("Zip: Adding missing .zip to end of input file name, new name = " << newName.c_str());
    }
  // check the LibArchive version since .zip is only supported after 2.8.4,
  // before that we have to use .tar.gz
#if ARCHIVE_VERSION_NUMBER < 2008004
  // have to use .tar.gz
  vtkDebugMacro("Zip: have to use .tar.gz, starting from newName " << newName.c_str() << ", of size " << newName.size() << ", taking four letters away from the end");
  newName.erase(newName.size() - 4);
  vtkDebugMacro("Zip: name without .zip " <<  newName.c_str());
  newName.append(".tar.gz");
  vtkWarningMacro("Zip: Have an older version of LibArchive which only supports .tar.gz and not zip.\nChanging archive extension to tar.gz, newName = " << newName.c_str());
#endif

  vtkDebugMacro("Zip: file name = " << newName << ", temp dir = " << tempDir);



  // if the zip file is not absolute, put it in the temp dir
  if (vtksys::SystemTools::FileIsFullPath(newName.c_str()))
    {
    archiveFileName = newName;
    }
  else
    {
    vtksys_stl::vector<vtksys_stl::string> archivePathComponents;
    vtksys::SystemTools::SplitPath(tempDir, archivePathComponents);
    archivePathComponents.push_back(newName);
    archiveFileName = vtksys::SystemTools::JoinPath(archivePathComponents);
    }

  vtkDebugMacro("Zip: saving to file " << archiveFileName);

  // does it exist already?
  if (vtksys::SystemTools::FileExists(archiveFileName.c_str(), true))
    {
    // for now, just delete it
    vtkWarningMacro("Zip: removing old file " << archiveFileName.c_str());
    if (!vtksys::SystemTools::RemoveFile(archiveFileName.c_str()))
      {
      vtkErrorMacro("Zip: unable to remove " << archiveFileName.c_str() << ", cannot make a new archive");
      return NULL;
      }
    }

  // zip things up in a folder with the name of the archive file
  vtksys_stl::string rootDir = vtksys::SystemTools::GetParentDirectory(archiveFileName.c_str());
  vtksys_stl::vector<vtksys_stl::string> rootPathComponents;
  vtksys::SystemTools::SplitPath(rootDir.c_str(), rootPathComponents);
  // GetFilenameWithoutExtension just returns the file name without path, add
  // it to the end of the parent dir
  rootPathComponents.push_back(vtksys::SystemTools::GetFilenameWithoutExtension(archiveFileName));
  rootDir = vtksys::SystemTools::JoinPath(rootPathComponents);
  vtkDebugMacro("Zip: Using root dir of " << rootDir);

  // remove the directory if it does exist
  if (vtksys::SystemTools::FileExists(rootDir.c_str(), false))
    {
    vtkWarningMacro("Zip: removing zip scene directory " << rootDir.c_str());
    if (!vtksys::SystemTools::RemoveADirectory(rootDir.c_str()))
      {
      vtkErrorMacro("Zip: Error removing zip scene directory " << rootDir.c_str() << ", cannot make a fresh archive.");
      return NULL;
      }
    }
  // create the zip directory
  if (!vtksys::SystemTools::FileExists(rootDir.c_str(), false))
    {
    if (!vtksys::SystemTools::MakeDirectory(rootDir.c_str()))
      {
      vtkErrorMacro("Zip: Unable to make temporary directory " << rootDir);
      return NULL;
      }
    }

  // create a new scene
  vtkSmartPointer<vtkMRMLScene> zipScene = vtkSmartPointer<vtkMRMLScene>::New();

  zipScene->SetRootDirectory(rootDir.c_str());

  // put the mrml scene file in the new directory
  vtksys_stl::string urlStr = vtksys::SystemTools::GetFilenameWithoutExtension(newName) + vtksys_stl::string(".mrml");
  rootPathComponents.push_back(urlStr);
  urlStr =  vtksys::SystemTools::JoinPath(rootPathComponents);
  zipScene->SetURL(urlStr.c_str());
  rootPathComponents.pop_back();
  vtkDebugMacro("Zip: set new scene url to " << zipScene->GetURL());

  // create a data directory
  vtksys_stl::vector<vtksys_stl::string> pathComponents;
  vtksys::SystemTools::SplitPath(rootDir.c_str(), pathComponents);
  pathComponents.push_back("Data");
  vtksys_stl::string dataDir =  vtksys::SystemTools::JoinPath(pathComponents);
  vtkDebugMacro("Zip: using data dir of " << dataDir);

  // create the data dir
  if (!vtksys::SystemTools::FileExists(dataDir.c_str()))
    {
    if (!vtksys::SystemTools::MakeDirectory(dataDir.c_str()))
      {
      vtkErrorMacro("Zip: Unable to make data directory " << dataDir);
      return NULL;
      }
    }

  // copy all the nodes into a new scene so can work on it without changing
  // the current scene
  int numNodes = this->GetMRMLScene()->GetNumberOfNodes();
  for (int i = 0; i < numNodes; ++i)
    {
    vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNthNode(i);
    if (!mrmlNode)
      {
      vtkErrorMacro("Zip: unable to get " << i << "th node from scene with " << numNodes << " nodes");
      break;
      }
    vtkMRMLNode *copyNode = zipScene->CopyNode(mrmlNode);
    if (!copyNode)
      {
      vtkErrorMacro("Zip: unable to make a copy of node " << i << " with id " << (mrmlNode->GetID() ? mrmlNode->GetID() : "NULL"));
      }
    else
      {
      if (copyNode->IsA("vtkMRMLStorableNode"))
        {
        // adjust the file paths
        vtkMRMLStorableNode *storableNode = vtkMRMLStorableNode::SafeDownCast(copyNode);
        if (storableNode && storableNode->GetSaveWithScene())
          {
          vtkMRMLStorageNode *snode = storableNode->GetStorageNode();
          if (!snode)
            {
            // have to create one!
            vtkWarningMacro("Zip: creating a new storage node for " << storableNode->GetID());
            snode = storableNode->CreateDefaultStorageNode();
            if (snode)
              {
              std::string storageFileName = std::string(storableNode->GetName()) + std::string(".") + std::string(snode->GetDefaultWriteFileExtension());
              vtkDebugMacro("Zip: new file name = " << storageFileName.c_str());
              snode->SetFileName(storageFileName.c_str());
              zipScene->AddNode(snode);
              storableNode->SetAndObserveStorageNodeID(snode->GetID());
              snode->Delete();
              snode = storableNode->GetStorageNode();
              }
            else
              {
              vtkErrorMacro("Zip: cannot make a new storage node for storable node " << storableNode->GetID());
              }
            }
          if (snode)
            {
            snode->SetDataDirectory(dataDir.c_str());
            vtkDebugMacro("Zip: set data directory to " << dataDir.c_str() << ", storable node " << storableNode->GetID() << " file name is now: " << snode->GetFileName());
            // deal with existing files
            if (vtksys::SystemTools::FileExists(snode->GetFileName(), true))
              {
              vtkWarningMacro("Zip: file " << snode->GetFileName() << " already exists, renaming!");
              vtksys_stl::string baseName = vtksys::SystemTools::GetFilenameWithoutExtension(snode->GetFileName());
              vtksys_stl::string ext = vtksys::SystemTools::GetFilenameExtension(snode->GetFileName());
              bool uniqueName = false;
              vtksys_stl::string uniqueFileName;
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
              vtkDebugMacro("Zip: found unique file name " << uniqueFileName.c_str());
              snode->SetFileName(uniqueFileName.c_str());

              }
            snode->WriteData(storableNode);
            }
          }
        }
      }
    }
  // create a scene view, using the snapshot passed in if any
  vtkMRMLSceneViewNode * newSceneViewNode = vtkMRMLSceneViewNode::New();
  newSceneViewNode->SetScene(zipScene);
  newSceneViewNode->SetName(zipScene->GetUniqueNameByString("Slicer Data Bundle Scene View"));
  newSceneViewNode->SetSceneViewDescription("Scene at MRML file zip point");
  if (screenShot)
    {
    // assumes has been passed a screen shot of the full layout
    newSceneViewNode->SetScreenShotType(4);
    newSceneViewNode->SetScreenShot(screenShot);
    // mark it modified since read so that the screen shot will get saved to disk
    newSceneViewNode->ModifiedSinceReadOn();
    }
  // save the scene view
  newSceneViewNode->StoreScene();
  zipScene->AddNode(newSceneViewNode);
  // create a storage node
  vtkMRMLStorageNode *storageNode = newSceneViewNode->CreateDefaultStorageNode();
  // set the file name from the node name, using a relative path, it will go
  // at the same level as the  .mrml file
  std::string sceneViewFileName = std::string(newSceneViewNode->GetName()) + std::string(".png");
  storageNode->SetFileName(sceneViewFileName.c_str());
  zipScene->AddNode(storageNode);
  newSceneViewNode->SetAndObserveStorageNodeID(storageNode->GetID());
  // force a write
  storageNode->WriteData(newSceneViewNode);
  // clean up
  newSceneViewNode->Delete();
  storageNode->Delete();

  // write the scene to disk, changes paths to relative
  vtkDebugMacro("Zip: calling commit on the scene, to url " << zipScene->GetURL());
  zipScene->Commit();

  // collect a list of file names that will go into the zip archive
  // doing this after the commit so that get the relative paths
  std::vector<std::string> FilesToZip;
  // is the MRML Scene file name absolute?
  if (vtksys::SystemTools::FileIsFullPath(zipScene->GetURL()))
    {
    vtkDebugMacro("Zip: scene file name is absolute: " << zipScene->GetURL());
    FilesToZip.push_back(std::string(zipScene->GetURL()));
    }
  else
    {
    FilesToZip.push_back(std::string(zipScene->GetURL()));
    }
  int numberOfStorageNodes = zipScene->GetNumberOfNodesByClass("vtkMRMLStorageNode");
  for (int i = 0; i < numberOfStorageNodes; ++i)
    {
    vtkMRMLStorageNode *storageNode = vtkMRMLStorageNode::SafeDownCast(zipScene->GetNthNodeByClass(i, "vtkMRMLStorageNode"));
    if (storageNode && storageNode->GetFileName())
      {
      // get all the file names
      int numFileNames = storageNode->GetNumberOfFileNames();
      if (numFileNames == 0)
        {
        // just add the file name, otherwise it's also in the file list
        FilesToZip.push_back(std::string(storageNode->GetFileName()));
        }
      else
        {
        for (int f = 0; f < numFileNames; f++)
          {
          FilesToZip.push_back(std::string(storageNode->GetNthFileName(f)));
          }
        }
      }
    }
  if (this->GetDebug())
    {
    std::cout << "FilesToZip:" << std::endl;
    for (unsigned int z = 0; z < FilesToZip.size(); ++z)
      {
      std::cout << "\t" << z << ": " << FilesToZip[z].c_str() << std::endl;
      }
    }

  // now zip it up using LibArchive
  struct archive *a;
  struct archive_entry *entry, *dirEntry;
#ifndef _WIN32
  struct stat st;
#endif
  char buff[8192];
  int len;
  // have to read the contents of the files to add them to the archive
  FILE *fd;


  a = archive_write_new();

#ifdef HAVE_ZLIB_H
  vtkDebugMacro("Zip: HAVE_Z_LIB_H, can use compression");
#endif
#if ARCHIVE_VERSION_NUMBER < 2008004
  // create a .tar.gz archive
  archive_write_set_compression_gzip(a);
  archive_write_set_format_pax_restricted(a);
#else
  // create a zip archive
  const char *compression_type = "";
#ifdef HAVE_ZLIB_H
  compression_type = "zip:compression=deflate";
#else
  compression_type = "zip:compression=store";
#endif
  vtkDebugMacro("Zip: compression type = " << compression_type);

  archive_write_set_format_zip(a);
  archive_write_set_format_options(a, compression_type);
  // this line was in the example
  //archive_write_set_compression_none(a);
#endif

  archive_write_open_filename(a, archiveFileName.c_str());

  // add the data directory
  dirEntry = archive_entry_new();
  archive_entry_set_mtime(dirEntry, 11, 110);
  // want just the subdir and Data for the relative dir name in the archive
  vtksys_stl::string relativeDataDir = vtksys::SystemTools::RelativePath(rootDir.c_str(), dataDir.c_str());
  vtkDebugMacro("Zip: Adding data dir to .zip file: " << relativeDataDir);
  archive_entry_copy_pathname(dirEntry, relativeDataDir.c_str());
  archive_entry_set_mode(dirEntry, S_IFDIR | 0755);
  archive_entry_set_size(dirEntry, 512);
  archive_write_header(a, dirEntry);
  archive_entry_free(dirEntry);

  // add the files
  for (unsigned int f = 0; f < FilesToZip.size(); ++f)
    {
    const char *fname = FilesToZip[f].c_str();
#ifndef _WIN32
    stat(fname, &st);
#endif
    entry = archive_entry_new();
    // use a relative path for the entry file name, including the top
    // directory so it unzips into a directory of it's own
    std::string relFileName = vtksys::SystemTools::RelativePath(vtksys::SystemTools::GetParentDirectory(rootDir.c_str()).c_str(), fname);
    archive_entry_set_pathname(entry, relFileName.c_str());
    vtkDebugMacro("Zip: Set entry path name to '" << relFileName << "', from file name " << fname);
#ifndef _WIN32
    archive_entry_set_size(entry, st.st_size);
#else
    // size is required, for now use the vtksys call though it uses struct
    // stat as well
    unsigned long fileLength = vtksys::SystemTools::FileLength(fname);
    archive_entry_set_size(entry, fileLength);
#endif
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    archive_write_header(a, entry);
    fd = fopen(fname, "r");
    if (!fd)
      {
      vtkErrorMacro("Zip: unable to open file '" << (fname ? fname : "null") << "' for adding to archive");
      }
    else
      {
      len = fread(buff, sizeof(char), sizeof(buff), fd);
      while ( len > 0 )
        {
        archive_write_data(a, buff, len);
        len = fread(buff, sizeof(char), sizeof(buff), fd);
        }
      fclose(fd);
      }
    archive_entry_free(entry);
    }

  archive_write_close(a);
  // _finish will change to _free in libarchive3.0
  int retval = archive_write_finish(a);
  if (retval != ARCHIVE_OK)
    {
    vtkErrorMacro("Zip: error in writing the zip file " << archiveFileName << ", write returned " << retval << " instead of " << ARCHIVE_OK);
    return NULL;
    }
  else
    {
    vtkDebugMacro("Zip: succeeded in writing the zip file " << archiveFileName << ", write returned " << retval);
    }

  return archiveFileName.c_str();
}

//----------------------------------------------------------------------------
const char *vtkMRMLApplicationLogic::SaveSceneToSlicerDataBundleDirectory(const char *sdbDir, vtkImageData *screenShot)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory: no scene to bundle!");
    return NULL;
    }
  if (!sdbDir)
    {
    vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory: no directory given!");
    return NULL;
    }

  // if the path to the directory is not absolute, return
  if (!vtksys::SystemTools::FileIsFullPath(sdbDir))
    {
    vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory: given directory is not a full path: " << sdbDir);
    return NULL;
    }
  // is it a directory?
  if (!vtksys::SystemTools::FileIsDirectory(sdbDir))
    {
    vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory: given directory name is not actually a directory, try again!" << sdbDir);
    return NULL;
    }
  vtksys_stl::string rootDir = vtksys_stl::string(sdbDir);
  vtkDebugMacro("SaveSceneToSlicerDataBundleDirectory: Using root dir of " << rootDir);
  // need the components to build file names
  vtksys_stl::vector<vtksys_stl::string> rootPathComponents;
  vtksys::SystemTools::SplitPath(rootDir.c_str(), rootPathComponents);

  // remove the directory if it does exist
  if (vtksys::SystemTools::FileExists(rootDir.c_str(), false))
    {
    vtkWarningMacro("SaveSceneToSlicerDataBundleDirectory: removing SDB scene directory " << rootDir.c_str());
    if (!vtksys::SystemTools::RemoveADirectory(rootDir.c_str()))
      {
      vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory: Error removing SDB scene directory " << rootDir.c_str() << ", cannot make a fresh archive.");
      return NULL;
      }
    }
  // create the SDB directory
  if (!vtksys::SystemTools::FileExists(rootDir.c_str(), false))
    {
    if (!vtksys::SystemTools::MakeDirectory(rootDir.c_str()))
      {
      vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory: Unable to make temporary directory " << rootDir);
      return NULL;
      }
    }

    // create a new scene
  vtkSmartPointer<vtkMRMLScene> sdbScene = vtkSmartPointer<vtkMRMLScene>::New();

  sdbScene->SetRootDirectory(rootDir.c_str());

  // put the mrml scene file in the new directory
  vtksys_stl::string urlStr = vtksys::SystemTools::GetFilenameWithoutExtension(rootDir.c_str()) + vtksys_stl::string(".mrml");
  rootPathComponents.push_back(urlStr);
  urlStr =  vtksys::SystemTools::JoinPath(rootPathComponents);
  sdbScene->SetURL(urlStr.c_str());
  rootPathComponents.pop_back();
  vtkDebugMacro("SaveSceneToSlicerDataBundleDirectory: set new scene url to " << sdbScene->GetURL());

  // create a data directory
  vtksys_stl::vector<vtksys_stl::string> pathComponents;
  vtksys::SystemTools::SplitPath(rootDir.c_str(), pathComponents);
  pathComponents.push_back("Data");
  vtksys_stl::string dataDir =  vtksys::SystemTools::JoinPath(pathComponents);
  vtkDebugMacro("SaveSceneToSlicerDataBundleDirectory: using data dir of " << dataDir);

  // create the data dir
  if (!vtksys::SystemTools::FileExists(dataDir.c_str()))
    {
    if (!vtksys::SystemTools::MakeDirectory(dataDir.c_str()))
      {
      vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory: Unable to make data directory " << dataDir);
      return NULL;
      }
    }

  // copy all the nodes into a new scene so can work on it without changing
  // the current scene
  int numNodes = this->GetMRMLScene()->GetNumberOfNodes();
  for (int i = 0; i < numNodes; ++i)
    {
    vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNthNode(i);
    if (!mrmlNode)
      {
      vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory: unable to get " << i << "th node from scene with " << numNodes << " nodes");
      break;
      }
    vtkMRMLNode *copyNode = sdbScene->CopyNode(mrmlNode);
    if (!copyNode)
      {
      vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory: unable to make a copy of node " << i << " with id " << (mrmlNode->GetID() ? mrmlNode->GetID() : "NULL"));
      }
    else
      {
      if (copyNode->IsA("vtkMRMLStorableNode"))
        {
        // adjust the file paths
        vtkMRMLStorableNode *storableNode = vtkMRMLStorableNode::SafeDownCast(copyNode);
        if (storableNode && storableNode->GetSaveWithScene())
          {
          vtkMRMLStorageNode *snode = storableNode->GetStorageNode();
          if (!snode)
            {
            // have to create one!
            vtkWarningMacro("SaveSceneToSlicerDataBundleDirectory: creating a new storage node for " << storableNode->GetID());
            snode = storableNode->CreateDefaultStorageNode();
            if (snode)
              {
              std::string storageFileName = std::string(storableNode->GetName()) + std::string(".") + std::string(snode->GetDefaultWriteFileExtension());
              vtkDebugMacro("SaveSceneToSlicerDataBundleDirectory: new file name = " << storageFileName.c_str());
              snode->SetFileName(storageFileName.c_str());
              sdbScene->AddNode(snode);
              storableNode->SetAndObserveStorageNodeID(snode->GetID());
              snode->Delete();
              snode = storableNode->GetStorageNode();
              }
            else
              {
              vtkErrorMacro("SaveSceneToSlicerDataBundleDirectory: cannot make a new storage node for storable node " << storableNode->GetID());
              }
            }
          if (snode)
            {
            snode->SetDataDirectory(dataDir.c_str());
            vtkDebugMacro("SaveSceneToSlicerDataBundleDirectory: set data directory to " << dataDir.c_str() << ", storable node " << storableNode->GetID() << " file name is now: " << snode->GetFileName());
            // deal with existing files
            if (vtksys::SystemTools::FileExists(snode->GetFileName(), true))
              {
              vtkWarningMacro("SaveSceneToSlicerDataBundleDirectory: file " << snode->GetFileName() << " already exists, renaming!");
              vtksys_stl::string baseName = vtksys::SystemTools::GetFilenameWithoutExtension(snode->GetFileName());
              vtksys_stl::string ext = vtksys::SystemTools::GetFilenameExtension(snode->GetFileName());
              bool uniqueName = false;
              vtksys_stl::string uniqueFileName;
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
              vtkDebugMacro("SaveSceneToSlicerDataBundleDirectory: found unique file name " << uniqueFileName.c_str());
              snode->SetFileName(uniqueFileName.c_str());

              }
            snode->WriteData(storableNode);
            }
          }
        }
      }
    }
  // create a scene view, using the snapshot passed in if any
  vtkMRMLSceneViewNode * newSceneViewNode = vtkMRMLSceneViewNode::New();
  newSceneViewNode->SetScene(sdbScene);
  newSceneViewNode->SetName(sdbScene->GetUniqueNameByString("Slicer Data Bundle Scene View"));
  newSceneViewNode->SetSceneViewDescription("Scene at MRML file save point");
  if (screenShot)
    {
    // assumes has been passed a screen shot of the full layout
    newSceneViewNode->SetScreenShotType(4);
    newSceneViewNode->SetScreenShot(screenShot);
    // mark it modified since read so that the screen shot will get saved to disk
    newSceneViewNode->ModifiedSinceReadOn();
    }
  // save the scene view
  newSceneViewNode->StoreScene();
  sdbScene->AddNode(newSceneViewNode);
  // create a storage node
  vtkMRMLStorageNode *storageNode = newSceneViewNode->CreateDefaultStorageNode();
  // set the file name from the node name, using a relative path, it will go
  // at the same level as the  .mrml file
  std::string sceneViewFileName = std::string(newSceneViewNode->GetName()) + std::string(".png");
  storageNode->SetFileName(sceneViewFileName.c_str());
  sdbScene->AddNode(storageNode);
  newSceneViewNode->SetAndObserveStorageNodeID(storageNode->GetID());
  // force a write
  storageNode->WriteData(newSceneViewNode);
  // clean up
  newSceneViewNode->Delete();
  storageNode->Delete();

  // write the scene to disk, changes paths to relative
  vtkDebugMacro("SaveSceneToSlicerDataBundleDirectory: calling commit on the scene, to url " << sdbScene->GetURL());
  sdbScene->Commit();

  return sdbScene->GetURL();
}

//----------------------------------------------------------------------------
int vtkMRMLApplicationLogic::LoadDefaultParameterSets(vtkMRMLScene *scene,
                                                      const std::vector<std::string>& directories)
{

  // build up the vector
  vtksys_stl::vector<vtksys_stl::string> filesVector;
  vtksys_stl::vector<vtksys_stl::string> filesToLoad;
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
    vtksys_stl::string dirString = directories[d];
    //vtkDebugMacro("\nLoadDefaultParameterSets: checking for parameter sets in dir " << d << " = " << dirString.c_str());

    filesVector.clear();
    filesVector.push_back(dirString);
    filesVector.push_back(vtksys_stl::string("/"));

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
        filesVector.push_back(vtksys_stl::string(findData.cFileName));
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
        filesVector.push_back(vtksys_stl::string(dirp->d_name));
#endif

        vtksys_stl::string fileToCheck = vtksys::SystemTools::JoinPath(filesVector);
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
  vtksys_stl::vector<vtksys_stl::string>::iterator fit;
  for (fit = filesToLoad.begin(); fit != filesToLoad.end(); ++fit)
    {
    scene->SetURL( (*fit).c_str() );
    scene->Import();
    }

  // restore URL and root dir
  scene->SetURL(url.c_str());
  scene->SetRootDirectory(rootdir.c_str());

  return filesToLoad.size();
}
