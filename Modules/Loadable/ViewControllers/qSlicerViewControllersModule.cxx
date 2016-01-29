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
#include <QtPlugin>
#include <QSettings>

// SlicerQt includes

// Slices QTModule includes
#include "qSlicerViewControllersModule.h"
#include "qSlicerViewControllersModuleWidget.h"

#include "vtkSlicerViewControllersLogic.h"

#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerViewControllersModule, qSlicerViewControllersModule);

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
