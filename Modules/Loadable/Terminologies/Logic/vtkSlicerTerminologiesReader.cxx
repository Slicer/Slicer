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

#include "vtkSlicerTerminologiesReader.h"

// Logic includes
#include "vtkSlicerTerminologiesModuleLogic.h"
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLI18N.h>

// VTK includes
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkSlicerTerminologiesReader);

//----------------------------------------------------------------------------
vtkSlicerTerminologiesReader::vtkSlicerTerminologiesReader()
{
  this->SetFileType("TerminologyFile");
  this->SetDescription(vtkMRMLTr("qSlicerTerminologiesReader", "Terminology"));
  this->SetNameFilters(std::vector<std::string>{ "Terminology (*.term.json)", "Terminology (*.json)" });
}

//----------------------------------------------------------------------------
vtkSlicerTerminologiesReader::~vtkSlicerTerminologiesReader() = default;

//----------------------------------------------------------------------------
void vtkSlicerTerminologiesReader::SetTerminologiesLogic(vtkSlicerTerminologiesModuleLogic* logic)
{
  this->TerminologiesLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic* vtkSlicerTerminologiesReader::GetTerminologiesLogic()
{
  return this->TerminologiesLogic;
}

//----------------------------------------------------------------------------
double vtkSlicerTerminologiesReader::CanLoadFileConfidence(const std::string& filePath)
{
  double confidence = this->Superclass::CanLoadFileConfidence(filePath);
  // Confidence for .json file is 0.55 (5 characters in the file extension matched),
  // for composite file extensions (.term.json) it would be 0.6.
  // Therefore, confidence below 0.56 means that we got a generic file extension
  // that we need to inspect further.
  if (confidence > 0 && confidence < 0.56 && EndsWithNoCase(filePath, "JSON"))
  {
    // Terminology json files contain a schema URL like /anatomic-context-schema.json
    // or /segment-context-schema.json around position 200, read a bit further
    // to account for slight variations in the header.
    std::string header = ReadFileHeader(filePath, 400);
    if (!header.empty())
    {
      bool looksLikeTerminology = header.find("/anatomic-context-schema.json") != std::string::npos //
                                  || header.find("/segment-context-schema.json") != std::string::npos;
      confidence = (looksLikeTerminology ? 0.6 : 0.4);
    }
  }
  return confidence;
}

//----------------------------------------------------------------------------
bool vtkSlicerTerminologiesReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  if (!properties || !this->TerminologiesLogic)
  {
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");
  return this->TerminologiesLogic->LoadContextFromFile(fileName);
}
