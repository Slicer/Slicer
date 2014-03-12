#!/usr/bin/env python

import argparse
import fnmatch
import os
import re
import sys

sourcePatterns = [
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

argValueFormats = {
  "addModule": "TYPE:NAME",
  "createExtension": "[TYPE:]NAME",
}

templateCategories = [
  "extensions",
  "modules",
]

templatePaths = {}

reModuleInsertPlaceholder = re.compile("(?<=\n)([ \t]*)## NEXT_MODULE")
reAddSubdirectory = \
  re.compile("(?<=\n)([ \t]*)add_subdirectory[(][^)]+[)][^\n]*\n")

#-----------------------------------------------------------------------------
def isSourceFile(name):
  for pat in sourcePatterns:
    if fnmatch.fnmatch(name, pat):
      return True

  return False

#-----------------------------------------------------------------------------
def isTemplateCategory(name, categories, relPath):
  if not os.path.isdir(os.path.join(relPath, name)):
    return False

  name = name.lower()
  return name in categories and name in templateCategories

#-----------------------------------------------------------------------------
def listSources(directory):
  for root, subFolders, files in os.walk(directory):
    for f in files:
      if isSourceFile(f):
        f = os.path.join(root, f)
        yield f[len(directory) + 1:] # strip common dir

#-----------------------------------------------------------------------------
def copyAndReplace(inFile, template, destination, key, name):
  outFile = os.path.join(destination, inFile.replace(key, name))
  print("creating '%s'" % outFile)
  path = os.path.dirname(outFile)
  if not os.path.exists(path):
    os.makedirs(path)

  with open(os.path.join(template, inFile)) as fp:
    contents = fp.read()
  contents = contents.replace(key, name)
  contents = contents.replace(key.upper(), name.upper())
  with open(outFile, "w") as fp:
    fp.write(contents)

#-----------------------------------------------------------------------------
def copyTemplate(args, category, kind, name):
  templates = templatePaths[category]
  if not kind.lower() in templates:
    print("'%s' is not a known extension template" % kind)
    exit()

  destination = os.path.join(args.destination, name)
  if os.path.exists(destination):
    print("create %s: refusing to overwrite existing directory '%s'" \
          % (category, destination))
    exit()

  template = templates[kind.lower()]
  key = "TemplateKey" # TODO allow user key

  print("copy template '%s' to '%s', replacing '%s' -> '%s'" %
        (template, destination, key, name))
  for f in listSources(template):
    copyAndReplace(f, template, destination, key, name)

  return destination

#-----------------------------------------------------------------------------
def addTemplateCategoryPaths(category, path):
  if not category in templatePaths:
    templatePaths[category] = {}

  for entry in os.listdir(path):
    entryPath = os.path.join(path, entry)
    if os.path.isdir(entryPath):
      templatePaths[category][entry.lower()] = entryPath

#-----------------------------------------------------------------------------
def addTemplatePaths(categories, basePath):
  if not os.path.exists(basePath):
    return

  basePath = os.path.realpath(basePath)

  for entry in os.listdir(basePath):
    if isTemplateCategory(entry, categories, basePath):
      addTemplateCategoryPaths(entry.lower(), os.path.join(basePath, entry))

#-----------------------------------------------------------------------------
def addModuleToScript(name, contents):
  contents = "\n" + contents
  pat = "%sadd_subdirectory(%s)\n"

  # Try to insert before placeholder
  m = reModuleInsertPlaceholder.search(contents)
  if m is not None:
    return contents[1:m.start()] + \
           pat % (m.group(1), name) + \
           contents[m.start():]

  # No? Try to insert after last add_subdirectory
  for m in reAddSubdirectory.finditer(contents):
    pass

  if m is not None:
    print m.groups(), m.start(), m.end()
    return contents[1:m.end()] + \
           pat % (m.group(1), name) + \
           contents[m.end():]

  # Still no? Oh, dear...
  print("failed to find insertion point for module"
        " in parent CMakeLists.txt")
  exit()

#-----------------------------------------------------------------------------
def addModuleToProject(path, name):
  cmakeFile = os.path.join(path, "CMakeLists.txt")
  if not os.path.exists(cmakeFile):
    print("failed to add module to project '%s': no CMakeLists.txt found" %
          path)
    exit()

  with open(cmakeFile) as fp:
    contents = fp.read()
  with open(cmakeFile, "w") as fp:
    fp.write(addModuleToScript(name, contents))

#-----------------------------------------------------------------------------
def createExtension(args, name, kind="default"):
  args.destination = copyTemplate(args, "extensions", kind, name)
  print("created extension '%s'" % name)

#-----------------------------------------------------------------------------
def addModule(args, kind, name):
  addModuleToProject(args.destination, name)
  copyTemplate(args, "modules", kind, name)
  print("created module '%s'" % name)

#=============================================================================
class WizardHelpFormatter(argparse.HelpFormatter):
  #---------------------------------------------------------------------------
  def _metavar_formatter(self, action, default_metavar):
    if action.dest in argValueFormats:
      default_metavar = argValueFormats[action.dest]
    return super(WizardHelpFormatter, self)._metavar_formatter(action,
                                                               default_metavar)

#-----------------------------------------------------------------------------
def main():

  parser = argparse.ArgumentParser(description="Slicer Wizard",
                                   formatter_class=WizardHelpFormatter)
  parser.add_argument("--addModule", action="append",
                      help="add new TYPE module NAME to an existing project"
                           " in the destination directory;"
                           " may use more than once")
  parser.add_argument("--createExtension",
                      help="create extension NAME"
                           " under the destination directory;"
                           " any modules are added to the new extension"
                           " (default type: 'default')")
  parser.add_argument("destination", default=os.getcwd(), nargs="?",
                      help="location of output files (default: '.')")
  args = parser.parse_args()

  scriptPath = os.path.dirname(os.path.realpath(__file__))
  addTemplatePaths(templateCategories,
                   os.path.join(scriptPath, "..", "Templates"))

  if args.createExtension is None and args.addModule is None:
    print("no action was requested!")
    exit()

  if args.createExtension is not None:
    extArgs = args.createExtension.split(":")
    extArgs.reverse()
    createExtension(args, *extArgs)

  if args.addModule is not None:
    for module in args.addModule:
      addModule(args, *module.split(":"))

#=============================================================================

if __name__ == "__main__":
  main()
