#!/usr/bin/env python

import os
import re

from . import CMakeParser

#-----------------------------------------------------------------------------
def _isCommand(token, name):
  return isinstance(token, CMakeParser.Command) and token.text.lower() == name

#-----------------------------------------------------------------------------
def _trimIndent(indent):
  indent = "\n" + indent
  n = indent.rindex("\n")
  return indent[n:]

#=============================================================================
class ExtensionProject(object):
  _moduleInsertPlaceholder = "# NEXT_MODULE"

  #---------------------------------------------------------------------------
  def __init__(self, path):
    cmakeFile = os.path.join(path, "CMakeLists.txt")
    if not os.path.exists(cmakeFile):
      raise IOError("CMakeLists.txt not found")

    self._scriptPath = cmakeFile

    with open(cmakeFile) as fp:
      self._scriptContents = CMakeParser.CMakeScript(fp.read())

  #---------------------------------------------------------------------------
  def _addModuleToScript(self, name):
    indent = ""
    after = -1

    for n in xrange(len(self._scriptContents.tokens)):
      t = self._scriptContents.tokens[n]

      if isinstance(t, CMakeParser.Comment) and \
         t.text.startswith(self._moduleInsertPlaceholder):
        indent = t.indent
        after = n
        t.indent = _trimIndent(t.indent)
        break

      if _isCommand(t, "add_subdirectory"):
        indent = _trimIndent(t.indent)
        after = n + 1

    if after < 0:
      raise EOFError("failed to find insertion point for module")

    arguments = [CMakeParser.String(text=name)]
    t = CMakeParser.Command(text="add_subdirectory", arguments=arguments,
                            indent=indent)
    self._scriptContents.tokens.insert(after, t)

  #---------------------------------------------------------------------------
  def addModule(self, name):
    self._addModuleToScript(name)
    with open(self._scriptPath, "w") as fp:
      fp.write(str(self._scriptContents))
