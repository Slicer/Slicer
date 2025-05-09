import importlib
import logging
import traceback

from .AbstractScriptedSegmentEditorAutoCompleteEffect import *
from .AbstractScriptedSegmentEditorEffect import *
from .AbstractScriptedSegmentEditorLabelEffect import *
from .AbstractScriptedSegmentEditorPaintEffect import *
from .SegmentEditorDrawEffect import *
from .SegmentEditorFillBetweenSlicesEffect import *
from .SegmentEditorGrowFromSeedsEffect import *
from .SegmentEditorHollowEffect import *
from .SegmentEditorIslandsEffect import *
from .SegmentEditorLevelTracingEffect import *
from .SegmentEditorLogicalEffect import *
from .SegmentEditorMarginEffect import *
from .SegmentEditorMaskVolumeEffect import *
from .SegmentEditorSmoothingEffect import *
from .SegmentEditorThresholdEffect import *
from SegmentEditorEffects import *

editorEffectNames = [
    "SegmentEditorDrawEffect",
    "SegmentEditorFillBetweenSlicesEffect",
    "SegmentEditorGrowFromSeedsEffect",
    "SegmentEditorHollowEffect",
    "SegmentEditorIslandsEffect",
    "SegmentEditorLevelTracingEffect",
    "SegmentEditorLogicalEffect",
    "SegmentEditorMarginEffect",
    "SegmentEditorMaskVolumeEffect",
    "SegmentEditorSmoothingEffect",
    "SegmentEditorThresholdEffect",
]


def registerEffects():
    import logging
    import traceback

    import SegmentEditorEffects

    effectsPath = SegmentEditorEffects.__path__[0].replace("\\", "/")

    import qSlicerSegmentationsEditorEffectsPythonQt as qSlicerSegmentationsEditorEffects

    for editorEffectName in SegmentEditorEffects.editorEffectNames:
        try:
            instance = qSlicerSegmentationsEditorEffects.qSlicerSegmentEditorScriptedEffect(None)
            effectFilename = effectsPath + "/" + editorEffectName + ".py"
            instance.setPythonSource(effectFilename)
            instance.self().register()
        except Exception as e:
            logging.error("Error instantiating " + editorEffectName + ":\n" + traceback.format_exc())
