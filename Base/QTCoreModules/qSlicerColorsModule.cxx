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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QSettings>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerColorsModule.h"
#include "qSlicerColorsModuleWidget.h"

// Slicer Logic includes
#include "vtkSlicerColorLogic.h"

//-----------------------------------------------------------------------------
class qSlicerColorsModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerColorsModule::qSlicerColorsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerColorsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerColorsModule::~qSlicerColorsModule()
{
}

//-----------------------------------------------------------------------------
QIcon qSlicerColorsModule::icon()const
{
  return QIcon(":/Icons/Colors.png");
}

//-----------------------------------------------------------------------------
QString qSlicerColorsModule::title()const
{
  return "Colors";
}

//-----------------------------------------------------------------------------
void qSlicerColorsModule::setup()
{
  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  QStringList paths = app->settings()->value("QTCoreModules/Colors/ColorFilePaths").toStringList();
#ifdef Q_OS_WIN32
  QString joinedPaths = paths.join(";");
#else
  QString joinedPaths = paths.join(":");
#endif
  vtkSlicerColorLogic* colorLogic = vtkSlicerColorLogic::SafeDownCast(this->logic());
  // Warning: If the logic has already created the color nodes (AddDefaultColorNodes),
  // setting the user color file paths doesn't trigger any action to add new nodes.
  // It's something that must be fixed into the logic, not here
  colorLogic->SetUserColorFilePaths(joinedPaths.toLatin1());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerColorsModule::createWidgetRepresentation()
{
  return new qSlicerColorsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerColorsModule::createLogic()
{
  return vtkSlicerColorLogic::New();
}

//-----------------------------------------------------------------------------
QString qSlicerColorsModule::helpText()const
{
  QString help =
    "The <b>Colors Module</b> manages color look up tables.<br>"
    "For more information see <a href=\"%1/Modules:Colors-Documentation-3.6\">"
    "%1/Modules:Colors-Documentation-3.6</a><br>"
    "Tables are used by mappers to translate between an integer and a colour "
    "value for display of models and volumes.<br>Slicer supports three kinds "
    "of tables:<br>"
    "1. Continuous scales, like the greyscale table.<br>"
    "2. Parametric tables, defined by an equation, such as the FMRIPA table.<br>"
    "3. Discrete tables, such as those read in from a file.<br><br>"
    "You can specify a directory from which to read color files using the "
    "View -> Application Settings window, Module Settings frame, in the "
    "User defined color file paths section. The color file format is a plain "
    "text file with the .txt or .ctbl extension. Each line in the file has:<br>"
    "label\tname\tR\tG\tB\tA\nlabel is an integer, name a string, and RGBA are "
    "0-255.<br><br>Users are only allowed to edit User type tables. Use the "
    "Edit frame to create a new color table (you can copy from an existing "
    "one), and save it to a file using the File, Save interface.";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerColorsModule::acknowledgementText()const
{
  QString about =
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer "
    "Community.<br>"
    "See <a href=\"http://www.slicer.org\">www.slicer.org</a> for details."
    "This Color module was developed by Nicole Aucoin, SPL, BWH (Ron Kikinis).";
  return about;
}
