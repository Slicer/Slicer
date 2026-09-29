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

#include "vtkSlicerColorsReader.h"

// Logic includes
#include "vtkSlicerColorLogic.h"
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLColorNode.h>

// VTK includes
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkSlicerColorsReader);

//----------------------------------------------------------------------------
vtkSlicerColorsReader::vtkSlicerColorsReader()
{
  this->SetFileType("ColorTableFile");
  this->SetDescription("Color");
  this->SetNameFilters(std::vector<std::string>{ "Color (*.csv *.tsv *.txt *.ctbl *.cxml)" });
}

//----------------------------------------------------------------------------
vtkSlicerColorsReader::~vtkSlicerColorsReader() = default;

//----------------------------------------------------------------------------
void vtkSlicerColorsReader::SetColorLogic(vtkSlicerColorLogic* logic)
{
  this->ColorLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerColorLogic* vtkSlicerColorsReader::GetColorLogic()
{
  return this->ColorLogic;
}

//----------------------------------------------------------------------------
double vtkSlicerColorsReader::CanLoadFileConfidence(const std::string& filePath)
{
  double confidence = this->Superclass::CanLoadFileConfidence(filePath);
  // Confidence for .txt file is 0.54 (4 characters in the file extension matched),
  // for more specific file extensions (.ctbl, .cxml) it would be 0.55.
  // Therefore, confidence below 0.55 means that we got a generic file extension
  // that we need to inspect further.
  if (confidence > 0 && confidence < 0.55 && EndsWithNoCase(filePath, "TXT"))
  {
    // Color table text files start with "# Color table file"
    std::string header = ReadFileHeader(filePath, 100);
    if (!header.empty())
    {
      confidence = (header.find("# Color table file") != std::string::npos ? 0.6 : 0.4);
    }
  }
  return confidence;
}

//----------------------------------------------------------------------------
bool vtkSlicerColorsReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  if (!properties || !this->ColorLogic)
  {
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");
  const bool userType = true; // allow editing of color nodes loaded via GUI
  vtkMRMLColorNode* node = this->ColorLogic->LoadColorFile(fileName.c_str(), nullptr, this->GetUserMessages(), userType);
  if (!node)
  {
    return false;
  }
  this->AddLoadedNodeID(node->GetID());
  return true;
}
