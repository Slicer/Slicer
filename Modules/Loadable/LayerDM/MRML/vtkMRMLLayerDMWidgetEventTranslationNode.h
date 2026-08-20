#pragma once

#include "vtkSlicerLayerDMModuleMRMLExport.h"

// Slicer includes
#include <vtkMRMLNode.h>
#include <vtkMRMLAbstractWidget.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkEvent.h>

// STL includes
#include <map>

class vtkMRMLInteractionEventData;

/// MRML node responsible for storing widget event translation.
///
/// Provides utility static methods to access / register scene translation nodes for multi widget configuration
/// scenarios.
class VTK_SLICER_LAYERDM_MODULE_MRML_EXPORT vtkMRMLLayerDMWidgetEventTranslationNode : public vtkMRMLNode
{
public:
  static vtkMRMLLayerDMWidgetEventTranslationNode* New();
  vtkTypeMacro(vtkMRMLLayerDMWidgetEventTranslationNode, vtkMRMLNode);

  vtkMRMLLayerDMWidgetEventTranslationNode(const vtkMRMLLayerDMWidgetEventTranslationNode&) = delete;
  void operator=(const vtkMRMLLayerDMWidgetEventTranslationNode&) = delete;

  void PrintSelf(ostream& os, vtkIndent indent) override;
  void Copy(vtkMRMLNode* node) override;
  void ReadXMLAttributes(const char** atts) override;
  void WriteXML(ostream& of, int indent) override;

  vtkMRMLNode* CreateNodeInstance() override;
  const char* GetNodeTagName() override;

  /// Returns the end interaction event value matching the input start interaction event.
  /// If no match is found, returns vtkMRMLAbstractWidget::WidgetEventNone value.
  ///
  /// \sa SetEventTranslationClickAndDrag
  static unsigned long GetEndInteractionEvent(unsigned long startInteractionEvent);

  /// Returns the click event value matching the input release event.
  /// If no match is found, returns vtkMRMLAbstractWidget::WidgetEventNone value.
  static unsigned long GetClickEvent(unsigned long releaseEvent);

  /// Set translation for mouse click events.
  void SetTranslation(int widgetState, unsigned long interactionEvent, unsigned long widgetEvent, int modifier = vtkEvent::NoModifier);

  /// Set translation for mouse click and drag events.
  void SetTranslationClickAndDrag(int widgetState,
                                  unsigned long interactionEvent,
                                  int widgetStateDragging,
                                  unsigned long widgetStartEvent,
                                  unsigned long widgetEndEvent,
                                  int modifiers = vtkEvent::NoModifier);

  /// Set translation for keystroke events
  void SetTranslationKeyboard(int widgetState,
                              const std::string& keySym,
                              unsigned long widgetEvent,
                              int modifier = vtkEvent::NoModifier,
                              int repeatCount = 1,
                              unsigned long keyEvent = vtkCommand::KeyPressEvent);

  /// Removes all translation events mapping to the given input event.
  ///
  /// \sa BlockTranslationEvent
  /// \sa Clear
  /// \return Number of erased translation events.
  int RemoveTranslationEvent(unsigned long widgetEvent);

  /// Block the widget event translation. Instead of returning this event, the \sa Translate method will return
  /// vtkMRMLAbstractWidget::WidgetEventNone instead.
  ///
  /// \sa BlockAllTranslationEvents
  /// \return previous block state for input event
  bool BlockTranslationEvent(unsigned long widgetEvent, bool isBlocked);

  /// Block all translation events.
  ///
  /// \sa BlockTranslationEvent
  /// \return previous block state
  bool BlockAllTranslationEvents(bool isBlocked);

  /// Get event from input event data and current widget state.
  /// If the event translation is blocked or specific widget event is blocked, returns
  /// vtkMRMLAbstractWidget::WidgetEventNone.
  /// \sa BlockTranslationEvent
  /// \sa BlockAllTranslationEvents
  unsigned long Translate(int widgetState, vtkMRMLInteractionEventData* eventData);

  /// Clears all widget translations contained in the node
  void Clear();

  /// \return the number of event translations currently mapped.
  int GetNumberOfTranslations() const;

protected:
  vtkMRMLLayerDMWidgetEventTranslationNode() = default;
  ~vtkMRMLLayerDMWidgetEventTranslationNode() override = default;

private:
  /// Simple struct compatible with std::map key usage
  struct EventKey
  {
    bool operator==(const EventKey& other) const { return this->AsTuple() == other.AsTuple(); }
    bool operator<(const EventKey& other) const { return this->AsTuple() < other.AsTuple(); }
    std::tuple<int, unsigned long, int, int, std::string> AsTuple() const { return std::make_tuple(widgetState, eventId, modifier, repeatCount, keySym); }

    /// Slicer event data never generate repeat counts less than 1.
    /// Make sure the repeat count is always greater than one
    static int thresholdRepeatCount(int repeatCount) { return std::max(1, repeatCount); }

    int widgetState{ vtkMRMLAbstractWidget::WidgetStateAny };
    unsigned long eventId{ vtkCommand::NoEvent };
    int modifier{ vtkEvent::NoModifier };
    int repeatCount{ 1 };
    std::string keySym{};
  };

  void SetTranslation(const EventKey& key, unsigned long widgetEvent);
  unsigned long Translate(EventKey key) const;
  bool IsWidgetEventBlocked(unsigned long widgetEvent) const;
  unsigned long GetWidgetEvent(const EventKey& key) const;

  static std::string ToString(const std::map<EventKey, unsigned long>& eventMap, const vtkIndent* indent, const std::string& eol);
  static std::string ToString(const std::pair<EventKey, unsigned long>& eventPair);

  static std::map<EventKey, unsigned long> EventMapFromString(const std::string& value);
  static std::pair<EventKey, unsigned long> EventPairFromString(const std::string& value);

  std::map<EventKey, unsigned long> EventMap{};
  std::set<unsigned long> BlockedEvents{};
  bool IsBlocked{};
};
