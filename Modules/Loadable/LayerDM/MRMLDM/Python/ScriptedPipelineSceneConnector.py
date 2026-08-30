import slicer
from slicer import (
    vtkMRMLLayerDMPipelineFactory,
    vtkMRMLLayerDMPipelineScriptedCreator,
)


class ScriptedPipelineSceneConnector:
    """
    Convenience base class connecting the scripted LayerDM pipelines of one module to the scene.

    Subclass and override:
        - GetPipelines: return the pipeline classes handled by the module.
        - GetSceneObservers: optionally return (eventId, callback) pairs to observe on the MRML scene.
        - OnConnected: optionally perform additional setup after connecting (for instance adding
            data nodes to the scene).

    Expected usage in a scripted module:
        - Instantiate the subclass at module level.
        - Call Connect at application startup and from the module widget's setup. Connect removes any
            previous connection first, so re-connecting from setup makes a module reload rebind the
            reloaded pipeline classes (the LayerDM pipeline managers automatically recreate the pipelines
            of a creator which was removed from the factory).
        - Call Disconnect from the module widget's cleanup, which is invoked by the module reload
            machinery before the widget is rebuilt and when the module is unloaded.
    """

    def __init__(self, scene=None):
        """
        :param scene: Optional MRML scene to connect to. If None, the slicer.mrmlScene singleton is used
            when Connect is called. Passing the scene explicitly avoids depending on the singleton, which
            is not available in trame-slicer.
        """
        self._scene = scene
        self._connectedScene = None
        self._pipelineCreator = None
        self._sceneObserverTags = []

    def GetScene(self):
        """
        Returns the scene this connector is connected to.
        While disconnected, returns the scene it would connect to (the scene given at construction,
        or the slicer.mrmlScene singleton).
        Subclass overrides should use this instead of the slicer.mrmlScene singleton.
        """
        if self._connectedScene is not None:
            return self._connectedScene
        return self._scene if self._scene is not None else slicer.mrmlScene

    def GetPipelines(self) -> list:
        """
        Override to return the pipeline classes handled by the module.
        Each class should provide a TryCreatePipeline(viewNode, node) class method returning a new pipeline
        instance or None. The classes are queried in order until one returns a pipeline.
        """
        return []

    def GetSceneObservers(self) -> list:
        """Override to return (eventId, callback) pairs to observe on the MRML scene while connected."""
        return []

    def OnConnected(self) -> None:
        """Override to perform additional setup after connecting (for instance adding data nodes to the scene)."""

    def Connect(self) -> None:
        """
        Registers a pipeline creator for the GetPipelines classes, adds the GetSceneObservers observers and
        calls OnConnected. Any previous connection held by this instance is removed first.

        :raises TypeError: If any of the GetPipelines classes doesn't implement TryCreatePipeline.
            Checked eagerly, as a missing TryCreatePipeline would otherwise only fail when a node is
            added to the scene, far from the actual mistake.
        """
        pipelines = self.GetPipelines()
        for pipeline in pipelines:
            if not callable(getattr(pipeline, "TryCreatePipeline", None)):
                raise TypeError(
                    f"{pipeline.__name__} cannot be used with {type(self).__name__}: "
                    "it does not implement the TryCreatePipeline(viewNode, node) class method.")

        self.Disconnect()
        self._connectedScene = self._scene if self._scene is not None else slicer.mrmlScene

        def tryCreate(viewNode, node):
            for pipeline in pipelines:
                ret = pipeline.TryCreatePipeline(viewNode, node)
                if ret is not None:
                    return ret
            return None

        self._pipelineCreator = vtkMRMLLayerDMPipelineScriptedCreator()
        self._pipelineCreator.SetPythonCallback(tryCreate)
        vtkMRMLLayerDMPipelineFactory.GetInstance().AddPipelineCreator(self._pipelineCreator)

        for eventId, callback in self.GetSceneObservers():
            self._sceneObserverTags.append(self._connectedScene.AddObserver(eventId, callback))

        self.OnConnected()

    def Disconnect(self) -> None:
        """Undoes the connection done by Connect. Safe to call when nothing is connected."""
        if self._pipelineCreator is not None:
            vtkMRMLLayerDMPipelineFactory.GetInstance().RemovePipelineCreator(self._pipelineCreator)
            self._pipelineCreator = None
        for tag in self._sceneObserverTags:
            self._connectedScene.RemoveObserver(tag)
        self._sceneObserverTags = []
        self._connectedScene = None
