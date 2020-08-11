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

//-----------------------------------------------------------------------------
class qSlicerCLIModulePrivate
{
public:
  typedef qSlicerCLIModulePrivate Self;
  qSlicerCLIModulePrivate();

  QString           TempDirectory;

  ModuleDescription                 Desc;
};

//-----------------------------------------------------------------------------
// qSlicerCLIModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCLIModulePrivate::qSlicerCLIModulePrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerCLIModule methods

//-----------------------------------------------------------------------------
qSlicerCLIModule::qSlicerCLIModule(QWidget* _parent):Superclass(_parent)
  , d_ptr(new qSlicerCLIModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCLIModule::~qSlicerCLIModule() = default;

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
  bool developerModeEnabled = settings.value("Developer/PreserveCLIModuleDataFiles", false).toBool();
  if (developerModeEnabled)
    {
    logic->DeleteTemporaryFilesOff();
    }

  if (d->Desc.GetParameterValue("AllowInMemoryTransfer") == "false")
    {
    logic->SetAllowInMemoryTransfer(0);
    }

  return logic;
}

//-----------------------------------------------------------------------------
QString qSlicerCLIModule::title()const
{
  Q_D(const qSlicerCLIModule);
  return QString::fromStdString(d->Desc.GetTitle());
}

//-----------------------------------------------------------------------------
QStringList qSlicerCLIModule::categories()const
{
  Q_D(const qSlicerCLIModule);
  return QString::fromStdString(d->Desc.GetCategory()).split(';');
}

//-----------------------------------------------------------------------------
QStringList qSlicerCLIModule::contributors()const
{
  Q_D(const qSlicerCLIModule);
  return QStringList() << QString::fromStdString(d->Desc.GetContributor());
}

//-----------------------------------------------------------------------------
int qSlicerCLIModule::index()const
{
  Q_D(const qSlicerCLIModule);
  bool ok = false;
  int index = QString::fromStdString(d->Desc.GetIndex()).toInt(&ok);
  return (ok ? index : -1);
}

//-----------------------------------------------------------------------------
QString qSlicerCLIModule::acknowledgementText()const
{
  Q_D(const qSlicerCLIModule);
  return QString::fromStdString(d->Desc.GetAcknowledgements());
}

//-----------------------------------------------------------------------------
QImage qSlicerCLIModule::logo()const
{
  Q_D(const qSlicerCLIModule);
  return this->moduleLogoToImage(d->Desc.GetLogo());
}

//-----------------------------------------------------------------------------
QString qSlicerCLIModule::helpText()const
{
  Q_D(const qSlicerCLIModule);
  QString help = QString::fromStdString(d->Desc.GetDescription());
  if (!d->Desc.GetDocumentationURL().empty())
    {
    help += QString("<p>For more information see the <a href=\"%1\">online documentation</a>.</p>")
      .arg(QString::fromStdString(d->Desc.GetDocumentationURL()));
    }
  return help;
}

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerCLIModule, const QString&, setTempDirectory, TempDirectory);
CTK_GET_CPP(qSlicerCLIModule, QString, tempDirectory, TempDirectory);

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setEntryPoint(const QString& entryPoint)
{
  Q_D(qSlicerCLIModule);
  d->Desc.SetTarget(entryPoint.toStdString());
}

//-----------------------------------------------------------------------------
QString qSlicerCLIModule::entryPoint()const
{
  Q_D(const qSlicerCLIModule);
  return QString::fromStdString(d->Desc.GetTarget());
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setModuleType(const QString& moduleType)
{
  Q_D(qSlicerCLIModule);
  d->Desc.SetType(moduleType.toStdString());
}

//-----------------------------------------------------------------------------
QString qSlicerCLIModule::moduleType()const
{
  Q_D(const qSlicerCLIModule);
  return QString::fromStdString(d->Desc.GetType());
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setXmlModuleDescription(const QString& xmlModuleDescription)
{
  Q_D(qSlicerCLIModule);
  //qDebug() << "xmlModuleDescription:" << xmlModuleDescription;
  Q_ASSERT(!this->entryPoint().isEmpty());

  // Parse module description
  ModuleDescriptionParser parser;
  if (parser.Parse(xmlModuleDescription.toStdString(), d->Desc) != 0)
    {
    qWarning() << "Failed to parse xml module description:\n"
               << xmlModuleDescription;
    return;
    }

  // Set properties

  // Register the module description in the master list
  vtkMRMLCommandLineModuleNode::RegisterModuleDescription(d->Desc);
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
  d->Desc.SetLogo(logo);
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

//-----------------------------------------------------------------------------
ModuleDescription& qSlicerCLIModule::moduleDescription()
{
  Q_D(qSlicerCLIModule);
  return d->Desc;
}

//-----------------------------------------------------------------------------
QStringList qSlicerCLIModule::associatedNodeTypes()const
{
  return QStringList() << "vtkMRMLCommandLineModuleNode";
}
