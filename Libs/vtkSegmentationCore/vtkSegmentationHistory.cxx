/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SegmentationCore includes
#include "vtkSegmentationHistory.h"
#include "vtkSegmentationConverterFactory.h"
#include "vtkSegmentation.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>

// std includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSegmentationHistory);

//----------------------------------------------------------------------------
vtkSegmentationHistory::vtkSegmentationHistory()
{
  this->Segmentation = nullptr;

  this->MaximumNumberOfStates = 5;

  this->LastRestoredState = 0;
  this->RestoreStateInProgress = false;

  this->SegmentationModifiedCallbackCommand = vtkCallbackCommand::New();
  this->SegmentationModifiedCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->SegmentationModifiedCallbackCommand->SetCallback(vtkSegmentationHistory::OnSegmentationModified);
}

//----------------------------------------------------------------------------
vtkSegmentationHistory::~vtkSegmentationHistory()
{
  this->SetSegmentation(nullptr);

  if (this->SegmentationModifiedCallbackCommand)
    {
    this->SegmentationModifiedCallbackCommand->SetClientData(nullptr);
    this->SegmentationModifiedCallbackCommand->Delete();
    this->SegmentationModifiedCallbackCommand = nullptr;
    }
}

//-----------------------------------------------------------------------------
void vtkSegmentationHistory::SetSegmentation(vtkSegmentation* segmentation)
{
  if (segmentation == this->Segmentation)
    {
    return;
    }

  if (this->Segmentation)
    {
    this->Segmentation->RemoveObserver(this->SegmentationModifiedCallbackCommand);
    }
  this->RemoveAllStates();

  vtkSetObjectBodyMacro(Segmentation, vtkSegmentation, segmentation);
  if (this->Segmentation)
    {
    // These events invalidate future states (no redo will be available after these)
    this->Segmentation->AddObserver(vtkSegmentation::SegmentAdded, this->SegmentationModifiedCallbackCommand);
    this->Segmentation->AddObserver(vtkSegmentation::SegmentRemoved, this->SegmentationModifiedCallbackCommand);
    this->Segmentation->AddObserver(vtkSegmentation::SegmentModified, this->SegmentationModifiedCallbackCommand);
    this->Segmentation->AddObserver(vtkSegmentation::MasterRepresentationModified, this->SegmentationModifiedCallbackCommand);
    //this->Segmentation->AddObserver(vtkSegmentation::ContainedRepresentationNamesModified, this->SegmentationModifiedCallbackCommand);
    }
}

//----------------------------------------------------------------------------
void vtkSegmentationHistory::PrintSelf(ostream& os, vtkIndent indent)
{
  // vtkObject's PrintSelf prints a long list of registered events, which
  // is too long and not useful, therefore we don't call vtkObject::PrintSelf
  // but print essential information on the vtkObject base.
  os << indent << "Debug: " << (this->Debug ? "On\n" : "Off\n");
  os << indent << "Modified Time: " << this->GetMTime() << "\n";

  os << indent << "Number of saved states:  " << this->SegmentationStates.size() << "\n";
}

//---------------------------------------------------------------------------
bool vtkSegmentationHistory::SaveState()
{
  if (this->Segmentation == nullptr)
    {
    vtkWarningMacro("vtkSegmentation::SaveState failed: segmentation is invalid");
    return false;
    }

  if (this->GetMaximumNumberOfStates() < 1)
    {
    vtkWarningMacro("vtkSegmentation::SaveState failed: MaximumNumberOfStates is less than 1");
    return false;
    }

  this->RemoveAllNextStates();

  SegmentationState newSegmentationState;

  std::vector<std::string> segmentIDs;
  this->Segmentation->GetSegmentIDs(segmentIDs);
  newSegmentationState.SegmentIds = segmentIDs;
  std::map<vtkDataObject*, vtkDataObject*> savedObjects;
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
    {
    vtkSegment* segment = this->Segmentation->GetSegment(*segmentIDIt);
    if (segment == nullptr)
      {
      vtkErrorMacro("Failed to save state of segment " << *segmentIDIt);
      continue;
      }
    // Previous saved state of the segment
    // (if the new state has exactly the same representation then only a shallow copy will be made)
    vtkSegment* baselineSegment = nullptr;
    if (this->SegmentationStates.size() > 0)
      {
      SegmentsMap::iterator baselineSegmentIt = this->SegmentationStates.back().Segments.find(*segmentIDIt);
      if (baselineSegmentIt != this->SegmentationStates.back().Segments.end())
        {
        baselineSegment = baselineSegmentIt->second.GetPointer();
        }
      }

    vtkSmartPointer<vtkSegment> segmentClone = vtkSmartPointer<vtkSegment>::New();
    vtkSegmentation::CopySegment(segmentClone, segment, baselineSegment, savedObjects);
    newSegmentationState.Segments[*segmentIDIt] = segmentClone;
    }
  this->SegmentationStates.push_back(newSegmentationState);

  // Set the current state as last restored state
  this->LastRestoredState = (unsigned int)this->SegmentationStates.size();
  this->RemoveAllObsoleteStates();

  this->Modified();
  return true;
}

//---------------------------------------------------------------------------
bool vtkSegmentationHistory::RestorePreviousState()
{
  if (this->Segmentation == nullptr)
    {
    vtkWarningMacro("vtkSegmentation::RestorePreviousState failed: segmentation is invalid");
    return false;
    }

  if (this->LastRestoredState < 1)
    {
    vtkWarningMacro("vtkSegmentation::RestorePreviousState failed: There are no previous state available for restore");
    return false;
    }
  if (this->SegmentationStates.size() < this->LastRestoredState)
    {
    vtkErrorMacro("vtkSegmentation::RestorePreviousState failed: There are no previous state available for restore (internal error)");
    return false;
    }
  int stateToRestore = this->LastRestoredState - 1;
  if (this->SegmentationStates.size() == this->LastRestoredState)
    {
    // Save the current state to make sure the user can redo the undo operation
    this->SaveState();
    // this->SegmentationStates.size() - 1 is the state that we've just saved
    // this->SegmentationStates.size() - 2 is the state that was the last saved state before
    stateToRestore = (int)this->SegmentationStates.size() - 2;
    }
  return this->RestoreState(stateToRestore);
}

//---------------------------------------------------------------------------
bool vtkSegmentationHistory::RestoreNextState()
{
  if (this->Segmentation == nullptr)
    {
    vtkWarningMacro("vtkSegmentation::RestoreNextState failed: segmentation is invalid");
    return false;
    }
  if (this->LastRestoredState + 1 >= this->SegmentationStates.size())
    {
    vtkWarningMacro("vtkSegmentation::RestoreNextState failed: There are no next state available for restore");
    return false;
    }
  return this->RestoreState(this->LastRestoredState + 1);
}

//---------------------------------------------------------------------------
bool vtkSegmentationHistory::RestoreState(unsigned int stateIndex)
{
  this->RestoreStateInProgress = true;

  SegmentationState restoredState = this->SegmentationStates[stateIndex];

  std::set<std::string> segmentIDsToKeep;
  std::map<vtkDataObject*, vtkDataObject*> restoredRepresentations;
  for (SegmentsMap::iterator restoredSegmentsIt = restoredState.Segments.begin();
    restoredSegmentsIt != restoredState.Segments.end(); ++restoredSegmentsIt)
    {
    vtkSegment* segmentToRestore = restoredSegmentsIt->second;
    segmentIDsToKeep.insert(restoredSegmentsIt->first);
    vtkSmartPointer<vtkSegment> segment = this->Segmentation->GetSegment(restoredSegmentsIt->first);
    if (segment == nullptr)
      {
      segment = vtkSmartPointer<vtkSegment>::New();
      this->Segmentation->AddSegment(segment, restoredSegmentsIt->first);
      }

    vtkSegmentation::CopySegment(segment, segmentToRestore, nullptr, restoredRepresentations);

    std::vector<std::string> restoredRepresentationNames;
    segmentToRestore->GetContainedRepresentationNames(restoredRepresentationNames);
    std::vector<std::string> currentRepresentationNames;
    segment->GetContainedRepresentationNames(currentRepresentationNames);
    // Remove representations that are not in the restoring segment
    for (std::string representationName : currentRepresentationNames)
      {
      if (std::find(restoredRepresentationNames.begin(), restoredRepresentationNames.end(), representationName) == restoredRepresentationNames.end())
        {
        segment->RemoveRepresentation(representationName);
        }
      }
    }

  // Removed segments that were not in the restored state
  std::vector<std::string> segmentIDs;
  this->Segmentation->GetSegmentIDs(segmentIDs);
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
    {
    if (segmentIDsToKeep.find(*segmentIDIt) != segmentIDsToKeep.end())
      {
      // found this segment in the list of segments to keep
      continue;
      }
    this->Segmentation->RemoveSegment(*segmentIDIt);
    }

  this->Segmentation->ReorderSegments(restoredState.SegmentIds);

  this->LastRestoredState = stateIndex;

  this->RestoreStateInProgress = false;
  this->Modified();
  return true;
}

//---------------------------------------------------------------------------
bool vtkSegmentationHistory::IsRestorePreviousStateAvailable()
{
  if (this->LastRestoredState < 1)
    {
    return false;
    }
  return true;
}

bool vtkSegmentationHistory::IsRestoreNextStateAvailable()
{
  if (this->LastRestoredState + 1 >= this->SegmentationStates.size())
    {
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
void vtkSegmentationHistory::RemoveAllNextStates()
{
  bool modified = false;
  while ((this->SegmentationStates.size() > this->LastRestoredState + 1) && (!this->SegmentationStates.empty()))
    {
    this->SegmentationStates.pop_back();
    modified = true;
    }
  if (modified)
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkSegmentationHistory::RemoveAllObsoleteStates()
{
  bool modified = false;
  while ((this->SegmentationStates.size() > this->MaximumNumberOfStates) && (!this->SegmentationStates.empty()))
    {
    this->SegmentationStates.pop_front();
    this->LastRestoredState--;
    modified = true;
   }
  if (modified)
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkSegmentationHistory::SetMaximumNumberOfStates(unsigned int maximumNumberOfStates)
{
  if (maximumNumberOfStates == this->MaximumNumberOfStates)
    {
    return;
    }
  this->MaximumNumberOfStates = maximumNumberOfStates;
  this->RemoveAllObsoleteStates();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSegmentationHistory::OnSegmentationModified(vtkObject* vtkNotUsed(caller),
  unsigned long vtkNotUsed(eid),
  void* clientData,
  void* vtkNotUsed(callData))
{
  vtkSegmentationHistory* self = reinterpret_cast<vtkSegmentationHistory*>(clientData);
  if (!self)
    {
    return;
    }

  if (self->RestoreStateInProgress)
    {
    // This object causes the changes, this object handles it
    return;
    }
  self->RemoveAllNextStates();
  if (self->LastRestoredState != self->SegmentationStates.size())
    {
    self->LastRestoredState = (unsigned int)self->SegmentationStates.size();
    self->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkSegmentationHistory::RemoveAllStates()
{
  this->SegmentationStates.clear();
  this->LastRestoredState = 0;
  this->Modified();
}

//---------------------------------------------------------------------------
int vtkSegmentationHistory::GetNumberOfStates()
{
  return this->SegmentationStates.size();
}
