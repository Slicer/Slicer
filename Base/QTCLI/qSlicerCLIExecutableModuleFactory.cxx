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
#include <QDebug>
#include <QProcess>
#include <QStringList>

// SlicerQt includes
#include "qSlicerCLIExecutableModuleFactory.h"
#include "qSlicerCLIModule.h"
#include "qSlicerCLIModuleFactoryHelper.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreCommandOptions.h"
#include "qSlicerUtils.h"

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactoryItem::qSlicerCLIExecutableModuleFactoryItem(
  const QString& path):Superclass(path)
{
}

//-----------------------------------------------------------------------------
bool qSlicerCLIExecutableModuleFactoryItem::load()
{
  return true;
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerCLIExecutableModuleFactoryItem::instanciator()
{
  // Using a scoped pointer ensures the memory will be cleaned if instanciator
  // fails before returning the module. See QScopedPointer::take()
  QScopedPointer<qSlicerCLIModule> module(new qSlicerCLIModule());
  module->setModuleType("CommandLineModule");
  module->setEntryPoint(this->path());

  QProcess cli;
  cli.start(this->path(), QStringList(QString("--xml")));
  bool res = cli.waitForFinished(5000);
  if (!res)
    {
    return 0;
    }
  QString xmlDescription = cli.readAllStandardOutput();
  if (xmlDescription.isEmpty())
    {
    if (this->verbose())
      {
      qWarning() << "Failed to retrieve Xml Description - Path:" << this->path();
      }
    return 0;
    }
  if (!xmlDescription.startsWith("<?xml"))
    {
    qWarning() << "For command line executable: " << this->path();
    qWarning() << "XML description doesn't start right away."
               << "There is extra output before the XML is print.";
    xmlDescription.remove(0, xmlDescription.indexOf("<?xml"));
    }

  module->setXmlModuleDescription(xmlDescription.toLatin1());
  module->setTempDirectory(
    qSlicerCoreApplication::application()->coreCommandOptions()->tempDirectory());

  return module.take();
}

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactory::qSlicerCLIExecutableModuleFactory()
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIExecutableModuleFactory::registerItems()
{
  this->registerAllFileItems(qSlicerCLIModuleFactoryHelper::modulePaths());
}

//-----------------------------------------------------------------------------
ctkAbstractFactoryItem<qSlicerAbstractCoreModule>* qSlicerCLIExecutableModuleFactory
::createFactoryFileBasedItem(const QFileInfo& file)
{
  if (!file.isExecutable() ||
      !qSlicerUtils::isCLIExecutable(file.absoluteFilePath()))
    {
    return 0;
    }
  return new qSlicerCLIExecutableModuleFactoryItem(file.filePath());
}

//-----------------------------------------------------------------------------
QString qSlicerCLIExecutableModuleFactory::fileNameToKey(const QString& objectName)const
{
  return qSlicerCLIExecutableModuleFactory::extractModuleName(objectName);
}

//-----------------------------------------------------------------------------
QString qSlicerCLIExecutableModuleFactory::extractModuleName(const QString& executableName)
{
  return qSlicerUtils::extractModuleNameFromLibraryName(executableName);
}
