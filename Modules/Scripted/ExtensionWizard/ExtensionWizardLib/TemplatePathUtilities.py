import os

from __main__ import slicer

_userTemplatePathKey = "ExtensionWizard/TemplatePaths"

#-----------------------------------------------------------------------------
def userTemplatePathKey(category=None):
  if category is None:
    return _userTemplatePathKey
  else:
    return "%s/%s" % (_userTemplatePathKey, category)

#-----------------------------------------------------------------------------
def builtinTemplatePath():
  path = os.path.join(slicer.app.slicerHome, slicer.app.slicerSharePath,
                      "Wizard", "Templates")
  return path if os.path.exists(path) else None
