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
import shutil
import sys
import tempfile

from SlicerAppTesting import *

"""
Usage:
    SlicerOptionModulesToIgnoreTest.py /path/to/Slicer
"""

if __name__ == '__main__':

  debug = False # Set to True to:
                #  * display the path of the created extension_dir
                #  * avoid deleting the extension_dir

  if len(sys.argv) != 3:
    print(os.path.basename(sys.argv[0]) +" /path/to/Slicer /path/to/slicerExtensionWizard")
    exit(EXIT_FAILURE)

  slicer_executable = os.path.expanduser(sys.argv[1])
  slicer_extension_wizard = os.path.expanduser(sys.argv[2])

  common_args = ['--testing', '--disable-builtin-modules', '--no-main-window']

  # Prerequisites:
  #  * create temporary extension directory
  #  * create an extension with four scripted modules: A, B, C and D

  extension_dir = tempfile.mkdtemp()
  assert os.path.isdir(extension_dir)
  if debug:
    print("extension_dir: %s" % extension_dir)

  moduleNames = ['A', 'B', 'C', 'D']

  additional_module_paths = ['--additional-module-paths']
  additional_module_paths.extend(['%s/Test/%s' % (extension_dir, moduleName) for moduleName in moduleNames])

  args = ['--create', 'Test']
  for moduleName in moduleNames:
    args.extend(['--addModule', 'scripted:%s' % moduleName])
  args.extend([extension_dir])
  (returnCode, stdout, stderr) = run(slicer_extension_wizard, args, shell=True)
  assert returnCode == EXIT_SUCCESS
  print("=> ok\n")

  try:

    # Check that the modules are loaded
    args = list(common_args)
    args.extend(['--python-code', 'assert len([module for module in dir(slicer.moduleNames) if module in ["A", "B", "C", "D"]]) == 4, "Failed to load modules"'])
    args.extend(additional_module_paths)
    (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, args)
    assert returnCode == EXIT_SUCCESS
    print("=> ok\n")

    # Update settings disabling module B
    args = list(common_args)
    args.extend(['--keep-temporary-settings'])
    args.extend(['--python-code', 'slicer.app.moduleManager().factoryManager().modulesToIgnore = ["B"]'])
    args.extend(additional_module_paths)
    (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, args)
    assert returnCode == EXIT_SUCCESS
    print("=> ok\n")

    # Run application to check that module B is disabled
    args = list(common_args)
    args.extend(['--keep-temporary-settings'])
    args.extend(['--python-code', 'assert len([module for module in dir(slicer.moduleNames) if module in ["A", "C", "D"]]) == 3, "Failed to disable modules"'])
    args.extend(additional_module_paths)
    (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, args)
    assert returnCode == EXIT_SUCCESS
    print("=> ok\n")

    # Run application given "--modules-to-ignore A,D" and check that A, D and B are disabled
    args = list(common_args)
    args.extend(['--keep-temporary-settings'])
    args.extend(['--modules-to-ignore', 'A,D'])
    args.extend(['--python-code', 'assert len([module for module in dir(slicer.moduleNames) if module in ["C"]]) == 1, "Failed to disable modules"'])
    args.extend(additional_module_paths)
    (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, args)
    assert returnCode == EXIT_SUCCESS
    print("=> ok\n")

    # Run application to check that only B remains disabled
    args = list(common_args)
    args.extend(['--keep-temporary-settings'])
    args.extend(['--python-code', 'assert len([module for module in dir(slicer.moduleNames) if module in ["A", "C", "D"]]) == 3, "Failed to disable modules"'])
    args.extend(additional_module_paths)
    (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, args)
    assert returnCode == EXIT_SUCCESS
    print("=> ok\n")

  finally:
    if not debug:
      shutil.rmtree(extension_dir)
