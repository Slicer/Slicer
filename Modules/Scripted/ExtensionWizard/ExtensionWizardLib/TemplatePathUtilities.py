import os

import slicer

_userTemplatePathKey = "ExtensionWizard/TemplatePaths"

#-----------------------------------------------------------------------------
def userTemplatePathKey(category=None):
  if category is None:
    return _userTemplatePathKey
  else:
    return "%s/%s" % (_userTemplatePathKey, category)

#-----------------------------------------------------------------------------
def builtinTemplatePath():
  # Look for templates in source directory first
  path = slicer.util.sourceDir()

  if path is not None:
    path = os.path.join(path, "Utilities", "Templates")

    if os.path.exists(path):
      return path

  # Look for installed templates
  path = os.path.join(slicer.app.slicerHome, slicer.app.slicerSharePath,
                      "Wizard", "Templates")

  if os.path.exists(path):
    return path

  # No templates found
  return None
