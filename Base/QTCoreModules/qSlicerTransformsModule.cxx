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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qSlicerTransformsModule.h"

// SlicerQt includes
#include "vtkSlicerTransformLogic.h"
#include "qSlicerTransformsModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerTransformsModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerTransformsModule::qSlicerTransformsModule(QObject* _parentObject)
  : Superclass(_parentObject)
  , d_ptr(new qSlicerTransformsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTransformsModule::~qSlicerTransformsModule()
{
}

//-----------------------------------------------------------------------------
QIcon qSlicerTransformsModule::icon()const
{
  return QIcon(":/Icons/Transforms.png");
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerTransformsModule::createWidgetRepresentation()
{
  return new qSlicerTransformsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTransformsModule::createLogic()
{
  return vtkSlicerTransformLogic::New();
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsModule::helpText()const
{
  QString help =
    "The Transforms Module creates and edits transforms.<br>"
    "<a href=%1/Modules:Transforms-Documentation-3.4>%1/Modules:Transforms-"
    "Documentation-3.6</a>";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsModule::acknowledgementText()const
{
  QString acknowledgement =
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer "
    "Community.<br>"
    "See <a href=\"http://www.slicer.org\">www.slicer.org</a> for details.<br>"
    "The Transforms module was contributed by Alex Yarmarkovich, Isomics Inc. "
      "with help from others at SPL, BWH (Ron Kikinis)<br>";
  return acknowledgement;
}
