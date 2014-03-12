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
  def project(self):
    for t in self._scriptContents.tokens:
      if _isCommand(t, "project") and len(t.arguments):
        return t.arguments[0].text

    raise EOFError("could not find project")

  #---------------------------------------------------------------------------
  def getValue(self, name, default=None):
    for t in self._scriptContents.tokens:
      if _isCommand(t, "set") and len(t.arguments) and \
         t.arguments[0].text == name:
        if len(t.arguments) < 2:
          return None

        return t.arguments[1].text

    if default is not None:
      return default

    raise KeyError("script does not set %r" % name)

  #---------------------------------------------------------------------------
  def setValue(self, name, value):
    for t in self._scriptContents.tokens:
      if _isCommand(t, "set") and len(t.arguments) and \
         t.arguments[0].text == name:
        if len(t.arguments) < 2:
          t.arguments.append(CMakeParser.String(text=value, indent=" ",
                                                prefix="\"", suffix="\""))

        else:
          varg = t.arguments[1]
          varg.text = value
          varg.prefix = "\""
          varg.suffix = "\""

        return

    raise KeyError("script does not set %r" % name)

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

  #---------------------------------------------------------------------------
  def save(self, destination=None):
    if destination is None:
      destination = self._scriptPath

    with open(destination, "w") as fp:
      fp.write(str(self._scriptContents))
