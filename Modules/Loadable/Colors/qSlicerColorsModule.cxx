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

#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>

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

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyColorLegendPlugin.h"

// DisplayableManager initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkSlicerColorsModuleMRMLDisplayableManager)

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

  // DisplayableManager initialization
  // Register color legend displayable manager for slice and 3D views
  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
    "vtkMRMLColorLegendDisplayableManager");
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
    "vtkMRMLColorLegendDisplayableManager");

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

  QStringList paths = qSlicerCoreApplication::application()->toSlicerHomeAbsolutePaths(
    app->userSettings()->value("QTCoreModules/Colors/ColorFilePaths").toStringList());
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

  // Register Subject Hierarchy core plugins
  qSlicerSubjectHierarchyColorLegendPlugin* colorLegendPlugin = new qSlicerSubjectHierarchyColorLegendPlugin();
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(colorLegendPlugin);
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
    "The <b>Colors Module</b> manages color look up tables, stored in Color nodes.<br>"
    "These tables translate between a numeric value and a color "
    "for displaying of various data types, such as volumes and models.<br>"
    "Two lookup table types are available:<br>"
    "<ul>"
    "<li>Discrete table: List of named colors are specified (example: GenericAnatomyColors). "
    "Discrete tables can be used for continuous mapping as well, in this case the colors "
    "are used as samples at equal distance within the specified range, and smoothly "
    "interpolating between them (example: Grey).</li>"
    "<li>Continuous scale: Color is specified for arbitrarily chosen numerical values "
    "and color value can be computed by smoothly interpolating between these values "
    "(example: PET-DICOM). No names are specified for colors.</li>"
    "All built-in color tables are read-only. To edit colors, create a copy "
    "of the color table by clicking on the 'copy' folder icon.<br>";
  help += this->defaultDocumentationLink();
  return help;
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
  moduleContributors << QString("Mikhail Polkovnikov (IHEP)");
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
