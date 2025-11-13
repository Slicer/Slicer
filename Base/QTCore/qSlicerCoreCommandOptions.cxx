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

  // clang-format off
  QHash<QString, QVariant> ParsedArgs;
  QString                  ExtraPythonScript;
  int                      ExtraPythonScriptProcessedArgumentsCount;
  bool                     RunPythonAndExit;
  // clang-format on
};

//-----------------------------------------------------------------------------
// qSlicerCoreCommandOptionsPrivate methods

//-----------------------------------------------------------------------------
qSlicerCoreCommandOptionsPrivate::qSlicerCoreCommandOptionsPrivate(qSlicerCoreCommandOptions& object)
  : q_ptr(&object)
  , ExtraPythonScriptProcessedArgumentsCount(0)
  , RunPythonAndExit(false)
{
}

//-----------------------------------------------------------------------------
void qSlicerCoreCommandOptionsPrivate::init()
{
  Q_Q(qSlicerCoreCommandOptions);
  q->setArgumentPrefix("--", "-");       // Use Unix-style argument names
  q->enableSettings("disable-settings"); // Enable QSettings support
}

//-----------------------------------------------------------------------------
// qSlicerCoreCommandOptions methods

//-----------------------------------------------------------------------------
qSlicerCoreCommandOptions::qSlicerCoreCommandOptions()
  : Superclass()
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

  // If the first argument is a Python script, enable the 'shebang' mode.
  //
  // If there is a -I argument before a Python script (.py), enable running the script.
  //
  // This a commonly used command-line switch of Python.exe to "isolate Python from the user's environment".
  // For example, VS Code runs Slicer using these arguments to query its Python interpreter version:
  //   .../Slicer.exe -I \
  //     ...\.vscode\extensions\ms-python.python-2023.6.1\pythonFiles\get_output_via_markers.py \
  //     ...\.vscode\extensions\ms-python.python-2023.6.1\pythonFiles\interpreterInfo.py

  QStringList unparsedArguments = this->unparsedArguments();
  QString extraPythonScript;
  int extraPythonScriptProcessedArgumentsCount = 0;
  if (unparsedArguments.size() > 0 && unparsedArguments.at(0).endsWith(".py"))
  {
    extraPythonScript = unparsedArguments.at(0);
    extraPythonScriptProcessedArgumentsCount = 1;
  }
  else if (unparsedArguments.size() > 1 && unparsedArguments.at(0) == "-I" && unparsedArguments.at(1).endsWith(".py"))
  {
    extraPythonScript = unparsedArguments.at(1);
    extraPythonScriptProcessedArgumentsCount = 2;
  }
  if (!extraPythonScript.isEmpty())
  {
    if (!this->pythonScript().isEmpty())
    {
      qWarning() << "Ignore script specified using '--python-script'";
    }
    this->setExtraPythonScript(extraPythonScript);
    this->setExtraPythonScriptProcessedArgumentsCount(extraPythonScriptProcessedArgumentsCount);
    this->setRunPythonAndExit(true);
  }

  if (!d->ParsedArgs.value("c").toString().isEmpty())
  {
    this->setRunPythonAndExit(true);
  }

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::displayHelpAndExit() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value(/*no tr*/ "help").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::ignoreRest() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("ignore-rest").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::ignoreSlicerRC() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("ignore-slicerrc").toBool() || //
         this->isTestingEnabled();
}

//-----------------------------------------------------------------------------
QStringList qSlicerCoreCommandOptions::additionalModulePaths() const
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
bool qSlicerCoreCommandOptions::disableScriptedLoadableModules() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-scripted-loadable-modules").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::disableBuiltInScriptedLoadableModules() const
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
CTK_GET_CPP(qSlicerCoreCommandOptions, int, extraPythonScriptProcessedArgumentsCount, ExtraPythonScriptProcessedArgumentsCount);
CTK_SET_CPP(qSlicerCoreCommandOptions, int, setExtraPythonScriptProcessedArgumentsCount, ExtraPythonScriptProcessedArgumentsCount);

//-----------------------------------------------------------------------------
QString qSlicerCoreCommandOptions::pythonCode() const
{
  Q_D(const qSlicerCoreCommandOptions);
  QString pythonCode = d->ParsedArgs.value("python-code").toString();
  if (!pythonCode.isEmpty())
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
  return d->ParsedArgs.value(/*no tr*/ "version").toBool();
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
  return d->ParsedArgs.value(/*no tr*/ "home").toBool();
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
  return this->displayHelpAndExit()            //
         || this->displayVersionAndExit()      //
         || this->displayProgramPathAndExit()  //
         || this->displayHomePathAndExit()     //
         || this->displaySettingsPathAndExit() //
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
bool qSlicerCoreCommandOptions::verbose() const
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
bool qSlicerCoreCommandOptions::disableTerminalOutputs() const
{
  Q_D(const qSlicerCoreCommandOptions);
#if defined(Q_OS_WIN32) && !defined(Slicer_BUILD_WIN32_CONSOLE)
  return true;
#else
  return d->ParsedArgs.value("disable-terminal-outputs").toBool();
#endif
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::settingsDisabled() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-settings").toBool() || //
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
  return d->ParsedArgs.value(/*no tr*/ "testing").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCoreCommandOptions::isPythonDisabled() const
{
#ifdef Slicer_USE_PYTHONQT
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs.value("disable-python").toBool();
#else
  return true;
#endif
}

//-----------------------------------------------------------------------------
void qSlicerCoreCommandOptions::addArguments()
{
  this->addArgument("ignore-rest",
                    "-",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Ignores the rest of the labeled arguments following this flag.",
                    QVariant(false),
                    true);

  this->addArgument(/*no tr*/ "help",
                    "h",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Display available command line arguments.");

#ifdef Slicer_USE_PYTHONQT
  QString testingDescription = /*no tr*/ "Activate testing mode. It implies --disable-settings and --ignore-slicerrc.";
#else
  QString testingDescription = /*no tr*/ "Activate testing mode. It implies --disable-settings.";
#endif
  this->addArgument(/*no tr*/ "testing",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    testingDescription,
                    QVariant(false));

#ifdef Slicer_USE_PYTHONQT
  this->addArgument("disable-python",
                    "",
# if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
# else
                    QVariant::Bool,
# endif
                    /*no tr*/ "Disable python support. This is equivalent to build the application with Slicer_USE_PYTHONQT=OFF.");

  this->addArgument("python-script",
                    "",
# if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::QString,
# else
                    QVariant::String,
# endif
                    /*no tr*/ "Python script to execute after slicer loads.");

  this->addArgument("python-code",
                    "",
# if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::QString,
# else
                    QVariant::String,
# endif
                    /*no tr*/ "Python code to execute after slicer loads.");

  this->addArgument("",
                    "c",
# if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::QString,
# else
                    QVariant::String,
# endif
                    /*no tr*/ "Python code to execute after slicer loads. By default, no modules are loaded and Slicer exits afterward.");

  this->addArgument("ignore-slicerrc",
                    "",
# if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
# else
                    QVariant::Bool,
# endif
                    /*no tr*/ "Do not load the Slicer resource file (~/.slicerrc.py).");
#endif

  this->addArgument("additional-module-path",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::QString,
#else
                    QVariant::String,
#endif
                    /*no tr*/ "Additional module path to consider when searching for modules to load.");

  this->addArgument("additional-module-paths",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::QStringList,
#else
                    QVariant::StringList,
#endif
                    /*no tr*/ "List of additional module path to consider when searching for modules to load.");

  this->addArgument("modules-to-ignore",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::QString,
#else
                    QVariant::String,
#endif
                    /*no tr*/ "Comma separated list of modules that should *NOT* be loaded.");

  this->addArgument("disable-modules",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Disable the loading of any Modules.");

  this->addArgument("disable-builtin-modules",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Disable the loading of builtin Modules.");

  this->addArgument("disable-cli-modules",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Disable the loading of any Command Line Modules.");

  this->addArgument("disable-builtin-cli-modules",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Disable the loading of builtin Command Line Modules.");

  this->addArgument("disable-loadable-modules",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Disable the loading of any Loadable Modules.");

  this->addArgument("disable-builtin-loadable-modules",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Disable the loading of builtin Loadable Modules.");

#ifdef Slicer_USE_PYTHONQT
  this->addArgument("disable-scripted-loadable-modules",
                    "",
# if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
# else
                    QVariant::Bool,
# endif
                    /*no tr*/ "Disable the loading of any Scripted Loadable Modules.");

  this->addArgument("disable-builtin-scripted-loadable-modules",
                    "",
# if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
# else
                    QVariant::Bool,
# endif
                    /*no tr*/ "Disable the loading of builtinScripted Loadable Modules.");
#endif

  this->addArgument(/*no tr*/ "version",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Display version information and exits.");

  this->addArgument("program-path",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Display application program path and exits.");

  this->addArgument(/*no tr*/ "home",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Display home path and exits.");

  this->addArgument("settings-path",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Display settings path and exits.");

  this->addArgument("temporary-path",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Display temporary path and exits.");

  this->addArgument("application-information",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Display application information in the terminal.");

  this->addArgument("verbose-module-discovery",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Enable verbose output during module discovery process.");

  this->addArgument("disable-settings",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Start application ignoring user settings and using new temporary settings.");

  this->addArgument("keep-temporary-settings",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Indicate whether temporary settings should be maintained.");

  this->addArgument("disable-message-handlers",
                    "",
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
#else
                    QVariant::Bool,
#endif
                    /*no tr*/ "Start application disabling the 'terminal' message handlers.");

#if defined(Q_OS_WIN32) && !defined(Slicer_BUILD_WIN32_CONSOLE)
#else
  this->addArgument("disable-terminal-outputs",
                    "",
# if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                    QMetaType::Bool,
# else
                    QVariant::Bool,
# endif
                    /*no tr*/ "Start application disabling stdout/stderr outputs and capturing outputs only using the error log.");
#endif
}

//-----------------------------------------------------------------------------
QHash<QString, QVariant> qSlicerCoreCommandOptions::parsedArgs() const
{
  Q_D(const qSlicerCoreCommandOptions);
  return d->ParsedArgs;
}
