#!/usr/bin/env python

import os
import re

#=============================================================================
class ExtensionProject(object):
  _reModuleInsertPlaceholder = re.compile("(?<=\n)([ \t]*)## NEXT_MODULE")
  _reAddSubdirectory = \
    re.compile("(?<=\n)([ \t]*)add_subdirectory\\s*[(][^)]+[)][^\n]*\n")

  #---------------------------------------------------------------------------
  def __init__(self, path):
    cmakeFile = os.path.join(path, "CMakeLists.txt")
    if not os.path.exists(cmakeFile):
      raise IOError("CMakeLists.txt not found")

    self._scriptPath = cmakeFile

    with open(cmakeFile) as fp:
      self._scriptContents = fp.read()

  #---------------------------------------------------------------------------
  def _addModuleToScript(self, name):
    contents = "\n" + self._scriptContents
    pat = "%sadd_subdirectory(%s)\n"

    m = self._reModuleInsertPlaceholder.search(contents)
    if m is not None:
      # Inserting before placeholder
      n = m.start()

    else:
      for m in self._reAddSubdirectory.finditer(contents, re.IGNORECASE):
        pass

      if m is None:
        raise EOFError("failed to find insertion point for module")

      # Inserting after last add_subdirectory
      n = m.end()

    self._scriptContents = \
      contents[1:n] + pat % (m.group(1), name) + contents[n:]

  #---------------------------------------------------------------------------
  def addModule(self, name):
    self._addModuleToScript(name)
    with open(self._scriptPath, "w") as fp:
      fp.write(self._scriptContents)
