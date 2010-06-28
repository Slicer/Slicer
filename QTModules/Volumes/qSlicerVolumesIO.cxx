// Qt includes
#include <QDebug>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerVolumesIO.h"
#include "qSlicerVolumesIOOptionsWidget.h"

// Logic includes
#include "vtkSlicerVolumesLogic.h"

// MRML includes
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>

//-----------------------------------------------------------------------------
qSlicerVolumesIO::qSlicerVolumesIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerVolumesIO::description()const
{
  return "Volume";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerVolumesIO::fileType()const
{
  return qSlicerIO::VolumeFile;
}

//-----------------------------------------------------------------------------
QString qSlicerVolumesIO::extensions()const
{
  return "*.hdr *.nhdr *.nrrd *.mhd *.mha *.vti *.nii *.gz *.mgz *img";
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerVolumesIO::options()const
{
  return new qSlicerVolumesIOOptionsWidget;
}

//-----------------------------------------------------------------------------
bool qSlicerVolumesIO::load(const IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name = fileName;
  if (properties.contains("name"))
    {
    name = properties["name"].toString();
    }
  int options = 0;
  if (properties.contains("labelmap"))
    {
    options |= properties["labelmap"].toBool() ? 0x1 : 0x0;
    }
  if (properties.contains("center"))
    {
    options |= properties["center"].toBool() ? 0x2 : 0x0;
    }
  if (properties.contains("singleFile"))
    {
    options |= properties["singleFile"].toBool() ? 0x4 : 0x0;
    }
  if (properties.contains("autowindowlevel"))
    {
    options |= properties["autowindowlevel"].toBool() ? 0x8 : 0x0;
    }
  if (properties.contains("discardorientation"))
    {
    options |= properties["discardorientation"].toBool() ? 0x10 : 0x0;
    }
  vtkSmartPointer<vtkStringArray> fileList;
  if (properties.contains("fileNames"))
    {
    fileList = vtkSmartPointer<vtkStringArray>::New();
    foreach(QString file, properties["fileNames"].toStringList())
      {
      fileList->InsertNextValue(file.toLatin1().data());
      }
    }
  vtkSlicerVolumesLogic* volumesLogic =
    vtkSlicerVolumesLogic::SafeDownCast(
      qSlicerCoreApplication::application()->moduleManager()
      ->module("volumes")->logic());
  Q_ASSERT(volumesLogic);
  vtkMRMLVolumeNode* node = volumesLogic->AddArchetypeVolume(
    fileName.toLatin1().data(),
    name.toLatin1().data(),
    options,
    fileList.GetPointer());
  if (node)
    {
    vtkSlicerApplicationLogic* appLogic =
      qSlicerCoreApplication::application()->appLogic();
    vtkMRMLSelectionNode* selectedNode = appLogic->GetSelectionNode();
    if (selectedNode)
      {
      selectedNode->SetReferenceActiveVolumeID(node->GetID());
      appLogic->PropagateVolumeSelection();
      }
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  return node != 0;
}
