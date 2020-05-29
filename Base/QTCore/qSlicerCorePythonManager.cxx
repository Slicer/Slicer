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
#include "qSlicerUtils.h"
#include "qSlicerCorePythonManager.h"
#include "qSlicerScriptedUtils_p.h"
#include "vtkSlicerConfigure.h"

// VTK includes
#include <vtkPythonUtil.h>
#include <vtkVersion.h>

//-----------------------------------------------------------------------------
qSlicerCorePythonManager::qSlicerCorePythonManager(QObject* _parent)
  : Superclass(_parent)
{
  this->Factory = nullptr;

  // If it applies, disable import of user site packages
  QString noUserSite = qgetenv("PYTHONNOUSERSITE");
  Py_NoUserSiteDirectory = noUserSite.toInt();

  // Import site module to ensure the 'site-packages' directory
  // is added to the python path. (see site.addsitepackages function).
  int flags = this->initializationFlags();
  flags &= ~(PythonQt::IgnoreSiteModule); // Clear bit
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
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  if (app)
    {
    // Add object to python interpreter context
    this->addObjectToPythonMain("_qSlicerCoreApplicationInstance", app);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::addVTKObjectToPythonMain(const QString& name, vtkObject * object)
{
  // Split name using '.'
  QStringList moduleNameList = name.split('.', QString::SkipEmptyParts);

  // Remove the last part
  QString attributeName = moduleNameList.takeLast();

  bool success = qSlicerScriptedUtils::setModuleAttribute(
        moduleNameList.join("."),
        attributeName,
        vtkPythonUtil::GetObjectFromPointer(object));
  if (!success)
    {
    qCritical() << "qSlicerCorePythonManager::addVTKObjectToPythonMain - "
                   "Failed to add VTK object:" << name;
    }
}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::appendPythonPath(const QString& path)
{
  // TODO Make sure PYTHONPATH is updated
  this->executeString(QString("import sys; sys.path.append(%1); del sys").arg(qSlicerCorePythonManager::toPythonStringLiteral(path)));
}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::appendPythonPaths(const QStringList& paths)
{
  foreach(const QString& path, paths)
    {
    this->appendPythonPath(path);
    }
}

//-----------------------------------------------------------------------------
QString qSlicerCorePythonManager::toPythonStringLiteral(QString path)
{
  path = path.replace("\\", "\\\\");
  path = path.replace("'", "\\'");
  // since we enclose string in single quotes, double-quotes do not require escaping
  return "'" + path + "'";
}
