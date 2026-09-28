/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware SAS

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Thibault Pelletier, Kitware SAS,
  and was partially funded by ANR grants ANR-22-CE45-0034 and ANR-18-RHUS-005.

==============================================================================*/

#ifndef __vtkMRMLLayerDMObjectEventObserver_h
#define __vtkMRMLLayerDMObjectEventObserver_h

// LayerDM includes
#include "vtkSlicerLayerDMModuleMRMLExport.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STL includes
#include <functional>
#include <map>
#include <set>
#include <variant>
#include <vector>

class vtkCallbackCommand;

/// \brief VTK object observer with one callback endpoint when an event is triggered.
/// Can observe multiple objects and multiple events per object.
///
/// Depending on the callback used, event id and call data can either be forwarded or ignored.
class VTK_SLICER_LAYERDM_MODULE_MRML_EXPORT vtkMRMLLayerDMObjectEventObserver : public vtkObject
{
public:
  struct UpdateGuard;
  static vtkMRMLLayerDMObjectEventObserver* New();
  vtkTypeMacro(vtkMRMLLayerDMObjectEventObserver, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// @{
  /// Remove previous monitored events from \param prevObj and observe events from the \param obj
  /// If both obj are the same, does nothing.
  /// On event triggered, calls the update set by \sa SetUpdateCallback.
  ///
  /// \warning prevObj is not mutated by this call. To update the pointer, a manual set is required after update.
  bool UpdateObservation(vtkObject* prevObj, vtkObject* obj, unsigned long event = vtkCommand::ModifiedEvent);
  bool UpdateObservation(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events);
  /// @}

  /// Remove observers attached to the input object.
  /// Use \sa UpdateObservation to update the observed events for a new object (RemoveObservations is then called automatically).
  void RemoveObservations(vtkObject* obj);

  /// @{
  /// Set the callback triggered when one of the observed objects and event is invoked.
  void SetUpdateCallback(const std::function<void(vtkObject* node)>& callback);
  void SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId)>& callback);
  void SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId, void* callData)>& callback);
  /// @}

  /// Forget the update callback.
  void ClearCallback();

  /// Set update callback blocked.
  /// @return previous blocked state.
  bool SetBlocked(bool isBlocked);
  bool IsBlocked() const;

  /// Helper update guard.
  /// Blocks update during struct lifetime for the given input observer.
  struct UpdateGuard
  {
    UpdateGuard(vtkMRMLLayerDMObjectEventObserver* obs);
    ~UpdateGuard();

  private:
    vtkMRMLLayerDMObjectEventObserver* Observer;
    bool WasBlocked{};
  };

protected:
  vtkMRMLLayerDMObjectEventObserver();
  ~vtkMRMLLayerDMObjectEventObserver() override;

private:
  void AddObservation(vtkObject* obj, unsigned long event);

  /// Dispatch an observed event to the update callback.
  void InvokeCallback(vtkObject* obj, unsigned long eventId, void* callData);

  /// Called when an observed object invokes vtkCommand::DeleteEvent.
  /// Forgets the object, then forwards the event to the update callback if the object was observed for it.
  void OnObjectDeleted(vtkObject* obj);

  vtkSmartPointer<vtkCallbackCommand> UpdateCommand;
  std::map<vtkObject*, std::map<unsigned long, unsigned long>> ObservedEventsMap;
  std::set<vtkObject*> DeleteEventObservers;
  std::variant<std::function<void(vtkObject* node)>,
               std::function<void(vtkObject* node, unsigned long eventId)>,
               std::function<void(vtkObject* node, unsigned long eventId, void* callData)>>
    Callback;
  bool Blocked;
};

#endif
