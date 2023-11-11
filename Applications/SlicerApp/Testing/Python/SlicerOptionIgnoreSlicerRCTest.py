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

from SlicerAppTesting import *

"""
Usage:
    SlicerOptionIgnoreSlicerRCTest.py /path/to/Slicer
"""

if __name__ == "__main__":
    debug = False
    # Set to True to:
    #  * display the path of the created slicerrc file
    #  * avoid deleting the slicerrc file

    if len(sys.argv) != 2:
        print(os.path.basename(sys.argv[0]) + " /path/to/Slicer")
        exit(EXIT_FAILURE)

    slicer_executable = os.path.expanduser(sys.argv[1])
    common_args = ["--disable-modules", "--no-main-window"]

    fd, slicerrc = tempfile.mkstemp()
    assert os.path.isfile(slicerrc)
    try:
        # Create a slicerrc file that creates SLICERRCTESTOUTPUT file when executed
        with os.fdopen(fd, "w") as file:
            file.write("# Generated by SlicerOptionIgnoreSlicerRCTest.py\n" +\
                       "import os\n" +\
                       "fd = os.open(os.environ['SLICERRCTESTOUTPUT'], os.O_RDWR|os.O_CREAT)\n" +\
                       "os.write(fd, 'Generated by SlicerOptionIgnoreSlicerRCTest.py test when slicerrc executed'.encode())\n" +\
                       "os.close(fd)\n")

        slicerrctestoutput = slicerrc + ".out"
        os.environ["SLICERRC"] = slicerrc
        os.environ["SLICERRCTESTOUTPUT"] = slicerrctestoutput
        if debug:
            print("SLICERRC=%s" % slicerrc)
            print("SLICERRCTESTOUTPUT=%s" % slicerrctestoutput)

        # Check that slicerrc file is loaded
        args = list(common_args)
        (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, args)
        assert os.path.isfile(slicerrctestoutput)
        os.remove(slicerrctestoutput)
        assert returnCode == EXIT_SUCCESS
        print("=> ok\n")

        # Check that --ignore-slicerrc works
        args = list(common_args)
        args.extend(["--ignore-slicerrc"])
        (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, args)
        assert not os.path.isfile(slicerrctestoutput)
        assert returnCode == EXIT_SUCCESS
        print("=> ok\n")

        # Check that --testing implies --ignore-slicerrc
        args = list(common_args)
        args.extend(["--testing"])
        (returnCode, stdout, stderr) = runSlicerAndExit(slicer_executable, args)
        assert not os.path.isfile(slicerrctestoutput)
        assert returnCode == EXIT_SUCCESS
        print("=> ok\n")
    finally:
        if not debug:
            os.remove(slicerrc)
            if os.path.isfile(slicerrctestoutput):
                os.remove(slicerrctestoutput)
