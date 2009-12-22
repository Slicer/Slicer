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
class qSlicerCoreIOManagerPrivate: public qCTKPrivate<qSlicerCoreIOManager>
{
public:
  qSlicerCoreIOManagerPrivate()
    {
    this->ExtensionFileType = new QSettings(":/default-extension-filetype",
                                            QSettings::IniFormat);
    }
  ~qSlicerCoreIOManagerPrivate()
    {
    delete this->ExtensionFileType; 
    }
  
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
void qSlicerCoreIOManager::loadScene(vtkMRMLScene* mrmlScene, const QString& filename)
{
  Q_ASSERT(mrmlScene);
  
  // Convert to lowercase
  QString filenameLc = filename.toLower();
    
  if (filenameLc.endsWith(".mrml"))
    {
    mrmlScene->SetURL(filenameLc.toLatin1());
    mrmlScene->Connect();
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

  if (mrmlScene->GetErrorCode() != 0 )
    {
    qDebug() << "Failed to load scene:" << QString::fromStdString(mrmlScene->GetErrorMessage());
    }
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::importScene(vtkMRMLScene* mrmlScene, const QString& filename)
{
  Q_ASSERT(mrmlScene);
  
  // Convert to lowercase
  QString filenameLc = filename.toLower();

  if (filenameLc.endsWith(".mrml"))
    {
    mrmlScene->SetURL(filenameLc.toLatin1());
    mrmlScene->Import();
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

  if (mrmlScene->GetErrorCode() != 0 )
    {
    qDebug() << "Failed to load scene:" << QString::fromStdString(mrmlScene->GetErrorMessage());
    }
}

//-----------------------------------------------------------------------------
void qSlicerCoreIOManager::closeScene(vtkMRMLScene* mrmlScene)
{
  Q_ASSERT(mrmlScene);
  
  mrmlScene->Clear(false);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreIOManager::fileTypeFromExtension(const QString& extension)
{
  QCTK_D(qSlicerCoreIOManager);
  d->ExtensionFileType->beginGroup("ExtensionFileType");
  return d->ExtensionFileType->value(extension.toLower(), "Unknown").toString();
}

