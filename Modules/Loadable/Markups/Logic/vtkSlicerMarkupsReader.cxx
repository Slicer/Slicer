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

#include "vtkSlicerMarkupsReader.h"

// Logic includes
#include "vtkSlicerMarkupsLogic.h"
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLI18N.h>
#include <vtkMRMLMessageCollection.h>

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

vtkStandardNewMacro(vtkSlicerMarkupsReader);

//----------------------------------------------------------------------------
vtkSlicerMarkupsReader::vtkSlicerMarkupsReader()
{
  this->SetFileType("MarkupsFile");
  this->SetDescription(vtkMRMLTr("qSlicerMarkupsReader", "Markups"));
  this->SetNameFilters(std::vector<std::string>{ vtkMRMLTr("qSlicerMarkupsReader", "Markups") + " (*.mrk.json)",
                                                 vtkMRMLTr("qSlicerMarkupsReader", "Markups") + " (*.json)",
                                                 vtkMRMLTr("qSlicerMarkupsReader", "Markups Fiducials") + " (*.fcsv)" });
}

//----------------------------------------------------------------------------
vtkSlicerMarkupsReader::~vtkSlicerMarkupsReader() = default;

//----------------------------------------------------------------------------
void vtkSlicerMarkupsReader::SetMarkupsLogic(vtkSlicerMarkupsLogic* logic)
{
  this->MarkupsLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerMarkupsLogic* vtkSlicerMarkupsReader::GetMarkupsLogic()
{
  return this->MarkupsLogic;
}

//----------------------------------------------------------------------------
double vtkSlicerMarkupsReader::CanLoadFileConfidence(const std::string& filePath)
{
  double confidence = this->Superclass::CanLoadFileConfidence(filePath);
  // Confidence for .json file is 0.55 (5 characters in the file extension matched),
  // for composite file extensions (.mrk.json) it would be 0.59.
  // Therefore, confidence below 0.56 means that we got a generic file extension
  // that we need to inspect further.
  if (confidence > 0 && confidence < 0.56 && EndsWithNoCase(filePath, "JSON"))
  {
    // Markups json files contain a schema URL like .../Schema/markups-schema-v1.0.4.json
    // around position 150, read a bit further to account for slight variations in the header.
    std::string header = ReadFileHeader(filePath, 300);
    if (!header.empty())
    {
      confidence = (header.find("/markups-schema-v1.") != std::string::npos ? 0.6 : 0.4);
    }
  }
  return confidence;
}

//----------------------------------------------------------------------------
bool vtkSlicerMarkupsReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  if (!properties || !this->MarkupsLogic)
  {
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");
  std::string name = properties->GetStringProperty("name");

  // pass to logic to do the loading
  this->GetUserMessages()->ClearMessages();
  char* nodeIDs = this->MarkupsLogic->LoadMarkups(fileName.c_str(), name.c_str(), this->GetUserMessages());
  if (!nodeIDs)
  {
    return false;
  }
  // returned a comma separated list of ids of the nodes that were loaded
  std::stringstream nodeIDsStream(nodeIDs);
  std::string nodeID;
  while (std::getline(nodeIDsStream, nodeID, ','))
  {
    if (!nodeID.empty())
    {
      this->AddLoadedNodeID(nodeID.c_str());
    }
  }
  return true;
}
