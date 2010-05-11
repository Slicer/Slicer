// Qt includes
#include <QDebug>
#include <QString>
#include <QVariant>

// SlicerQt includes
//#include "qSlicerAbstractModule.h"
//#include "qSlicerCoreApplication.h"
//#include "qSlicerModuleManager.h"
#include "qSlicerModelsIO.h"

// Logic includes
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include <vtkMRMLModelNode.h>
#include <vtkMRMLStorageNode.h>

//-----------------------------------------------------------------------------
qSlicerModelsIO::qSlicerModelsIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerModelsIO::description()const
{
  return "Model";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerModelsIO::fileType()const
{
  return qSlicerIO::ModelFile;
}

//-----------------------------------------------------------------------------
QString qSlicerModelsIO::extensions()const
{
  return "*.vtk *.vtp *.g *.byu *.stl *.orig"
         "*.inflated *.sphere *.white *.smoothwm *.pial";
}

//-----------------------------------------------------------------------------
bool qSlicerModelsIO::load(const IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();
  
  // vtkSlicerModelsLogic* modelsLogic = 
  //   vtkSlicerModelsLogic::SafeDownCast(
  //     qSlicerCoreApplication::application()->moduleManager()
  //     ->module("Models")->logic());
  vtkSlicerModelsLogic* modelsLogic = vtkSlicerModelsLogic::New();
  modelsLogic->SetMRMLScene(this->mrmlScene());
  Q_ASSERT(modelsLogic);
  vtkMRMLModelNode* node = modelsLogic->AddModel(
    fileName.toLatin1().data());
  if (node)
    {
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  modelsLogic->Delete();
  return node != 0;
}


//-----------------------------------------------------------------------------
qSlicerScalarOverlayIO::qSlicerScalarOverlayIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerScalarOverlayIO::description()const
{
  return "Scalar Overlay";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerScalarOverlayIO::fileType()const
{
  return qSlicerIO::ScalarOverlayFile;
}

//-----------------------------------------------------------------------------
QString qSlicerScalarOverlayIO::extensions()const
{
  return "*.*";
}

//-----------------------------------------------------------------------------
bool qSlicerScalarOverlayIO::load(const IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  Q_ASSERT(properties.contains("modelNodeId"));
  QString fileName = properties["fileName"].toString();
  QString modelNodeId = properties["modelNodeId"].toString();
  vtkMRMLModelNode* modelNode = 
    vtkMRMLModelNode::SafeDownCast(
      this->mrmlScene()->GetNodeByID(modelNodeId.toLatin1().data()));
  Q_ASSERT(modelNode);

  // vtkSlicerModelsLogic* modelsLogic = 
  //   vtkSlicerModelsLogic::SafeDownCast(
  //     qSlicerCoreApplication::application()->moduleManager()
  //     ->module("Models")->logic());
  vtkSlicerModelsLogic* modelsLogic = vtkSlicerModelsLogic::New();
  Q_ASSERT(modelsLogic);
  vtkMRMLStorageNode* node = modelsLogic->AddScalar(fileName.toLatin1().data(), modelNode);
  if (node)
    {
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  modelsLogic->Delete();
  return node != 0;
}

