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

#include "vtkSlicerAnnotationsReader.h"

// Logic includes
#include "vtkSlicerMarkupsLogic.h"
#include <vtkMRMLIOOptionsDescription.h>
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLI18N.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <regex>

vtkStandardNewMacro(vtkSlicerAnnotationsReader);

//----------------------------------------------------------------------------
vtkSlicerAnnotationsReader::vtkSlicerAnnotationsReader()
{
  this->SetFileType("AnnotationFile");
  this->SetDescription(vtkMRMLTr("qSlicerAnnotationsReader", "Annotation"));
  this->SetNameFilters(std::vector<std::string>{ vtkMRMLTr("qSlicerAnnotationsReader", "Annotations") + " (*.acsv)" });
}

//----------------------------------------------------------------------------
vtkSlicerAnnotationsReader::~vtkSlicerAnnotationsReader() = default;

//----------------------------------------------------------------------------
void vtkSlicerAnnotationsReader::SetMarkupsLogic(vtkSlicerMarkupsLogic* logic)
{
  this->MarkupsLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerMarkupsLogic* vtkSlicerAnnotationsReader::GetMarkupsLogic()
{
  return this->MarkupsLogic;
}

//----------------------------------------------------------------------------
bool vtkSlicerAnnotationsReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  vtkMRMLScene* scene = this->GetScene();
  if (!properties || !this->MarkupsLogic || !scene)
  {
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");
  std::string name = properties->GetStringProperty("name", vtksys::SystemTools::GetFilenameWithoutExtension(vtksys::SystemTools::GetFilenameName(fileName)));

  // file type
  int fileType = vtkSlicerMarkupsLogic::AnnotationNone;
  std::string annotationType = properties->GetStringProperty("annotationType");
  if (annotationType == "fiducial")
  {
    fileType = vtkSlicerMarkupsLogic::AnnotationFiducial;
  }
  else if (annotationType == "ruler")
  {
    fileType = vtkSlicerMarkupsLogic::AnnotationRuler;
  }
  else if (annotationType == "roi")
  {
    fileType = vtkSlicerMarkupsLogic::AnnotationROI;
  }
  else if (properties->GetBoolProperty("fiducial"))
  {
    fileType = vtkSlicerMarkupsLogic::AnnotationFiducial;
  }
  else if (properties->GetBoolProperty("ruler"))
  {
    fileType = vtkSlicerMarkupsLogic::AnnotationRuler;
  }
  else if (properties->GetBoolProperty("roi"))
  {
    fileType = vtkSlicerMarkupsLogic::AnnotationROI;
  }

  char* nodeID = this->MarkupsLogic->LoadAnnotation(fileName.c_str(), name.c_str(), fileType);
  if (!nodeID)
  {
    return false;
  }
  this->AddLoadedNodeID(nodeID);
  if (properties->HasProperty("name"))
  {
    std::string uname = scene->GetUniqueNameByString(properties->GetStringProperty("name").c_str());
    vtkMRMLNode* node = scene->GetNodeByID(nodeID);
    if (node)
    {
      node->SetName(uname.c_str());
    }
  }
  return true;
}

//----------------------------------------------------------------------------
void vtkSlicerAnnotationsReader::GetOptionsDescription(vtkMRMLIOOptionsDescription* description)
{
  if (!description)
  {
    return;
  }
  std::vector<std::string> names;
  std::string annotationType = "fiducial";
  // Because '_' is considered as a word character, word boundary does not consider '_' as a word boundary.
  static const std::regex fiducialName(R"((\b|_)(F)(\b|_))");
  static const std::regex rulerName(R"((\b|_)(M)(\b|_))");
  static const std::regex roiName(R"((\b|_)(R)(\b|_))");
  for (const std::string& filePath : description->GetFileNames())
  {
    std::string fileName = vtksys::SystemTools::GetFilenameName(filePath);
    // base name: file name up to the first '.'
    std::string fileBaseName = fileName.substr(0, fileName.find('.'));
    if (vtksys::SystemTools::FileExists(filePath, /*isFile=*/true))
    {
      names.push_back(fileBaseName);
    }
    if (std::regex_search(fileBaseName, fiducialName))
    {
      annotationType = "fiducial";
    }
    else if (std::regex_search(fileBaseName, rulerName))
    {
      annotationType = "ruler";
    }
    else if (std::regex_search(fileBaseName, roiName))
    {
      annotationType = "roi";
    }
  }
  description->AddStringListOption("name", "", vtkMRMLTr("qSlicerAnnotationModuleIOOptionsWidget", "Name of the loaded annotation."), names);
  description->AddEnumOption("annotationType", "", "", vtkVariant(annotationType));
  description->AddEnumChoice("annotationType", vtkVariant("fiducial"), vtkMRMLTr("qSlicerAnnotationModuleIOOptionsWidget", "Fiducial"));
  description->AddEnumChoice("annotationType", vtkVariant("ruler"), vtkMRMLTr("qSlicerAnnotationModuleIOOptionsWidget", "Ruler"));
  description->AddEnumChoice("annotationType", vtkVariant("roi"), vtkMRMLTr("qSlicerAnnotationModuleIOOptionsWidget", "ROI"));
}
