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

#include "vtkSlicerTablesReader.h"

// Logic includes
#include "vtkSlicerTablesLogic.h"
#include <vtkSlicerApplicationLogic.h>
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLI18N.h>
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLTableNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSQLiteDatabase.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkSlicerTablesReader);

//----------------------------------------------------------------------------
vtkSlicerTablesReader::vtkSlicerTablesReader()
{
  this->SetFileType("TableFile");
  this->SetDescription("Table");
  this->SetNameFilters(std::vector<std::string>{ "Table (*.tsv)", "Table (*.csv)", "Table (*.txt)", "Table (*.db)", "Table (*.db3)", "Table (*.sqlite)", "Table (*.sqlite3)" });
}

//----------------------------------------------------------------------------
vtkSlicerTablesReader::~vtkSlicerTablesReader() = default;

//----------------------------------------------------------------------------
void vtkSlicerTablesReader::SetTablesLogic(vtkSlicerTablesLogic* logic)
{
  this->TablesLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerTablesLogic* vtkSlicerTablesReader::GetTablesLogic()
{
  return this->TablesLogic;
}

//----------------------------------------------------------------------------
double vtkSlicerTablesReader::CanLoadFileConfidence(const std::string& filePath)
{
  double confidence = this->Superclass::CanLoadFileConfidence(filePath);
  // .txt file is more likely a simple text file than a table
  if (confidence > 0 && EndsWithNoCase(filePath, "TXT"))
  {
    confidence = 0.4;
  }
  return confidence;
}

//----------------------------------------------------------------------------
bool vtkSlicerTablesReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  this->GetUserMessages()->ClearMessages();
  vtkMRMLScene* scene = this->GetScene();
  if (!properties || !scene)
  {
    vtkErrorMacro("Load failed: invalid properties or scene");
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");
  std::string name = properties->GetStringProperty("name", vtksys::SystemTools::GetFilenameWithoutExtension(vtksys::SystemTools::GetFilenameName(fileName)));
  std::string uname = scene->GetUniqueNameByString(name.c_str());
  std::string password = properties->GetStringProperty("password");

  // Check if the file is sqlite
  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fileName);
  if (extension.empty())
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                        vtkMRMLI18N::Format(vtkMRMLTr("qSlicerTablesReader", "Table reading failed: no file extension specified: %1"), fileName.c_str()));
    return false;
  }
  if (extension == ".db" || extension == ".db3" || extension == ".sqlite" || extension == ".sqlite3")
  {
    uname = "";
    if (password.empty())
    {
      std::string dbname = std::string("sqlite://") + fileName;
      vtkSmartPointer<vtkSQLiteDatabase> database = vtkSmartPointer<vtkSQLiteDatabase>::Take(vtkSQLiteDatabase::SafeDownCast(vtkSQLiteDatabase::CreateFromURL(dbname.c_str())));
      if (database && !database->Open("", vtkSQLiteDatabase::USE_EXISTING))
      {
        // Give a chance to the application to ask the user for a password
        this->InvokeEvent(PasswordRequestedEvent, &password);
      }
    }
  }

  vtkMRMLTableNode* node = nullptr;
  if (this->TablesLogic)
  {
    node = this->TablesLogic->AddTable(fileName.c_str(), uname.c_str(), true, password.c_str(), this->GetUserMessages());
  }
  if (!node)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, vtkMRMLI18N::Format(vtkMRMLTr("qSlicerTablesReader", "Failed to read table from  '%1'"), fileName.c_str()));
    return false;
  }

  // Show table in viewers
  vtkSlicerApplicationLogic* appLogic = this->TablesLogic->GetApplicationLogic();
  vtkMRMLSelectionNode* selectionNode = appLogic ? appLogic->GetSelectionNode() : nullptr;
  if (selectionNode)
  {
    selectionNode->SetActiveTableID(node->GetID());
  }
  if (appLogic)
  {
    appLogic->PropagateTableSelection();
  }
  this->AddLoadedNodeID(node->GetID());
  return true;
}
