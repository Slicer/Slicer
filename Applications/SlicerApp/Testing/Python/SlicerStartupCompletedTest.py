#!/usr/bin/env python

#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 1U24CA194354-01
#

from __future__ import print_function
import os
import sys
import tempfile

from SlicerAppTesting import *

"""
This test verifies that startupCompleted() signal is emitted.

It uses an output file for communication because on Windows,
standard output is not always enabled.

Usage:
    SlicerOptionIgnoreSlicerRCTest.py /path/to/Slicer
"""

if __name__ == '__main__':

  debug = False # Set to True to:
                #  * display the path of the expected test output file
                #  * avoid deleting the created temporary directory

  if len(sys.argv) != 2:
    print(os.path.basename(sys.argv[0]) +" /path/to/Slicer")
    exit(EXIT_FAILURE)

  temporaryModuleDirPath = tempfile.mkdtemp().replace('\\','/')
  try:

    # Copy helper module that creates a file when startup completed event is received
    currentDirPath = os.path.dirname(os.path.abspath(__file__)).replace('\\','/')
    from shutil import copyfile
    copyfile(currentDirPath+'/SlicerStartupCompletedTestHelperModule.py',
      temporaryModuleDirPath+'/SlicerStartupCompletedTestHelperModule.py')
  
    slicer_executable = os.path.expanduser(sys.argv[1])
    common_args = [
      '--testing',
      '--no-splash',
      '--disable-builtin-cli-modules',
      '--disable-builtin-loadable-modules',
      '--disable-builtin-scripted-loadable-modules',
      '--additional-module-path', temporaryModuleDirPath,
      ]

    test_output_file = temporaryModuleDirPath + "/StartupCompletedTest.out"
    os.environ['SLICER_STARTUP_COMPLETED_TEST_OUTPUT'] = test_output_file
    if debug:
      print("SLICER_STARTUP_COMPLETED_TEST_OUTPUT=%s" % test_output_file)

    # Test startupCompleted with main window
    args = list(common_args)
    (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, args)
    assert(os.path.isfile(test_output_file))
    os.remove(test_output_file)
    assert(returnCode == EXIT_SUCCESS)
    print("Test startupCompleted with main window - passed\n")

    # Test startupCompleted without main window
    args = list(common_args)
    args.extend(['--no-main-window'])
    (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, args)
    assert(os.path.isfile(test_output_file))
    assert(returnCode == EXIT_SUCCESS)
    print("Test startupCompleted without main window - passed\n")

  finally:
    if not debug:
      import shutil
      shutil.rmtree(temporaryModuleDirPath)
