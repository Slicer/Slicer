#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

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
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMObjectEventObserver : public vtkObject
{
public:
  static vtkMRMLLayerDMObjectEventObserver* New();
  vtkTypeMacro(vtkMRMLLayerDMObjectEventObserver, vtkObject);

  /// @{
  /// Remove previous monitored events from \param prevObj and observe events from the \param obj
  /// If both obj are the same, does nothing.
  /// On event triggerred, calls the update set by \sa SetUpdateCallback.
  ///
  /// \warning prevObj is not mutated by this call. To update the pointer, a manual set is required after update.
  bool UpdateObserver(vtkObject* prevObj, vtkObject* obj, unsigned long event = vtkCommand::ModifiedEvent);
  bool UpdateObserver(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events);
  /// @}

  /// Remove observers attached to the input object.
  /// Use \sa UpdateObserver to update the observed events for a new object (RemoveObserver is then called automatically).
  void RemoveObserver(vtkObject* obj);

  /// @{
  /// Set the callback triggerred when one of the observed objects and event is invoked.
  void SetUpdateCallback(const std::function<void(vtkObject* node)>& callback);
  void SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId)>& callback);
  void SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId, void* callData)>& callback);
  /// @}

  /// Set update callback blocked.
  /// @return previous blocked state.
  bool SetBlocked(bool isBlocked);

protected:
  vtkMRMLLayerDMObjectEventObserver();
  ~vtkMRMLLayerDMObjectEventObserver() override;

private:
  void AddObserver(vtkObject* obj, unsigned long event);

  vtkSmartPointer<vtkCallbackCommand> m_updateCommand{};
  std::map<vtkWeakPointer<vtkObject>, std::set<unsigned long>> m_obsMap{};

  std::variant<std::function<void(vtkObject* node)>,
               std::function<void(vtkObject* node, unsigned long eventId)>,
               std::function<void(vtkObject* node, unsigned long eventId, void* callData)>>
    m_callback;
  bool m_isBlocked;
};
