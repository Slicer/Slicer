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
#include <QSettings>

// Markups Logic includes
#include <vtkSlicerMarkupsLogic.h>

// MRMLDisplayableManager includes
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>

// QTGUI includes
#include <qSlicerApplication.h>
//#include <qSlicerCoreApplication.h>
#include <qSlicerIOManager.h>
#include <qSlicerNodeWriter.h>

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyMarkupsPlugin.h"

// Markups includes
#include "qSlicerMarkupsModule.h"
#include "qSlicerMarkupsModuleWidget.h"
#include "qSlicerMarkupsReader.h"
#include "qSlicerMarkupsWriter.h"
//#include "qSlicerMarkupsSettingsPanel.h"
//#include "vtkSlicerMarkupsLogic.h"
#include "vtkMRMLMarkupsDisplayNode.h"

// DisplayableManager initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkSlicerMarkupsModuleMRMLDisplayableManager)

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markups
class qSlicerMarkupsModulePrivate
{
public:
  qSlicerMarkupsModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerMarkupsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModulePrivate::qSlicerMarkupsModulePrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerMarkupsModule methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModule::qSlicerMarkupsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMarkupsModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsModule::categories()const
{
  return QStringList() << "" << "Informatics";
}

//-----------------------------------------------------------------------------
qSlicerMarkupsModule::~qSlicerMarkupsModule() = default;

//-----------------------------------------------------------------------------
QString qSlicerMarkupsModule::helpText()const
{
  QString help =
    "A module to create and manage markups in 2D and 3D."
    " Replaces the Annotations module for fiducials.";
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
  this->Superclass::setup();

  // Register displayable managers (same displayable manager handles both slice and 3D views)
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager("vtkMRMLMarkupsDisplayableManager");
  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager("vtkMRMLMarkupsDisplayableManager");

  // Register IO
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  qSlicerMarkupsReader *markupsReader = new qSlicerMarkupsReader(vtkSlicerMarkupsLogic::SafeDownCast(this->logic()), this);
  ioManager->registerIO(markupsReader);
  ioManager->registerIO(new qSlicerMarkupsWriter(this));

  // settings
  /*
  if (qSlicerApplication::application())
    {
    qSlicerMarkupsSettingsPanel* panel =
      new qSlicerMarkupsSettingsPanel;
    qSlicerApplication::application()->settingsDialog()->addPanel(
      "Markups", panel);
    panel->setMarkupsLogic(vtkSlicerMarkupsLogic::SafeDownCast(this->logic()));
    }
  */

  // Register Subject Hierarchy core plugins
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyMarkupsPlugin());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerMarkupsModule::createWidgetRepresentation()
{
  return new qSlicerMarkupsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerMarkupsModule::createLogic()
{
  return vtkSlicerMarkupsLogic::New();
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsModule::associatedNodeTypes() const
{
  return QStringList()
    << "vtkMRMLAnnotationFiducialNode"
    << "vtkMRMLMarkupsDisplayNode"
    << "vtkMRMLMarkupsFiducialNode"
    << "vtkMRMLMarkupsLineNode"
    << "vtkMRMLMarkupsAngleNode"
    << "vtkMRMLMarkupsCurveNode"
    << "vtkMRMLMarkupsClosedCurveNode"
    << "vtkMRMLMarkupsPlaneNode"
    << "vtkMRMLMarkupsROINode"
    << "vtkMRMLMarkupsFiducialStorageNode"
    << "vtkMRMLMarkupsJsonStorageNode";
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
  if (settings.contains("Markups/PropertiesLabelVisibility"))
    {
    markupsDisplayNode->SetPropertiesLabelVisibility(settings.value("Markups/PropertiesLabelVisibility").toBool());
    }
  if (settings.contains("Markups/PointLabelsVisibility"))
    {
    markupsDisplayNode->SetPointLabelsVisibility(settings.value("Markups/PointLabelsVisibility").toBool());
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
    markupsDisplayNode->SetGlyphType(vtkMRMLMarkupsDisplayNode::GetGlyphTypeFromString(
      settings.value("Markups/GlyphType").toString().toUtf8()));
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
    markupsDisplayNode->SetColor(qcolor.redF(), qcolor.greenF(), qcolor.blueF());
    }
  if (settings.contains("Markups/Opacity"))
    {
    markupsDisplayNode->SetOpacity(settings.value("Markups/Opacity").toDouble());
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
  settings.setValue("Markups/PropertiesLabelVisibility", markupsDisplayNode->GetPropertiesLabelVisibility());

  settings.setValue("Markups/PointLabelsVisibility", markupsDisplayNode->GetPointLabelsVisibility());
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
    vtkMRMLMarkupsDisplayNode::GetTextPropertyAsString(markupsDisplayNode->GetTextProperty())));

  color = markupsDisplayNode->GetSelectedColor();
  settings.setValue("Markups/SelectedColor", QColor::fromRgbF(color[0], color[1], color[2]));
  color = markupsDisplayNode->GetColor();
  settings.setValue("Markups/UnselectedColor", QColor::fromRgbF(color[0], color[1], color[2]));
  color = markupsDisplayNode->GetActiveColor();
  settings.setValue("Markups/ActiveColor", QColor::fromRgbF(color[0], color[1], color[2]));
  settings.setValue("Markups/Opacity", markupsDisplayNode->GetOpacity());
}
