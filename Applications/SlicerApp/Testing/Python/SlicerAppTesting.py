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

#
#  W A R N I N G
#  -------------
#
# This file is not part of the Slicer API.  It exists purely as an
# implementation detail.  This python module may change from version to
# version without notice, or even be removed.
#
# We mean it.
#

from __future__ import print_function

import os
import pipes
import subprocess
import sys
import time

__all__ = ['EXIT_FAILURE', 'EXIT_SUCCESS', 'run', 'runSlicer', 'runSlicerAndExit', 'timecall']

EXIT_FAILURE=1
EXIT_SUCCESS=0


def run(executable, arguments=[], verbose=True, shell=False):
  """Run ``executable`` with provided ``arguments``.
  """
  if verbose:
    print("%s %s" % (os.path.basename(executable), " ".join([pipes.quote(arg) for arg in arguments])))
  arguments.insert(0, executable)
  if shell:
    arguments = " ".join([pipes.quote(arg) for arg in arguments])
  p = subprocess.Popen(args=arguments, stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE, shell=shell)
  stdout, stderr = p.communicate()

  if p.returncode != EXIT_SUCCESS:
    print('STDERR: ' + stderr, file=sys.stderr)

  return (p.returncode, stdout, stderr)

def runSlicer(slicer_executable, arguments=[], verbose=True):
  """Run ``slicer_executable`` with provided ``arguments``.
  """
  args = ['--no-splash']
  args.extend(arguments)
  return run(slicer_executable, args, verbose)

def runSlicerAndExit(slicer_executable, arguments=[], verbose=True):
  """Run ``slicer_executable`` with provided ``arguments`` and exit.
  """
  arguments.insert(0, '--exit-after-startup')
  return runSlicer(slicer_executable, arguments, verbose)

def timecall(method):
  """Wrap ``method`` and return its execution time.
  """
  def wrapper(*args, **kwargs):
    start = time.time()
    result = method(*args, **kwargs)
    duration = time.time() - start
    return (duration, result)
  return wrapper
