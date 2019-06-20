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
#  and was partially funded by NIH grant 1R01EB021391
#

import os
import sys
import tempfile

from SlicerAppTesting import *

"""
This test verifies that an exception raised in scripted module widget cleanup
function leads to an failure exit code.

It uses an output file for communication because on Windows,
standard output is not always enabled.

Usage:
    ScriptedModuleCleanupTest.py /path/to/Slicer
"""

if __name__ == "__main__":

  if len(sys.argv) != 2:
      print(os.path.basename(sys.argv[0]) +" /path/to/Slicer")
      exit(EXIT_FAILURE)

  debug = False # Set to True to:
                #  * display the path of the expected test output file
                #  * avoid deleting the created temporary directory

  temporaryModuleDirPath = tempfile.mkdtemp().replace('\\','/')
  try:
    # Copy helper module that creates a file when startup completed event is received
    currentDirPath = os.path.dirname(os.path.abspath(__file__)).replace('\\','/')
    from shutil import copyfile
    copyfile(currentDirPath+'/ScriptedModuleCleanupTestHelperModule.py',
      temporaryModuleDirPath+'/ModuleCleanup.py')

    slicer_executable = os.path.expanduser(sys.argv[1])
    slicer_arguments = [
      '--no-splash',
      '--testing',
      '--disable-builtin-modules',
      '--additional-module-path', temporaryModuleDirPath,
      '--python-code', 'slicer.util.selectModule("ModuleCleanup")'
    ]

    test_output_file = temporaryModuleDirPath + "/ModuleCleanupTest.out"
    os.environ['SLICER_MODULE_CLEANUP_TEST_OUTPUT'] = test_output_file
    if debug:
      print("SLICER_MODULE_CLEANUP_TEST_OUTPUT=%s" % test_output_file)

    # Test
    (returnCode, stdout, stderr) = run(slicer_executable, slicer_arguments)
    assert(os.path.isfile(test_output_file))
    assert(returnCode == EXIT_FAILURE)

  finally:
    if not debug:
      import shutil
      shutil.rmtree(temporaryModuleDirPath)
