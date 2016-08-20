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

#include "vtkSegmentation.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSegmentationHistory);

//----------------------------------------------------------------------------
vtkSegmentationHistory::vtkSegmentationHistory()
{
  this->Segmentation = NULL;

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
  this->SetSegmentation(NULL);

  if (this->SegmentationModifiedCallbackCommand)
    {
    this->SegmentationModifiedCallbackCommand->SetClientData(NULL);
    this->SegmentationModifiedCallbackCommand->Delete();
    this->SegmentationModifiedCallbackCommand = NULL;
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
    this->Segmentation->AddObserver(vtkCommand::ModifiedEvent, this->SegmentationModifiedCallbackCommand);
    this->Segmentation->AddObserver(vtkSegmentation::MasterRepresentationModified, this->SegmentationModifiedCallbackCommand);
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
  if (this->Segmentation == NULL)
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
  for (std::vector<std::string>::iterator segmentIDIt = segmentIDs.begin(); segmentIDIt != segmentIDs.end(); ++segmentIDIt)
    {
    vtkSegment* segment = this->Segmentation->GetSegment(*segmentIDIt);
    if (segment == NULL)
      {
      vtkErrorMacro("Failed to save state of segment " << *segmentIDIt);
      continue;
      }
    vtkSmartPointer<vtkSegment> segmentClone = vtkSmartPointer<vtkSegment>::New();
    segmentClone->DeepCopy(segment);
    newSegmentationState.Segments[*segmentIDIt] = segmentClone;
    }
  this->SegmentationStates.push_back(newSegmentationState);
  // Set the current state as last restored state
  this->LastRestoredState = this->SegmentationStates.size() - 1;
  this->RemoveAllObsoleteStates();

  // TODO: remove this after proper modification observation is fixed
  this->LastRestoredState = this->SegmentationStates.size();
  this->Modified();

  return true;
}

//---------------------------------------------------------------------------
bool vtkSegmentationHistory::RestorePreviousState()
{
  if (this->Segmentation == NULL)
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
    }
  return this->RestoreState(stateToRestore);
}

//---------------------------------------------------------------------------
bool vtkSegmentationHistory::RestoreNextState()
{
  if (this->Segmentation == NULL)
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
bool vtkSegmentationHistory::RestoreState(int stateIndex)
{
  this->RestoreStateInProgress = true;

  SegmentationState restoredState = this->SegmentationStates[stateIndex];

  std::set<std::string> segmentIDsToKeep;
  for (SegmentsMap::iterator restoredSegmentsIt = restoredState.Segments.begin();
    restoredSegmentsIt != restoredState.Segments.end(); ++restoredSegmentsIt)
    {
    segmentIDsToKeep.insert(restoredSegmentsIt->first);
    vtkSegment* segment = this->Segmentation->GetSegment(restoredSegmentsIt->first);
    if (segment != NULL)
      {
      segment->DeepCopy(restoredSegmentsIt->second);
      segment->Modified();
      }
    else
      {
      vtkSmartPointer<vtkSegment> newSegment = vtkSmartPointer<vtkSegment>::New();
      newSegment->DeepCopy(restoredSegmentsIt->second);
      this->Segmentation->AddSegment(newSegment);
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
    modified = true;
   }
  if (modified)
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkSegmentationHistory::SetMaximumNumberOfStates(int maximumNumberOfStates)
{
  if (maximumNumberOfStates = this->MaximumNumberOfStates)
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
  self->LastRestoredState = self->SegmentationStates.size();
  self->Modified();
}

//---------------------------------------------------------------------------
void vtkSegmentationHistory::RemoveAllStates()
{
  this->SegmentationStates.clear();
  this->LastRestoredState = 0;
  this->Modified();
}
