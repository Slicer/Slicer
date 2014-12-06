""" This module contains functions for writing info or error messages to the application log."""

from __future__ import print_function
import sys
from __main__ import qt, slicer

__all__ = ['info', 'error']

# Define an object (__m) to store all module global variables
class ModuleGlobals(object): pass
__m = ModuleGlobals()
__m.errorPopup = None

def info(logMessage, showMessageBox=False, autoCloseMsec=1000):
  """Write info message to the application log
  If showMessageBox is True then this method displays a small dialog and
  waits until the user clicks on OK button or after autoCloseMsec millisecond
  elapses.
  This does two things: 1) it lets the event loop catch up
  to the state of the test so that rendering and widget updates
  have all taken place before the test continues and 2) it
  shows the user/developer/tester the state of the test
  so that we'll know when it breaks.
  """
  print(logMessage)
  if showMessageBox:
    __m.errorPopup = qt.QDialog()
    layout = qt.QVBoxLayout()
    __m.errorPopup.setLayout(layout)
    label = qt.QLabel(logMessage,__m.errorPopup)
    layout.addWidget(label)
    if autoCloseMsec>0:
      qt.QTimer.singleShot(autoCloseMsec, __m.errorPopup.close)
    __m.errorPopup.exec_()

def error(logMessage, showMessageBox=False):
  """Write error message to the application log
  If showMessageBox is True then this method displays a small dialog box and waits
  until the user clicks on OK button.
  """
  print(logMessage, file=sys.stderr)
  if showMessageBox:
    qt.QMessageBox.warning(slicer.util.mainWindow(), "Slicer module error", logMessage)
