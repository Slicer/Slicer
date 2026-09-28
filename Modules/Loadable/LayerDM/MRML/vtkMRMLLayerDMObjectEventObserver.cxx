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

#include "vtkMRMLLayerDMObjectEventObserver.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLayerDMObjectEventObserver);

namespace
{
//-----------------------------------------------------------------------------
template <class... Ts>
struct Overloaded : Ts...
{
  using Ts::operator()...;
};

template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;
} // namespace

//-----------------------------------------------------------------------------
vtkMRMLLayerDMObjectEventObserver::vtkMRMLLayerDMObjectEventObserver()
  : UpdateCommand(vtkSmartPointer<vtkCallbackCommand>::New())
  , Blocked(false)
{
  this->UpdateCommand->SetClientData(this);
  this->UpdateCommand->SetCallback(
    [](vtkObject* caller, unsigned long eid, void* clientData, void* callData)
    {
      auto client = static_cast<vtkMRMLLayerDMObjectEventObserver*>(clientData);

      if (eid == vtkCommand::DeleteEvent)
      {
        client->OnObjectDeleted(caller);
        return;
      }

      if (client->Blocked)
      {
        return;
      }

      client->InvokeCallback(caller, eid, callData);
    });
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMObjectEventObserver::InvokeCallback(vtkObject* obj, unsigned long eventId, void* callData)
{
  try
  {
    // Dispatch to callback depending on current std variant content
    std::visit(Overloaded{ [&](const std::function<void(vtkObject* node)>& f) { f(obj); },
                           [&](const std::function<void(vtkObject* node, unsigned long id)>& f) { f(obj, eventId); },
                           [&](const std::function<void(vtkObject* node, unsigned long id, void* data)>& f) { f(obj, eventId, callData); } },
               this->Callback);
  }
  catch (const std::bad_function_call&)
  {
    // Ignore unset function callbacks
  }
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMObjectEventObserver::~vtkMRMLLayerDMObjectEventObserver()
{
  for (const auto& [object, eventTags] : this->ObservedEventsMap)
  {
    for (const auto& [event, tag] : eventTags)
    {
      object->RemoveObserver(tag);
    }
  }
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMObjectEventObserver::UpdateObservation(vtkObject* prevObj, vtkObject* obj, unsigned long event)
{
  return this->UpdateObservation(prevObj, obj, std::vector<unsigned long>{ event });
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMObjectEventObserver::UpdateObservation(vtkObject* prevObj, vtkObject* obj, const std::vector<unsigned long>& events)
{
  if (prevObj == obj)
  {
    return false;
  }

  this->RemoveObservations(prevObj);
  for (const auto& event : events)
  {
    this->AddObservation(obj, event);
  }
  return true;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMObjectEventObserver::SetUpdateCallback(const std::function<void(vtkObject* node)>& callback)
{
  this->Callback = callback;
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMObjectEventObserver::SetBlocked(bool isBlocked)
{
  bool wasBlocked = this->Blocked;
  this->Blocked = isBlocked;
  return wasBlocked;
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMObjectEventObserver::IsBlocked() const
{
  return this->Blocked;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMObjectEventObserver::SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId)>& callback)
{
  this->Callback = callback;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMObjectEventObserver::SetUpdateCallback(const std::function<void(vtkObject* node, unsigned long eventId, void* callData)>& callback)
{
  this->Callback = callback;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMObjectEventObserver::ClearCallback()
{
  this->Callback = std::function<void(vtkObject * node)>{};
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMObjectEventObserver::AddObservation(vtkObject* node, unsigned long event)
{
  if (!node)
  {
    return;
  }

  auto& observedEvents = this->ObservedEventsMap[node];

  // Always observe the object destruction, so that the object can be removed from the observed map while its
  // address is still valid.
  if (observedEvents.find(vtkCommand::DeleteEvent) == std::end(observedEvents))
  {
    observedEvents[vtkCommand::DeleteEvent] = node->AddObserver(vtkCommand::DeleteEvent, this->UpdateCommand);
  }

  if (event == vtkCommand::DeleteEvent)
  {
    this->DeleteEventObservers.insert(node);
    return;
  }

  if (observedEvents.find(event) != std::end(observedEvents))
  {
    return;
  }

  observedEvents[event] = node->AddObserver(event, this->UpdateCommand);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMObjectEventObserver::OnObjectDeleted(vtkObject* node)
{
  if (!node)
  {
    return;
  }

  const bool notifyOwner = this->DeleteEventObservers.erase(node) > 0;
  this->ObservedEventsMap.erase(node);
  if (notifyOwner)
  {
    this->InvokeCallback(node, vtkCommand::DeleteEvent, nullptr);
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMObjectEventObserver::RemoveObservations(vtkObject* node)
{
  if (!node)
  {
    return;
  }

  this->DeleteEventObservers.erase(node);

  if (const auto found = this->ObservedEventsMap.find(node); found != std::end(this->ObservedEventsMap))
  {
    for (const auto& [event, tag] : found->second)
    {
      node->RemoveObserver(tag);
    }
    this->ObservedEventsMap.erase(found);
  }
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMObjectEventObserver::UpdateGuard::UpdateGuard(vtkMRMLLayerDMObjectEventObserver* obs)
  : Observer(obs)
{
  if (this->Observer)
  {
    this->WasBlocked = this->Observer->SetBlocked(true);
  }
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMObjectEventObserver::UpdateGuard::~UpdateGuard()
{
  if (this->Observer)
  {
    this->Observer->SetBlocked(this->WasBlocked);
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMObjectEventObserver::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Blocked: " << (this->Blocked ? "true" : "false") << std::endl;
  os << indent << "Number of observed objects: " << this->ObservedEventsMap.size() << std::endl;
}
