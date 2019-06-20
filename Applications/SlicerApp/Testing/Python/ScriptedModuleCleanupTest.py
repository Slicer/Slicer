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

import argparse
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
    ScriptedModuleCleanupTest.py /path/to/Slicer [--with-testing]
"""


def check_exit_code(slicer_executable, testing_enabled=True, debug=False):
  """
  If debug is set to True:
    * display the path of the expected test output file
    * avoid deleting the created temporary directory
  """

  temporaryModuleDirPath = tempfile.mkdtemp().replace('\\','/')
  try:
    # Copy helper module that creates a file when startup completed event is received
    currentDirPath = os.path.dirname(os.path.abspath(__file__)).replace('\\','/')
    from shutil import copyfile
    copyfile(currentDirPath+'/ScriptedModuleCleanupTestHelperModule.py',
      temporaryModuleDirPath+'/ModuleCleanup.py')

    common_arguments = [
      '--no-splash',
      '--disable-builtin-modules',
      '--additional-module-path', temporaryModuleDirPath,
      '--python-code', 'slicer.util.selectModule("ModuleCleanup")'
    ]

    test_output_file = temporaryModuleDirPath + "/ModuleCleanupTest.out"
    os.environ['SLICER_MODULE_CLEANUP_TEST_OUTPUT'] = test_output_file
    if debug:
      print("SLICER_MODULE_CLEANUP_TEST_OUTPUT=%s" % test_output_file)

    # Test
    args = list(common_arguments)
    if testing_enabled:
      args.append('--testing')
    else:
      args.append('--exit-after-startup')
    (returnCode, stdout, stderr) = run(slicer_executable, args)

    assert(os.path.isfile(test_output_file))

    if testing_enabled:
      assert(returnCode == EXIT_FAILURE)
    else:
      assert(returnCode == EXIT_SUCCESS)

  finally:
    if not debug:
      import shutil
      shutil.rmtree(temporaryModuleDirPath)

if __name__ == "__main__":

  parser = argparse.ArgumentParser()
  parser.add_argument("/path/to/Slicer")
  parser.add_argument('--with-testing', dest='testing_enabled', action='store_true')
  args = parser.parse_args()

  slicer_executable = os.path.expanduser(getattr(args, "/path/to/Slicer"))
  check_exit_code(slicer_executable, testing_enabled=args.testing_enabled)
