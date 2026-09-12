import slicer
from slicer import vtkSlicerLayerDMLogic, vtkMRMLLayerDMWidgetEventTranslationNode
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleTest


class LogicTest(ScriptedLoadableModuleTest):
    def setUp(self):
        slicer.mrmlScene.Clear(0)

    def test_can_attach_translation_nodes_to_other_nodes(self):
        node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode")
        assert node is not None

        tl_node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLLayerDMWidgetEventTranslationNode")
        assert tl_node is not None

        vtkSlicerLayerDMLogic.SetWidgetEventTranslationNode(node, tl_node)
        assert vtkSlicerLayerDMLogic.GetWidgetEventTranslationNode(node) == tl_node

    def test_can_register_singleton_translation_nodes(self):
        assert vtkSlicerLayerDMLogic.GetWidgetEventTranslationSingleton(slicer.mrmlScene, "MyTLNodeSingleton") is None
        tl_node = vtkSlicerLayerDMLogic.CreateWidgetEventTranslationSingleton(slicer.mrmlScene, "MyTLNodeSingleton")
        assert tl_node is not None
        assert isinstance(tl_node, vtkMRMLLayerDMWidgetEventTranslationNode)
        assert not tl_node.GetSaveWithScene()

    def test_singleton_translation_nodes_are_created_once(self):
        tl_node = vtkSlicerLayerDMLogic.CreateWidgetEventTranslationSingleton(slicer.mrmlScene, "MyTLNodeSingleton")
        assert vtkSlicerLayerDMLogic.CreateWidgetEventTranslationSingleton(slicer.mrmlScene, "MyTLNodeSingleton") == tl_node

    def test_can_attach_singleton_tl_node(self):
        node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode")
        assert node is not None
        tl_node = vtkSlicerLayerDMLogic.CreateWidgetEventTranslationSingleton(slicer.mrmlScene, "MyTLNodeSingleton")
        vtkSlicerLayerDMLogic.SetWidgetEventTranslationNode(node, tl_node)
        assert vtkSlicerLayerDMLogic.GetWidgetEventTranslationNode(node) == tl_node
