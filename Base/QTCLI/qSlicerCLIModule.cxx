/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#include "qSlicerCLIModule.h"

// SlicerQT includes
#include "vtkSlicerCLIModuleLogic.h"
#include "qSlicerCLIModuleWidget.h"

// ModuleDescriptionParser includes
#include <ModuleDescription.h>
#include <ModuleDescriptionParser.h>

//-----------------------------------------------------------------------------
class qSlicerCLIModulePrivate: public qCTKPrivate<qSlicerCLIModule>
{
public:
  QCTK_DECLARE_PUBLIC(qSlicerCLIModule);
  typedef qSlicerCLIModulePrivate Self;
  qSlicerCLIModulePrivate();

  QString           Title;
  QString           Acknowledgement;
  QString           Help;
  QString           Category;
  QString           Contributor;
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
}

//-----------------------------------------------------------------------------
// qSlicerCLIModule methods

//-----------------------------------------------------------------------------
qSlicerCLIModule::qSlicerCLIModule(QWidget* _parent):Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qSlicerCLIModule);
}

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setup()
{
  QCTK_D(qSlicerCLIModule);
  
  // Temporary directory should be set before the module is initialized
  Q_ASSERT(!d->TempDirectory.isEmpty());

  // Set temp directory 
  vtkSlicerCLIModuleLogic* myLogic = vtkSlicerCLIModuleLogic::SafeDownCast(this->logic());
  myLogic->SetTemporaryDirectory(d->TempDirectory.toLatin1());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget * qSlicerCLIModule::createWidgetRepresentation()
{
  QCTK_D(qSlicerCLIModule);
  return new qSlicerCLIModuleWidget(&d->Desc);
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerCLIModule::createLogic()
{
  return vtkSlicerCLIModuleLogic::New();
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerCLIModule, QString, title, Title);
QCTK_GET_CXX(qSlicerCLIModule, QString, category, Category);
QCTK_GET_CXX(qSlicerCLIModule, QString, contributor, Contributor);
QCTK_GET_CXX(qSlicerCLIModule, QString, acknowledgementText, Acknowledgement);
QCTK_GET_CXX(qSlicerCLIModule, QString, helpText, Help);
QCTK_SET_CXX(qSlicerCLIModule, const QString&, setTempDirectory, TempDirectory);

//-----------------------------------------------------------------------------
void qSlicerCLIModule::setXmlModuleDescription(const char* xmlModuleDescription)
{
  QCTK_D(qSlicerCLIModule);
  //qDebug() << "xmlModuleDescription:" << xmlModuleDescription;

  // Parse module description
  ModuleDescription desc;
  ModuleDescriptionParser parser;
  if (parser.Parse(std::string(xmlModuleDescription), desc) != 0)
    {
    qWarning() << "Failed to parse xml module description";
    return;
    }

  // Set properties
  d->Title = QString::fromStdString(desc.GetTitle());
  d->Acknowledgement = QString::fromStdString(desc.GetAcknowledgements());
  d->Contributor = QString::fromStdString(desc.GetContributor());
  d->Category = QString::fromStdString(desc.GetCategory());

  d->ProcessInformation = desc.GetProcessInformation();

  QString help =
    "%1<br>"
    "For more detailed documentation see:<br>"
    "%2";

  d->Help = help.arg(
    QString::fromStdString(desc.GetDescription())).arg(
    QString::fromStdString(desc.GetDocumentationURL()));

  d->Desc = desc; 
}
