
import os
ICON_DIR = os.path.dirname(os.path.realpath(__file__)) + '/Resources/Icons/'
del os

from ColorBox import *
from EditBox import *
from EditColor import *
from EditOptions import *
from HelperBox import *
from LabelEffect import LabelEffectOptions, LabelEffectTool, LabelEffectLogic

# Built-in effects
from PaintEffect import *
from DrawEffect import *
from ThresholdEffect import *
from RectangleEffect import *
from LevelTracingEffect import *
from MakeModelEffect import *
from MorphologyEffect import *
from ErodeEffect import *
from DilateEffect import *
from IslandEffect import *
from IdentifyIslandsEffect import *
from ChangeIslandEffect import *
from RemoveIslandsEffect import *
from SaveIslandEffect import *
from ChangeLabelEffect import *
from GrowCutEffect import *
from WatershedFromMarkerEffect import *
from FastMarchingEffect import *
from WandEffect import *

# XXX Importing this module last will ensure effect
#     calling "EditUtil.EditUtil()" will work.
import EditUtil
