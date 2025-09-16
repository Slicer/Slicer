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

import os
import sys
import tempfile

from SlicerAppTesting import EXIT_SUCCESS, runSlicer

"""
This test verifies that a scripted module can modify `sys.path` during
import (module load) and that those modifications are preserved after
Slicer completes startup (both with and without the main window).

Rationale:
During startup, Slicer modules may extend `sys.path` to enable imports.
Updating argv using PyConfig followed by `_PyInterpreterState_SetConfig`
can replace `sys.path` from the config's module_search_paths. This test
ensures `sys.path` modifications performed during module discovery remain
in effect after startup completes.

This test was based on `SlicerStartupCompletedTest.py` and it relies on
`SlicerStartupPreserveSysPathTestHelperModule.py` and the `SlicerStartupPreserveSysPathTestHelperPackage`
Python package.

It uses an output file for communication because on Windows,
standard output is not always enabled.

Usage:
    SlicerStartupPreserveSysPathTest.py /path/to/Slicer
"""

if __name__ == "__main__":
    debug = False
    # Set to True to:
    #  * display the path of the expected test output file
    #  * avoid deleting the created temporary directory

    if len(sys.argv) != 2:
        print(os.path.basename(sys.argv[0]) + " /path/to/Slicer")
        exit(EXIT_FAILURE)

    temporaryModuleDirPath = tempfile.mkdtemp().replace("\\", "/") + "/lib/Slicer-X.Y/qt-scripted-modules"
    os.makedirs(temporaryModuleDirPath, exist_ok=True)
    try:
        # Copy helper module that:
        # (1) imports the helper package that updates sys.path at module load
        # (2) removes the output file if it already exists (the startup script recreates it)
        currentDirPath = os.path.dirname(__file__).replace("\\", "/")
        from shutil import copyfile, copytree
        copyfile(currentDirPath + "/SlicerStartupPreserveSysPathTestHelperModule.py",
                 temporaryModuleDirPath + "/SlicerStartupPreserveSysPathTestHelperModule.py")
        copytree(currentDirPath + "/SlicerStartupPreserveSysPathTestHelperPackage",
                 temporaryModuleDirPath + "/SlicerStartupPreserveSysPathTestHelperPackage")

        slicer_executable = os.path.expanduser(sys.argv[1])
        common_args = [
            "--testing",
            "--disable-builtin-cli-modules",
            "--disable-builtin-loadable-modules",
            "--disable-builtin-scripted-loadable-modules",
            "--additional-module-path", temporaryModuleDirPath,
            "--python-script", currentDirPath + "/SlicerStartupPreserveSysPathTestHelperScript.py",
        ]

        test_output_file = temporaryModuleDirPath + "/StartupModuleTest.out"
        os.environ["SLICER_STARTUP_MODULE_TEST_OUTPUT"] = test_output_file
        if debug:
            print("SLICER_STARTUP_MODULE_TEST_OUTPUT=%s" % test_output_file)

        # Test sys.path updates done while importing SlicerStartupPreserveSysPathTestHelperPackage
        # during module load persists with main window
        args = list(common_args)
        (returnCode, stdout, stderr) = runSlicer(slicer_executable, args)
        assert returnCode == EXIT_SUCCESS, stdout + "\n" + stderr
        assert os.path.isfile(test_output_file), "startup helper script did not produce the output file"
        os.remove(test_output_file)
        print("Test startupCompleted with main window - passed\n")

        # Test sys.path updates done while importing SlicerStartupPreserveSysPathTestHelperPackage
        # during module load persists without main window
        args = list(common_args)
        args.extend(["--no-main-window"])
        (returnCode, stdout, stderr) = runSlicer(slicer_executable, args)
        assert returnCode == EXIT_SUCCESS, stdout + "\n" + stderr
        assert os.path.isfile(test_output_file), "startup helper script did not produce the output file"
        print("Test startupCompleted without main window - passed\n")

    finally:
        if not debug:
            import shutil
            shutil.rmtree(temporaryModuleDirPath)
