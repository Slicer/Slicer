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

#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT

// Qt includes
#include <QDebug>
#ifdef Slicer_USE_PYTHONQT
# include <QDir>
# include <QVariant>
#endif

// CTK includes
#ifdef Slicer_USE_PYTHONQT
# include <ctkScopedCurrentDir.h>
#endif

// SlicerQt includes
#include "qSlicerLoadableModule.h"
#ifdef Slicer_USE_PYTHONQT
# include "qSlicerCoreApplication.h"
# include "qSlicerCorePythonManager.h"
#endif

//-----------------------------------------------------------------------------
class qSlicerLoadableModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerLoadableModule::qSlicerLoadableModule(QObject* _parentObject)
  : Superclass(_parentObject)
  , d_ptr(new qSlicerLoadableModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerLoadableModule::~qSlicerLoadableModule()
{
}

//-----------------------------------------------------------------------------
bool qSlicerLoadableModule::importModulePythonExtensions(
    qSlicerCorePythonManager * pythonManager, const QString& intDir, const QString& modulePath)
{
#ifdef Slicer_USE_PYTHONQT
  if(!pythonManager)
    {
    return false;
    }
  QDir modulePathWithoutIntDir = QFileInfo(modulePath).dir();
  if (intDir ==  modulePathWithoutIntDir.dirName())
    {
    modulePathWithoutIntDir.cdUp();
    }
  QString pythonPath = modulePathWithoutIntDir.filePath("Python");
  QStringList paths; paths << QFileInfo(modulePath).dir().absolutePath() << pythonPath;
  pythonManager->appendPythonPaths(paths);
  // Update current application directory, so that *PythonD modules can be loaded
  ctkScopedCurrentDir scopedCurrentDir(modulePathWithoutIntDir.absolutePath());
  pythonManager->executeString(QString(
        "from slicer.util import importVTKClassesFromDirectory;"
        "importVTKClassesFromDirectory('%1', 'slicer.modulelogic', filematch='vtkSlicer*ModuleLogic.py');"
        "importVTKClassesFromDirectory('%1', 'slicer.modulemrml', filematch='vtkSlicer*ModuleMRML.py');"
        ).arg(pythonPath));
  pythonManager->executeString(QString(
        "from slicer.util import importQtClassesFromDirectory;"
        "importQtClassesFromDirectory('%1', 'slicer.modulewidget', filematch='qSlicer*PythonQt.*');"
        ).arg(modulePathWithoutIntDir.absolutePath()));
  return !pythonManager->pythonErrorOccured();
#else
  Q_UNUSED(pythonManager);
  Q_UNUSED(indDir);
  Q_UNUSED(modulePath);
  return false;
#endif
}

//-----------------------------------------------------------------------------
void qSlicerLoadableModule::setup()
{
#ifndef QT_NO_DEBUG
  Q_D(qSlicerLoadableModule);
#endif
  Q_ASSERT(d != 0);

#ifdef Slicer_USE_PYTHONQT
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  if (app && !app->isEmbeddedModule(this->path()))
    {
    // By convention, if the module is not embedded,
    // "<MODULEPATH>/Python" will be appended to PYTHONPATH
    Self::importModulePythonExtensions(app->corePythonManager(), app->intDir(), this->path());
    }
#endif
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModule::helpText()const
{
  qDebug() << "WARNING: " << this->metaObject()->className()
           << "::helpText() is not implemented";
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModule::acknowledgementText()const
{
  qDebug() << "WARNING: " << this->metaObject()->className()
           << "::acknowledgementText - Not implemented";
  return QString();
}
