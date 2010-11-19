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

// Qt includes
#include <QStringList>

// SlicerQt includes
#include "qSlicerCLIExecutableModuleFactory.h"
#include "qSlicerCLIModule.h"
#include "qSlicerCLIModuleFactoryHelper.h"

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactoryItem::qSlicerCLIExecutableModuleFactoryItem(
  const QString& itemPath):Superclass(),Path(itemPath)
{
}

//-----------------------------------------------------------------------------
bool qSlicerCLIExecutableModuleFactoryItem::load()
{
  return false;
}

//-----------------------------------------------------------------------------
QString qSlicerCLIExecutableModuleFactoryItem::path()const
{
  return this->Path;
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerCLIExecutableModuleFactoryItem::instanciator()
{
  qDebug() << "CmdLineExecutableModuleItem::instantiate - name:" << this->path();
  return 0;
}

//-----------------------------------------------------------------------------
class qSlicerCLIExecutableModuleFactoryPrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactory::qSlicerCLIExecutableModuleFactory()
  : Superclass()
  , d_ptr(new qSlicerCLIExecutableModuleFactoryPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactory::~qSlicerCLIExecutableModuleFactory()
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIExecutableModuleFactory::registerItems()
{
  
}

//-----------------------------------------------------------------------------
ctkAbstractFactoryItem<qSlicerAbstractCoreModule>* qSlicerCLIExecutableModuleFactory
::createFactoryPluginItem(const QFileInfo& plugin)
{
  return new qSlicerCLIExecutableModuleFactoryItem(plugin.filePath());
}

//-----------------------------------------------------------------------------
// QString qSlicerCLIExecutableModuleFactory::objectNameToKey(const QString& objectName)
// {
//   return Self::extractModuleName(objectName);
// }

//-----------------------------------------------------------------------------
QString qSlicerCLIExecutableModuleFactory::extractModuleName(const QString& executableName)
{
  QString moduleName = executableName;

  // Remove extension if needed
  int index = moduleName.indexOf(".");
  if (index > 0)
    {
    moduleName.truncate(index);
    }

  return moduleName.toLower();
}
