#!/usr/bin/env python

from __future__ import print_function
import sys
import os
import fnmatch


def findSource(dir):
  fileList = []
  for root, subFolders, files in os.walk(dir):
      for file in files:
          if fnmatch.fnmatch(file, "*.h") or \
              fnmatch.fnmatch(file, "*.cxx") or \
              fnmatch.fnmatch(file, "*.cpp") or \
              fnmatch.fnmatch(file, "CMakeLists.txt") or \
              fnmatch.fnmatch(file, "*.cmake") or \
              fnmatch.fnmatch(file, "*.ui") or \
              fnmatch.fnmatch(file, "*.qrc") or \
              fnmatch.fnmatch(file, "*.py") or \
              fnmatch.fnmatch(file, "*.xml") or \
              fnmatch.fnmatch(file, "*.xml.in") or \
              fnmatch.fnmatch(file, "*.md5") or \
              fnmatch.fnmatch(file, "*.png") or \
              fnmatch.fnmatch(file, "*.dox"):
            file = os.path.join(root,file)
            file = file[len(dir):] # strip common dir
            fileList.append(file)
  return fileList

def copyAndReplace(inFile, template, target, key, moduleName):
  newFile = os.path.join( target, inFile.replace(key, moduleName) )
  print ("creating %s" % newFile)
  path = os.path.dirname(newFile)
  if not os.path.exists(path):
      os.makedirs(path)

  fp = open(os.path.join(template,inFile))
  contents = fp.read()
  fp.close()
  contents = contents.replace(key, moduleName)
  contents = contents.replace(key.upper(), moduleName.upper())
  fp = open(newFile, "w")
  fp.write(contents)
  fp.close()


def usage():
  print ("")
  print ("Usage:")
  print ("ModuleWizard [--template <dir>] [--templateKey <key>] [--target <dir>] <moduleName>")
  print ("  --template default ./Extensions/Testing/LoadableExtensionTemplate")
  print ("  --templateKey default is dirname of template")
  print ("  --target default ./Modules/Loadable/<moduleName>")
  print ("Examples (from Slicer source directory):")
  print ("  ./Utilities/Scripts/ModuleWizard.py --template ./Extensions/Testing/LoadableExtensionTemplate --target ../MyExtension MyExtension")
  print ("  ./Utilities/Scripts/ModuleWizard.py --template ./Extensions/Testing/ScriptedLoadableExtensionTemplate --target ../MyScript MyScript")
  print ("  ./Utilities/Scripts/ModuleWizard.py --template ./Extensions/Testing/EditorExtensionTemplate --target ../MyEditorEffect MyEditorEffect")
  print ("  ./Utilities/Scripts/ModuleWizard.py --template ./Extensions/Testing/CLIExtensionTemplate --target ../MyCLI MyCLI")
  print ("  ./Utilities/Scripts/ModuleWizard.py --template ./Extensions/Testing/SuperBuildExtensionTemplate --target ../MySuperBuild MySuperBuild")
  print ("")

def main(argv):

  template = ""
  templateKey = ""
  target = ""
  moduleName = ""

  while argv != []:
    arg = argv.pop(0)
    if arg == "--template":
      template = argv.pop(0)
      continue
    if arg == "--templateKey":
      templateKey = argv.pop(0)
      continue
    if arg == "--target":
      target = argv.pop(0)
      continue
    if arg == "--help":
      usage()
      exit()
    moduleName = arg

  if moduleName == "":
    print ("Please specify module name")
    usage()
    exit()

  if template == "":
    template = "Extensions/Testing/LoadableExtensionTemplate/"
  if template[-1] != '/':
    template += '/'

  if templateKey == "":
    templateKey = os.path.split(template[:-1])[-1]

  if target == "":
    target = "Modules/Loadable/" + moduleName

  if os.path.exists(target):
    print((target, "exists - delete it first"))
    exit()

  if not os.path.exists(template):
    print((template, "does not exist - run from Slicer source dir or specify with --template"))
    usage()
    exit()

  print ("\nWill copy \n\t%s \nto \n\t%s \nreplacing \"%s\" with \"%s\"\n" % (template, target, templateKey, moduleName))
  sources = findSource( template )
  print (sources)

  for file in sources:
    copyAndReplace(file, template, target, templateKey, moduleName)


  print ('\nModule %s created!' % moduleName)

if __name__ == "__main__":
  main(sys.argv[1:])
