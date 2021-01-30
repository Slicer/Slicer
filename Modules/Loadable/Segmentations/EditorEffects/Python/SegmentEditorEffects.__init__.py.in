
from .AbstractScriptedSegmentEditorEffect import *
from .AbstractScriptedSegmentEditorLabelEffect import *
from .AbstractScriptedSegmentEditorPaintEffect import *
from .AbstractScriptedSegmentEditorAutoCompleteEffect import *

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

import logging
import importlib
import traceback
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
  "SegmentEditorThresholdEffect"
  ]

def registerEffects():
  import SegmentEditorEffects
  import traceback
  import logging
  effectsPath = SegmentEditorEffects.__path__[0].replace('\\','/')
  import qSlicerSegmentationsEditorEffectsPythonQt as qSlicerSegmentationsEditorEffects
  for editorEffectName in SegmentEditorEffects.editorEffectNames:
    try:
      instance = qSlicerSegmentationsEditorEffects.qSlicerSegmentEditorScriptedEffect(None)
      effectFilename = effectsPath+'/'+editorEffectName+'.py'
      instance.setPythonSource(effectFilename)
      instance.self().register()
    except Exception as e:
      logging.error("Error instantiating "+editorEffectName+":\n"+traceback.format_exc())
