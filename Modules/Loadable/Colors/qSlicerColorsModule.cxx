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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QtPlugin>
#include <QSettings>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerCoreIOManager.h"

// Colors includes
#include "qSlicerColorsIO.h"
#include "qSlicerColorsModule.h"
#include "qSlicerColorsModuleWidget.h"

// Slicer Logic includes
#include "vtkSlicerColorLogic.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerColorsModule, qSlicerColorsModule);

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
QStringList qSlicerColorsModule::categories()const
{
  return QStringList() << "Informatics";
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
  vtkSlicerColorLogic* colorLogic = vtkSlicerColorLogic::SafeDownCast(this->logic());
  app->coreIOManager()->registerIO(
    new qSlicerColorsIO(colorLogic, this));

  QStringList paths = app->settings()->value("QTCoreModules/Colors/ColorFilePaths").toStringList();
#ifdef Q_OS_WIN32
  QString joinedPaths = paths.join(";");
#else
  QString joinedPaths = paths.join(":");
#endif
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
    "For more information see <a href=\"%1/Documentation/4.0/Modules/Colors\">"
    "%1/Documentation/4.0/Modules/Colors</a><br>"
    "Tables are used by mappers to translate between an integer and a colour "
    "value for display of models and volumes.<br>Slicer supports three kinds "
    "of tables:<br>"
    "1. Continuous scales, like the greyscale table.<br>"
    "2. Parametric tables, defined by an equation, such as the FMRIPA table.<br>"
    "3. Discrete tables, such as those read in from a file.<br><br>"
    "You can create a duplicate of a color table to allow editing the "
    "names and values and color by clicking on the folder+ icon.<br>"
    "You can then save the new color table via the File -> Save interface.<br>"
    "The color file format is a plain text file with the .txt or .ctbl extension. "
    "Each line in the file has:<br>"
    "label\tname\tR\tG\tB\tA<br>label is an integer, name a string, and RGBA are "
    "0-255.";
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

//-----------------------------------------------------------------------------
QStringList qSlicerColorsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Nicole Aucoin (SPL)");
  moduleContributors << QString("Julien Finet (Kitware Inc.)");
  return moduleContributors;
}
