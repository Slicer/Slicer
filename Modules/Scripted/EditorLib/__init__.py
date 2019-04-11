
import os
ICON_DIR = os.path.dirname(os.path.realpath(__file__)) + '/Resources/Icons/'
del os

from .ColorBox import ColorBox
from .EditUtil import EditUtil, UndoRedo
from .EditBox import EditBox
from .EditColor import EditColor
from .EditOptions import HelpButton, EditOptions
from .HelperBox import HelperBox

# Built-in effects
for effectName in [
  'Effect',
  'LabelEffect',
  'PaintEffect',
  'DrawEffect',
  'ThresholdEffect',
  'RectangleEffect',
  'LevelTracingEffect',
  'MakeModelEffect',
  'MorphologyEffect',
  'ErodeEffect',
  'DilateEffect',
  'IslandEffect',
  'IdentifyIslandsEffect',
  'ChangeIslandEffect',
  'RemoveIslandsEffect',
  'SaveIslandEffect',
  'ChangeLabelEffect',
  'GrowCutEffect',
  'WatershedFromMarkerEffect',
  'FastMarchingEffect',
  'WandEffect'
]:
  exec("from .{0} import {0}Options, {0}Tool, {0}Logic, {0}".format(effectName))
