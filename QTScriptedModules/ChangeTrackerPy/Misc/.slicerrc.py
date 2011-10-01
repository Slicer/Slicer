"""Helper functions for developing - see bottom for key bindings"""

def tracker():
  print "SlicerRC - tracker setup..."
  import imp, sys
  p = '/home/pieper/hacks/itrack'
  if not sys.path.__contains__(p):
    sys.path.insert(0,p)

  mod = "itrack"
  sourceFile = p + "/itrack.py"
  fp = open(sourceFile, "r")
  globals()[mod] = imp.load_module(mod, fp, sourceFile, ('.py', 'r', imp.PY_SOURCE))
  fp.close()

  globals()['i'] = i = globals()[mod].itrack()
  i.capture()

def changetracker():
  print "SlicerRC - changetracker setup..."
  import imp, sys, os
  ctPath = '%s/../../Slicer4/QTScriptedModules/ChangeTrackerPy/' % slicer.app.slicerHome
  if not sys.path.__contains__(ctPath):
    sys.path.insert(0,ctPath)

  modules = ("Helper", "ChangeTrackerStep", "ChangeTrackerSelectScansStep", "ChangeTrackerDefineROIStep", "ChangeTrackerSegmentROIStep", "ChangeTrackerAnalyzeROIStep","ChangeTrackerReportROIStep")
  for mod in modules:
    sourceFile = ctPath + "/ChangeTrackerWizard/" + mod + ".py"
    fp = open(sourceFile, "r")
    globals()[mod] = imp.load_module(mod, fp, sourceFile, ('.py', 'r', imp.PY_SOURCE))
    fp.close()
    exec("globals()['ChangeTrackerWizard'].%s = globals()['%s'].%s" % (mod,mod,mod))

  mod = "ChangeTracker"
  sourceFile = ctPath + "/ChangeTracker.py"
  fp = open(sourceFile, "r")
  globals()[mod] = imp.load_module(mod, fp, sourceFile, ('.py', 'r', imp.PY_SOURCE))
  fp.close()

  globals()['ct'] = e = globals()[mod].ChangeTrackerWidget()


def endoscopy():
  print "SlicerRC - endoscopy setup..."
  import imp, sys, os
  endoPath = '%s/../../Slicer4/QTScriptedModules/Scripts' % os.environ['Slicer_HOME']
  if not sys.path.__contains__(endoPath):
    sys.path.insert(0,endoPath)

  mod = "Endoscopy"
  sourceFile = endoPath + "/Endoscopy.py"
  fp = open(sourceFile, "r")
  globals()[mod] = imp.load_module(mod, fp, sourceFile, ('.py', 'r', imp.PY_SOURCE))
  fp.close()

  globals()['e'] = e = globals()[mod].EndoscopyWidget()

def editor():
  print "SlicerRC - editor setup..."
  import imp, sys, os
  import slicer
  tcl("set ::guipath $::env(Slicer_HOME)/../../Slicer4/Base/GUI/Tcl")
  tcl("if { [lsearch $::auto_path $::guipath] == -1 } { set ::auto_path [list $::env(Slicer_HOME)/../../Slicer4/Base/GUI/Tcl $::auto_path] } ")
  tcl("package forget SlicerBaseGUITcl")
  tcl("package require SlicerBaseGUITcl")
  tcl("EffectSWidget::RemoveAll")
  tcl("EffectSWidget::Add DrawEffect")

  if not getNodes().has_key('2006-spgr'):
    slicer.mrmlScene.SetURL('/home/pieper/data/edit/edit-small.mrml')
    slicer.mrmlScene.Connect()

  if 0 and not getNodes().has_key('CTA-cardio'):
    slicer.mrmlScene.SetURL('/home/pieper/data/edit/edit.mrml')
    slicer.mrmlScene.Connect()

  editorLibPath = '%s/../../Slicer4/QTScriptedModules/EditorLib' % os.environ['Slicer_HOME']
  if not sys.path.__contains__(editorLibPath):
    sys.path.insert(0, editorLibPath)
  editorPath = '%s/../../Slicer4/QTScriptedModules/Scripts' % os.environ['Slicer_HOME']
  if not sys.path.__contains__(editorPath):
    sys.path.insert(0,editorPath)


  modules = ("EditUtil", "EditColor", "EditOptions", "EditBox", "ColorBox", "HelperBox")
  for mod in modules:
    sourceFile = editorLibPath + "/" + mod + ".py"
    fp = open(sourceFile, "r")
    globals()[mod] = imp.load_module(mod, fp, sourceFile, ('.py', 'r', imp.PY_SOURCE))
    fp.close()
    exec("globals()['EditorLib'].%s = globals()['%s'].%s" % (mod,mod,mod))

  mod = "Editor"
  sourceFile = editorPath + "/Editor.py"
  fp = open(sourceFile, "r")
  globals()[mod] = imp.load_module(mod, fp, sourceFile, ('.py', 'r', imp.PY_SOURCE))
  fp.close()

  globals()['e'] = e = globals()[mod].EditorWidget()

def fileScan():
  print "SlicerRC - fileScan setup..."
  import imp, sys, os
  p = '%s/../../Slicer4/QTScriptedModules/Scripts' % os.environ['Slicer_HOME']
  if not sys.path.__contains__(p):
    sys.path.insert(0,p)


  mod = "FileScan"
  sourceFile = p + "/FileScan.py"
  fp = open(sourceFile, "r")
  globals()[mod] = imp.load_module(mod, fp, sourceFile, ('.py', 'r', imp.PY_SOURCE))
  fp.close()

  globals()['e'] = e = globals()[mod].FileScanWidget()

def performance():
  print "SlicerRC - performance setup..."
  import os
  execfile( slicer.app.slicerHome + "/../../Slicer4/Base/Testing/Performance.py")

  timeProbe()


def setupMacros():
  """Set up hot keys for various development scenarios"""
  
  import qt
  global tracker, endoscopy, editor, fileScan, performance, changetracker
  
  print "SlicerRC - Install custom keyboard shortcuts"
  
  macros = (
    ("Ctrl+0", loadSlicerRCFile),
    ("Ctrl+1", tracker),
    ("Ctrl+2", endoscopy),
    ("Ctrl+3", editor),
    ("Ctrl+4", fileScan),
    ("Ctrl+5", performance),
    ("Ctrl+6", changetracker)
    )
      
  for keys,f in macros:
    k = qt.QKeySequence(keys)
    s = qt.QShortcut(k,mainWindow())
    s.connect('activated()', f)
    s.connect('activatedAmbiguously()', f)
    print "SlicerRC - '%s' -> '%s'" % (keys, f.__name__)

# Install macros
if mainWindow(verbose=False): setupMacros()


# Display current time
from time import gmtime, strftime
print "Slicer RC file loaded [%s]" % strftime("%d/%m/%Y %H:%M:%S", gmtime())

# TODO: always show shell when using macros (doesn't work yet)
pythonShell()
