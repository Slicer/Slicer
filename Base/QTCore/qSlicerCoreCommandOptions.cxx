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
#include <QDebug>
#include <QDir>

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerCoreCommandOptions.h"

//-----------------------------------------------------------------------------
class qSlicerCoreCommandOptionsPrivate
{
  Q_DECLARE_PUBLIC(qSlicerCoreCommandOptions);
protected:
  qSlicerCoreCommandOptions* q_ptr;
public:
  qSlicerCoreCommandOptionsPrivate(qSlicerCoreCommandOptions& object);

  void init();

  QHash<QString, QVariant> ParsedArgs;
  QString                  ExtraPythonScript;
  bool                     RunPythonAndExit;
};

//-----------------------------------------------------------------------------
// qSlicerCoreCommandOptionsPrivate methods

//-----------------------------------------------------------------------------
qSlicerCoreCommandOptionsPrivate::qSlicerCoreCommandOptionsPrivate(qSlicerCoreCommandOptions& object)
  : q_ptr(&object)
  , RunPythonAndExit(false)
{
}

//-----------------------------------------------------------------------------
void qSlicerCoreCommandOptionsPrivate::init()
{
  Q_Q(qSlicerCoreCommandOptions);
  q->setArgumentPrefix("--", "-"); // Use Unix-style argument names
  q->enableSettings("disable-settings"); // Enable QSettings support
}

//-----------------------------------------------------------------------------
// qSlicerCoreCommandOptions methods

//-----------------------------------------------------------------------------
qSlicerCoreCommandOptions::qSlicerCoreCommandOptions():Superclass()
, d_ptr(new qSlicerCoreCommandOptionsPrivate(*this))
{
  Q_D(qSlicerCoreCommandOptions);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerCoreCommandOptions::~qSlicerCoreCommandOptions() = default;

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::parse(const QStringList& arguments)
{
  Q_D(qSlicerCoreCommandOptions);

  this->addArguments();

  bool ok = false;
  d->ParsedArgs = this->parseArguments(arguments, &ok);
  if (!ok)
    {
    return false;
    }

  // If first unparsed argument is python script, enable 'shebang' mode
  QStringList unparsedArguments = this->unparsedArguments();
  if (unparsedArguments.size() > 0 && unparsedArguments.at(0).endsWith(".py"))
    {
    if(!this->pythonScript().isEmpty())
      {
      qWarning() << "Ignore script specified using '--python-script'";
      }
    this->setExtraPythonScript(unparsedArguments.at(0));
    this->setRunPythonAndExit(true);
    }

  if (!d->ParsedArgs.value("c").toString().isEmpty())
    {
    this->setRunPythonAndExit(true);
    }

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::displayHelpAndExit()const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("help").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::ignoreRest() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("ignore-rest").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::ignoreSlicerRC()const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("ignore-slicerrc").toBool() ||
      this->isTestingEnabled();
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreCommandOptions::additionalModulePaths()const
{
  Q_D(const qSlicerCoreCommandOptions);
  QStringList allAdditionalModulePaths;

  // note the singular form: 'path' not 'paths'
  QString additionalModulePath = d->ParsedArgs.value("additional-module-path").toString();
  if (!additionalModulePath.isEmpty())
    {
    allAdditionalModulePaths << additionalModulePath;
    }
  // handle rest of pathS
  allAdditionalModulePaths.append(d->ParsedArgs.value("additional-module-paths").toStringList());
  return allAdditionalModulePaths;
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::disableModules() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-modules").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::disableBuiltInModules() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-builtin-modules").toBool();
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreCommandOptions::modulesToIgnore() const
{
  Q_D(const qSlicerCoreCommandOptions);
  QString modulesToIgnore = d->ParsedArgs.value("modules-to-ignore").toString();
  return modulesToIgnore.size() == 0 ? QStringList() : modulesToIgnore.split(",");
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::disableCLIModules() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-cli-modules").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::disableBuiltInCLIModules() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-builtin-cli-modules").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::disableLoadableModules() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-loadable-modules").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::disableBuiltInLoadableModules() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-builtin-loadable-modules").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::disableScriptedLoadableModules()const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-scripted-loadable-modules").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::disableBuiltInScriptedLoadableModules()const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-builtin-scripted-loadable-modules").toBool();
}

//-----------------------------------------------------------------------------
QString qSlicerCoreCommandOptions::pythonScript() const
{
  Q_D(const qSlicerCoreCommandOptions);
  // QDir::fromNativeSeparators is needed as users may specify path
  // with native separators, for example
  //     Slicer.exe --python-script c:\folder\subfolder\script.py
  // but Python requires / as directory separator.
  return QDir::fromNativeSeparators(d->ParsedArgs.value("python-script").toString());
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerCoreCommandOptions, QString, extraPythonScript, ExtraPythonScript);
CTK_SET_CPP(qSlicerCoreCommandOptions, const QString&, setExtraPythonScript, ExtraPythonScript);

//-----------------------------------------------------------------------------
QString qSlicerCoreCommandOptions::pythonCode() const
{
  Q_D(const qSlicerCoreCommandOptions);
  QString pythonCode = d->ParsedArgs.value("python-code").toString();
  if(!pythonCode.isEmpty())
    {
    return pythonCode;
    }
  else
    {
    return d->ParsedArgs.value("c").toString();
    }
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerCoreCommandOptions, bool, runPythonAndExit, RunPythonAndExit);
CTK_SET_CPP(qSlicerCoreCommandOptions, bool, setRunPythonAndExit, RunPythonAndExit);

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::displayVersionAndExit() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("version").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::displayProgramPathAndExit() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("program-path").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::displayHomePathAndExit() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("home").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::displaySettingsPathAndExit() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("settings-path").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::displayTemporaryPathAndExit() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("temporary-path").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::displayMessageAndExit() const
{
  return
      this->displayHelpAndExit()
      || this->displayVersionAndExit()
      || this->displayProgramPathAndExit()
      || this->displayHomePathAndExit()
      || this->displaySettingsPathAndExit()
      || this->displayTemporaryPathAndExit();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::displayApplicationInformation() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("application-information").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::verboseModuleDiscovery() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("verbose-module-discovery").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::verbose()const
{
  return !this->runPythonAndExit();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::disableMessageHandlers() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-message-handlers").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::disableTerminalOutputs()const
{
  Q_D(const qSlicerCoreCommandOptions);
#if defined (Q_OS_WIN32) && !defined (Slicer_BUILD_WIN32_CONSOLE)
  return true;
#else
  return d->ParsedArgs.value("disable-terminal-outputs").toBool();
#endif
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::settingsDisabled() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-settings").toBool() ||
      this->isTestingEnabled();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::keepTemporarySettings() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("keep-temporary-settings").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::isTestingEnabled() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("testing").toBool();
}

#ifdef Slicer_USE_PYTHONQT
//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::isPythonDisabled() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-python").toBool();
}
#endif

//-----------------------------------------------------------------------------
void qSlicerCoreCommandOptions::addArguments()
{
  this->addArgument("ignore-rest", "-", QVariant::Bool,
                    "Ignores the rest of the labeled arguments following this flag.",
                    QVariant(false), true);

  this->addArgument("help", "h", QVariant::Bool,
                    "Display available command line arguments.");

#ifdef Slicer_USE_PYTHONQT
  QString testingDescription = "Activate testing mode. It implies --disable-settings and --ignore-slicerrc.";
#else
  QString testingDescription = "Activate testing mode. It implies --disable-settings.";
#endif
  this->addArgument("testing", "", QVariant::Bool,
                    testingDescription,
                    QVariant(false));

#ifdef Slicer_USE_PYTHONQT
  this->addArgument("disable-python", "", QVariant::Bool,
                    "Disable python support. This is equivalent to build the application with Slicer_USE_PYTHONQT=OFF.");

  this->addArgument("python-script", "", QVariant::String,
                    "Python script to execute after slicer loads.");

  this->addArgument("python-code", "", QVariant::String,
                    "Python code to execute after slicer loads.");

  this->addArgument("", "c", QVariant::String,
                    "Python code to execute after slicer loads. By default, no modules are loaded and Slicer exits afterward.");

  this->addArgument("ignore-slicerrc", "", QVariant::Bool,
                    "Do not load the Slicer resource file (~/.slicerrc.py).");
#endif

  this->addArgument("additional-module-path", "", QVariant::String,
                    "Additional module path to consider when searching for modules to load.");

  this->addArgument("additional-module-paths", "", QVariant::StringList,
                    "List of additional module path to consider when searching for modules to load.");

  this->addArgument("modules-to-ignore", "", QVariant::String,
                    "Comma separated list of modules that should *NOT* be loaded.");

  this->addArgument("disable-modules", "", QVariant::Bool,
                    "Disable the loading of any Modules.");

  this->addArgument("disable-builtin-modules", "", QVariant::Bool,
                    "Disable the loading of builtin Modules.");

#ifdef Slicer_BUILD_CLI_SUPPORT
  this->addArgument("disable-cli-modules", "", QVariant::Bool,
                    "Disable the loading of any Command Line Modules.");

  this->addArgument("disable-builtin-cli-modules", "", QVariant::Bool,
                    "Disable the loading of builtin Command Line Modules.");
#endif

  this->addArgument("disable-loadable-modules", "", QVariant::Bool,
                    "Disable the loading of any Loadable Modules.");

  this->addArgument("disable-builtin-loadable-modules", "", QVariant::Bool,
                    "Disable the loading of builtin Loadable Modules.");

#ifdef Slicer_USE_PYTHONQT
  this->addArgument("disable-scripted-loadable-modules", "", QVariant::Bool,
                    "Disable the loading of any Scripted Loadable Modules.");

  this->addArgument("disable-builtin-scripted-loadable-modules", "", QVariant::Bool,
                    "Disable the loading of builtinScripted Loadable Modules.");
#endif

  this->addArgument("version", "", QVariant::Bool,
                    "Display version information and exits.");

  this->addArgument("program-path", "", QVariant::Bool,
                    "Display application program path and exits.");

  this->addArgument("home", "", QVariant::Bool,
                    "Display home path and exits.");

  this->addArgument("settings-path", "", QVariant::Bool,
                    "Display settings path and exits.");

  this->addArgument("temporary-path", "", QVariant::Bool,
                    "Display temporary path and exits.");

  this->addArgument("application-information", "", QVariant::Bool,
                    "Display application information in the terminal.");

  this->addArgument("verbose-module-discovery", "", QVariant::Bool,
                    "Enable verbose output during module discovery process.");

  this->addArgument("disable-settings", "", QVariant::Bool,
                    "Start application ignoring user settings and using new temporary settings.");

  this->addArgument("keep-temporary-settings", "", QVariant::Bool,
                    "Indicate whether temporary settings should be maintained.");

  this->addArgument("disable-message-handlers", "", QVariant::Bool,
                    "Start application disabling the 'terminal' message handlers.");

#if defined (Q_OS_WIN32) && !defined (Slicer_BUILD_WIN32_CONSOLE)
#else
  this->addArgument("disable-terminal-outputs", "", QVariant::Bool,
                    "Start application disabling stdout/stderr outputs and capturing outputs only using the error log.");
#endif
}

//-----------------------------------------------------------------------------
QHash<QString, QVariant> qSlicerCoreCommandOptions::parsedArgs() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs;
}
