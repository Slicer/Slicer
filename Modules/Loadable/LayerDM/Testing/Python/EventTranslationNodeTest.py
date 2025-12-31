import os
from tempfile import TemporaryDirectory

import slicer
from slicer import vtkMRMLAbstractWidget, vtkMRMLInteractionEventData
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest
from vtk import vtkCommand, vtkEvent


class EventTranslationNodeTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)
        self.tl_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLLayerDMWidgetEventTranslationNode")
        self.no_event = vtkMRMLAbstractWidget.WidgetEventNone

    def save_restore_scene(self):
        node_id = self.tl_node.GetID()

        with TemporaryDirectory() as tmp_dir:
            scene_path = os.path.join(tmp_dir, "scene.mrb")
            slicer.util.saveScene(scene_path)
            slicer.mrmlScene.Clear(0)
            slicer.util.loadScene(scene_path)

        return slicer.mrmlScene.GetNodeByID(node_id)

    def click_event(self, event_type, modifier=vtkEvent.AnyModifier):
        event_data = vtkMRMLInteractionEventData()
        event_data.SetType(event_type)
        event_data.SetModifiers(modifier)
        return event_data

    def key_event(self, key, *, modifier=vtkEvent.AnyModifier):
        event_data = vtkMRMLInteractionEventData()
        event_data.SetType(vtkCommand.KeyPressEvent)
        event_data.SetModifiers(modifier)
        event_data.SetKeySym(key)
        return event_data

    def test_can_be_created_from_scene(self):
        assert self.tl_node is not None

    def test_can_be_saved_and_restored_from_scene(self):
        self.tl_node.SetTranslation(
            vtkMRMLAbstractWidget.WidgetStateOnWidget,
            vtkCommand.LeftButtonReleaseEvent,
            vtkMRMLAbstractWidget.WidgetEventPick,
            vtkEvent.ControlModifier,
        )
        self.tl_node.SetTranslationKeyboard(
            vtkMRMLAbstractWidget.WidgetStateIdle, "Delete", vtkMRMLAbstractWidget.WidgetEventReset
        )

        loaded_node = self.save_restore_scene()
        assert loaded_node is not None
        assert loaded_node.GetNumberOfTranslations() == 2

        click = self.click_event(vtkCommand.LeftButtonReleaseEvent, vtkEvent.ControlModifier)
        e1 = loaded_node.Translate(vtkMRMLAbstractWidget.WidgetStateOnWidget, click)
        assert e1 == vtkMRMLAbstractWidget.WidgetEventPick

        e2 = loaded_node.Translate(vtkMRMLAbstractWidget.WidgetStateIdle, self.key_event("Delete"))
        assert e2 == vtkMRMLAbstractWidget.WidgetEventReset

    def test_saving_and_restoring_empty_returns_empty(self):
        loaded_node = self.save_restore_scene()
        assert loaded_node.GetNumberOfTranslations() == 0

    def test_can_translate_click_events(self):
        self.tl_node.SetTranslation(
            vtkMRMLAbstractWidget.WidgetStateOnWidget,
            vtkCommand.LeftButtonReleaseEvent,
            vtkMRMLAbstractWidget.WidgetEventPick,
        )

        actual_event = self.tl_node.Translate(
            vtkMRMLAbstractWidget.WidgetStateOnWidget, self.click_event(vtkCommand.LeftButtonReleaseEvent)
        )
        assert actual_event == vtkMRMLAbstractWidget.WidgetEventPick

    def test_can_translate_key_events(self):
        self.tl_node.SetTranslationKeyboard(
            vtkMRMLAbstractWidget.WidgetStateOnWidget,
            "Delete",
            vtkMRMLAbstractWidget.WidgetEventUser,
        )

        actual_event = self.tl_node.Translate(vtkMRMLAbstractWidget.WidgetStateOnWidget, self.key_event("Delete"))
        assert actual_event == vtkMRMLAbstractWidget.WidgetEventUser

    def test_translate_is_differentiated_by_modifiers(self):
        self.tl_node.SetTranslation(
            vtkMRMLAbstractWidget.WidgetStateOnWidget,
            vtkCommand.LeftButtonReleaseEvent,
            vtkMRMLAbstractWidget.WidgetEventPick,
        )

        self.tl_node.SetTranslation(
            vtkMRMLAbstractWidget.WidgetStateOnWidget,
            vtkCommand.LeftButtonReleaseEvent,
            vtkMRMLAbstractWidget.WidgetEventReset,
            vtkEvent.ControlModifier,
        )

        e1 = self.tl_node.Translate(
            vtkMRMLAbstractWidget.WidgetStateOnWidget, self.click_event(vtkCommand.LeftButtonReleaseEvent)
        )

        e2 = self.tl_node.Translate(
            vtkMRMLAbstractWidget.WidgetStateOnWidget,
            self.click_event(vtkCommand.LeftButtonReleaseEvent, vtkEvent.ControlModifier),
        )

        assert e1 == vtkMRMLAbstractWidget.WidgetEventPick
        assert e2 == vtkMRMLAbstractWidget.WidgetEventReset

    def test_translate_has_any_state_source(self):
        self.tl_node.SetTranslation(
            vtkMRMLAbstractWidget.WidgetStateAny,
            vtkCommand.LeftButtonReleaseEvent,
            vtkMRMLAbstractWidget.WidgetEventUser,
        )

        actual_event = self.tl_node.Translate(
            vtkMRMLAbstractWidget.WidgetStateOnWidget, self.click_event(vtkCommand.LeftButtonReleaseEvent)
        )
        assert actual_event == vtkMRMLAbstractWidget.WidgetEventUser

    def test_specific_states_have_precedence_over_any_state(self):
        self.tl_node.SetTranslation(
            vtkMRMLAbstractWidget.WidgetStateAny,
            vtkCommand.LeftButtonReleaseEvent,
            vtkMRMLAbstractWidget.WidgetEventUser,
        )

        self.tl_node.SetTranslation(
            vtkMRMLAbstractWidget.WidgetStateOnWidget,
            vtkCommand.LeftButtonReleaseEvent,
            vtkMRMLAbstractWidget.WidgetEventMenu,
        )

        actual_event = self.tl_node.Translate(
            vtkMRMLAbstractWidget.WidgetStateOnWidget, self.click_event(vtkCommand.LeftButtonReleaseEvent)
        )
        assert actual_event == vtkMRMLAbstractWidget.WidgetEventMenu

    def test_can_translate_drag_events(self):
        dragging_state = vtkMRMLAbstractWidget.WidgetStateUser + 1
        start_event = vtkMRMLAbstractWidget.WidgetEventUser + 1
        end_event = vtkMRMLAbstractWidget.WidgetEventUser + 2

        self.tl_node.SetTranslationClickAndDrag(
            vtkMRMLAbstractWidget.WidgetStateOnWidget,
            vtkCommand.LeftButtonPressEvent,
            dragging_state,
            start_event,
            end_event,
        )

        actual_start_event = self.tl_node.Translate(
            vtkMRMLAbstractWidget.WidgetStateOnWidget, self.click_event(vtkCommand.LeftButtonPressEvent)
        )
        assert actual_start_event == start_event

        actual_dragging_event = self.tl_node.Translate(dragging_state, self.click_event(vtkCommand.MouseMoveEvent))
        assert actual_dragging_event == vtkMRMLAbstractWidget.WidgetEventMouseMove

        actual_end_event = self.tl_node.Translate(dragging_state, self.click_event(vtkCommand.LeftButtonReleaseEvent))
        assert actual_end_event == end_event

    def configure_menu_event(self):
        widget_state = vtkMRMLAbstractWidget.WidgetStateOnWidget
        self.tl_node.SetTranslation(
            widget_state,
            vtkCommand.LeftButtonReleaseEvent,
            vtkMRMLAbstractWidget.WidgetEventUser,
        )

        self.tl_node.SetTranslation(
            widget_state,
            vtkCommand.MiddleButtonReleaseEvent,
            vtkMRMLAbstractWidget.WidgetEventMenu,
        )

        self.tl_node.SetTranslation(
            widget_state,
            vtkCommand.RightButtonReleaseEvent,
            vtkMRMLAbstractWidget.WidgetEventMenu,
        )
        return widget_state

    def test_can_remove_given_event_types(self):
        widget_state =self.configure_menu_event()
        assert self.tl_node.RemoveTranslationEvent(vtkMRMLAbstractWidget.WidgetEventMenu) == 2
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.MiddleButtonReleaseEvent)) == self.no_event
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.RightButtonReleaseEvent)) == self.no_event
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.LeftButtonReleaseEvent)) != self.no_event

    def test_can_block_event_types(self):
        widget_state =self.configure_menu_event()
        was_blocked = self.tl_node.BlockTranslationEvent(vtkMRMLAbstractWidget.WidgetEventMenu, True)
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.MiddleButtonReleaseEvent)) == self.no_event
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.RightButtonReleaseEvent)) == self.no_event
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.LeftButtonReleaseEvent)) != self.no_event

        self.tl_node.BlockTranslationEvent(vtkMRMLAbstractWidget.WidgetEventMenu, was_blocked)
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.MiddleButtonReleaseEvent)) != self.no_event
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.RightButtonReleaseEvent)) != self.no_event

    def test_can_block_all_event_types(self):
        widget_state =self.configure_menu_event()
        was_blocked = self.tl_node.BlockAllTranslationEvents(True)
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.MiddleButtonReleaseEvent)) == self.no_event
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.RightButtonReleaseEvent)) == self.no_event
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.LeftButtonReleaseEvent)) == self.no_event

        self.tl_node.BlockAllTranslationEvents(was_blocked)
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.MiddleButtonReleaseEvent)) != self.no_event
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.RightButtonReleaseEvent)) != self.no_event
        assert self.tl_node.Translate(widget_state, self.click_event(vtkCommand.LeftButtonReleaseEvent)) != self.no_event
