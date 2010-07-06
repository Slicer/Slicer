// Qt includes
#include <QDebug>
#include <QString>
#include <QVariant>

// SlicerQt includes
//#include "qSlicerAbstractModule.h"
//#include "qSlicerCoreApplication.h"
//#include "qSlicerModuleManager.h"
#include "qSlicerFiducialsIO.h"

// Logic includes
#include "vtkSlicerFiducialsLogic.h"

// MRML includes
#include <vtkMRMLFiducialListNode.h>

//-----------------------------------------------------------------------------
qSlicerFiducialsIO::qSlicerFiducialsIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerFiducialsIO::description()const
{
  return "Fiducials";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerFiducialsIO::fileType()const
{
  return qSlicerIO::FiducialListFile;
}

//-----------------------------------------------------------------------------
QString qSlicerFiducialsIO::extensions()const
{
  return "*.fcsv";
}

//-----------------------------------------------------------------------------
bool qSlicerFiducialsIO::load(const IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  //vtkSlicerTransformLogic* transformLogic =
  //  vtkSlicerTransformLogic::SafeDownCast(
  //    qSlicerCoreApplication::application()->moduleManager()
  //    ->module("Fiducials")->logic());
  //Q_ASSERT(transformLogic && transformLogic->GetMRMLScene() == this->mrmlScene());
  vtkSlicerFiducialsLogic* fiducialsLogic = vtkSlicerFiducialsLogic::New();
  fiducialsLogic->SetMRMLScene(this->mrmlScene());
  vtkMRMLFiducialListNode* node = fiducialsLogic->LoadFiducialList(
    fileName.toLatin1().data());
  if (node)
    {
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  fiducialsLogic->Delete();
  return node != 0;
}

// TODO: add the save() method. Use vtkSlicerTransformLogic::SaveTransform()
