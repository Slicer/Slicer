/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

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
#include "vtkSlicerApplicationLogic.h"

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
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
QStringList qSlicerVolumesIO::extensions()const
{
  // pic files are bio-rad images (see itkBioRadImageIO)
  return QStringList()
    << "Volume (*.hdr *.nhdr *.nrrd *.mhd *.mha *.vti *.nii *.gz *.mgz *.img *.pic)"
    << "Dicom (*.dcm)"
    << "Image (*.png *.tif *.tiff *.jpg *.jpeg)";
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
    vtkMRMLApplicationLogic* mrmlAppLogic =
      qSlicerCoreApplication::application()->mrmlApplicationLogic();
    vtkSlicerApplicationLogic* slicerLogic =
      qSlicerCoreApplication::application()->appLogic();
    vtkMRMLSelectionNode* selectionNode =
      mrmlAppLogic ? mrmlAppLogic->GetSelectionNode() : 0;
    if (!selectionNode)
      {
      // support old way
      selectionNode = slicerLogic ? slicerLogic->GetSelectionNode() : 0;
      }
    if (selectionNode)
      {
      if (vtkMRMLScalarVolumeNode::SafeDownCast(node) &&
          vtkMRMLScalarVolumeNode::SafeDownCast(node)->GetLabelMap())
        {
        selectionNode->SetReferenceActiveLabelVolumeID(node->GetID());
        }
      else
        {
        selectionNode->SetReferenceActiveVolumeID(node->GetID());
        }
      if (mrmlAppLogic)
        {
        mrmlAppLogic->PropagateVolumeSelection();
        // TODO: slices should probably be fitting automatically..
        mrmlAppLogic->FitSliceToAll();
        }
      else if (slicerLogic)
        {
        slicerLogic->PropagateVolumeSelection();
        }
      }
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  return node != 0;
}
