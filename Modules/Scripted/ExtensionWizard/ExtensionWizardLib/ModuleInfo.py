import os

import slicer

#=============================================================================
#
# _ui_CreateComponentDialog
#
#=============================================================================
#=============================================================================
#
# ModuleInfo
#
#=============================================================================
class ModuleInfo(object):
  #---------------------------------------------------------------------------
  def __init__(self, path, key=None):
    self.path = path
    self.searchPath = os.path.dirname(path)

    if key is None:
      self.key = os.path.splitext(os.path.basename(path))[0]
    else:
      self.key = key

  #---------------------------------------------------------------------------
  def __repr__(self):
    return "ModuleInfo(key=%(key)r, path=%(path)r)" % self.__dict__

  #---------------------------------------------------------------------------
  def __str__(self):
    return self.path

  #---------------------------------------------------------------------------
  @staticmethod
  def findModules(path, depth):
    result = []
    entries = [os.path.join(path, entry) for entry in os.listdir(path)]

    if depth > 0:
      for entry in filter(os.path.isdir, entries):
        result += ModuleInfo.findModules(entry, depth - 1)

    for entry in filter(os.path.isfile, entries):
      # __init__.py is not a module but an embedded Python library
      # that a module will load.
      if entry.endswith(".py") and not entry.endswith("__init__.py"):
        result.append(ModuleInfo(entry))

    return result
