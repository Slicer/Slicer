/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

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
  bool UpdateObserver(vtkObject* prevObj, vtkObject* obj, unsigned long event = vtkCommand::ModifiedEvent);
  bool UpdateObserver(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events);
  /// @}

  /// Remove observers attached to the input object.
  /// Use \sa UpdateObserver to update the observed events for a new object (RemoveObserver is then called automatically).
  void RemoveObserver(vtkObject* obj);

  /// @{
  /// Set the callback triggered when one of the observed objects and event is invoked.
  void SetUpdateCallback(const std::function<void(vtkObject* node)>& callback);
  void SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId)>& callback);
  void SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId, void* callData)>& callback);
  /// @}

  /// Set the callback triggered when one of the observed objects is about to be destroyed.
  ///
  /// Every observed object is also observed for vtkCommand::DeleteEvent, so that objects used as keys of
  /// ordered containers can be removed from them while their address is still valid. VTK invokes the event
  /// before clearing the weak pointers to the object, so the object passed to the callback is still valid and
  /// can be used to look up and erase container entries.
  ///
  /// Unlike the update callback, this callback is invoked even when the observer is blocked: blocking is meant
  /// to suppress display updates, not to skip lifetime bookkeeping.
  ///
  /// \sa SetBlocked
  void SetDeleteCallback(const std::function<void(vtkObject* node)>& callback);

  /// Forget the update and delete callbacks.
  ///
  /// Owners which capture themselves in their callbacks are expected to call this in their destructor:
  /// releasing an observed object during the owner destruction would otherwise invoke the delete callback and
  /// re-enter the owner while its members are being destroyed.
  void ClearCallbacks();

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
  void AddObserver(vtkObject* obj, unsigned long event);

  /// Called when an observed object invokes vtkCommand::DeleteEvent.
  /// Forgets the object, then forwards it to the delete callback if one is set.
  void OnObjectDeleted(vtkObject* obj);

  vtkSmartPointer<vtkCallbackCommand> UpdateCommand;

  /// For each observed object, maps the observed event ID to the observer tag returned by AddObserver.
  ///
  /// The object is used as a plain identity key and is never dereferenced without being known to be alive:
  /// every observed object is also observed for vtkCommand::DeleteEvent and is removed from the map during
  /// that event, so the map only ever contains live objects. A weak pointer must not be used as the key of an
  /// ordered container, as it nulls itself in place when its object is destroyed, silently changing the key of
  /// a live map node and breaking the ordering of the map.
  ///
  /// \sa OnObjectDeleted
  std::map<vtkObject*, std::map<unsigned long, unsigned long>> ObservedEventsMap;

  std::variant<std::function<void(vtkObject* node)>,
               std::function<void(vtkObject* node, unsigned long eventId)>,
               std::function<void(vtkObject* node, unsigned long eventId, void* callData)>>
    Callback;
  std::function<void(vtkObject* node)> DeleteCallback;
  bool Blocked;
};

#endif
