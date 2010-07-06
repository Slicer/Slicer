// Qt includes
#include <QDebug>
#include <QString>
#include <QVariant>

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerColorIO.h"

// Logic includes
#include "vtkSlicerColorLogic.h"

// MRML includes
#include <vtkMRMLColorNode.h>

//-----------------------------------------------------------------------------
qSlicerColorIO::qSlicerColorIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerColorIO::description()const
{
  return "Color";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerColorIO::fileType()const
{
  return qSlicerIO::ColorTableFile;
}

//-----------------------------------------------------------------------------
QString qSlicerColorIO::extensions()const
{
  return "*.txt";
}

//-----------------------------------------------------------------------------
bool qSlicerColorIO::load(const IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  // TODO:can be done when the module Color is ported to QT
  //vtkSlicerColorLogic* colorLogic =
  //  vtkSlicerColorLogic::SafeDownCast(
  //    qSlicerCoreApplication::application()->moduleManager()
  //    ->module("Color")->logic());
  // Q_ASSERT(colorLogic && colorLogic->GetMRMLScene() == this->mrmlScene());
  vtkSlicerColorLogic* colorLogic = vtkSlicerColorLogic::New();
  Q_ASSERT(colorLogic);
  colorLogic->SetMRMLScene(this->mrmlScene());
  vtkMRMLColorNode* node = 0;
  // TODO change return value of LoadColorFile into vtkMRMLColorNode*
  colorLogic->LoadColorFile(fileName.toLatin1().data());
  if (node)
    {
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  colorLogic->Delete();
  return node != 0;
}

// TODO: add the save() method. Use vtkSlicerColorLogic::SaveColor()
