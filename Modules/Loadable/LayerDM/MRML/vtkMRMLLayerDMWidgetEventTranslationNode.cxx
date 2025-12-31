#include "vtkMRMLLayerDMWidgetEventTranslationNode.h"

// Slicer includes
#include <vtkMRMLAbstractWidget.h>
#include <vtkMRMLInteractionEventData.h>

// VTK includes
#include <vtkObjectFactory.h>

// STL includes
#include <functional>

vtkMRMLNodeNewMacro(vtkMRMLLayerDMWidgetEventTranslationNode);

void vtkMRMLLayerDMWidgetEventTranslationNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "EventMap:" << std::endl;
  const auto& nextIndent = indent.GetNextIndent();
  os << ToString(this->EventMap, &nextIndent, "\n");
}

void vtkMRMLLayerDMWidgetEventTranslationNode::Copy(vtkMRMLNode* node)
{
  const auto other = vtkMRMLLayerDMWidgetEventTranslationNode::SafeDownCast(node);
  if (!other)
  {
    return;
  }
  this->EventMap = other->EventMap;
  this->Modified();
}

void vtkMRMLLayerDMWidgetEventTranslationNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);
  vtkMRMLReadXMLBeginMacro(atts);
  if (!strcmp(xmlReadAttName, "eventMap"))
  {
    this->EventMap = EventMapFromString(xmlReadAttValue);
    this->Modified();
  }
  vtkMRMLReadXMLEndMacro();
}

void vtkMRMLLayerDMWidgetEventTranslationNode::WriteXML(ostream& of, int indent)
{
  Superclass::WriteXML(of, indent);
  vtkMRMLWriteXMLBeginMacro(of);
  of << " eventMap=\"" << XMLAttributeEncodeString(ToString(this->EventMap, nullptr, "")) << "\"";
  vtkMRMLWriteXMLEndMacro();
}

const char* vtkMRMLLayerDMWidgetEventTranslationNode::GetNodeTagName()
{
  return "WidgetEventTranslationNode";
}

void vtkMRMLLayerDMWidgetEventTranslationNode::SetTranslation(int widgetState, unsigned long interactionEvent, unsigned long widgetEvent, int modifier)
{
  this->SetTranslation(EventKey{ widgetState, interactionEvent, modifier }, widgetEvent);
}

unsigned long vtkMRMLLayerDMWidgetEventTranslationNode::GetEndInteractionEvent(unsigned long startInteractionEvent)
{
  std::map<unsigned long, unsigned long> endInteractionMap{
    { vtkCommand::LeftButtonPressEvent, vtkCommand::LeftButtonReleaseEvent },
    { vtkCommand::MiddleButtonPressEvent, vtkCommand::MiddleButtonReleaseEvent },
    { vtkCommand::RightButtonPressEvent, vtkCommand::RightButtonReleaseEvent },
  };

  const auto it = endInteractionMap.find(startInteractionEvent);
  return it == endInteractionMap.end() ? vtkMRMLAbstractWidget::WidgetEventNone : it->second;
}

unsigned long vtkMRMLLayerDMWidgetEventTranslationNode::GetClickEvent(unsigned long releaseEvent)
{
  std::map<unsigned long, unsigned long> clickMap{
    { vtkCommand::LeftButtonReleaseEvent, vtkMRMLInteractionEventData::LeftButtonClickEvent },
    { vtkCommand::MiddleButtonReleaseEvent, vtkMRMLInteractionEventData::MiddleButtonClickEvent },
    { vtkCommand::RightButtonReleaseEvent, vtkMRMLInteractionEventData::RightButtonClickEvent },
  };

  const auto it = clickMap.find(releaseEvent);
  return it == clickMap.end() ? vtkMRMLAbstractWidget::WidgetEventNone : it->second;
}

void vtkMRMLLayerDMWidgetEventTranslationNode::SetTranslationClickAndDrag(int widgetState,
                                                                          unsigned long interactionEvent,
                                                                          int widgetStateDragging,
                                                                          unsigned long widgetStartEvent,
                                                                          unsigned long widgetEndEvent,
                                                                          int modifiers)
{
  const auto endInteractionEvent = GetEndInteractionEvent(interactionEvent);
  this->SetTranslation(widgetState, interactionEvent, widgetStartEvent, modifiers);
  this->SetTranslation(widgetStateDragging, vtkCommand::MouseMoveEvent, vtkMRMLAbstractWidget::WidgetEventMouseMove);
  this->SetTranslation(widgetStateDragging, endInteractionEvent, widgetEndEvent);
}

void vtkMRMLLayerDMWidgetEventTranslationNode::SetTranslationKeyboard(int widgetState,
                                                                      const std::string& keySym,
                                                                      unsigned long widgetEvent,
                                                                      int modifier,
                                                                      int repeatCount,
                                                                      unsigned long keyEvent)
{
  this->SetTranslation(EventKey{ widgetState, keyEvent, modifier, EventKey::thresholdRepeatCount(repeatCount), keySym }, widgetEvent);
}

int vtkMRMLLayerDMWidgetEventTranslationNode::RemoveTranslationEvent(unsigned long widgetEvent)
{
  int erasedCount{};
  for (auto it = this->EventMap.begin(); it != this->EventMap.end();)
  {
    if (it->second == widgetEvent)
    {
      it = this->EventMap.erase(it);
      erasedCount++;
    }
    else
    {
      ++it;
    }
  }

  if (erasedCount > 0)
  {
    this->Modified();
  }
  return erasedCount;
}

void vtkMRMLLayerDMWidgetEventTranslationNode::SetTranslation(const EventKey& key, unsigned long widgetEvent)
{
  this->EventMap[key] = widgetEvent;
  this->Modified();
}

bool vtkMRMLLayerDMWidgetEventTranslationNode::BlockTranslationEvent(unsigned long widgetEvent, bool isBlocked)
{
  const auto wasBlocked = this->IsWidgetEventBlocked(widgetEvent);
  if (isBlocked)
  {
    this->BlockedEvents.emplace(widgetEvent);
  }
  else
  {
    this->BlockedEvents.erase(widgetEvent);
  }
  return wasBlocked;
}

bool vtkMRMLLayerDMWidgetEventTranslationNode::BlockAllTranslationEvents(bool isBlocked)
{
  const auto wasBlocked = this->IsBlocked;
  this->IsBlocked = isBlocked;
  return wasBlocked;
}

unsigned long vtkMRMLLayerDMWidgetEventTranslationNode::Translate(int widgetState, vtkMRMLInteractionEventData* eventData)
{
  if (!eventData)
  {
    return vtkMRMLAbstractWidget::WidgetEventNone;
  }

  const auto eventId = static_cast<unsigned long>(eventData->GetType());
  EventKey key{ widgetState, eventId, eventData->GetModifiers() };

  if (eventId == vtkCommand::KeyPressEvent)
  {
    key.repeatCount = EventKey::thresholdRepeatCount(eventData->GetKeyRepeatCount());
    key.keySym = eventData->GetKeySym();
  }

  return this->Translate(key);
}

void vtkMRMLLayerDMWidgetEventTranslationNode::Clear()
{
  this->EventMap.clear();
  this->Modified();
}

int vtkMRMLLayerDMWidgetEventTranslationNode::GetNumberOfTranslations() const
{
  return static_cast<int>(this->EventMap.size());
}

unsigned long vtkMRMLLayerDMWidgetEventTranslationNode::Translate(EventKey key) const
{
  // Early return if the translation node is blocked
  if (this->IsBlocked)
  {
    return vtkMRMLAbstractWidget::WidgetEventNone;
  }

  // Define relaxations strategies for the key matching
  const auto noRelaxation = [](EventKey k) { return k; };
  const auto relaxModifier = [](EventKey k)
  {
    k.modifier = (k.modifier == vtkEvent::AnyModifier) ? vtkEvent::NoModifier : k.modifier;
    return k;
  };
  const auto relaxState = [](EventKey k)
  {
    k.widgetState = vtkMRMLAbstractWidget::WidgetStateAny;
    return k;
  };
  const auto relaxClick = [](EventKey k)
  {
    const auto clickEvent = GetClickEvent(k.eventId);
    k.eventId = clickEvent != vtkMRMLAbstractWidget::WidgetEventNone ? clickEvent : k.eventId;
    return k;
  };

  // Iterate on the different relaxation combinations and return the first match
  const std::vector<std::function<EventKey(EventKey)>> relaxations{ noRelaxation, relaxModifier, relaxState, relaxClick };
  for (int i = 0; i < relaxations.size(); i++)
  {
    key = relaxations[i](key);
    for (int j = i; j < relaxations.size(); j++)
    {
      if (const auto widgetEvent = this->GetWidgetEvent(relaxations[j](key)); widgetEvent != vtkMRMLAbstractWidget::WidgetEventNone)
      {
        return widgetEvent;
      }
    }
  }
  return vtkMRMLAbstractWidget::WidgetEventNone;
}

bool vtkMRMLLayerDMWidgetEventTranslationNode::IsWidgetEventBlocked(unsigned long widgetEvent) const
{
  return this->BlockedEvents.find(widgetEvent) != std::end(this->BlockedEvents);
}

unsigned long vtkMRMLLayerDMWidgetEventTranslationNode::GetWidgetEvent(const EventKey& key) const
{
  const auto it = this->EventMap.find(key);
  const auto widgetEvent = (it == this->EventMap.end()) ? vtkMRMLAbstractWidget::WidgetEventNone : it->second;
  return this->IsWidgetEventBlocked(widgetEvent) ? vtkMRMLAbstractWidget::WidgetEventNone : widgetEvent;
}

std::string vtkMRMLLayerDMWidgetEventTranslationNode::ToString(const std::pair<EventKey, unsigned long>& eventPair)
{
  const auto& [key, widgetEvent] = eventPair;
  std::stringstream ss;
  ss << "widgetState=" << key.widgetState << ",";
  ss << "eventId=" << key.eventId << ",";
  ss << "modifier=" << key.modifier << ",";
  ss << "repeatCount=" << key.repeatCount << ",";
  ss << "keySym=" << key.keySym << ",";
  ss << "widgetEvent=" << widgetEvent;
  return ss.str();
}

std::string vtkMRMLLayerDMWidgetEventTranslationNode::ToString(const std::map<EventKey, unsigned long>& eventMap, const vtkIndent* indent, const std::string& eol)
{
  std::stringstream ss;
  for (const auto& pair : eventMap)
  {
    if (indent)
    {
      ss << *indent;
    }

    ss << ToString(pair) << ";" << eol;
  }
  return ss.str();
}

std::map<vtkMRMLLayerDMWidgetEventTranslationNode::EventKey, unsigned long> vtkMRMLLayerDMWidgetEventTranslationNode::EventMapFromString(const std::string& value)
{
  std::stringstream ss(value);
  std::string token;
  constexpr auto delim = ';';

  std::map<EventKey, unsigned long> eventMap;
  while (std::getline(ss, token, delim))
  {
    const auto [key, widgetEvent] = EventPairFromString(token);
    eventMap[key] = widgetEvent;
  }
  return eventMap;
}

std::pair<vtkMRMLLayerDMWidgetEventTranslationNode::EventKey, unsigned long> vtkMRMLLayerDMWidgetEventTranslationNode::EventPairFromString(const std::string& value)
{
  // Helper function to trim the input string
  auto trim = [](std::string& s)
  {
    constexpr auto ws = " \t\n\r";
    const auto start = s.find_first_not_of(ws);
    if (start == std::string::npos)
    {
      s.clear();
      return;
    }

    const auto end = s.find_last_not_of(ws);
    s.erase(end + 1);
    s.erase(0, start);
  };

  // If input key name matches expected name, call the input assign function.
  // Swallows any exception raised during the assign function execution (for instance runtime errors).
  auto tryRead = [](const std::string& keyName, const std::string& name, const std::function<void()>& assignF)
  {
    if (keyName != name)
    {
      return;
    }
    try
    {
      assignF();
    }
    catch (...)
    {
    }
  };

  std::stringstream ss(value);
  std::string token;
  constexpr auto delim = ',';
  EventKey key;
  unsigned long widgetEvent = vtkMRMLAbstractWidget::WidgetEventNone;
  while (std::getline(ss, token, delim))
  {
    const auto pos = token.find('=');
    if (pos == std::string::npos)
    {
      continue;
    }

    std::string keyName = token.substr(0, pos);
    std::string val = token.substr(pos + 1);
    trim(keyName);
    trim(val);

    tryRead(keyName, "widgetState", [val, &key] { key.widgetState = std::stoi(val); });
    tryRead(keyName, "eventId", [val, &key] { key.eventId = std::stoul(val); });
    tryRead(keyName, "modifier", [val, &key] { key.modifier = std::stoi(val); });
    tryRead(keyName, "repeatCount", [val, &key] { key.repeatCount = EventKey::thresholdRepeatCount(std::stoi(val)); });
    tryRead(keyName, "keySym", [val, &key] { key.keySym = val; });
    tryRead(keyName, "widgetEvent", [val, &widgetEvent] { widgetEvent = std::stoul(val); });
  }
  return { key, widgetEvent };
}
