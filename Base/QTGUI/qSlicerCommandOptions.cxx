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

#include "qSlicerCommandOptions.h"
#include "qSlicerCoreApplication.h"

#include <QDebug>

// Slicer includes

//-----------------------------------------------------------------------------
// qSlicerCommandOptions methods

//-----------------------------------------------------------------------------
qSlicerCommandOptions::qSlicerCommandOptions()
  : Superclass()
{
}

//-----------------------------------------------------------------------------
bool qSlicerCommandOptions::disableToolTips() const
{
  return this->parsedArgs().value("disable-tooltips").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCommandOptions::noSplash() const
{
  return this->parsedArgs().value("no-splash").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCommandOptions::noMainWindow() const
{
  return this->parsedArgs().value("no-main-window").toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerCommandOptions::showPythonConsole() const
{
  bool show = this->parsedArgs().value("show-python-console").toBool();
  // Handle the deprecated argument name for a while, for backward compatibility:
  if (this->parsedArgs().value("show-python-interactor").toBool())
  {
    qWarning() << "show-python-interactor command-line argument is deprecated, use show-python-console instead";
    show = true;
  }
  return show;
}

//-----------------------------------------------------------------------------
bool qSlicerCommandOptions::showPythonInteractor() const
{
  qWarning() << "qSlicerCommandOptions::showPythonInteractor() method is deprecated, use "
                "qSlicerCommandOptions::showPythonInteractor() instead";
  return showPythonConsole();
}

//-----------------------------------------------------------------------------
bool qSlicerCommandOptions::enableQtTesting() const
{
#ifdef Slicer_USE_QtTesting
  return this->parsedArgs().value("qt-testing").toBool();
#else
  return false;
#endif
}

//-----------------------------------------------------------------------------
bool qSlicerCommandOptions::exitAfterStartup() const
{
  return this->parsedArgs().value("exit-after-startup").toBool();
}

//-----------------------------------------------------------------------------
void qSlicerCommandOptions::addArguments()
{
  this->Superclass::addArguments();

  this->addArgument("disable-tooltips", "", QVariant::Bool, "Disable toolstips in the user interface.");

  this->addArgument("no-splash", "", QVariant::Bool, "Disable the startup splash screen.");

  this->addArgument("no-main-window",
                    "",
                    QVariant::Bool,
                    "Disable display of the main slicer window.  Use with --python-script for alternate interface");

#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
  {
    this->addArgument("show-python-console", "", QVariant::Bool, "Show Python console at startup.");
    this->addArgument("show-python-interactor",
                      "",
                      QVariant::Bool,
                      "Show Python console at startup (deprecated, use show-python-console instead).");
  }
#endif

#ifdef Slicer_USE_QtTesting
  this->addArgument("qt-testing", "", QVariant::Bool, "Enable QtTesting in the user interface");
#endif

  this->addArgument(
    "exit-after-startup", "", QVariant::Bool, "Exit after startup is complete. Useful for measuring startup time");
}
