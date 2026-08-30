#include "vtkMRMLLayerDMObjectEventObserver.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkMRMLLayerDMObjectEventObserver);

template <class... Ts>
struct Overloaded : Ts...
{
  using Ts::operator()...;
};

template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

vtkMRMLLayerDMObjectEventObserver::vtkMRMLLayerDMObjectEventObserver()
  : UpdateCommand(vtkSmartPointer<vtkCallbackCommand>::New())
  , Blocked(false)
{
  this->UpdateCommand->SetClientData(this);
  this->UpdateCommand->SetCallback(
    [](vtkObject* caller, unsigned long eid, void* clientData, void* callData)
    {
      auto client = static_cast<vtkMRMLLayerDMObjectEventObserver*>(clientData);
      if (client->Blocked)
      {
        return;
      }

      try
      {
        // Dispatch to callback depending on current std variant content
        std::visit(Overloaded{ [&](const std::function<void(vtkObject * node)>& f) { f(caller); },
                               [&](const std::function<void(vtkObject * node, unsigned long eventId)>& f) { f(caller, eid); },
                               [&](const std::function<void(vtkObject * node, unsigned long eventId, void* callData)>& f) { f(caller, eid, callData); } },
                   client->Callback);
      }
      catch (const std::bad_function_call&)
      {
        // Ignore unset function callbacks
      }
    });
}

vtkMRMLLayerDMObjectEventObserver::~vtkMRMLLayerDMObjectEventObserver()
{
  for (const auto& obs : this->ObservedEventsMap)
  {
    if (obs.first)
    {
      for (auto& event : obs.second)
      {
        obs.first->RemoveObserver(event);
      }
    }
  }
}

bool vtkMRMLLayerDMObjectEventObserver::UpdateObserver(vtkObject* prevObj, vtkObject* obj, unsigned long event)
{
  return this->UpdateObserver(prevObj, obj, std::vector<unsigned long>{ event });
}

bool vtkMRMLLayerDMObjectEventObserver::UpdateObserver(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events)
{
  if (prevObj == obj)
  {
    return false;
  }

  this->RemoveObserver(prevObj);
  for (const auto& event : events)
  {
    this->AddObserver(obj, event);
  }
  return true;
}

void vtkMRMLLayerDMObjectEventObserver::SetUpdateCallback(const std::function<void(vtkObject* node)>& callback)
{
  this->Callback = callback;
}

bool vtkMRMLLayerDMObjectEventObserver::SetBlocked(bool isBlocked)
{
  bool wasBlocked = this->Blocked;
  this->Blocked = isBlocked;
  return wasBlocked;
}

bool vtkMRMLLayerDMObjectEventObserver::IsBlocked() const
{
  return this->Blocked;
}

void vtkMRMLLayerDMObjectEventObserver::SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId)>& callback)
{
  this->Callback = callback;
}

void vtkMRMLLayerDMObjectEventObserver::SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId, void* callData)>& callback)
{
  this->Callback = callback;
}

void vtkMRMLLayerDMObjectEventObserver::AddObserver(vtkObject* node, unsigned long event)
{
  if (!node)
  {
    return;
  }

  if (this->ObservedEventsMap.find(node) == std::end(this->ObservedEventsMap))
  {
    this->ObservedEventsMap[node] = std::set<unsigned long>{};
  }

  if (this->ObservedEventsMap[node].find(event) != std::end(this->ObservedEventsMap[node]))
  {
    return;
  }

  this->ObservedEventsMap[node].insert(node->AddObserver(event, this->UpdateCommand));
}

void vtkMRMLLayerDMObjectEventObserver::RemoveObserver(vtkObject* node)
{
  if (!node || this->ObservedEventsMap.find(node) == std::end(this->ObservedEventsMap))
  {
    return;
  }

  for (auto& event : this->ObservedEventsMap[node])
  {
    node->RemoveObserver(event);
  }

  this->ObservedEventsMap.erase(node);
}

vtkMRMLLayerDMObjectEventObserver::UpdateGuard::UpdateGuard(vtkMRMLLayerDMObjectEventObserver* obs)
  : Observer(obs)
{
  if (this->Observer)
  {
    this->WasBlocked = this->Observer->SetBlocked(true);
  }
}

vtkMRMLLayerDMObjectEventObserver::UpdateGuard::~UpdateGuard()
{
  if (this->Observer)
  {
    this->Observer->SetBlocked(this->WasBlocked);
  }
}
