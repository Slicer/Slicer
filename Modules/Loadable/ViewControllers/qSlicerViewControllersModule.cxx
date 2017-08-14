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
#include <QSettings>

// SlicerQt includes

// Slices QTModule includes
#include "qSlicerViewControllersModule.h"
#include "qSlicerViewControllersModuleWidget.h"

#include "vtkSlicerViewControllersLogic.h"

#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerViewControllersModule, qSlicerViewControllersModule);
#endif

//-----------------------------------------------------------------------------
class qSlicerViewControllersModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerViewControllersModule::qSlicerViewControllersModule(QObject* _parent)
  :Superclass(_parent)
  , d_ptr(new qSlicerViewControllersModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerViewControllersModule::~qSlicerViewControllersModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerViewControllersModule::acknowledgementText()const
{
  return "This module was developed by Jean-Christophe Fillion-Robin, Kitware Inc. "
         "This work was supported by NIH grant 3P41RR013218-12S1, "
         "NA-MIC, NAC and Slicer community.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerViewControllersModule::categories() const
{
  return QStringList() << "";
}

//-----------------------------------------------------------------------------
QIcon qSlicerViewControllersModule::icon() const
{
  return QIcon(":Icons/ViewControllers.png");
}

//-----------------------------------------------------------------------------
void qSlicerViewControllersModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
void qSlicerViewControllersModule::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  vtkSlicerViewControllersLogic* logic = vtkSlicerViewControllersLogic::SafeDownCast(this->logic());
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << " failed: logic is invalid";
    return;
    }
  // Update default view nodes from settings
  this->readDefaultSliceViewSettings(logic->GetDefaultSliceViewNode());
  this->readDefaultThreeDViewSettings(logic->GetDefaultThreeDViewNode());
  this->writeDefaultSliceViewSettings(logic->GetDefaultSliceViewNode());
  this->writeDefaultThreeDViewSettings(logic->GetDefaultThreeDViewNode());
  // Update all existing view nodes to default
  logic->ResetAllViewNodesToDefault();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerViewControllersModule::createWidgetRepresentation()
{
  return new qSlicerViewControllersModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerViewControllersModule::createLogic()
{
  return vtkSlicerViewControllersLogic::New();
}

//-----------------------------------------------------------------------------
void qSlicerViewControllersModule::readCommonViewSettings(vtkMRMLAbstractViewNode* defaultViewNode, QSettings& settings)
{
  if (settings.contains("OrientationMarkerType"))
    {
    defaultViewNode->SetOrientationMarkerType(vtkMRMLAbstractViewNode::GetOrientationMarkerTypeFromString(settings.value("OrientationMarkerType").toString().toLatin1()));
    }
  if (settings.contains("OrientationMarkerSize"))
    {
    defaultViewNode->SetOrientationMarkerSize(vtkMRMLAbstractViewNode::GetOrientationMarkerSizeFromString(settings.value("OrientationMarkerSize").toString().toLatin1()));
    }
  if (settings.contains("RulerType"))
    {
    defaultViewNode->SetRulerType(vtkMRMLAbstractViewNode::GetRulerTypeFromString(settings.value("RulerType").toString().toLatin1()));
    }
}

//-----------------------------------------------------------------------------
void qSlicerViewControllersModule::writeCommonViewSettings(vtkMRMLAbstractViewNode* defaultViewNode, QSettings& settings)
{
  settings.setValue("OrientationMarkerType",vtkMRMLAbstractViewNode::GetOrientationMarkerTypeAsString(defaultViewNode->GetOrientationMarkerType()));
  settings.setValue("OrientationMarkerSize",vtkMRMLAbstractViewNode::GetOrientationMarkerSizeAsString(defaultViewNode->GetOrientationMarkerSize()));
  settings.setValue("RulerType",vtkMRMLAbstractViewNode::GetRulerTypeAsString(defaultViewNode->GetRulerType()));
}

//-----------------------------------------------------------------------------
void qSlicerViewControllersModule::readDefaultThreeDViewSettings(vtkMRMLViewNode* defaultViewNode)
{
  if (!defaultViewNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: defaultViewNode is invalid";
    return;
    }
  QSettings settings;
  settings.beginGroup("Default3DView");
  if (settings.contains("BoxVisibility"))
    {
    defaultViewNode->SetBoxVisible(settings.value("BoxVisibility").toBool());
    }
  if (settings.contains("AxisLabelsVisibility"))
    {
    defaultViewNode->SetAxisLabelsVisible(settings.value("AxisLabelsVisibility").toBool());
    }
  if (settings.contains("UseOrthographicProjection"))
    {
    defaultViewNode->SetRenderMode(settings.value("UseOrthographicProjection").toBool() ? vtkMRMLViewNode::Orthographic : vtkMRMLViewNode::Perspective);
    }
  if (settings.contains("UseDepthPeeling"))
    {
    defaultViewNode->SetUseDepthPeeling(settings.value("UseDepthPeeling").toBool());
    }
  readCommonViewSettings(defaultViewNode, settings);
}

//-----------------------------------------------------------------------------
void qSlicerViewControllersModule::writeDefaultThreeDViewSettings(vtkMRMLViewNode* defaultViewNode)
{
  if (!defaultViewNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: defaultViewNode is invalid";
    return;
    }
  QSettings settings;
  settings.beginGroup("Default3DView");
  settings.setValue("BoxVisibility", bool(defaultViewNode->GetBoxVisible()));
  settings.setValue("AxisLabelsVisibility", bool(defaultViewNode->GetAxisLabelsVisible()));
  settings.setValue("UseOrthographicProjection", defaultViewNode->GetRenderMode()==vtkMRMLViewNode::Orthographic);
  settings.setValue("UseDepthPeeling", bool(defaultViewNode->GetUseDepthPeeling()));
  writeCommonViewSettings(defaultViewNode, settings);
}

//-----------------------------------------------------------------------------
void qSlicerViewControllersModule::readDefaultSliceViewSettings(vtkMRMLSliceNode* defaultViewNode)
{
  if (!defaultViewNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: defaultViewNode is invalid";
    return;
    }
  QSettings settings;
  settings.beginGroup("DefaultSliceView");
  readCommonViewSettings(defaultViewNode, settings);
}

//-----------------------------------------------------------------------------
void qSlicerViewControllersModule::writeDefaultSliceViewSettings(vtkMRMLSliceNode* defaultViewNode)
{
  if (!defaultViewNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: defaultViewNode is invalid";
    return;
    }
  QSettings settings;
  settings.beginGroup("DefaultSliceView");
  writeCommonViewSettings(defaultViewNode, settings);
}

//-----------------------------------------------------------------------------
QStringList qSlicerViewControllersModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Wendy Plesniak (SPL, BWH)");
  moduleContributors << QString("Jim Miller (GE)");
  moduleContributors << QString("Steve Pieper (Isomics)");
  moduleContributors << QString("Ron Kikinis (SPL, BWH)");
  moduleContributors << QString("Jean-Christophe Fillion-Robin (Kitware)");
  return moduleContributors;
}
