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

#include "qSlicerCLIModule.h"

// Qt includes
#include <QDebug>
#include <QSettings>

// CTK includes
#include <ctkWidgetsUtils.h>

// Slicer includes
#include "qMRMLNodeComboBox.h"
#include "qSlicerCLIModuleWidget.h"
#include "vtkSlicerCLIModuleLogic.h"

// SlicerExecutionModel includes
#include <ModuleDescription.h>
#include <ModuleDescriptionParser.h>
#include <ModuleLogo.h>
#include <ModuleProcessInformation.h>

//-----------------------------------------------------------------------------
class qSlicerCLIModulePrivate
{
public:
  typedef qSlicerCLIModulePrivate Self;
  qSlicerCLIModulePrivate();

  QString           Title;
  QString           Acknowledgement;
  QString           Help;
  QStringList       Categories;
  QStringList       Contributors;
  QImage            Logo;
  int               Index;

  QString           EntryPoint;
  QString           ModuleType;
  QString           TempDirectory;

  ModuleDescription                 Desc;
  ModuleProcessInformation*         ProcessInformation;
};

//-----------------------------------------------------------------------------
// qSlicerCLIModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCLIModulePrivate::qSlicerCLIModulePrivate()
{
  this->ProcessInformation = 0;
  this->Index = -1;
}

//-----------------------------------------------------------------------------
// qSlicerCLIModule methods

//-----------------------------------------------------------------------------
qSlicerCLIModule::qSlicerCLIModule(QWidget* _parent):Superclass(_parent)
  , d_ptr(new qSlicerCLIModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCLIModule::~qSlicerCLIModule()
{
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setup()
{
#ifndef QT_NO_DEBUG
  Q_D(qSlicerCLIModule);
  // Temporary directory should be set before the module is initialized
  Q_ASSERT(!d->TempDirectory.isEmpty());
#endif
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerCLIModule::createWidgetRepresentation()
{
  return new qSlicerCLIModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerCLIModule::createLogic()
{
  Q_D(qSlicerCLIModule);
  vtkSlicerCLIModuleLogic* logic = vtkSlicerCLIModuleLogic::New();
  logic->SetDefaultModuleDescription(d->Desc);

  // In developer mode keep the CLI modules input and output files
  QSettings settings;
  bool developerModeEnabled = settings.value("Developer/DeveloperMode", false).toBool();
  if (developerModeEnabled)
    {
    logic->DeleteTemporaryFilesOff();
    }

  return logic;
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerCLIModule, QString, title, Title);
CTK_GET_CPP(qSlicerCLIModule, QStringList, categories, Categories);
CTK_GET_CPP(qSlicerCLIModule, QStringList, contributors, Contributors);
CTK_GET_CPP(qSlicerCLIModule, int, index, Index);
CTK_GET_CPP(qSlicerCLIModule, QString, acknowledgementText, Acknowledgement);
CTK_GET_CPP(qSlicerCLIModule, QImage, logo, Logo);
CTK_GET_CPP(qSlicerCLIModule, QString, helpText, Help);
CTK_SET_CPP(qSlicerCLIModule, const QString&, setTempDirectory, TempDirectory);
CTK_GET_CPP(qSlicerCLIModule, QString, tempDirectory, TempDirectory);
CTK_SET_CPP(qSlicerCLIModule, const QString&, setEntryPoint, EntryPoint);
CTK_GET_CPP(qSlicerCLIModule, QString, entryPoint, EntryPoint);
CTK_SET_CPP(qSlicerCLIModule, const QString&, setModuleType, ModuleType);
CTK_GET_CPP(qSlicerCLIModule, QString, moduleType, ModuleType);

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setXmlModuleDescription(const QString& xmlModuleDescription)
{
  Q_D(qSlicerCLIModule);
  //qDebug() << "xmlModuleDescription:" << xmlModuleDescription;
  Q_ASSERT(!this->entryPoint().isEmpty());

  // Parse module description
  ModuleDescription desc;
  ModuleDescriptionParser parser;
  if (parser.Parse(xmlModuleDescription.toStdString(), desc) != 0)
    {
    qWarning() << "Failed to parse xml module description:\n"
               << xmlModuleDescription;
    return;
    }

  // Set properties
  d->Title = QString::fromStdString(desc.GetTitle());
  d->Acknowledgement = QString::fromStdString(desc.GetAcknowledgements());
  d->Contributors = QStringList() << QString::fromStdString(desc.GetContributor());
  d->Logo = this->moduleLogoToImage(desc.GetLogo());
  bool ok = false;
  d->Index = QString::fromStdString(desc.GetIndex()).toInt(&ok);
  if (!ok)
    {
    d->Index = -1;
    }
  d->Categories = QStringList() << QString::fromStdString(desc.GetCategory()).split(';');

  d->ProcessInformation = desc.GetProcessInformation();

  QString help =
    "%1<br>"
    "For more detailed documentation see the online documentation at"
    "<a href=\"%2\">%2</a>";

  d->Help = help.arg(
    QString::fromStdString(desc.GetDescription())).arg(
    QString::fromStdString(desc.GetDocumentationURL()));

  // Set module type
  desc.SetType(this->moduleType().toStdString());

  // Set module entry point
  desc.SetTarget(this->entryPoint().toStdString());

  // Register the module description in the master list
  vtkMRMLCommandLineModuleNode::RegisterModuleDescription(desc);

  d->Desc = desc;
}

//-----------------------------------------------------------------------------
vtkSlicerCLIModuleLogic* qSlicerCLIModule::cliModuleLogic()
{
  vtkSlicerCLIModuleLogic* myLogic = vtkSlicerCLIModuleLogic::SafeDownCast(this->logic());
  return myLogic;
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setLogo(const ModuleLogo& logo)
{
  Q_D(qSlicerCLIModule);
  d->Logo = this->moduleLogoToImage(logo);
}

//-----------------------------------------------------------------------------
QImage qSlicerCLIModule::moduleLogoToImage(const ModuleLogo& logo)
{
  if (logo.GetBufferLength() == 0)
    {
    return QImage();
    }
  return ctk::kwIconToQImage(reinterpret_cast<const unsigned char*>(logo.GetLogo()),
                             logo.GetWidth(), logo.GetHeight(),
                             logo.GetPixelSize(), logo.GetBufferLength(),
                             logo.GetOptions());
}

