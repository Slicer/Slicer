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

// SlicerQt includes
#include "qSlicerROIModule.h"
#include "qSlicerROIModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerROIModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerROIModule::qSlicerROIModule(QObject* parentObject)
  :Superclass(parentObject)
  , d_ptr(new qSlicerROIModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerROIModule::~qSlicerROIModule()
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerROIModule::createWidgetRepresentation()
{
  return new qSlicerROIModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerROIModule::createLogic()
{
  return 0;
}

//-----------------------------------------------------------------------------
QString qSlicerROIModule::helpText()const
{
  QString help =
    "The ROI module creates and edits Region Of Interest (ROI). "
    "An ROI is a rectangle region in the RAS space. It is defined by the "
    "location of it's center and three extents of it's sides. The ROI can be "
    "rotated and/or translated relative to the RAS space by placing it inside "
    "the Transformation node in the Data module. The ROI module allows user to "
    "create new ROI's, change their visibility, and also change the size and "
    "location of ROI's.<br>Region Of Interest (ROI) is used in other modules such "
    "as ClipModels, VolumeRendering, etc. <br>For more information see "
    "<a href=\"%1/Modules:ROI-Module-Documentation-3.6\">%1/Modules:ROIModule-"
    "Documentation-3.6</a>";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerROIModule::acknowledgementText()const
{
  QString about =
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer "
    "Community.<br>"
    "See <a href=\"http://www.slicer.org\">www.slicer.org</a> for details.<br>"
    "The ROI module was contributed by Alex Yarmarkovich, Isomics "
    "Inc. with help from others at SPL, BWH (Ron Kikinis)";
  return about;
}

//-----------------------------------------------------------------------------
QStringList qSlicerROIModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Alex Yarmarkovich (Isomics)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QStringList qSlicerROIModule::categories()const
{
  QStringList moduleCategories;
  moduleCategories << QString();
  return moduleCategories;
}
