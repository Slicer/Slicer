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
#include <QtPlugin>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerNodeWriter.h"
#include "vtkSlicerTransformLogic.h"

// Transforms includes
#include "qSlicerTransformsIO.h"
#include "qSlicerTransformsModule.h"
#include "qSlicerTransformsModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerTransformsModule, qSlicerTransformsModule);


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
QStringList qSlicerTransformsModule::categories() const
{
  return QStringList() << "" << "Registration";
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
    "<a href=%1/Documentation/%2.%3/Modules/Transforms>%1/Documentation/%2.%3/Modules/Transforms</a>";
  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsModule::acknowledgementText()const
{
  QString acknowledgement =
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer "
    "Community.<br>"
    "See <a href=\"http://www.slicer.org\">www.slicer.org</a> for details.<br>"
    "The Transforms module was contributed by Alex Yarmarkovich, Isomics Inc. "
    "with help from others at SPL, BWH (Ron Kikinis) and Kitware Inc.";
  return acknowledgement;
}

//-----------------------------------------------------------------------------
QStringList qSlicerTransformsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Alex Yarmarkovich (Isomics)");
  moduleContributors << QString("Jean-Christophe Fillion-Robin (Kitware)");
  moduleContributors << QString("Julien Finet (Kitware)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModule::setup()
{
  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  vtkSlicerTransformLogic* transformLogic =
    vtkSlicerTransformLogic::SafeDownCast(this->logic());
  app->coreIOManager()->registerIO(
    new qSlicerTransformsIO(transformLogic, this));
  app->coreIOManager()->registerIO(new qSlicerNodeWriter(
    "Transforms", QString("TransformFile"),
    QStringList() << "vtkMRMLTransformNode", this));
}
