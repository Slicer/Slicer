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

import os
import pipes
import subprocess
import time

__all__ = ['EXIT_FAILURE', 'EXIT_SUCCESS', 'run', 'runSlicer', 'runSlicerAndExit', 'timecall']

EXIT_FAILURE=1
EXIT_SUCCESS=0


def run(executable, arguments=[], verbose=False):
  """Run ``executable`` with provided ``arguments``.
  """
  print("%s %s" % (os.path.basename(executable), " ".join([pipes.quote(arg) for arg in arguments])))
  arguments.insert(0, executable)
  p = subprocess.Popen(args=arguments, stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
  stdout, stderr = p.communicate()

  if verbose:
    print('STDOUT: ' + stdout)

  if p.returncode != EXIT_SUCCESS:
    if not verbose:
      print('STDOUT: ' + stdout)
    print('STDERR: ' + stderr)
    verbose = True

  return p.returncode

def runSlicer(slicer_executable, arguments=[], verbose=False):
  """Run ``slicer_executable`` with provided ``arguments``.
  """
  args = ['--no-splash']
  args.extend(arguments)
  return run(slicer_executable, args, verbose)

def runSlicerAndExit(slicer_executable, arguments=[], verbose=False):
  """Run ``slicer_executable`` with provided ``arguments`` and exit.
  """
  arguments.insert(0, '--exit-after-startup')
  return runSlicer(slicer_executable, arguments, verbose)

def timecall(method):
  """Wrap ``method`` and print its execution time.
  """
  def wrapper(*args, **kwargs):
    try:
      start = time.time()
      result = method(*args, **kwargs)
      return result
    finally:
      print("{:.3f} seconds".format(time.time() - start))
      print("")
  return wrapper
