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

#include "qSlicerCommandOptions.h"
#include "qSlicerCoreApplication.h"

// Slicer includes
#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT

//-----------------------------------------------------------------------------
// qSlicerCommandOptions methods

//-----------------------------------------------------------------------------
qSlicerCommandOptions::qSlicerCommandOptions():Superclass()
{
}

//-----------------------------------------------------------------------------
bool qSlicerCommandOptions::disableToolTips()const
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
bool qSlicerCommandOptions::showPythonInteractor() const
{
  return this->parsedArgs().value("show-python-interactor").toBool();
}

//-----------------------------------------------------------------------------
void qSlicerCommandOptions::addArguments()
{
  this->Superclass::addArguments();

  this->addArgument("disable-tooltips", "", QVariant::Bool,
                    "Disables toolstips in the user interface.");

  this->addArgument("no-splash", "", QVariant::Bool,
                    "Disables the startup splash screen.");

  this->addArgument("no-main-window", "", QVariant::Bool,
                    "Disables display of the main slicer window.  Use with --python-script for alternate interface");

#ifdef Slicer_USE_PYTHONQT
  if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
    this->addArgument("show-python-interactor", "", QVariant::Bool,
                      "Show Python interactor at startup.");
    }
#endif
}
