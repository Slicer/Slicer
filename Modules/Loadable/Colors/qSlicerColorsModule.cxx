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
#include <QSettings>

// CTK includes
#include <ctkColorDialog.h>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerNodeWriter.h"

// Colors includes
#include "qSlicerColorsModule.h"
#include "qSlicerColorsModuleWidget.h"
#include "qSlicerColorsReader.h"

// qMRML includes
#include <qMRMLColorPickerWidget.h>

// Slicer Logic includes
#include <vtkSlicerApplicationLogic.h>
#include "vtkSlicerColorLogic.h"

//-----------------------------------------------------------------------------
class qSlicerColorsModulePrivate
{
public:
  qSlicerColorsModulePrivate();
  QSharedPointer<qMRMLColorPickerWidget> ColorDialogPickerWidget;
};

//-----------------------------------------------------------------------------
qSlicerColorsModulePrivate::qSlicerColorsModulePrivate()
{
  this->ColorDialogPickerWidget =
    QSharedPointer<qMRMLColorPickerWidget>(new qMRMLColorPickerWidget(nullptr));
}

//-----------------------------------------------------------------------------
qSlicerColorsModule::qSlicerColorsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerColorsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerColorsModule::~qSlicerColorsModule() = default;

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
void qSlicerColorsModule::setup()
{
  Q_D(qSlicerColorsModule);
  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  vtkSlicerColorLogic* colorLogic = vtkSlicerColorLogic::SafeDownCast(this->logic());
  if (this->appLogic() != nullptr)
    {
    this->appLogic()->SetColorLogic(colorLogic);
    }
  app->coreIOManager()->registerIO(
    new qSlicerColorsReader(colorLogic, this));
  app->coreIOManager()->registerIO(new qSlicerNodeWriter(
    "Colors", QString("ColorTableFile"),
    QStringList() << "vtkMRMLColorNode", true, this));

  QStringList paths = app->userSettings()->value("QTCoreModules/Colors/ColorFilePaths").toStringList();
#ifdef Q_OS_WIN32
  QString joinedPaths = paths.join(";");
#else
  QString joinedPaths = paths.join(":");
#endif
  // Warning: If the logic has already created the color nodes (AddDefaultColorNodes),
  // setting the user color file paths doesn't trigger any action to add new nodes.
  // It's something that must be fixed into the logic, not here
  colorLogic->SetUserColorFilePaths(joinedPaths.toUtf8());

  // Color picker
  d->ColorDialogPickerWidget->setMRMLColorLogic(colorLogic);
  ctkColorDialog::addDefaultTab(d->ColorDialogPickerWidget.data(),
                                "Labels", SIGNAL(colorSelected(QColor)),
                                SIGNAL(colorNameSelected(QString)));
  ctkColorDialog::setDefaultTab(1);
}

//-----------------------------------------------------------------------------
void qSlicerColorsModule::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerColorsModule);
  /// tbd: might be set too late ?
  d->ColorDialogPickerWidget->setMRMLScene(scene);
  this->Superclass::setMRMLScene(scene);
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
    "For more information see the <a href=\"%1/Documentation/%2.%3/Modules/Colors\">"
    "online documentation</a><br>"
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
  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
}

//-----------------------------------------------------------------------------
QString qSlicerColorsModule::acknowledgementText()const
{
  QString about =
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community.";
  return about;
}

//-----------------------------------------------------------------------------
QStringList qSlicerColorsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Nicole Aucoin (SPL, BWH)");
  moduleContributors << QString("Julien Finet (Kitware)");
  moduleContributors << QString("Ron Kikinis (SPL, BWH)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
bool qSlicerColorsModule::isHidden()const
{
  return false;
}

//-----------------------------------------------------------------------------
QStringList qSlicerColorsModule::associatedNodeTypes() const
{
  return QStringList() << "vtkMRMLColorNode";
}
