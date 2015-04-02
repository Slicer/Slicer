/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qSlicerScriptedUtils_p.h"

// PythonQt includes
#include <PythonQt.h>

//-----------------------------------------------------------------------------
bool qSlicerScriptedUtils::executeFile(const QString& fileName, PyObject * global_dict, const QString& className)
{
  if (!className.isEmpty())
    {
    PyDict_SetItemString(global_dict, "__name__", PyString_FromString(className.toLatin1()));
    }

  PyObject* pyRes = 0;
  if (fileName.endsWith(".py"))
    {
    pyRes = PyRun_String(QString("execfile('%1')").arg(fileName).toLatin1(),
                         Py_file_input, global_dict, global_dict);
    }
  else if (fileName.endsWith(".pyc"))
    {
    pyRes = PyRun_String(
          QString("with open('%1', 'rb') as f:import imp;imp.load_module('__main__', f, '%1', ('.pyc', 'rb', 2))").arg(fileName).toLatin1(),
          Py_file_input, global_dict, global_dict);
    }
  if (!pyRes)
    {
    PythonQt::self()->handleError();
    qCritical() << "setPythonSource - Failed to execute file" << fileName << "!";
    return false;
    }
  Py_DECREF(pyRes);
  if (!className.isEmpty())
    {
    PyDict_SetItemString(global_dict, "__name__", PyString_FromString("__main__"));
    }
  return true;
}
