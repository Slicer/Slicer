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

// Qt includes
#include <QBitArray>
#include <QSettings>

// CTK includes
#include <ctkVTKPythonQtWrapperFactory.h>

// PythonQt includes
#include <PythonQt.h>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCorePythonManager.h"
#include "qSlicerScriptedUtils_p.h"

// VTK includes
#include <vtkPythonUtil.h>
#include <vtkVersion.h>

//-----------------------------------------------------------------------------
qSlicerCorePythonManager::qSlicerCorePythonManager(QObject* _parent)
  : Superclass(_parent)
{
  this->Factory = nullptr;

  // https://docs.python.org/3/c-api/init_config.html#init-config
  PyStatus status;
  PyConfig config;
  PyConfig_InitPythonConfig(&config);

  config.parse_argv = 0;

  // If it applies, disable import of user site packages
  const QString noUserSite = qgetenv("PYTHONNOUSERSITE");
  config.user_site_directory = noUserSite.toInt(); // disable user site packages

  status = PyConfig_SetString(&config, &config.program_name, L"Slicer");
  if (PyStatus_Exception(status))
  {
    PyConfig_Clear(&config);
    Py_ExitStatusException(status);
  }

  status = Py_InitializeFromConfig(&config);
  if (PyStatus_Exception(status))
  {
    PyConfig_Clear(&config);
    Py_ExitStatusException(status);
  }

  PyConfig_Clear(&config);

  int flags = this->initializationFlags();
  flags |= PythonQt::PythonAlreadyInitialized; // Set bit
  this->setInitializationFlags(flags);
}

//-----------------------------------------------------------------------------
qSlicerCorePythonManager::~qSlicerCorePythonManager()
{
  if (this->Factory)
  {
    delete this->Factory;
    this->Factory = nullptr;
  }
}

//-----------------------------------------------------------------------------
QStringList qSlicerCorePythonManager::pythonPaths()
{
  return Superclass::pythonPaths();
}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::preInitialization()
{
  Superclass::preInitialization();
  this->Factory = new ctkVTKPythonQtWrapperFactory;
  this->addWrapperFactory(this->Factory);
  qSlicerCoreApplication* const app = qSlicerCoreApplication::application();
  if (app)
  {
    // Add object to python interpreter context
    this->addObjectToPythonMain("_qSlicerCoreApplicationInstance", app);
  }
}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::addVTKObjectToPythonMain(const QString& name, vtkObject* object)
{
  // Split name using '.'
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  QStringList moduleNameList = name.split('.', Qt::SkipEmptyParts);
#else
  QStringList moduleNameList = name.split('.', QString::SkipEmptyParts);
#endif

  // Remove the last part
  const QString attributeName = moduleNameList.takeLast();

  const bool success = qSlicerScriptedUtils::setModuleAttribute(moduleNameList.join("."), attributeName, vtkPythonUtil::GetObjectFromPointer(object));
  if (!success)
  {
    qCritical() << "qSlicerCorePythonManager::addVTKObjectToPythonMain - "
                   "Failed to add VTK object:"
                << name;
  }
}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::appendPythonPath(const QString& path)
{
  // TODO Make sure PYTHONPATH is updated
  this->executeString(QString("import sys, os\n"
                              "___path = os.path.abspath(%1)\n"
                              "if ___path not in sys.path:\n"
                              "  sys.path.append(___path)\n"
                              "del sys, os")
                        .arg(qSlicerCorePythonManager::toPythonStringLiteral(path)));
}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::appendPythonPaths(const QStringList& paths)
{
  for (const QString& path : paths)
  {
    this->appendPythonPath(path);
  }
}

//-----------------------------------------------------------------------------
QString qSlicerCorePythonManager::toPythonStringLiteral(QString path)
{
  return ctkAbstractPythonManager::toPythonStringLiteral(path);
}
