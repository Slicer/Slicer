/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCoreIOManager.h"

// SlicerQT includes
#include "qSlicerCoreApplication.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

// QT includes
#include <QString>
#include <QSettings>
#include <QStringList>
#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerCoreIOManagerPrivate: public qCTKPrivate<qSlicerCoreIOManager>
{
  qSlicerCoreIOManagerPrivate()
    {
    this->ExtensionFileType = new QSettings(":/default-extension-filetype",
                                            QSettings::IniFormat);
    }
  ~qSlicerCoreIOManagerPrivate()
    {
    delete this->ExtensionFileType; 
    }
  
  vtkSmartPointer<vtkMRMLScene>  MRMLScene;
  QSettings*                     ExtensionFileType;
};

//-----------------------------------------------------------------------------
qSlicerCoreIOManager::qSlicerCoreIOManager()
{
  QCTK_INIT_PRIVATE(qSlicerCoreIOManager);
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager::~qSlicerCoreIOManager()
{
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::printAdditionalInfo()
{
  QCTK_D(qSlicerCoreIOManager);
  qDebug() << "ExtensionFileType:";
  d->ExtensionFileType->beginGroup("ExtensionFileType");
  QStringList keys = d->ExtensionFileType->childKeys();
  foreach(const QString& key, keys)
    {
    qDebug() << key << " = " << d->ExtensionFileType->value(key);
    }
}

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerCoreIOManager, vtkMRMLScene*, setMRMLScene, MRMLScene);

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::loadScene(const QString& filename)
{
  QCTK_D(qSlicerCoreIOManager);
  Q_ASSERT(d->MRMLScene);
  
  // Convert to lowercase
  QString filenameLc = filename.toLower();
    
  if (filenameLc.endsWith(".mrml"))
    {
    d->MRMLScene->SetURL(filenameLc.toLatin1());
    d->MRMLScene->Connect();
    }
  else if (filenameLc.endsWith(".xml"))
    {
    qDebug() << "Loading Slicer2Scene... NOT implemented";
    // TODO See ImportSlicer2Scene script
    }
  else if (filenameLc.endsWith(".xcat"))
    {
    qDebug() << "Loading Catalog... NOT implemented";
    // TODO See XcatalogImport script
    }
  else
    {
    qWarning() << "Unknown type of scene file:" << filenameLc; 
    }
  // TODO save last open path
  //this->LoadSceneDialog->SaveLastPathToRegistry("OpenPath");

  if (d->MRMLScene->GetErrorCode() != 0 )
    {
    qDebug() << "Failed to load scene:" << QString::fromStdString(d->MRMLScene->GetErrorMessage());
    }
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::importScene(const QString& filename)
{
  QCTK_D(qSlicerCoreIOManager);
  Q_ASSERT(d->MRMLScene);
  
  // Convert to lowercase
  QString filenameLc = filename.toLower();

  if (filenameLc.endsWith(".mrml"))
    {
    d->MRMLScene->SetURL(filenameLc.toLatin1());
    d->MRMLScene->Import();
    }
  else if (filenameLc.endsWith(".xml"))
    {
    qDebug() << "Importing Slicer2Scene... NOT implemented";
    // TODO See ImportSlicer2Scene script
    }
  else if (filenameLc.endsWith(".xcat"))
    {
    qDebug() << "Importing Catalog... NOT implemented";
    // TODO See XcatalogImport script
    }
  else
    {
    qWarning() << "Unknown type of scene file:" << filenameLc; 
    }
  // TODO save last open path
  //this->LoadSceneDialog->SaveLastPathToRegistry("OpenPath");

  if (d->MRMLScene->GetErrorCode() != 0 )
    {
    qDebug() << "Failed to load scene:" << QString::fromStdString(d->MRMLScene->GetErrorMessage());
    }
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::closeScene()
{
  QCTK_D(qSlicerCoreIOManager);
  Q_ASSERT(d->MRMLScene);
  
  d->MRMLScene->Clear(false);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::fileTypeFromExtension(const QString& extension)
{
  QCTK_D(qSlicerCoreIOManager);
  d->ExtensionFileType->beginGroup("ExtensionFileType");
  return d->ExtensionFileType->value(extension.toLower(), "Unknown").toString();
}

