// Qt includes
#include <QDebug>
#include <QString>
#include <QVariant>

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerTransformsIO.h"

// Logic includes
#include "vtkSlicerTransformLogic.h"

// MRML includes
#include <vtkMRMLTransformNode.h>

//-----------------------------------------------------------------------------
qSlicerTransformsIO::qSlicerTransformsIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsIO::description()const
{
  return "Transform";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTransformsIO::fileType()const
{
  return qSlicerIO::TransformFile;
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsIO::extensions()const
{
  return "*.tfm *.mat *.txt";
}

//-----------------------------------------------------------------------------
bool qSlicerTransformsIO::load(const IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  vtkSlicerTransformLogic* transformLogic =
    vtkSlicerTransformLogic::SafeDownCast(
      qSlicerCoreApplication::application()->moduleManager()
      ->module("transforms")->logic());
  Q_ASSERT(transformLogic && transformLogic->GetMRMLScene() == this->mrmlScene());
  //transformLogic->SetMRMLScene(this->mrmlScene());
  vtkMRMLTransformNode* node = transformLogic->AddTransform(
    fileName.toLatin1().data(), this->mrmlScene());
  if (node)
    {
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  return node != 0;
}

// TODO: add the save() method. Use vtkSlicerTransformLogic::SaveTransform()
