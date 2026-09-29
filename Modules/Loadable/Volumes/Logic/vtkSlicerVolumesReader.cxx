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

#include "vtkSlicerVolumesReader.h"

// Logic includes
#include "vtkSlicerVolumesLogic.h"
#include <vtkSlicerApplicationLogic.h>
#include <vtkMRMLFileIOManager.h>
#include <vtkMRMLIOOptionsDescription.h>
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLColorLogic.h>
#include <vtkMRMLI18N.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLVolumeArchetypeStorageNode.h>
#include <vtkMRMLVolumeDisplayNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkWeakPointer.h>
#include <vtksys/FStream.hxx>
#include <vtksys/SystemTools.hxx>

// ITK includes
#include <itkArchetypeSeriesFileNames.h>

// STD includes
#include <algorithm>
#include <regex>

vtkStandardNewMacro(vtkSlicerVolumesReader);

//----------------------------------------------------------------------------
vtkSlicerVolumesReader::vtkSlicerVolumesReader()
{
  this->SetFileType("VolumeFile");
  this->SetDescription(vtkMRMLTr("qSlicerVolumesReader", "Volume"));
  // pic files are bio-rad images (see itkBioRadImageIO)
  this->SetNameFilters(
    std::vector<std::string>{ vtkMRMLTr("qSlicerVolumesReader", "Volume") + " (*.hdr *.nhdr *.nrrd *.mhd *.mha *.mnc *.nii *.nii.gz *.mgh *.mgz *.mgh.gz *.img *.img.gz *.pic)",
                              vtkMRMLTr("qSlicerVolumesReader", "Dicom") + " (*.dcm *.ima)",
                              vtkMRMLTr("qSlicerVolumesReader", "Image") + " (*.png *.tif *.tiff *.jpg *.jpeg)",
                              vtkMRMLTr("qSlicerVolumesReader", "All Files") + " (*)" });
}

//----------------------------------------------------------------------------
vtkSlicerVolumesReader::~vtkSlicerVolumesReader() = default;

//----------------------------------------------------------------------------
void vtkSlicerVolumesReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesReader::SetVolumesLogic(vtkSlicerVolumesLogic* logic)
{
  this->VolumesLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic* vtkSlicerVolumesReader::GetVolumesLogic()
{
  return this->VolumesLogic;
}

//----------------------------------------------------------------------------
double vtkSlicerVolumesReader::CanLoadFileConfidence(const std::string& filePath)
{
  double confidence = this->Superclass::CanLoadFileConfidence(filePath);
  // Confidence for .nrrd and .nhdr file is 0.55 (5 characters in the file extension matched)
  if (confidence <= 0)
  {
    return confidence;
  }
  // Inspect the content to recognize DWI volumes.
  if (!EndsWithNoCase(filePath, "NRRD") && !EndsWithNoCase(filePath, "NHDR"))
  {
    return confidence;
  }
  vtksys::ifstream file(filePath.c_str(), std::ios::in);
  if (!file.is_open())
  {
    return confidence;
  }
  std::string line;
  std::getline(file, line);
  if (line.compare(0, 4, "NRRD") != 0)
  {
    return confidence;
  }
  // The nrrd header is separated by the data by a blank line, so read everything up to there
  // since diffusion scans can have a long list of gradients and modality can be
  // near the end of the header
  static const std::regex modalityRe("modality:([^\\n]+)");
  while (std::getline(file, line))
  {
    if (line.find_first_not_of(" \t\r\n") == std::string::npos)
    {
      break;
    }
    std::smatch modalityMatch;
    if (std::regex_search(line, modalityMatch, modalityRe))
    {
      std::string modalityStr = modalityMatch[1].str();
      std::transform(modalityStr.begin(), modalityStr.end(), modalityStr.begin(), ::tolower);
      if (modalityStr.find("dwmri") != std::string::npos)
      {
        // This is a DWMRI image, we are confident that it is not just a general image sequence.
        // Therefore we set higher confidence than the generic sequence reader's confidence of 0.6.
        confidence = 0.7;
        break;
      }
    }
  }
  return confidence;
}

//----------------------------------------------------------------------------
bool vtkSlicerVolumesReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  if (!properties || !this->VolumesLogic)
  {
    vtkErrorMacro("Load failed: invalid properties or volumes logic");
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");
  std::string name = properties->GetStringProperty("name", vtksys::SystemTools::GetFilenameWithoutExtension(vtksys::SystemTools::GetFilenameName(fileName)));

  int options = 0;
  options |= properties->GetBoolProperty("labelmap") ? 0x1 : 0x0;
  options |= properties->GetBoolProperty("center") ? 0x2 : 0x0;
  options |= properties->GetBoolProperty("singleFile") ? 0x4 : 0x0;
  options |= properties->GetBoolProperty("autoWindowLevel") ? 0x8 : 0x0;
  options |= properties->GetBoolProperty("discardOrientation") ? 0x10 : 0x0;
  bool propagateVolumeSelection = properties->GetBoolProperty("show", true);

  vtkSmartPointer<vtkStringArray> fileList;
  if (properties->HasProperty("fileNames"))
  {
    fileList = vtkSmartPointer<vtkStringArray>::New();
    for (const std::string& seriesFileName : properties->GetStringListProperty("fileNames"))
    {
      fileList->InsertNextValue(seriesFileName);
    }
  }

  this->GetUserMessages()->ClearMessages();
  // Weak pointer is used because the node may be deleted if the scene is closed
  // right after reading.
  vtkWeakPointer<vtkMRMLVolumeNode> node = this->VolumesLogic->AddArchetypeVolume(fileName.c_str(), name.c_str(), options, fileList, this->GetUserMessages());
  if (!node)
  {
    return false;
  }
  std::string colorNodeID = properties->GetStringProperty("colorNodeID");
  if (!colorNodeID.empty())
  {
    vtkMRMLVolumeDisplayNode* displayNode = node->GetVolumeDisplayNode();
    if (displayNode)
    {
      displayNode->SetAndObserveColorNodeID(colorNodeID.c_str());
    }
  }
  if (propagateVolumeSelection)
  {
    vtkSlicerApplicationLogic* appLogic = this->VolumesLogic->GetApplicationLogic();
    vtkMRMLSelectionNode* selectionNode = appLogic ? appLogic->GetSelectionNode() : nullptr;
    if (selectionNode)
    {
      if (vtkMRMLLabelMapVolumeNode::SafeDownCast(node))
      {
        selectionNode->SetActiveLabelVolumeID(node->GetID());
      }
      else
      {
        selectionNode->SetActiveVolumeID(node->GetID());
      }
      appLogic->PropagateVolumeSelection(); // includes FitSliceToBackground by default
    }
  }
  this->AddLoadedNodeID(node->GetID());
  return true;
}

//----------------------------------------------------------------------------
std::string vtkSlicerVolumesReader::ExamineFileList(std::vector<std::string>& fileList, vtkMRMLIOProperties* ioProperties)
{
  if (!ioProperties)
  {
    return std::string();
  }
  //
  // Check each file to see if it's recognized as part of a series.  If so,
  // keep it as the archetype and remove all the others from the list
  //
  for (size_t fileIndex = 0; fileIndex < fileList.size(); ++fileIndex)
  {
    const std::string archetypeFile = fileList[fileIndex];
    std::vector<std::string> candidateFiles;
    try
    {
      itk::ArchetypeSeriesFileNames::Pointer seriesNames = itk::ArchetypeSeriesFileNames::New();
      seriesNames->SetArchetype(archetypeFile);
      candidateFiles = seriesNames->GetFileNames();
    }
    catch (itk::ExceptionObject& exception)
    {
      // For example, the directory of the file cannot be read
      vtkDebugMacro("ExamineFileList: cannot examine file " << archetypeFile << ": " << exception.GetDescription());
      continue;
    }
    if (candidateFiles.size() <= 1)
    {
      continue;
    }
    std::vector<std::string> remainingFiles;
    for (const std::string& path : fileList)
    {
      if (path != archetypeFile && std::find(candidateFiles.begin(), candidateFiles.end(), path) != candidateFiles.end())
      {
        continue;
      }
      remainingFiles.push_back(path);
    }
    fileList = remainingFiles;
    ioProperties->SetBoolProperty("singleFile", false);
    return archetypeFile;
  }
  return std::string();
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesReader::GetOptionsDescription(vtkMRMLIOOptionsDescription* description)
{
  if (!description)
  {
    return;
  }
  // Determine default options from the file names
  std::vector<std::string> names;
  bool onlyNumberInName = false;
  bool onlyNumberInExtension = false;
  bool hasLabelMapName = false;
  // Storage node is used for getting the file name without the known file extension
  // (to determine node name accurately when there are multiple '.' characters in the filename).
  vtkNew<vtkMRMLVolumeArchetypeStorageNode> storageNode;
  storageNode->SetScene(this->GetScene());
  // Because '_' is considered as a word character (\w), \b
  // doesn't consider '_' as a word boundary.
  static const std::regex labelMapName(R"((\b|_)([Ll]abel(s)?)(\b|_))");
  static const std::regex segName(R"((\b|_)([Ss]eg)(\b|_))");
  static const std::regex onlyNumber(R"(^[0-9.\-_@()~]+$)");
  for (const std::string& filePath : description->GetFileNames())
  {
    std::string fileName = vtksys::SystemTools::GetFilenameName(filePath);
    // base name: file name up to the first '.'
    std::string fileBaseName = fileName.substr(0, fileName.find('.'));
    if (vtksys::SystemTools::FileExists(filePath, /*isFile=*/true))
    {
      fileBaseName = storageNode->GetFileNameWithoutExtension(fileName.c_str());
      names.push_back(fileBaseName);
      // Single file
      // If the name (or the extension) is just a number, then it must be a 2D
      // slice from a 3D volume, so uncheck Single File.
      onlyNumberInName = std::regex_match(fileBaseName, onlyNumber);
      std::string::size_type lastDot = fileName.rfind('.');
      std::string suffix = (lastDot == std::string::npos) ? std::string() : fileName.substr(lastDot + 1);
      onlyNumberInExtension = !suffix.empty() && suffix.find_first_not_of("0123456789") == std::string::npos;
    }
    if (std::regex_search(fileBaseName, labelMapName) || std::regex_search(fileBaseName, segName))
    {
      hasLabelMapName = true;
    }
  }

  description->AddStringListOption("name", "", vtkMRMLTr("qSlicerVolumesIOOptionsWidget", "Name of the loaded volume. Separate names by ';' if multiple files are loaded."), names);
  description->AddBoolOption("labelmap",
                             vtkMRMLTr("qSlicerVolumesIOOptionsWidget", "LabelMap"),
                             vtkMRMLTr("qSlicerVolumesIOOptionsWidget", "Load the volume as a labelmap (each voxel value representing a segmented structure)."),
                             hasLabelMapName);
  description->AddBoolOption(
    "singleFile",
    vtkMRMLTr("qSlicerVolumesIOOptionsWidget", "Single File"),
    vtkMRMLTr("qSlicerVolumesIOOptionsWidget", "Only load the selected file. The application will not attempt to look for similar files that can make up the complete volume."),
    !onlyNumberInName && !onlyNumberInExtension);
  description->AddBoolOption("center",
                             vtkMRMLTr("qSlicerVolumesIOOptionsWidget", "Centered"),
                             vtkMRMLTr("qSlicerVolumesIOOptionsWidget", "Ignore image position information that is specified in the image header."),
                             false);
  description->AddBoolOption("discardOrientation",
                             vtkMRMLTr("qSlicerVolumesIOOptionsWidget", "Ignore Orientation"),
                             vtkMRMLTr("qSlicerVolumesIOOptionsWidget", "Ignore axis orientation information that is specified in the image header."),
                             false);
  description->AddBoolOption(
    "show", vtkMRMLTr("qSlicerVolumesIOOptionsWidget", "Show"), vtkMRMLTr("qSlicerVolumesIOOptionsWidget", "Show volume in slice viewers after loading is completed."), true);

  // Default color node depends on whether the volume is loaded as a labelmap
  std::string defaultColorNodeID;
  vtkMRMLApplicationLogic* appLogic = this->VolumesLogic ? this->VolumesLogic->GetMRMLApplicationLogic() : nullptr;
  if (!appLogic && this->GetFileIOManager())
  {
    appLogic = this->GetFileIOManager()->GetApplicationLogic();
  }
  vtkMRMLColorLogic* colorLogic = appLogic ? appLogic->GetColorLogic() : nullptr;
  if (colorLogic)
  {
    const char* colorNodeID = description->GetBoolOptionValue("labelmap") ? colorLogic->GetDefaultLabelMapColorNodeID() : colorLogic->GetDefaultVolumeColorNodeID();
    defaultColorNodeID = colorNodeID ? colorNodeID : "";
  }
  description->AddNodeOption("colorNodeID",
                             "",
                             vtkMRMLTr("qSlicerVolumesIOOptionsWidget", "Color table node used to display this volume."),
                             defaultColorNodeID,
                             std::vector<std::string>{ "vtkMRMLColorTableNode", "vtkMRMLProceduralColorNode" },
                             false);
  description->SetOptionShowHidden("colorNodeID", true);
  description->SetOptionWidget("colorNodeID", "colorTable");
}
