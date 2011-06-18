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

// SlicerQt includes
#include "qSlicerEventBrokerModule.h"
#include "qSlicerEventBrokerModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerEventBrokerModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerEventBrokerModule::qSlicerEventBrokerModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerEventBrokerModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerEventBrokerModule::~qSlicerEventBrokerModule()
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerEventBrokerModule::createWidgetRepresentation()
{
  return new qSlicerEventBrokerModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerEventBrokerModule::createLogic()
{
  return 0;
}

//-----------------------------------------------------------------------------
QString qSlicerEventBrokerModule::category()const
{
  return "Developer Tools";
}

//-----------------------------------------------------------------------------
QString qSlicerEventBrokerModule::helpText()const
{
  QString help = "%1";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerEventBrokerModule::acknowledgementText()const
{
  QString acknowledgement = "This module was developed by Julien Finet, Kitware Inc. "
      "This work was supported by NIH grant 3P41RR013218-12S1, "
      "NA-MIC, NAC and Slicer community.";
  return acknowledgement;
}
