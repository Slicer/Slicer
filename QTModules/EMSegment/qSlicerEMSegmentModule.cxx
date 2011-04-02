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

// EMSegment Logic includes
#include <vtkSlicerEMSegmentLogic.h>

// EMSegment QTModule includes
#include "qSlicerEMSegmentModule.h"
#include "qSlicerEMSegmentModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerEMSegmentModule, qSlicerEMSegmentModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentModule::qSlicerEMSegmentModule(QObject* _parent)
  :Superclass(_parent)
  , d_ptr(new qSlicerEMSegmentModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentModule::~qSlicerEMSegmentModule()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentModule::helpText()const
{
  QString help =
    "EMSegment Module: Use this module to segment a set of images (target images) using the "
    "tree-based EM segmentation algorithm of K. Pohl et al."
    "<ul>"
    "<li>Use the pull down menu to select a collection of parameters to edit"
    " (or create a new collection).</li>"
    "<li>Use the <b>Back</b> and <b>Next</b> to navigate through the "
    "stages of filling in the algorithm parameters.</li>"
    "<li>When the parameters are specified, use the button on the "
    "last step to start the segmentation process.</li>"
    "</ul>"
    "See <a href=%1/Modules:EMSegmentTemplateBuilder3.6>Modules:EMSegmentTemplateBuilder3.6</a>";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerEMSegmentModule::createWidgetRepresentation()
{
  return new qSlicerEMSegmentModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerEMSegmentModule::createLogic()
{
  return vtkSlicerEMSegmentLogic::New();
}

//-----------------------------------------------------------------------------
QIcon qSlicerEMSegmentModule::icon() const
{
  return QIcon(":/Icons/EMSegment.png");
}
