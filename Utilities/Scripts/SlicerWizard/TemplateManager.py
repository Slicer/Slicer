import fnmatch
import logging
import os

from .Utilities import die

_sourcePatterns = [
  "*.h",
  "*.cxx",
  "*.cpp",
  "CMakeLists.txt",
  "*.cmake",
  "*.ui",
  "*.qrc",
  "*.py",
  "*.xml",
  "*.xml.in",
  "*.md5",
  "*.png",
  "*.dox",
]

_templateCategories = [
  "extensions",
  "modules",
]

#-----------------------------------------------------------------------------
def isSourceFile(name):
  for pat in _sourcePatterns:
    if fnmatch.fnmatch(name, pat):
      return True

  return False

#-----------------------------------------------------------------------------
def isTemplateCategory(name, relPath):
  if not os.path.isdir(os.path.join(relPath, name)):
    return False

  name = name.lower()
  return name in _templateCategories

#-----------------------------------------------------------------------------
def listSources(directory):
  for root, subFolders, files in os.walk(directory):
    for f in files:
      if isSourceFile(f):
        f = os.path.join(root, f)
        yield f[len(directory) + 1:] # strip common dir

#=============================================================================
class TemplateManager(object):
  #---------------------------------------------------------------------------
  def __init__(self):
    self._paths = {}
    self._keys = {}

    for c in _templateCategories:
      self._paths[c] = {}

  #---------------------------------------------------------------------------
  def _getKey(self, kind):
    if kind in self._keys:
      return self._keys[kind]

    return "TemplateKey"

  #---------------------------------------------------------------------------
  def _copyAndReplace(self, inFile, template, destination, key, name):
    outFile = os.path.join(destination, inFile.replace(key, name))
    logging.info("creating '%s'" % outFile)
    path = os.path.dirname(outFile)
    if not os.path.exists(path):
      os.makedirs(path)

    with open(os.path.join(template, inFile)) as fp:
      contents = fp.read()
    contents = contents.replace(key, name)
    contents = contents.replace(key.upper(), name.upper())
    with open(outFile, "w") as fp:
      fp.write(contents)

  #---------------------------------------------------------------------------
  def copyTemplate(self, destination, category, kind, name):
    templates = self._paths[category]
    if not kind.lower() in templates:
      raise KeyError("'%s' is not a known extension template" % kind)

    kind = kind.lower()

    destination = os.path.join(destination, name)
    if os.path.exists(destination):
      raise IOError("create %s: refusing to overwrite"
                    " existing directory '%s'" % (category, destination))

    template = templates[kind]
    key = self._getKey(kind)

    logging.info("copy template '%s' to '%s', replacing '%s' -> '%s'" %
          (template, destination, key, name))
    for f in listSources(template):
      self._copyAndReplace(f, template, destination, key, name)

    return destination

  #---------------------------------------------------------------------------
  def addCategoryPath(self, category, path):
    for entry in os.listdir(path):
      entryPath = os.path.join(path, entry)
      if os.path.isdir(entryPath):
        self._paths[category][entry.lower()] = entryPath

  #---------------------------------------------------------------------------
  def addPath(self, basePath):
    if not os.path.exists(basePath):
      return

    basePath = os.path.realpath(basePath)

    for entry in os.listdir(basePath):
      if isTemplateCategory(entry, basePath):
        self.addCategoryPath(entry.lower(), os.path.join(basePath, entry))

  #---------------------------------------------------------------------------
  def listTemplates(self):
    for c in _templateCategories:
      logging.info("Available templates for category '%s':" % c)

      if len(self._paths[c]):
        for t in sorted(self._paths[c].keys()):
          logging.info("  '%s' ('%s')" % (t, self._getKey(t)))

      else:
        logging.info("  (none)")

      logging.info("")

  #---------------------------------------------------------------------------
  def parseArguments(self, args):
    # Add user-specified template paths
    if args.templatePath is not None:
      for tp in args.templatePath:
        tpParts = tp.split("=", 1)

        if len(tpParts) == 1:
          if not os.path.exists(tp):
            die("template path '%s' does not exist" % tp)
          if not os.path.isdir(tp):
            die("template path '%s' is not a directory" % tp)

          self.addPath(tp)

        else:
          if tpParts[0].lower() not in _templateCategories:
            die(("'%s' is not a recognized template category" % tpParts[0],
                 "recognized categories: %s" % ", ".join(_templateCategories)))

          if not os.path.exists(tpParts[1]):
            die("template path '%s' does not exist" % tpParts[1])
          if not os.path.isdir(tpParts[1]):
            die("template path '%s' is not a directory" % tpParts[1])

          self.addCategoryPath(tpParts[0].lower(),
                               os.path.realpath(tpParts[1]))

    # Set user-specified template keys
    if args.templateKey is not None:
      for tk in args.templateKey:
        tkParts = tk.split("=")
        if len(tkParts) != 2:
          die("template key '%s' malformatted: expected 'NAME=KEY'" % tk)

        self._keys[tkParts[0]] = tkParts[1]
