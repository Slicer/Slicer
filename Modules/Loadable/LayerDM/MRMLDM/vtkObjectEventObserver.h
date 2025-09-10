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
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkObjectEventObserver : public vtkObject
{
public:
  static vtkObjectEventObserver* New();
  vtkTypeMacro(vtkObjectEventObserver, vtkObject);

  bool UpdateObserver(vtkObject* prevObj, vtkObject* obj, unsigned long event = vtkCommand::ModifiedEvent);
  bool UpdateObserver(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events);
  void SetUpdateCallback(const std::function<void(vtkObject* node)>& callback);
  void SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId)>& callback);
  void SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId, void* callData)>& callback);

protected:
  vtkObjectEventObserver();
  ~vtkObjectEventObserver() override;

private:
  void AddObserver(vtkObject* obj, unsigned long event);
  void RemoveObserver(vtkObject* obj);

  vtkSmartPointer<vtkCallbackCommand> m_updateCommand{};
  std::map<vtkWeakPointer<vtkObject>, std::set<unsigned long>> m_obsMap{};

  std::variant<std::function<void(vtkObject* node)>,
               std::function<void(vtkObject* node, unsigned long eventId)>,
               std::function<void(vtkObject* node, unsigned long eventId, void* callData)>>
    m_callback;
};
