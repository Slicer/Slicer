/*=========================================================================

 Copyright (c) ProxSim ltd., Kwun Tong, Hong Kong. All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
 and development was supported by ProxSim ltd.

=========================================================================*/

#include "vtkMRMLAbstractWidget.h"

#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLAbstractWidgetRepresentation.h"

// VTK includes
#include "vtkEvent.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkWidgetEventTranslator.h"

//----------------------------------------------------------------------
vtkMRMLAbstractWidget::vtkMRMLAbstractWidget()
{
  this->WidgetRep = nullptr;
  this->WidgetState = vtkMRMLAbstractWidget::WidgetStateIdle;

//  this->NeedToRender = false;
}

//----------------------------------------------------------------------
vtkMRMLAbstractWidget::~vtkMRMLAbstractWidget()
{
  this->SetRenderer(nullptr);
}

//----------------------------------------------------------------------
void vtkMRMLAbstractWidget::SetEventTranslation(int widgetState, unsigned long interactionEvent, int modifiers, unsigned long widgetEvent)
{
  if (widgetState >= static_cast<int>(this->EventTranslators.size()))
    {
    this->EventTranslators.resize(widgetState +1);
    }
  vtkWidgetEventTranslator* translator = this->EventTranslators[widgetState];
  if (!translator)
    {
    this->EventTranslators[widgetState] = vtkSmartPointer<vtkWidgetEventTranslator>::New();
    translator = this->EventTranslators[widgetState];
    }
  vtkNew<vtkMRMLInteractionEventData> ed;
  ed->SetType(interactionEvent);
  ed->SetModifiers(modifiers);
  translator->SetTranslation(interactionEvent, ed, widgetEvent);
}

//----------------------------------------------------------------------
void vtkMRMLAbstractWidget::SetEventTranslation(unsigned long interactionEvent, int modifiers, unsigned long widgetEvent)
{
  this->SetEventTranslation(WidgetStateAny, interactionEvent, modifiers, widgetEvent);
}

//----------------------------------------------------------------------
void vtkMRMLAbstractWidget::SetEventTranslationClickAndDrag(int widgetState, unsigned long startInteractionEvent,
  int modifiers, int widgetStateDragging,
  unsigned long widgetStartEvent, unsigned long widgetEndEvent)
{
  unsigned long endInteractionEvent = WidgetEventNone;
  switch (startInteractionEvent)
    {
    case vtkCommand::LeftButtonPressEvent: endInteractionEvent = vtkCommand::LeftButtonReleaseEvent; break;
    case vtkCommand::MiddleButtonPressEvent: endInteractionEvent = vtkCommand::MiddleButtonReleaseEvent; break;
    case vtkCommand::RightButtonPressEvent: endInteractionEvent = vtkCommand::RightButtonReleaseEvent; break;
    }
  this->SetEventTranslation(widgetState, startInteractionEvent, modifiers, widgetStartEvent);
  this->SetEventTranslation(widgetStateDragging, vtkCommand::MouseMoveEvent, vtkEvent::AnyModifier, WidgetEventMouseMove);
  this->SetEventTranslation(widgetStateDragging, endInteractionEvent, vtkEvent::AnyModifier, widgetEndEvent);
}

//----------------------------------------------------------------------
void vtkMRMLAbstractWidget::SetKeyboardEventTranslation(
  int widgetState, int modifier, char keyCode,
  int repeatCount, const char* keySym, unsigned long widgetEvent)
{
  if (widgetState >= static_cast<int>(this->EventTranslators.size()))
    {
    this->EventTranslators.resize(widgetState +1);
    }
  vtkWidgetEventTranslator* translator = this->EventTranslators[widgetState];
  if (!translator)
    {
    this->EventTranslators[widgetState] = vtkSmartPointer<vtkWidgetEventTranslator>::New();
    translator = this->EventTranslators[widgetState];
    }
  translator->SetTranslation(vtkCommand::KeyPressEvent, modifier, keyCode,
    repeatCount, keySym, widgetEvent);
}

//----------------------------------------------------------------------
void vtkMRMLAbstractWidget::SetKeyboardEventTranslation(
  int modifier, char keyCode,
  int repeatCount, const char* keySym, unsigned long widgetEvent)
{
  this->SetKeyboardEventTranslation(WidgetStateAny, modifier, keyCode,
    repeatCount, keySym, widgetEvent);
}

//-------------------------------------------------------------------------
void vtkMRMLAbstractWidget::SetRepresentation(vtkMRMLAbstractWidgetRepresentation *rep)
{
  if (rep == this->WidgetRep)
    {
    // no change
    return;
    }

  if (this->WidgetRep)
    {
    if (this->Renderer)
      {
      this->WidgetRep->SetRenderer(nullptr);
      this->Renderer->RemoveViewProp(this->WidgetRep);
      }
    }

  this->WidgetRep = rep;

  if (this->Renderer != nullptr && this->WidgetRep != nullptr)
    {
    this->WidgetRep->SetRenderer(this->Renderer);
    this->Renderer->AddViewProp(this->WidgetRep);
    }
}

//-------------------------------------------------------------------------
vtkMRMLAbstractWidgetRepresentation* vtkMRMLAbstractWidget::GetRepresentation()
{
  return this->WidgetRep;
}

//-------------------------------------------------------------------------
void vtkMRMLAbstractWidget::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData/*=nullptr*/)
{
  if (!this->WidgetRep)
    {
    return;
    }

  this->WidgetRep->UpdateFromMRML(caller, event, callData);
}

//----------------------------------------------------------------------
void vtkMRMLAbstractWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "WidgetState: " << this->WidgetState << endl;
}

//-----------------------------------------------------------------------------
unsigned long vtkMRMLAbstractWidget::TranslateInteractionEventToWidgetEvent(
  vtkMRMLInteractionEventData* eventData)
{
  if (!eventData)
    {
    return WidgetEventNone;
    }

  // Try to process with a state-specific translator
  if (this->WidgetState < static_cast<int>(this->EventTranslators.size()))
    {
    vtkWidgetEventTranslator* translator = this->EventTranslators[this->WidgetState];
    if (translator)
      {
      unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(translator, eventData);
      if (widgetEvent != WidgetEventNone)
        {
        return widgetEvent;
        }
      }
    }

  // Try to process with the state-independent translator
  unsigned long widgetEvent = WidgetEventNone;
  if (WidgetStateAny < this->EventTranslators.size())
    {
    vtkWidgetEventTranslator* translator = this->EventTranslators[WidgetStateAny];
    if (translator)
      {
      // There is an event translator for this state
      widgetEvent = this->TranslateInteractionEventToWidgetEvent(translator, eventData);
      }
    }

  return widgetEvent;
}

//-----------------------------------------------------------------------------
unsigned long vtkMRMLAbstractWidget::TranslateInteractionEventToWidgetEvent(
  vtkWidgetEventTranslator* translator, vtkMRMLInteractionEventData* eventData)
{
  unsigned long widgetEvent = WidgetEventNone;

  if (!eventData)
    {
    return widgetEvent;
    }

  if (eventData->GetType() == vtkCommand::KeyPressEvent)
    {
    // We package keypress events information into event data,
    // unpack it for the event translator
    int modifier = eventData->GetModifiers();
    char keyCode = eventData->GetKeyCode();
    int repeatCount = eventData->GetKeyRepeatCount();
    const char* keySym = nullptr;
    if (!eventData->GetKeySym().empty())
      {
      keySym = eventData->GetKeySym().c_str();
      }

    // If neither the ctrl nor the shift keys are pressed, give
    // NoModifier a preference over AnyModifer.
    if (modifier == vtkEvent::AnyModifier)
      {
      widgetEvent = translator->GetTranslation(vtkCommand::KeyPressEvent,
        vtkEvent::NoModifier, keyCode, repeatCount, keySym);
      }
    if (widgetEvent == WidgetEventNone)
      {
      widgetEvent = translator->GetTranslation(vtkCommand::KeyPressEvent,
        modifier, keyCode, repeatCount, keySym);
      }
    }
  else
    {
    widgetEvent = translator->GetTranslation(eventData->GetType(), eventData);
    }

  return widgetEvent;
}

//-----------------------------------------------------------------------------
bool vtkMRMLAbstractWidget::CanProcessInteractionEvent(vtkMRMLInteractionEventData* vtkNotUsed(eventData), double &vtkNotUsed(distance2))
{
  return false;
}

//-----------------------------------------------------------------------------
bool vtkMRMLAbstractWidget::ProcessInteractionEvent(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  return false;
}

//-----------------------------------------------------------------------------
void vtkMRMLAbstractWidget::Leave(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  this->SetWidgetState(WidgetStateIdle);
}

//-------------------------------------------------------------------------
int vtkMRMLAbstractWidget::GetMouseCursor()
{
  if (this->WidgetState == WidgetStateIdle)
    {
    return VTK_CURSOR_DEFAULT;
    }
  else
    {
    return VTK_CURSOR_HAND;
    }
}

//-------------------------------------------------------------------------
bool vtkMRMLAbstractWidget::GetGrabFocus()
{
  // we need to grab focus when interactively dragging points
  return this->GetInteractive();
}

//-------------------------------------------------------------------------
bool vtkMRMLAbstractWidget::GetInteractive()
{
  switch (this->WidgetState)
    {
    case WidgetStateTranslate:
    case WidgetStateScale:
    case WidgetStateRotate:
      return true;
    default:
      return false;
    }
}

//-------------------------------------------------------------------------
bool vtkMRMLAbstractWidget::GetNeedToRender()
{
  if (!this->WidgetRep)
    {
    return false;
    }
  return this->WidgetRep->GetNeedToRender();
/*
  if (this->NeedToRender)
    {
    return true;
    }
  if (this->WidgetRep && this->WidgetRep->GetNeedToRender())
    {
    return true;
    }
  return false;*/
}

//-------------------------------------------------------------------------
void vtkMRMLAbstractWidget::NeedToRenderOff()
{
  if (!this->WidgetRep)
    {
    return;
    }
  this->WidgetRep->NeedToRenderOff();
/*
  this->NeedToRender = false;
  if (this->WidgetRep)
    {
    this->WidgetRep->NeedToRenderOff();
    }
*/
}

//----------------------------------------------------------------------
vtkRenderer* vtkMRMLAbstractWidget::GetRenderer()
{
  return this->Renderer;
}

//----------------------------------------------------------------------
void vtkMRMLAbstractWidget::SetRenderer(vtkRenderer* renderer)
{
  if (renderer == this->Renderer)
    {
    return;
    }

  if (this->Renderer != nullptr && this->WidgetRep != nullptr)
    {
    this->Renderer->RemoveViewProp(this->WidgetRep);
    }

  this->Renderer = renderer;

  if (this->WidgetRep != nullptr && this->Renderer != nullptr)
    {
    this->WidgetRep->SetRenderer(this->Renderer);
    this->Renderer->AddViewProp(this->WidgetRep);
    }
}

//---------------------------------------------------------------------------
const char* vtkMRMLAbstractWidget::GetAssociatedNodeID(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  if (!this->WidgetRep)
    {
    return nullptr;
    }
  // is there a volume in the background?
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(this->WidgetRep->GetViewNode());
  if (!sliceNode)
    {
    // this only works for slice views for now
    return nullptr;
    }
  // find the slice composite node in the scene with the matching layout name
  vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
  if (!mrmlAppLogic)
    {
    return nullptr;
    }
  vtkMRMLSliceLogic *sliceLogic = mrmlAppLogic->GetSliceLogic(sliceNode);
  if (!sliceLogic)
    {
    return nullptr;
    }
  vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode(sliceNode);
  if (!sliceCompositeNode)
    {
    return nullptr;
    }
  if (sliceCompositeNode->GetBackgroundVolumeID())
    {
    return sliceCompositeNode->GetBackgroundVolumeID();
    }
  else if (sliceCompositeNode->GetForegroundVolumeID())
    {
    return sliceCompositeNode->GetForegroundVolumeID();
    }
  else if (sliceCompositeNode->GetLabelVolumeID())
    {
    return sliceCompositeNode->GetLabelVolumeID();
    }
  return nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractWidget::SetMRMLApplicationLogic(vtkMRMLApplicationLogic* applicationLogic)
{
  this->ApplicationLogic = applicationLogic;
}

//---------------------------------------------------------------------------
vtkMRMLApplicationLogic* vtkMRMLAbstractWidget::GetMRMLApplicationLogic()
{
  return this->ApplicationLogic;
}

//---------------------------------------------------------------------------
vtkMRMLInteractionNode* vtkMRMLAbstractWidget::GetInteractionNode()
{
  if (!this->WidgetRep)
    {
    return nullptr;
    }
  vtkMRMLAbstractViewNode* viewNode = this->WidgetRep->GetViewNode();
  if (!viewNode)
    {
    return nullptr;
    }
  return viewNode->GetInteractionNode();
}
