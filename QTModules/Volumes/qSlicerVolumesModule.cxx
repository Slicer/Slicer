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

// Qt includes
#include <QtPlugin>

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerCoreIOManager.h>

// Volumes Logic includes
#include <vtkSlicerVolumesLogic.h>

// Volumes QTModule includes
#include "qSlicerVolumesIO.h"
#include "qSlicerVolumesModule.h"
#include "qSlicerVolumesModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerVolumesModule, qSlicerVolumesModule);

//-----------------------------------------------------------------------------
class qSlicerVolumesModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerVolumesModule::qSlicerVolumesModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumesModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumesModule::~qSlicerVolumesModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerVolumesModule::helpText()const
{
  QString help = QString(
    "The Volumes Module loads and adjusts display parameters of volume data.\n"
    "<a>%1/Documentation-3.6</a>\n\nThe Diffusion Editor allows modifying "
    "parameters (gradients, bValues, measurement frame) of DWI data and "
    "provides a quick way to interpret them. "
    "For that it estimates a tensor and shows glyphs and tracts "
    "for visual exploration. Help for Diffusion Editor: "
    "<a>%1/Modules:Volumes:Diffusion_Editor-Documentation</a>");
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerVolumesModule::acknowledgementText()const
{
  QString acknowledgement = QString(
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer "
    "Community. See <a>http://www.slicer.org</a> for details.\n"
    "The Volumes module was contributed by Alex Yarmarkovich, Isomics Inc. "
    "(Steve Pieper) with help from others at SPL, BWH (Ron Kikinis). \n\n"
    "The Diffusion Editor was developed by Kerstin Kessel.");
  return acknowledgement;
}

//-----------------------------------------------------------------------------
QIcon qSlicerVolumesModule::icon()const
{
  return QIcon(":/Icons/Volumes.png");
}

//-----------------------------------------------------------------------------
void qSlicerVolumesModule::setup()
{
  this->Superclass::setup();
  qSlicerCoreApplication::application()->coreIOManager()->registerIO(
    new qSlicerVolumesIO(this));
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerVolumesModule::createWidgetRepresentation()
{
  return new qSlicerVolumesModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVolumesModule::createLogic()
{
  return vtkSlicerVolumesLogic::New();
}
