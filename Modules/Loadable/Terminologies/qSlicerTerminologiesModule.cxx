/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Terminologies includes
#include "qSlicerTerminologiesModule.h"
#include "qSlicerTerminologiesModuleWidget.h"
#include "qSlicerTerminologiesReader.h"
#include "vtkSlicerTerminologiesModuleLogic.h"

// Qt includes
#include <QDebug> 
#include <QDir>

// Slicer includes
#include <qSlicerApplication.h>
#include <qSlicerModuleManager.h>
#include "qSlicerIOManager.h"

//-----------------------------------------------------------------------------
/// \ingroup SlicerRt_QtModules_Terminologies
class qSlicerTerminologiesModulePrivate
{
public:
  qSlicerTerminologiesModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTerminologiesModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTerminologiesModulePrivate::qSlicerTerminologiesModulePrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerTerminologiesModule methods

//-----------------------------------------------------------------------------
qSlicerTerminologiesModule::qSlicerTerminologiesModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTerminologiesModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTerminologiesModule::~qSlicerTerminologiesModule() = default;

//-----------------------------------------------------------------------------
QString qSlicerTerminologiesModule::helpText()const
{
  QString help = 
    "The Terminologies module enables viewing and editing terminology dictionaries used for segmentation"
    "For more information see the <a href=\"%1/Documentation/%2.%3/Modules/Terminologies\">online documentation</a>.<br>";
  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
}

//-----------------------------------------------------------------------------
QString qSlicerTerminologiesModule::acknowledgementText()const
{
  return "This work is part of SparKit project, funded by Cancer Care Ontario (CCO)'s ACRU program and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTerminologiesModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Csaba Pinter (Queen's)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QStringList qSlicerTerminologiesModule::categories()const
{
  return QStringList() << "Informatics";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTerminologiesModule::dependencies()const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologiesModule::setup()
{
  this->Superclass::setup();

  vtkSlicerTerminologiesModuleLogic* terminologiesLogic = vtkSlicerTerminologiesModuleLogic::SafeDownCast(this->logic());

  // Register IOs
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  ioManager->registerIO(new qSlicerTerminologiesReader(terminologiesLogic, this));
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerTerminologiesModule::createWidgetRepresentation()
{
  return new qSlicerTerminologiesModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTerminologiesModule::createLogic()
{
  // Determine the settings folder path
  QDir settingsDir(qSlicerCoreApplication::application()->slicerUserSettingsFilePath());
  settingsDir.cdUp(); // Remove Slicer.ini from the path
  QString settingsDirPath = settingsDir.absolutePath();
  settingsDirPath.append("/Terminologies");

  // Setup logic
  vtkSlicerTerminologiesModuleLogic* logic = vtkSlicerTerminologiesModuleLogic::New();
  logic->SetUserContextsPath(settingsDirPath.toUtf8().constData());

  return logic;
}
