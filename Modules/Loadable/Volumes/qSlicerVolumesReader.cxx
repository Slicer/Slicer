/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
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
#include <QFileInfo>

// SlicerQt includes
#include "qSlicerVolumesIOOptionsWidget.h"
#include "qSlicerVolumesReader.h"

// Logic includes
#include <vtkSlicerApplicationLogic.h>
#include "vtkSlicerVolumesLogic.h"

// MRML includes
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>

//-----------------------------------------------------------------------------
class qSlicerVolumesReaderPrivate
{
  public:
  vtkSmartPointer<vtkSlicerVolumesLogic> Logic;
};

//-----------------------------------------------------------------------------
qSlicerVolumesReader::qSlicerVolumesReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumesReaderPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumesReader::qSlicerVolumesReader(vtkSlicerVolumesLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumesReaderPrivate)
{
  this->setLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerVolumesReader::~qSlicerVolumesReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerVolumesReader::setLogic(vtkSlicerVolumesLogic* logic)
{
  Q_D(qSlicerVolumesReader);
  d->Logic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerVolumesLogic* qSlicerVolumesReader::logic()const
{
  Q_D(const qSlicerVolumesReader);
  return d->Logic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerVolumesReader::description()const
{
  return "Volume";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerVolumesReader::fileType()const
{
  return QString("VolumeFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumesReader::extensions()const
{
  // pic files are bio-rad images (see itkBioRadImageIO)
  return QStringList()
    << "Volume (*.hdr *.nhdr *.nrrd *.mhd *.mha *.vti *.nii *.nii.gz *.mgh *.mgz *.mgh.gz *.img *.img.gz *.pic)"
    << "Dicom (*.dcm *.ima)"
    << "Image (*.png *.tif *.tiff *.jpg *.jpeg)"
    << "All Files (*)";
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerVolumesReader::options()const
{
  // set the mrml scene on the options widget to allow selecting a color node
  qSlicerIOOptionsWidget* options = new qSlicerVolumesIOOptionsWidget;
  options->setMRMLScene(this->mrmlScene());
  return options;
}

//-----------------------------------------------------------------------------
bool qSlicerVolumesReader::load(const IOProperties& properties)
{
  Q_D(qSlicerVolumesReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name = QFileInfo(fileName).baseName();
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
  if (properties.contains("autoWindowLevel"))
    {
    options |= properties["autoWindowLevel"].toBool() ? 0x8: 0x0;
    }
  if (properties.contains("discardOrientation"))
    {
    options |= properties["discardOrientation"].toBool() ? 0x10 : 0x0;
    }
  vtkSmartPointer<vtkStringArray> fileList;
  if (properties.contains("fileNames"))
    {
    fileList = vtkSmartPointer<vtkStringArray>::New();
    foreach(QString file, properties["fileNames"].toStringList())
      {
      fileList->InsertNextValue(file.toLatin1());
      }
    }
  Q_ASSERT(d->Logic);
  vtkMRMLVolumeNode* node = d->Logic->AddArchetypeVolume(
    fileName.toLatin1(),
    name.toLatin1(),
    options,
    fileList.GetPointer());
  if (node)
    {
    if (properties.contains("colorNodeID"))
      {
      QString colorNodeID = properties["colorNodeID"].toString();
      if (node->GetDisplayNode())
        {
        node->GetDisplayNode()->SetAndObserveColorNodeID(colorNodeID.toLatin1());
        }
      }
    vtkSlicerApplicationLogic* appLogic =
      d->Logic->GetApplicationLogic();
    vtkMRMLSelectionNode* selectionNode =
      appLogic ? appLogic->GetSelectionNode() : 0;
    if (selectionNode)
      {
      if (vtkMRMLLabelMapVolumeNode::SafeDownCast(node))
        {
        selectionNode->SetReferenceActiveLabelVolumeID(node->GetID());
        }
      else
        {
        selectionNode->SetReferenceActiveVolumeID(node->GetID());
        }
      if (appLogic)
        {
        appLogic->PropagateVolumeSelection(); // includes FitSliceToAll by default
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
