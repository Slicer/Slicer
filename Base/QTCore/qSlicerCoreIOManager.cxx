#include "qSlicerCoreIOManager.h"

// SlicerQT includes
#include "qSlicerCoreApplication.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

// QT includes
#include <QString>
#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerCoreIOManagerPrivate: public qCTKPrivate<qSlicerCoreIOManager>
{
  vtkSmartPointer<vtkMRMLScene>  MRMLScene;
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

