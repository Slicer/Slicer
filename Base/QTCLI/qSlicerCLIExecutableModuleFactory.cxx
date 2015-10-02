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
#include <QProcess>

// SlicerQt includes
#include "qSlicerCLIExecutableModuleFactory.h"
#include "qSlicerCLIModule.h"
#include "qSlicerCLIModuleFactoryHelper.h"
#include "qSlicerUtils.h"
#include <vtkSlicerCLIModuleLogic.h>

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactoryItem::qSlicerCLIExecutableModuleFactoryItem(
  const QString& newTempDirectory) : TempDirectory(newTempDirectory)
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
  // Using a scoped pointer ensures the memory will be cleaned if instantiator
  // fails before returning the module. See QScopedPointer::take()
  QScopedPointer<qSlicerCLIModule> module(new qSlicerCLIModule());
  module->setModuleType("CommandLineModule");
  module->setEntryPoint(this->path());

  ctkScopedCurrentDir scopedCurrentDir(QFileInfo(this->path()).path());

  int cliProcessTimeoutInMs = 5000;
  QProcess cli;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("ITK_AUTOLOAD_PATH", "");
  cli.setProcessEnvironment(env);
  cli.start(this->path(), QStringList(QString("--xml")));
  bool res = cli.waitForFinished(cliProcessTimeoutInMs);
  if (!res)
    {
    this->appendInstantiateErrorString(QString("CLI executable: %1").arg(this->path()));
    QString errorString;
    switch(cli.error())
      {
      case QProcess::FailedToStart:
        errorString = QLatin1String(
              "The process failed to start. Either the invoked program is missing, or "
              "you may have insufficient permissions to invoke the program.");
        break;
      case QProcess::Crashed:
        errorString = QLatin1String(
              "The process crashed some time after starting successfully.");
        break;
      case QProcess::Timedout:
        errorString = QString(
              "The process timed out after %1 msecs.").arg(cliProcessTimeoutInMs);
        break;
      case QProcess::WriteError:
        errorString = QLatin1String(
              "An error occurred when attempting to read from the process. "
              "For example, the process may not be running.");
        break;
      case QProcess::ReadError:
        errorString = QLatin1String(
              "An error occurred when attempting to read from the process. "
              "For example, the process may not be running.");
        break;
      case QProcess::UnknownError:
        errorString = QLatin1String(
              "Failed to execute process. An unknown error occurred.");
        break;
      }
    this->appendInstantiateErrorString(errorString);
    return 0;
    }
  QString errors = cli.readAllStandardError();
  if (!errors.isEmpty())
    {
    this->appendInstantiateErrorString(QString("CLI executable: %1").arg(this->path()));
    this->appendInstantiateErrorString(errors);
    // TODO: More investigation for the following behavior:
    // on my machine (Ubuntu 10.04 with ITKv4), having standard error trims the
    // standard output results. The following readAllStandardOutput() is then
    // missing chars and makes the XML invalid. I'm not sure if it's just on my
    // machine so there is a chance it succeeds to parse the XML description
    // on other machines.
    }
  QString xmlDescription = cli.readAllStandardOutput();
  if (xmlDescription.isEmpty())
    {
    this->appendInstantiateErrorString(QString("CLI executable: %1").arg(this->path()));
    this->appendInstantiateErrorString("Failed to retrieve Xml Description");
    return 0;
    }
  if (!xmlDescription.startsWith("<?xml"))
    {
    this->appendInstantiateWarningString(QString("CLI executable: %1").arg(this->path()));
    this->appendInstantiateWarningString(QLatin1String("XML description doesn't start right away."));
    this->appendInstantiateWarningString(QString("Output before '<?xml' is [%1]").arg(
                                           xmlDescription.mid(0, xmlDescription.indexOf("<?xml"))));
    xmlDescription.remove(0, xmlDescription.indexOf("<?xml"));
    }

  module->setXmlModuleDescription(xmlDescription.toLatin1());
  module->setTempDirectory(this->TempDirectory);
  module->setPath(this->path());
  module->setInstalled(qSlicerCLIModuleFactoryHelper::isInstalled(this->path()));
  module->setBuiltIn(qSlicerCLIModuleFactoryHelper::isBuiltIn(this->path()));

  this->CLIModule = module.data();

  return module.take();
}

//-----------------------------------------------------------------------------
void qSlicerCLIExecutableModuleFactoryItem::uninstantiate()
{
  this->CLIModule->cliModuleLogic()->KillProcesses();
  this->ctkAbstractFactoryFileBasedItem<qSlicerAbstractCoreModule>::uninstantiate();
}

//-----------------------------------------------------------------------------
// qSlicerCLIExecutableModuleFactoryPrivate

//-----------------------------------------------------------------------------
class qSlicerCLIExecutableModuleFactoryPrivate
{
  Q_DECLARE_PUBLIC(qSlicerCLIExecutableModuleFactory);
protected:
  qSlicerCLIExecutableModuleFactory* const q_ptr;
public:
  typedef qSlicerCLIExecutableModuleFactoryPrivate Self;
  qSlicerCLIExecutableModuleFactoryPrivate(qSlicerCLIExecutableModuleFactory& object);

private:
  QString TempDirectory;
};

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactoryPrivate::qSlicerCLIExecutableModuleFactoryPrivate(qSlicerCLIExecutableModuleFactory& object)
:q_ptr(&object)
{
  this->TempDirectory = QDir::tempPath();
}

//-----------------------------------------------------------------------------
// qSlicerCLIExecutableModuleFactory

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactory::qSlicerCLIExecutableModuleFactory()
  : d_ptr(new qSlicerCLIExecutableModuleFactoryPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactory::~qSlicerCLIExecutableModuleFactory()
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIExecutableModuleFactory::registerItems()
{
  QStringList modulePaths = qSlicerCLIModuleFactoryHelper::modulePaths();
  this->registerAllFileItems(modulePaths);
}

//-----------------------------------------------------------------------------
bool qSlicerCLIExecutableModuleFactory::isValidFile(const QFileInfo& file)const
{
  if (!this->Superclass::isValidFile(file))
    {
    return false;
    }
  if (!file.isExecutable())
    {
    return false;
    }
  return qSlicerUtils::isCLIExecutable(file.absoluteFilePath());
}

//-----------------------------------------------------------------------------
ctkAbstractFactoryItem<qSlicerAbstractCoreModule>* qSlicerCLIExecutableModuleFactory
::createFactoryFileBasedItem()
{
  Q_D(qSlicerCLIExecutableModuleFactory);
  return new qSlicerCLIExecutableModuleFactoryItem(d->TempDirectory);
}

//-----------------------------------------------------------------------------
QString qSlicerCLIExecutableModuleFactory::fileNameToKey(const QString& executableName)const
{
  return qSlicerUtils::extractModuleNameFromLibraryName(executableName);
}

//-----------------------------------------------------------------------------
void qSlicerCLIExecutableModuleFactory::setTempDirectory(const QString& newTempDirectory)
{
  Q_D(qSlicerCLIExecutableModuleFactory);
  d->TempDirectory = newTempDirectory;
}
