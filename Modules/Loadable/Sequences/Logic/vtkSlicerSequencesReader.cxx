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

#include "vtkSlicerSequencesReader.h"

// Logic includes
#include "vtkSlicerSequencesLogic.h"
#include <vtkSlicerApplicationLogic.h>
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLI18N.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSequenceBrowserNode.h>
#include <vtkMRMLSequenceNode.h>
#include <vtkMRMLVolumeNode.h>

// vtkITK includes
#include <vtkITKImageSequenceReader.h>

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <regex>

vtkStandardNewMacro(vtkSlicerSequencesReader);

//----------------------------------------------------------------------------
vtkSlicerSequencesReader::vtkSlicerSequencesReader()
{
  this->SetFileType("SequenceFile");
  this->SetDescription(vtkMRMLTr("qSlicerSequencesReader", "Sequence"));
  this->SetNameFilters(std::vector<std::string>{ vtkMRMLTr("qSlicerSequencesReader", "Sequence") + " (*.seq.mrb *.mrb)",
                                                 vtkMRMLTr("qSlicerSequencesReader", "Volume Sequence") + " (*.seq.nrrd *.seq.nhdr)",
                                                 vtkMRMLTr("qSlicerSequencesReader", "Volume Sequence") + " (*.nrrd *.nhdr)",
                                                 vtkMRMLTr("qSlicerSequencesReader", "Volume Sequence") + " (*.nii *.nii.gz)" });
}

//----------------------------------------------------------------------------
vtkSlicerSequencesReader::~vtkSlicerSequencesReader() = default;

//----------------------------------------------------------------------------
void vtkSlicerSequencesReader::SetSequencesLogic(vtkSlicerSequencesLogic* logic)
{
  this->SequencesLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerSequencesLogic* vtkSlicerSequencesReader::GetSequencesLogic()
{
  return this->SequencesLogic;
}

//----------------------------------------------------------------------------
double vtkSlicerSequencesReader::CanLoadFileConfidence(const std::string& filePath)
{
  double confidence = this->Superclass::CanLoadFileConfidence(filePath);
  if (confidence <= 0)
  {
    return confidence;
  }

  // NIfTI files store frames along the 4th image axis and do not have a sequence-specific file extension:
  // inspect the header to check if the image contains multiple frames.
  // Such images cannot be loaded as volumes or segmentations, therefore they are loaded as sequences by default.
  // If it looks like a sequence then use confidence of 0.58 (same as for NRRD sequences, see explanation below),
  // which is higher than the confidence of volume and segmentation readers for NIfTI files (at most 0.57).
  const bool isNrrdFile = EndsWithNoCase(filePath, "NRRD") || EndsWithNoCase(filePath, "NHDR");
  const bool isSceneFile = EndsWithNoCase(filePath, ".MRB");
  if (!isNrrdFile && !isSceneFile)
  {
    return vtkITKImageSequenceReader::IsImageSequenceFile(filePath.c_str()) ? 0.58 : 0.4;
  }

  // Confidence for .nrrd and .nhdr file is 0.55 (5 characters in the file extension matched),
  // for composite file extensions (.seq.nhdr) it would be 0.59.
  // Therefore, confidence below 0.56 means that we got a generic file extension
  // that we need to inspect further.
  // Unzipping the mrb file to inspect if it looks like a sequence would be too time-consuming,
  // therefore we only check NRRD files for now.
  if (confidence < 0.56 && isNrrdFile)
  {
    // The dimension and kinds fields are usually found at around position 500, but we
    // read a bit more just in case there are some extra fields.
    std::string header = ReadFileHeader(filePath, 800);
    if (!header.empty())
    {
      bool looksLikeSequence = false;
      // Supported 4D/5D NRRD files contain "dimension: 4" or "dimension: 5" line.
      static const std::regex dimensionRe("dimension:([^\\n]+)");
      std::smatch dimensionMatch;
      if (std::regex_search(header, dimensionMatch, dimensionRe))
      {
        int dimension = 0;
        bool ok = false;
        try
        {
          size_t parsedLength = 0;
          std::string dimensionStr = dimensionMatch[1].str();
          dimension = std::stoi(dimensionStr, &parsedLength);
          // Accept trailing whitespace only (same as QString::toInt)
          ok = dimensionStr.find_first_not_of(" \t\r", parsedLength) == std::string::npos;
        }
        catch (...)
        {
          ok = false;
        }
        if (ok && dimension == 4)
        {
          // Supported 4D NRRD files "kinds" field contain "time" or "list" axis.
          // We don't want to load 3D+color images or displacement field volumes.
          // For example: "kinds: space space space list".
          static const std::regex kindsRe("kinds:([^\\n]+)");
          std::smatch kindsMatch;
          if (std::regex_search(header, kindsMatch, kindsRe))
          {
            std::string kindsStr = kindsMatch[1].str();
            if (kindsStr.find("list") != std::string::npos || kindsStr.find("time") != std::string::npos)
            {
              looksLikeSequence = true;
            }
          }
          if (!looksLikeSequence)
          {
            // 4D image without list axis (for example, all axes are "domain" kind) cannot be loaded
            // as a volume or segmentation, but it can be loaded as a sequence.
            looksLikeSequence = vtkITKImageSequenceReader::IsImageSequenceFile(filePath.c_str());
          }
        }
        else if (ok && dimension == 5)
        {
          // Transform sequence
          looksLikeSequence = true;
        }
      }
      // If it looks like sequence then we need to set a confidence value that is larger than 0.55.
      // However, if we get a 4D sequence it may be some other 4D data set, such as .seg.nrrd.
      // We would not want a .seg.nrrd file to be recognized as sequence by default, so we need to set
      // the confidence value to smaller than 0.59. Therefore, if it looks like a sequence then we
      // use confidence of 0.58.
      confidence = (looksLikeSequence ? 0.58 : 0.4);
    }
  }
  return confidence;
}

//----------------------------------------------------------------------------
bool vtkSlicerSequencesReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  if (!this->SequencesLogic)
  {
    vtkErrorMacro("Load failed: Sequences logic is invalid.");
    return false;
  }
  vtkMRMLScene* scene = this->GetScene();
  if (!properties || !scene)
  {
    vtkErrorMacro("Load failed: invalid properties or scene");
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");

  vtkMRMLSequenceNode* node = this->SequencesLogic->AddSequence(fileName.c_str(), this->GetUserMessages());
  if (!node)
  {
    // errors are already logged and userMessages contain details that can be displayed to users
    return false;
  }

  if (properties->HasProperty("name"))
  {
    std::string customName = scene->GetUniqueNameByString(properties->GetStringProperty("name").c_str());
    node->SetName(customName.c_str());
  }

  std::vector<std::string> loadedNodeIDs;
  loadedNodeIDs.push_back(node->GetID());

  bool show = properties->GetBoolProperty("show", true); // show volume node in viewers
  vtkMRMLSequenceBrowserNode* browserNode = nullptr;
  if (show)
  {
    std::string browserCustomName = std::string(node->GetName()) + " browser";
    browserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceBrowserNode", browserCustomName));
  }
  if (browserNode)
  {
    loadedNodeIDs.push_back(browserNode->GetID());
    browserNode->SetAndObserveMasterSequenceNodeID(node->GetID());
    // Allow the application to show the sequence browser toolbar
    this->InvokeEvent(ShowSequenceBrowserRequestedEvent, browserNode);
    this->SequencesLogic->UpdateProxyNodesFromSequences(browserNode);
    vtkMRMLNode* proxyNode = browserNode->GetProxyNode(node);

    // Associate color node
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(proxyNode);
    if (displayableNode && properties->HasProperty("colorNodeID") && displayableNode->GetDisplayNode())
    {
      displayableNode->GetDisplayNode()->SetAndObserveColorNodeID(properties->GetStringProperty("colorNodeID").c_str());
    }

    // Propagate volume selection
    vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(proxyNode);
    if (volumeNode)
    {
      vtkSlicerApplicationLogic* appLogic = this->SequencesLogic->GetApplicationLogic();
      vtkMRMLSelectionNode* selectionNode = appLogic ? appLogic->GetSelectionNode() : nullptr;
      if (selectionNode)
      {
        if (vtkMRMLLabelMapVolumeNode::SafeDownCast(volumeNode))
        {
          selectionNode->SetActiveLabelVolumeID(volumeNode->GetID());
        }
        else
        {
          selectionNode->SetActiveVolumeID(volumeNode->GetID());
        }
        appLogic->PropagateVolumeSelection(); // includes FitSliceToBackground by default
      }
    }
  }

  this->SetLoadedNodeIDs(loadedNodeIDs);
  return true;
}
