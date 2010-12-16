/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// CTK includes
#include <ctkLogger.h>

// PythonQt includes
#include <PythonQt.h>

// SlicerQt includes
#include "qSlicerUtils.h"
#include "qSlicerCorePythonManager.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerBaseQTCorePythonQtDecorators.h"

// VTK includes
#include <vtkPythonUtil.h>

#include "vtkSlicerConfigure.h" // For VTK_DIR, CTK_DIR

// PythonQt wrapper initialization methods
void PythonQt_init_org_commontk_CTKCore(PyObject*);
void PythonQt_init_org_commontk_CTKScriptingPythonCore(PyObject*);
void PythonQt_init_org_commontk_CTKVisualizationVTKCore(PyObject*);

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtcore.qSlicerCorePythonManager");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
qSlicerCorePythonManager::qSlicerCorePythonManager(QObject* _parent) : Superclass(_parent)
{

}

//-----------------------------------------------------------------------------
qSlicerCorePythonManager::~qSlicerCorePythonManager()
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerCorePythonManager::pythonPaths()
{
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  
  QStringList paths;  
  paths << Superclass::pythonPaths();
  paths << app->slicerHome() + "/bin/" + app->intDir();
  paths << app->slicerHome() + "/bin/Python";

  QString executableExtension = qSlicerUtils::executableExtension();
  if (!app->isInstalled())
    {
    // Add here python path specific to the BUILD tree
    paths << qSlicerUtils::searchTargetInIntDir(QLatin1String(VTK_DIR)+"/bin",
                                                QString("vtkWrapPython%1").arg(executableExtension));
    paths << QString("%1/Wrapping/Python").arg(VTK_DIR);

    paths << QString("%1/CTK-build/bin/Python").arg(CTK_DIR);
    }
  else
    {
    // Add here python path specific to the INSTALLED tree
    }
  
  return paths; 
}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::preInitialization()
{
  Superclass::preInitialization();

  // Initialize wrappers
  PythonQt_init_org_commontk_CTKCore(0);
  PythonQt_init_org_commontk_CTKScriptingPythonCore(0);
  PythonQt_init_org_commontk_CTKVisualizationVTKCore(0);

  // Register decorators
  this->registerPythonQtDecorator(new qSlicerBaseQTBasePythonQtDecorators(this));

  qSlicerCoreApplication* app = qSlicerCoreApplication::application();

  // Add object to python interpreter context
  this->addObjectToPythonMain("_qSlicerCoreApplicationInstance", app);

}

//-----------------------------------------------------------------------------
void qSlicerCorePythonManager::addVTKObjectToPythonMain(const QString& name, vtkObject * object)
{
  if (name.isNull() || !object)
    {
    return;
    }
  // Split name using '.'
  QStringList moduleNameList = name.split('.', QString::SkipEmptyParts);

  // Remove the last part
  QString varName = moduleNameList.takeLast();

  PyObject * module = PythonQt::self()->getMainModule();

  // Loop over module name and try to import them one by one
  foreach(const QString& moduleName, moduleNameList)
    {
    module = PyImport_ImportModule(moduleName.toLatin1());
    Q_ASSERT(module);
    }

  // Add the object to the imported module
  int ret = PyModule_AddObject(module, varName.toLatin1(),
                               vtkPythonUtil::GetObjectFromPointer(object));
  Q_ASSERT(ret == 0);
  if (ret != 0)
    {
    logger.error(QString("Failed to add VTK object: %1").arg(name));
    }
}
