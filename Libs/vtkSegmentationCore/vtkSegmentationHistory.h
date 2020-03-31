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

#ifndef __vtkSegmentationHistory_h
#define __vtkSegmentationHistory_h

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>

// STD includes
#include <deque>
#include <map>
#include <vector>

#include "vtkSegmentationCoreConfigure.h"

class vtkCallbackCommand;
class vtkDataObject;
class vtkSegment;
class vtkSegmentation;

/// \ingroup SegmentationCore
class vtkSegmentationCore_EXPORT vtkSegmentationHistory : public vtkObject
{
public:
  static vtkSegmentationHistory* New();
  vtkTypeMacro(vtkSegmentationHistory, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Selects a segmentation that the states will be stored of. Current state of the segmentation is not stored.
  /// \param segmentation Segmentation to store. Deletes all stored states of the previously set segmentation.
  void SetSegmentation(vtkSegmentation* segmentation);

  /// Get segmentation that the states will be stored of.
  vtkGetMacro(Segmentation, vtkSegmentation*);

  /// Saves all master representations of the segmentation in its current state.
  /// States more recent than the last restored state are removed.
  /// \return Success flag
  bool SaveState();

  /// Restores previous state of the segmentation.
  /// \return Success flag
  bool RestorePreviousState();

  /// Check if it is possible to go back to a previous state (undo).
  /// \return True if the operation is allowed
  bool IsRestorePreviousStateAvailable();

  /// Restores next state of the segmentation.
  /// \return Success flag
  bool RestoreNextState();

  /// Check if it is possible to go restore the next state (redo).
  /// \return True if the operation is allowed
  bool IsRestoreNextStateAvailable();

  /// Delete all states from memory
  void RemoveAllStates();

  /// Limits how many states may be stored.
  /// If the number of stored states exceed the limit then the oldest state is removed.
  void SetMaximumNumberOfStates(unsigned int maximumNumberOfStates);

  /// Get the limit of how many states may be stored.
  vtkGetMacro(MaximumNumberOfStates, unsigned int);

  /// Get the current number of states.
  int GetNumberOfStates();

protected:
  /// Callback function called when the segmentation has been modified.
  /// It clears all states that are more recent than the last restored state.
  static void OnSegmentationModified(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  /// Delete all states that are more recent than the last restored state
  void RemoveAllNextStates();

  /// Delete all old states so that we keep only up to MaximumNumberOfStates states
  void RemoveAllObsoleteStates();

  /// Restores a state defined by stateIndex.
  bool RestoreState(unsigned int stateIndex);

protected:
  vtkSegmentationHistory();
  ~vtkSegmentationHistory() override;

  typedef std::map<std::string, vtkSmartPointer<vtkSegment> > SegmentsMap;

  struct SegmentationState
    {
    SegmentsMap Segments;
    std::vector<std::string> SegmentIds; // order of segments
    };

  vtkSegmentation* Segmentation;
  vtkCallbackCommand* SegmentationModifiedCallbackCommand;
  std::deque<SegmentationState> SegmentationStates;
  unsigned int MaximumNumberOfStates;

  // Index of the state in SegmentationStates that was restored last.
  // If index == size of states then it means that the segmentation has changed
  // since the last restored state.
  unsigned int LastRestoredState;

  bool RestoreStateInProgress;

private:
  vtkSegmentationHistory(const vtkSegmentationHistory&) = delete;
  void operator=(const vtkSegmentationHistory&) = delete;
};

#endif // __vtkSegmentation_h
