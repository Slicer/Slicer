/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
#include "qSlicerDataModule.h"
#include "qSlicerDataModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerDataModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerDataModule::qSlicerDataModule(QObject* parentObject)
  :Superclass(parentObject)
  , d_ptr(new qSlicerDataModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerDataModule::~qSlicerDataModule()
{
}

//-----------------------------------------------------------------------------
QIcon qSlicerDataModule::icon()const
{
  return QIcon(":/Icons/Data.png");
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerDataModule::createWidgetRepresentation()
{
  return new qSlicerDataModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerDataModule::createLogic()
{
  return 0;
}

//-----------------------------------------------------------------------------
QString qSlicerDataModule::helpText()const
{
  QString help =
    "The Data Module displays and permits operations on the MRML tree, and "
    "creates and edits transformation hierarchies.<br>"
    "The Load panels exposes options for loading data. Helpful comments can be "
    "opened by clicking on the \"information\" icons in each load panel.<br>"
    "<a href=\"%1/Modules:Data-Documentation-3.6\">%1/Modules:Data-Documentation"
    "-3.6</a>";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerDataModule::acknowledgementText()const
{
  QString about =
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, CTSC, and the Slicer "
    "Community.<br>"
    "See <a href=\"http://www.slicer.org\">www.slicer.org</a> for details.<br>"
    "The Data module was contributed by Alex Yarmarkovich, Isomics Inc. with "
    "help from others at SPL, BWH (Ron Kikinis, Wendy Plesniak)";
  return about;
}
