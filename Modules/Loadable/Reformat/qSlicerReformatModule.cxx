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

  This file was originally developed by Michael Jeulin-Lagarrigue, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QtPlugin>

// SlicerQt includes
#include "vtkSlicerTransformLogic.h"

// Reformat Logic includes
#include <vtkSlicerReformatLogic.h>

// Reformat includes
#include "qSlicerReformatModule.h"
#include "qSlicerReformatModuleWidget.h"

//------------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerReformatModule,
                 qSlicerReformatModule);

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Reformat
class qSlicerReformatModulePrivate
{
public:
  qSlicerReformatModulePrivate();
};

//------------------------------------------------------------------------------
// qSlicerReformatModulePrivate methods

//------------------------------------------------------------------------------
qSlicerReformatModulePrivate::qSlicerReformatModulePrivate()
{
}

//------------------------------------------------------------------------------
// qSlicerReformatModule methods

//------------------------------------------------------------------------------
qSlicerReformatModule::
qSlicerReformatModule(QObject* _parent) : Superclass(_parent),
  d_ptr(new qSlicerReformatModulePrivate)
{
}

//------------------------------------------------------------------------------
qSlicerReformatModule::~qSlicerReformatModule()
{
}

//------------------------------------------------------------------------------
QString qSlicerReformatModule::helpText()const
{
  QString help = 
      "The Transforms Reformat Widget Module creates "
      "and edits the Slice Node transforms.<br>"
      "<a href=\"%1/Documentation/4.0/Modules/Reformat\">"
      "%1/Documentation/4.0/Modules/Reformat</a>\n";
  return help.arg(this->slicerWikiUrl());
}

//------------------------------------------------------------------------------
QString qSlicerReformatModule::acknowledgementText()const
{
  QString acknowledgement =
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer "
    "Community.<br>"
    "See <a href=\"http://www.slicer.org\">www.slicer.org</a> for details.<br>"
    "The Slice Node Transform module was contributed by "
    "Michael Jeulin-Lagarrigue, Kitware Inc.";
  return acknowledgement;
}

//------------------------------------------------------------------------------
QIcon qSlicerReformatModule::icon()const
{
  return QIcon(":/Icons/Reformat.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerReformatModule::categories()const
{
  return QStringList() << "Registration.Specialized";
}

//------------------------------------------------------------------------------
void qSlicerReformatModule::setup()
{
  this->Superclass::setup();
}

//------------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerReformatModule::
createWidgetRepresentation()
{
  return new qSlicerReformatModuleWidget;
}

//------------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerReformatModule::createLogic()
{
  return vtkSlicerReformatLogic::New();
}
