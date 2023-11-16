from .AbstractScriptedSegmentEditorEffect import *

__all__ = ["AbstractScriptedSegmentEditorPaintEffect"]


class AbstractScriptedSegmentEditorPaintEffect(AbstractScriptedSegmentEditorEffect):
    """Abstract scripted segment editor Paint effects for effects implemented in python

    Paint effects are a subtype of general effects that operate on a subregion of the segment
    defined by brush strokes.

    An example of paint effect is ``SmoothingEffect``.

    To use the effect, follow these steps:

    1. Instantiation and registration

      Instantiate segment editor paint effect adaptor class from
      module (e.g. from setup function), and set python source::

        import qSlicerSegmentationsEditorEffectsPythonQt as effects
        scriptedEffect = effects.qSlicerSegmentEditorScriptedPaintEffect(None)
        scriptedEffect.setPythonSource(MyPaintEffect.filePath)

      Registration is automatic

    2. Call host C++ implementation using::

        self.scriptedEffect.functionName()

    2.a. Most frequently used methods are:

      * Parameter get/set: ``parameter``, ``integerParameter``, ``doubleParameter``, ``setParameter``
      * Add options widget: ``addOptionsWidget``
      * Coordinate transforms: ``rasToXy``, ``xyzToRas``, ``xyToRas``, ``xyzToIjk``, ``xyToIjk``
      * Convenience getters: ``renderWindow``, ``renderer``, ``viewNode``
      * Geometry getters: ``imageToWorldMatrix`` (for volume node and for oriented image data with segmentation)

    2.b. Always call API functions (the ones that are defined in the adaptor \
      class ``qSlicerSegmentEditorScriptedPaintEffect``) using the adaptor accessor ``self.scriptedEffect``.

      For example::

        self.scriptedEffect.updateGUIFromMRML()
    """

    def __init__(self, scriptedEffect):
        AbstractScriptedSegmentEditorEffect.__init__(self, scriptedEffect)
