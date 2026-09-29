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

==============================================================================*/

#include "vtkSlicerSceneWriter.h"

// Logic includes
#include <vtkMRMLFileIOManager.h>
#include <vtkSlicerApplicationLogic.h>
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLI18N.h>
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkSlicerSceneWriter);

//----------------------------------------------------------------------------
vtkSlicerSceneWriter::vtkSlicerSceneWriter()
{
  this->SetFileType("SceneFile");
  this->SetDescription(vtkMRMLTr("qSlicerSceneWriter", "MRML Scene"));
  this->SetNameFilters(std::vector<std::string>{ vtkMRMLTr("qSlicerSceneWriter", "MRML Scene") + " (.mrml)", //
                                                 vtkMRMLTr("qSlicerSceneWriter", "Medical Reality Bundle") + " (.mrb)",
                                                 vtkMRMLTr("qSlicerSceneWriter", "Slicer Data Bundle") + " (*)" });
}

//----------------------------------------------------------------------------
vtkSlicerSceneWriter::~vtkSlicerSceneWriter() = default;

//----------------------------------------------------------------------------
void vtkSlicerSceneWriter::SetApplicationLogic(vtkSlicerApplicationLogic* applicationLogic)
{
  this->ApplicationLogic = applicationLogic;
}

//----------------------------------------------------------------------------
vtkSlicerApplicationLogic* vtkSlicerSceneWriter::GetApplicationLogic()
{
  if (this->ApplicationLogic)
  {
    return this->ApplicationLogic;
  }
  // Use the application logic of the file IO manager where the writer is registered
  vtkMRMLFileIOManager* fileIOManager = this->GetFileIOManager();
  return fileIOManager ? vtkSlicerApplicationLogic::SafeDownCast(fileIOManager->GetApplicationLogic()) : nullptr;
}

//----------------------------------------------------------------------------
bool vtkSlicerSceneWriter::CanWriteObject(vtkObject* object)
{
  return vtkMRMLScene::SafeDownCast(object) != nullptr;
}

//----------------------------------------------------------------------------
bool vtkSlicerSceneWriter::Write(vtkMRMLIOProperties* properties)
{
  this->ClearWrittenNodeIDs();
  if (!properties || !this->GetScene())
  {
    vtkErrorMacro("Write failed: invalid properties or scene");
    return false;
  }
  std::string fullPath = vtksys::SystemTools::CollapseFullPath(properties->GetStringProperty("fileName"));
  std::string baseDir = vtksys::SystemTools::GetFilenamePath(fullPath);
  if (!vtksys::SystemTools::TestFileAccess(baseDir, vtksys::TEST_FILE_WRITE))
  {
    vtkWarningMacro("Failed to save " << fullPath << ": Path " << baseDir << " is not writable");
    this->GetUserMessages()->AddMessage(
      vtkCommand::ErrorEvent, vtkMRMLI18N::Format(vtkMRMLTr("qSlicerSceneWriter", "Failed to save scene as %1 (path %2 is not writeable)"), fullPath.c_str(), baseDir.c_str()));
    return false;
  }

  std::string extension = vtksys::SystemTools::GetFilenameLastExtension(fullPath);
  if (extension == ".mrml")
  {
    return this->WriteToMRML(properties);
  }
  else if (extension == ".mrb")
  {
    return this->WriteToMRB(properties);
  }
  return this->WriteToDirectory(properties);
}

//----------------------------------------------------------------------------
bool vtkSlicerSceneWriter::WriteToMRML(vtkMRMLIOProperties* properties)
{
  vtkMRMLScene* scene = this->GetScene();
  // set the mrml scene url first
  std::string fullPath = vtksys::SystemTools::CollapseFullPath(properties->GetStringProperty("fileName"));
  std::string baseDir = vtksys::SystemTools::GetFilenamePath(fullPath);
  scene->SetURL(fullPath.c_str());
  scene->SetRootDirectory(baseDir.c_str());

  vtkImageData* screenShot = vtkImageData::SafeDownCast(properties->GetObjectProperty("screenShot"));
  if (screenShot)
  {
    // screenshot is provided, save along with the scene mrml file
    vtkSlicerApplicationLogic* appLogic = this->GetApplicationLogic();
    if (appLogic && appLogic->GetMRMLScene() == scene)
    {
      appLogic->SaveSceneScreenshot(screenShot);
    }
    else
    {
      vtkWarningMacro("WriteToMRML: scene screenshot is not saved because application logic is not available");
    }
  }

  // write out the mrml file
  bool success = scene->Commit();
  if (!success)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, vtkMRMLI18N::Format(vtkMRMLTr("qSlicerSceneWriter", "Failed to save scene as %1"), fullPath.c_str()));
  }
  return success;
}

//----------------------------------------------------------------------------
bool vtkSlicerSceneWriter::WriteToMRB(vtkMRMLIOProperties* properties)
{
  vtkMRMLScene* scene = this->GetScene();
  std::string fullPath = vtksys::SystemTools::CollapseFullPath(properties->GetStringProperty("fileName"));
  std::string baseDir = vtksys::SystemTools::GetFilenamePath(fullPath);

  // Save URL and root directory so next time when the scene is saved,
  // again, the same folder and filename is used by default.
  scene->SetURL(fullPath.c_str());
  scene->SetRootDirectory(baseDir.c_str());

  vtkImageData* thumbnail = vtkImageData::SafeDownCast(properties->GetObjectProperty("screenShot"));
  bool success = scene->WriteToMRB(fullPath.c_str(), thumbnail, this->GetUserMessages());
  if (!success)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, vtkMRMLI18N::Format(vtkMRMLTr("qSlicerSceneWriter", "Failed to save scene as %1"), fullPath.c_str()));
    return false;
  }

  // Mark the storable nodes as modified since read, since that flag was reset
  // when the files were written out. If there was newly generated data in the
  // scene that only got saved to the MRB bundle directory, it would be marked
  // as unmodified since read when saving as a MRML file + data. This will not
  // disrupt multiple MRB saves.
  scene->SetStorableNodesModifiedSinceRead();
  vtkDebugMacro("Saved " << fullPath);
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerSceneWriter::WriteToDirectory(vtkMRMLIOProperties* properties)
{
  std::string saveDirName = properties->GetStringProperty("fileName");
  std::string saveDirFullPath = vtksys::SystemTools::CollapseFullPath(saveDirName);
  if (!vtksys::SystemTools::FileIsDirectory(saveDirFullPath))
  {
    vtksys::SystemTools::MakeDirectory(saveDirFullPath);
  }

  // The directory must be empty
  std::string error;
  vtksys::Directory saveDir;
  if (!vtksys::SystemTools::FileIsDirectory(saveDirFullPath) || !saveDir.Load(saveDirFullPath))
  {
    error = vtkMRMLTr("qSlicerSceneWriter", "fails to be created");
  }
  else
  {
    int numFiles = 0;
    for (unsigned long fileIndex = 0; fileIndex < saveDir.GetNumberOfFiles(); ++fileIndex)
    {
      std::string fileName = saveDir.GetFile(fileIndex);
      if (fileName != "." && fileName != "..")
      {
        ++numFiles;
      }
    }
    if (numFiles == 1)
    {
      error = vtkMRMLTr("qSlicerSceneWriter", "contains 1 file or directory");
    }
    else if (numFiles > 1)
    {
      error = vtkMRMLI18N::Format(vtkMRMLTr("qSlicerSceneWriter", "contains %1 files or directories"), std::to_string(numFiles).c_str());
    }
  }
  if (!error.empty())
  {
    std::string message = vtkMRMLI18N::Format(vtkMRMLTr("qSlicerSceneWriter",
                                                        "Selected directory\n\"%1\"\n%2.\n"
                                                        "Please choose an empty directory."),
                                              saveDirName.c_str(),
                                              error.c_str());
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, message);
    return false;
  }

  vtkSlicerApplicationLogic* appLogic = this->GetApplicationLogic();
  if (!appLogic)
  {
    vtkErrorMacro("WriteToDirectory failed: application logic is not available");
    return false;
  }
  vtkImageData* imageData = vtkImageData::SafeDownCast(properties->GetObjectProperty("screenShot"));
  bool success = appLogic->SaveSceneToSlicerDataBundleDirectory(saveDirName.c_str(), imageData);
  if (success)
  {
    vtkDebugMacro("Saved scene to dir " << saveDirName);
  }
  else
  {
    vtkErrorMacro("Error saving scene to directory " << saveDirName);
  }
  return success;
}
