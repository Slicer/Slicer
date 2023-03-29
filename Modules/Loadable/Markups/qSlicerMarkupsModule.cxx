/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  ==============================================================================*/

// Qt includes
#include <QDebug>
#include <QDockWidget>
#include <QSettings>
#include <QMainWindow>
#include <QMenu>

// MRMLDisplayableManager includes
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>

// QTGUI includes
#include <qSlicerApplication.h>
#include "qSlicerCoreApplication.h"
#include <qSlicerIOManager.h>
#include <qSlicerNodeWriter.h>
#include "qSlicerModuleManager.h"

#include "vtkMRMLScene.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyMarkupsPlugin.h"

// Markups module includes
#include "qSlicerAnnotationsReader.h"
#include "qSlicerMainWindow.h"
#include "qSlicerMarkupsModule.h"
#include "qSlicerMarkupsModuleWidget.h"
#include "qSlicerMarkupsReader.h"
#include "qSlicerMarkupsWriter.h"
#include <qSlicerMarkupsMeasurementsWidget.h>

// Markups nodes includes
#include "vtkMRMLMarkupsDisplayNode.h"

// Markups logic includes
#include "vtkSlicerMarkupsLogic.h"

// Markups widgets
#include "qMRMLMarkupsAngleMeasurementsWidget.h"
#include "qMRMLMarkupsCurveSettingsWidget.h"
#include "qMRMLMarkupsPlaneWidget.h"
#include "qMRMLMarkupsROIWidget.h"
#include "qMRMLMarkupsToolBar.h"
#include "qMRMLMarkupsOptionsWidgetsFactory.h"

// DisplayableManager initialization
#include <vtkAutoInit.h>

VTK_MODULE_INIT(vtkSlicerMarkupsModuleMRMLDisplayableManager);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markups
class qSlicerMarkupsModulePrivate
{
QVTK_OBJECT
Q_DECLARE_PUBLIC(qSlicerMarkupsModule);
protected:
  qSlicerMarkupsModule* const q_ptr;
public:
  qSlicerMarkupsModulePrivate(qSlicerMarkupsModule& object);

  /// Adds Markups toolbar to the application GUI
  virtual void addToolBar();

  virtual ~qSlicerMarkupsModulePrivate();
  qMRMLMarkupsToolBar* ToolBar;
  bool MarkupsModuleOwnsToolBar{ true };
  bool AutoShowToolBar{ true };
  vtkWeakPointer<vtkMRMLMarkupsNode> MarkupsToShow;
  QDockWidget* MeasurementPanelDockWidget;
  QAction* MeasurementPanelDockWidgetViewAction;
  qSlicerMarkupsMeasurementsWidget* MeasurementPanelWidget;
};

//-----------------------------------------------------------------------------
// qSlicerMarkupsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModulePrivate::qSlicerMarkupsModulePrivate(qSlicerMarkupsModule& object)
  : q_ptr(&object)
{
  this->ToolBar = new qMRMLMarkupsToolBar;
  this->ToolBar->setWindowTitle(QObject::tr("Markups"));
  this->ToolBar->setObjectName("MarkupsToolbar");
  this->ToolBar->setVisible(false);
}

//-----------------------------------------------------------------------------
qSlicerMarkupsModulePrivate::~qSlicerMarkupsModulePrivate()
{
  if (this->MarkupsModuleOwnsToolBar)
    {
    // the toolbar has not been added to the main window
    // so it is still owned by this class, therefore
    // we are responsible for deleting it
    delete this->ToolBar;
    this->ToolBar = nullptr;
    }

  /// NOTE: This prevents deletion of QWidgets by the destructor of the factory,
  /// which produces a segmentation fault. Though I can't confirm, I believe
  /// this behaviour is due to deletion of QWidgets in global scope destructors
  /// and how this conflicts with the lifecycle of QWidgets.
  qMRMLMarkupsOptionsWidgetsFactory::instance()->unregisterAll();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModulePrivate::addToolBar()
{
  Q_Q(qSlicerMarkupsModule);

  qSlicerMainWindow* mainWindow = qobject_cast<qSlicerMainWindow*>(qSlicerApplication::application()->mainWindow());
  if (mainWindow == nullptr)
    {
    qDebug("qSlicerMarkupsModulePrivate::addToolBar: no main window is available, toolbar is not added");
    return;
    }

  this->ToolBar->setWindowTitle("Markups");
  this->ToolBar->setObjectName("MarkupsToolBar");
  //// Add a toolbar break to make the sequence toolbar appear in a separate row
  //// (it is a long toolbar and would make many toolbar buttons disappear from
  //// all the standard toolbars if they are all displayed in a single row).
  mainWindow->addToolBarBreak();
  mainWindow->addToolBar(this->ToolBar);
  this->MarkupsModuleOwnsToolBar = false;
  foreach(QMenu * toolBarMenu, mainWindow->findChildren<QMenu*>())
    {
    if (toolBarMenu->objectName() == QString("WindowToolBarsMenu"))
      {
      toolBarMenu->addAction(this->ToolBar->toggleViewAction());
      break;
      }
    }

  //// Main window takes care of saving and restoring toolbar geometry and state.
  //// However, when state is restored the markups toolbar was not created yet.
  //// We need to restore the main window state again, now, that the Markups toolbar is available.
  QSettings settings;
  settings.beginGroup("MainWindow");
  bool restore = settings.value("RestoreGeometry", false).toBool();
  if (restore)
    {
    mainWindow->restoreState(settings.value("windowState").toByteArray());
    }
  this->ToolBar->initializeToolBarLayout();

  vtkMRMLScene* scene = qSlicerCoreApplication::application()->mrmlScene();
  if (scene)
    {
    this->MeasurementPanelDockWidget = new QDockWidget;
    this->MeasurementPanelDockWidget->setWindowTitle(QObject::tr("Markups Measurement Panel"));
    this->MeasurementPanelDockWidget->setObjectName("MeasurementPanelDockWidget");
    this->MeasurementPanelDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    this->MeasurementPanelDockWidget->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    this->MeasurementPanelDockWidgetViewAction = this->MeasurementPanelDockWidget->toggleViewAction();
    this->MeasurementPanelDockWidgetViewAction->setText(qSlicerMainWindow::tr("&Markups Measurement Panel"));
    this->MeasurementPanelDockWidgetViewAction->setToolTip(qSlicerMainWindow::tr("Show Markups Measurement Panel"));
    this->MeasurementPanelDockWidgetViewAction->setShortcuts({qSlicerMainWindow::tr("Ctrl+7"),});
    QObject::connect(this->MeasurementPanelDockWidgetViewAction, SIGNAL(toggled(bool)),
        q, SLOT(onMarkupsMeasurementsPanelToggled(bool)));
    this->MeasurementPanelDockWidgetViewAction->setIcon(QIcon(":/Icons/Measurements.svg"));

    // Set up show/hide action
    this->ToolBar->addAction(this->MeasurementPanelDockWidgetViewAction);

    this->MeasurementPanelWidget = new qSlicerMarkupsMeasurementsWidget();
    this->MeasurementPanelWidget->setObjectName("MeasurementPanelWidget");
    this->MeasurementPanelWidget->setMRMLScene(scene);

    this->MeasurementPanelDockWidget->setWidget(this->MeasurementPanelWidget);

    mainWindow->addDockWidget(Qt::RightDockWidgetArea, this->MeasurementPanelDockWidget);
    }
}

//-----------------------------------------------------------------------------
// qSlicerMarkupsModule methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModule::qSlicerMarkupsModule(QObject* _parent)
  : Superclass(_parent), d_ptr(new qSlicerMarkupsModulePrivate(*this))
{
  /*Q_D(qSlicerMarkupsModule);

  vtkMRMLScene* scene = qSlicerCoreApplication::application()->mrmlScene();
  if (scene)
    {
    // Need to listen for any new makrups nodes being added to show toolbar
    this->qvtkConnect(scene, vtkMRMLScene::NodeAddedEvent, this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
    }
    */
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsModule::categories()const
{
  return QStringList() << "" << qSlicerAbstractCoreModule::tr("Informatics");
}

//-----------------------------------------------------------------------------
qSlicerMarkupsModule::~qSlicerMarkupsModule() = default;

//-----------------------------------------------------------------------------
QString qSlicerMarkupsModule::helpText()const
{
  QString help =
    "A module to create and manage markups in 2D and 3D."
    " This module replaced the former Annotations module.";
  help += this->defaultDocumentationLink();
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerMarkupsModule::acknowledgementText()const
{
  return "This work was supported by NAMIC, NAC, and the Slicer Community.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Nicole Aucoin (BWH)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerMarkupsModule::icon()const
{
  return QIcon(":/Icons/Markups.png");
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModule::setup()
{
  Q_D(qSlicerMarkupsModule);
  this->Superclass::setup();

  vtkSlicerMarkupsLogic *logic = vtkSlicerMarkupsLogic::SafeDownCast(this->logic());
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": cannot get Markups logic.";
    return;
    }

  // Register displayable managers (same displayable manager handles both slice and 3D views)
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager("vtkMRMLMarkupsDisplayableManager");
  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager("vtkMRMLMarkupsDisplayableManager");

  // Register IO
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  ioManager->registerIO(new qSlicerMarkupsReader(logic, this));
  ioManager->registerIO(new qSlicerAnnotationsReader(logic, this));
  ioManager->registerIO(new qSlicerMarkupsWriter(this));

  // Add toolbar
  d->addToolBar();

  // Register Subject Hierarchy core plugins
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyMarkupsPlugin());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerMarkupsModule::createWidgetRepresentation()
{
  // Create and configure the additional widgets
  auto optionsWidgetFactory = qMRMLMarkupsOptionsWidgetsFactory::instance();
  optionsWidgetFactory->registerOptionsWidget(new qMRMLMarkupsAngleMeasurementsWidget());
  optionsWidgetFactory->registerOptionsWidget(new qMRMLMarkupsCurveSettingsWidget());
  optionsWidgetFactory->registerOptionsWidget(new qMRMLMarkupsPlaneWidget());
  optionsWidgetFactory->registerOptionsWidget(new qMRMLMarkupsROIWidget());

  // Create and configure module widget.
  auto moduleWidget = new qSlicerMarkupsModuleWidget();
  // Set the number of columns for the grid of "add markups buttons" to the number of markups
  // registered in this module.

  moduleWidget->setCreateMarkupsButtonsColumns(4);

  return moduleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerMarkupsModule::createLogic()
{
  return vtkSlicerMarkupsLogic::New();
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsModule::associatedNodeTypes() const
{
  // This module can edit properties
  return QStringList() << "vtkMRMLMarkupsNode";
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModule::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  vtkSlicerMarkupsLogic* logic = vtkSlicerMarkupsLogic::SafeDownCast(this->logic());
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << " failed: logic is invalid";
    return;
    }
  // Update default view nodes from settings
  this->readDefaultMarkupsDisplaySettings(logic->GetDefaultMarkupsDisplayNode());
  this->writeDefaultMarkupsDisplaySettings(logic->GetDefaultMarkupsDisplayNode());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModule::readDefaultMarkupsDisplaySettings(vtkMRMLMarkupsDisplayNode* markupsDisplayNode)
{
  if (!markupsDisplayNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: markupsDisplayNode is invalid";
    return;
    }
  QSettings settings;

  if (settings.contains("Markups/SnapMode"))
    {
    markupsDisplayNode->SetSnapMode(vtkMRMLMarkupsDisplayNode::GetSnapModeFromString(
      settings.value("Markups/SnapMode").toString().toUtf8()));
    }
  if (settings.contains("Markups/FillVisibility"))
    {
    markupsDisplayNode->SetFillVisibility(settings.value("Markups/FillVisibility").toBool());
    }
  if (settings.contains("Markups/OutlineVisibility"))
    {
    markupsDisplayNode->SetOutlineVisibility(settings.value("Markups/OutlineVisibility").toBool());
    }
  if (settings.contains("Markups/FillOpacity"))
    {
    markupsDisplayNode->SetFillOpacity(settings.value("Markups/FillOpacity").toDouble());
    }
  if (settings.contains("Markups/OutlineOpacity"))
    {
    markupsDisplayNode->SetOutlineOpacity(settings.value("Markups/OutlineOpacity").toDouble());
    }
  if (settings.contains("Markups/TextScale"))
    {
    markupsDisplayNode->SetTextScale(settings.value("Markups/TextScale").toDouble());
    }
  if (settings.contains("Markups/GlyphType"))
    {
    int glyphType = vtkMRMLMarkupsDisplayNode::GetGlyphTypeFromString(settings.value("Markups/GlyphType").toString().toUtf8());
    // If application settings is old then it may contain invalid GlyphType. In this case use Sphere3D glyph instead.
    if (glyphType == vtkMRMLMarkupsDisplayNode::GlyphTypeInvalid)
      {
      glyphType = vtkMRMLMarkupsDisplayNode::Sphere3D;
      }
    markupsDisplayNode->SetGlyphType(glyphType);
    }
  if (settings.contains("Markups/GlyphScale"))
    {
    markupsDisplayNode->SetGlyphScale(settings.value("Markups/GlyphScale").toDouble());
    }
  if (settings.contains("Markups/GlyphSize"))
    {
    markupsDisplayNode->SetGlyphSize(settings.value("Markups/GlyphSize").toDouble());
    }
  if (settings.contains("Markups/UseGlyphScale"))
    {
    markupsDisplayNode->SetUseGlyphScale(settings.value("Markups/UseGlyphScale").toBool());
    }

  if (settings.contains("Markups/SliceProjection"))
    {
    markupsDisplayNode->SetSliceProjection(settings.value("Markups/SliceProjection").toBool());
    }
  if (settings.contains("Markups/SliceProjectionUseFiducialColor"))
    {
    markupsDisplayNode->SetSliceProjectionUseFiducialColor(settings.value("Markups/SliceProjectionUseFiducialColor").toBool());
    }
  if (settings.contains("Markups/SliceProjectionOutlinedBehindSlicePlane"))
    {
    markupsDisplayNode->SetSliceProjectionOutlinedBehindSlicePlane(settings.value("Markups/SliceProjectionOutlinedBehindSlicePlane").toBool());
    }
  if (settings.contains("Markups/SliceProjectionColor"))
    {
    QVariant variant = settings.value("Markups/SliceProjectionColor");
    QColor qcolor = variant.value<QColor>();
    markupsDisplayNode->SetSliceProjectionColor(qcolor.redF(), qcolor.greenF(), qcolor.blueF());
    }
  if (settings.contains("Markups/SliceProjectionOpacity"))
    {
    markupsDisplayNode->SetSliceProjectionOpacity(settings.value("Markups/SliceProjectionOpacity").toDouble());
    }


  if (settings.contains("Markups/CurveLineSizeMode"))
    {
    markupsDisplayNode->SetCurveLineSizeMode(vtkMRMLMarkupsDisplayNode::GetCurveLineSizeModeFromString(
      settings.value("Markups/CurveLineSizeMode").toString().toUtf8()));
    }
  if (settings.contains("Markups/LineThickness"))
    {
    markupsDisplayNode->SetLineThickness(settings.value("Markups/LineThickness").toDouble());
    }
  if (settings.contains("Markups/LineDiameter"))
    {
    markupsDisplayNode->SetLineDiameter(settings.value("Markups/LineDiameter").toDouble());
    }

  if (settings.contains("Markups/LineColorFadingStart"))
    {
    markupsDisplayNode->SetLineColorFadingStart(settings.value("Markups/LineColorFadingStart").toDouble());
    }
  if (settings.contains("Markups/LineColorFadingEnd"))
    {
    markupsDisplayNode->SetLineColorFadingEnd(settings.value("Markups/LineColorFadingEnd").toDouble());
    }
  if (settings.contains("Markups/LineColorFadingSaturation"))
    {
    markupsDisplayNode->SetLineColorFadingSaturation(settings.value("Markups/LineColorFadingSaturation").toDouble());
    }
  if (settings.contains("Markups/LineColorFadingHueOffset"))
    {
    markupsDisplayNode->SetLineColorFadingHueOffset(settings.value("Markups/LineColorFadingHueOffset").toDouble());
    }

  if (settings.contains("Markups/OccludedVisibility"))
    {
    markupsDisplayNode->SetOccludedVisibility(settings.value("Markups/OccludedVisibility").toBool());
    }
  if (settings.contains("Markups/OccludedOpacity"))
    {
    markupsDisplayNode->SetOccludedOpacity(settings.value("Markups/OccludedOpacity").toDouble());
    }

  if (settings.contains("Markups/TextProperty"))
    {
    vtkMRMLMarkupsDisplayNode::UpdateTextPropertyFromString(
      settings.value("Markups/TextProperty").toString().toStdString(),
      markupsDisplayNode->GetTextProperty());
    }

  if (settings.contains("Markups/SelectedColor"))
    {
    QVariant variant = settings.value("Markups/SelectedColor");
    QColor qcolor = variant.value<QColor>();
    markupsDisplayNode->SetSelectedColor(qcolor.redF(), qcolor.greenF(), qcolor.blueF());
    }
  if (settings.contains("Markups/UnselectedColor"))
    {
    QVariant variant = settings.value("Markups/UnselectedColor");
    QColor qcolor = variant.value<QColor>();
    markupsDisplayNode->SetColor(qcolor.redF(), qcolor.greenF(), qcolor.blueF());
    }
  if (settings.contains("Markups/ActiveColor"))
    {
    QVariant variant = settings.value("Markups/ActiveColor");
    QColor qcolor = variant.value<QColor>();
    markupsDisplayNode->SetActiveColor(qcolor.redF(), qcolor.greenF(), qcolor.blueF());
    }
  if (settings.contains("Markups/Opacity"))
    {
    markupsDisplayNode->SetOpacity(settings.value("Markups/Opacity").toDouble());
    }
  if (settings.contains("Markups/InteractionHandleScale"))
    {
    markupsDisplayNode->SetInteractionHandleScale(settings.value("Markups/InteractionHandleScale").toDouble());
    }

}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModule::writeDefaultMarkupsDisplaySettings(vtkMRMLMarkupsDisplayNode* markupsDisplayNode)
{
  if (!markupsDisplayNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: markupsDisplayNode is invalid";
    return;
    }
  QSettings settings;

  settings.setValue("Markups/SnapMode", vtkMRMLMarkupsDisplayNode::GetSnapModeAsString(
    markupsDisplayNode->GetSnapMode()));
  settings.setValue("Markups/FillVisibility", markupsDisplayNode->GetFillVisibility());
  settings.setValue("Markups/OutlineVisibility", markupsDisplayNode->GetOutlineVisibility());
  settings.setValue("Markups/FillOpacity", markupsDisplayNode->GetFillOpacity());
  settings.setValue("Markups/OutlineOpacity", markupsDisplayNode->GetOutlineOpacity());
  settings.setValue("Markups/TextScale", markupsDisplayNode->GetTextScale());

  settings.setValue("Markups/GlyphType", vtkMRMLMarkupsDisplayNode::GetGlyphTypeAsString(markupsDisplayNode->GetGlyphType()));
  settings.setValue("Markups/GlyphScale", markupsDisplayNode->GetGlyphScale());
  settings.setValue("Markups/GlyphSize", markupsDisplayNode->GetGlyphSize());
  settings.setValue("Markups/UseGlyphScale", markupsDisplayNode->GetUseGlyphScale());

  settings.setValue("Markups/SliceProjection", markupsDisplayNode->GetSliceProjection());
  settings.setValue("Markups/SliceProjectionUseFiducialColor", markupsDisplayNode->GetSliceProjectionUseFiducialColor());
  settings.setValue("Markups/SliceProjectionOutlinedBehindSlicePlane", markupsDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane());
  double* color = markupsDisplayNode->GetSliceProjectionColor();
  settings.setValue("Markups/SliceProjectionColor", QColor::fromRgbF(color[0], color[1], color[2]));
  settings.setValue("Markups/SliceProjectionOpacity", markupsDisplayNode->GetSliceProjectionOpacity());

  settings.setValue("Markups/CurveLineSizeMode", vtkMRMLMarkupsDisplayNode::GetCurveLineSizeModeAsString(markupsDisplayNode->GetCurveLineSizeMode()));
  settings.setValue("Markups/LineThickness", markupsDisplayNode->GetLineThickness());
  settings.setValue("Markups/LineDiameter", markupsDisplayNode->GetLineDiameter());

  settings.setValue("Markups/LineColorFadingStart", markupsDisplayNode->GetLineColorFadingStart());
  settings.setValue("Markups/LineColorFadingEnd", markupsDisplayNode->GetLineColorFadingEnd());
  settings.setValue("Markups/LineColorFadingSaturation", markupsDisplayNode->GetLineColorFadingSaturation());
  settings.setValue("Markups/LineLineColorFadingHueOffset", markupsDisplayNode->GetLineColorFadingHueOffset());

  settings.setValue("Markups/OccludedVisibility", markupsDisplayNode->GetOccludedVisibility());
  settings.setValue("Markups/OccludedOpacity", markupsDisplayNode->GetOccludedOpacity());

  settings.setValue("Markups/TextProperty", QString::fromStdString(
    vtkMRMLDisplayNode::GetTextPropertyAsString(markupsDisplayNode->GetTextProperty())));

  color = markupsDisplayNode->GetSelectedColor();
  settings.setValue("Markups/SelectedColor", QColor::fromRgbF(color[0], color[1], color[2]));
  color = markupsDisplayNode->GetColor();
  settings.setValue("Markups/UnselectedColor", QColor::fromRgbF(color[0], color[1], color[2]));
  color = markupsDisplayNode->GetActiveColor();
  settings.setValue("Markups/ActiveColor", QColor::fromRgbF(color[0], color[1], color[2]));
  settings.setValue("Markups/Opacity", markupsDisplayNode->GetOpacity());

  settings.setValue("Markups/InteractionHandleScale", markupsDisplayNode->GetInteractionHandleScale());
}

//-----------------------------------------------------------------------------
qMRMLMarkupsToolBar* qSlicerMarkupsModule::toolBar()
{
  Q_D(qSlicerMarkupsModule);
  return d->ToolBar;
}

//-----------------------------------------------------------------------------
QDockWidget *qSlicerMarkupsModule::measurementPanelDockWidget()
{
  Q_D(qSlicerMarkupsModule);
  return d->MeasurementPanelDockWidget;
}

//-----------------------------------------------------------------------------
qSlicerMarkupsMeasurementsWidget *qSlicerMarkupsModule::measurementPanelWidget()
{
  Q_D(qSlicerMarkupsModule);
  return d->MeasurementPanelWidget;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModule::setToolBarVisible(bool visible)
{
  Q_D(qSlicerMarkupsModule);
  d->ToolBar->setVisible(visible);
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsModule::isToolBarVisible()
{
  Q_D(qSlicerMarkupsModule);
  return d->ToolBar->isVisible();
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsModule::autoShowToolBar()
{
  Q_D(qSlicerMarkupsModule);
  return d->AutoShowToolBar;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModule::setAutoShowToolBar(bool autoShow)
{
  Q_D(qSlicerMarkupsModule);
  d->AutoShowToolBar = autoShow;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModule::onMarkupsMeasurementsPanelToggled(bool toggled)
{
  Q_D(qSlicerMarkupsModule);
  if (toggled)
    {
    d->MeasurementPanelDockWidget->activateWindow();
    }
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsModule::showMarkups(vtkMRMLMarkupsNode* vtkNotUsed(markupsNode))
{
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  if (!app
      || !app->moduleManager()
      || !dynamic_cast<qSlicerMarkupsModule*>(app->moduleManager()->module("Markups")))
    {
    qCritical("Markups module is not available");
    return false;
    }
  qSlicerMarkupsModule* markupsModule = dynamic_cast<qSlicerMarkupsModule*>(app->moduleManager()->module("Markups"));
  if (markupsModule->autoShowToolBar())
    {
    markupsModule->setToolBarVisible(true);
    }
  return true;
}

/*
// --------------------------------------------------------------------------
void qSlicerMarkupsModule::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerMarkupsModule);

  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return;
    }

  qSlicerMarkupsModule::showMarkups(markupsNode);
}
*/
