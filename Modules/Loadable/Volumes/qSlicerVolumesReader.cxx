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

// std includes
#include <vector>
#include <algorithm>

// Qt includes
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

// Slicer includes
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

// ITK includes
#include <itkArchetypeSeriesFileNames.h>

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
qSlicerVolumesReader::~qSlicerVolumesReader() = default;

//-----------------------------------------------------------------------------
void qSlicerVolumesReader::setLogic(vtkSlicerVolumesLogic* logic)
{
  Q_D(qSlicerVolumesReader);
  d->Logic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerVolumesLogic* qSlicerVolumesReader::logic() const
{
  Q_D(const qSlicerVolumesReader);
  return d->Logic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerVolumesReader::description() const
{
  return tr("Volume");
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerVolumesReader::fileType() const
{
  return QString("VolumeFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumesReader::extensions() const
{
  // pic files are bio-rad images (see itkBioRadImageIO)
  return QStringList() //
         << tr("Volume") + " (*.hdr *.nhdr *.nrrd *.mhd *.mha *.mnc *.nii *.nii.gz *.mgh *.mgz *.mgh.gz *.img *.img.gz *.pic)" << tr("Dicom") + " (*.dcm *.ima)"
         << tr("Image") + " (*.png *.tif *.tiff *.jpg *.jpeg)" << tr("All Files") + " (*)";
}

//----------------------------------------------------------------------------
double qSlicerVolumesReader::canLoadFileConfidence(const QString& fileName) const
{
  double confidence = Superclass::canLoadFileConfidence(fileName);
  // Confidence for .nrrd and .nhdr file is 0.55 (5 characters in the file extension matched)
  if (confidence > 0)
  {
    // Inspect the content to recognize DWI volumes.
    QString upperCaseFileName = fileName.toUpper();
    if (upperCaseFileName.endsWith("NRRD") || upperCaseFileName.endsWith("NHDR"))
    {
      QFile file(fileName);
      if (file.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        QTextStream in(&file);
        QString line = in.readLine();
        if (line.startsWith("NRRD"))
        {
          // The nrrd header is separated by the data by a blank line, so read everything up to there
          // since diffusion scans can have a long list of gradients and modality can be
          // near the end of the header
          QRegularExpression modalityRe("modality:([^\\n]+)");
          while (!in.atEnd())
          {
            line = in.readLine();
            if (line.trimmed() == "")
            {
              break;
            }
            QRegularExpressionMatch modalityMatch = modalityRe.match(line);
            if (modalityMatch.hasMatch())
            {
              QString modalityStr = modalityMatch.captured(1);
              if (modalityStr.contains("dwmri", Qt::CaseInsensitive))
              {
                // This is a DWMRI image, we are confident that it is not just a general image sequence.
                // Therefore we set higher confidence than the generic sequence reader's confidence of 0.6.
                confidence = 0.7;
                break;
              }
            }
          }
        }
      }
    }
  }
  return confidence;
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerVolumesReader::options() const
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
    options |= properties["autoWindowLevel"].toBool() ? 0x8 : 0x0;
  }
  if (properties.contains("discardOrientation"))
  {
    options |= properties["discardOrientation"].toBool() ? 0x10 : 0x0;
  }
  bool propagateVolumeSelection = true;
  if (properties.contains("show"))
  {
    propagateVolumeSelection = properties["show"].toBool();
  }
  vtkSmartPointer<vtkStringArray> fileList;
  if (properties.contains("fileNames"))
  {
    fileList = vtkSmartPointer<vtkStringArray>::New();
    for (const QString& file : properties["fileNames"].toStringList())
    {
      fileList->InsertNextValue(file.toUtf8());
    }
  }
  Q_ASSERT(d->Logic);
  // Weak pointer is used because the node may be deleted if the scene is closed
  // right after reading.
  vtkWeakPointer<vtkMRMLVolumeNode> node = d->Logic->AddArchetypeVolume(fileName.toUtf8(), name.toUtf8(), options, fileList.GetPointer());
  if (node)
  {
    QString colorNodeID = properties.value("colorNodeID", QString()).toString();
    if (!colorNodeID.isEmpty())
    {
      vtkMRMLVolumeDisplayNode* displayNode = node->GetVolumeDisplayNode();
      if (displayNode)
      {
        displayNode->SetAndObserveColorNodeID(colorNodeID.toUtf8());
      }
    }
    if (propagateVolumeSelection)
    {
      vtkSlicerApplicationLogic* appLogic = d->Logic->GetApplicationLogic();
      vtkMRMLSelectionNode* selectionNode = appLogic ? appLogic->GetSelectionNode() : nullptr;
      if (selectionNode)
      {
        if (vtkMRMLLabelMapVolumeNode::SafeDownCast(node))
        {
          selectionNode->SetActiveLabelVolumeID(node->GetID());
        }
        else
        {
          selectionNode->SetActiveVolumeID(node->GetID());
        }
        if (appLogic)
        {
          appLogic->PropagateVolumeSelection(); // includes FitSliceToBackground by default
        }
      }
    }
    this->setLoadedNodes(QStringList(QString(node->GetID())));
  }
  else
  {
    this->setLoadedNodes(QStringList());
  }
  return node != nullptr;
}

//-----------------------------------------------------------------------------
bool qSlicerVolumesReader::examineFileInfoList(QFileInfoList& fileInfoList, QFileInfo& archetypeFileInfo, qSlicerIO::IOProperties& ioProperties) const
{

  //
  // Check each file to see if it's recognized as part of a series.  If so,
  // keep it as the archetype and remove all the others from the list
  //
  for (const QFileInfo& fileInfo : fileInfoList)
  {
    itk::ArchetypeSeriesFileNames::Pointer seriesNames = itk::ArchetypeSeriesFileNames::New();
    std::vector<std::string> candidateFiles;
    seriesNames->SetArchetype(fileInfo.absoluteFilePath().toStdString());
    candidateFiles = seriesNames->GetFileNames();
    if (candidateFiles.size() > 1)
    {
      archetypeFileInfo = fileInfo;
      QMutableListIterator<QFileInfo> fileInfoIterator(fileInfoList);
      while (fileInfoIterator.hasNext())
      {
        const QString& path = fileInfoIterator.next().absoluteFilePath();
        if (path == archetypeFileInfo.absoluteFilePath())
        {
          continue;
        }
        if (std::find(candidateFiles.begin(), candidateFiles.end(), path.toStdString()) != candidateFiles.end())
        {
          fileInfoIterator.remove();
        }
      }
      ioProperties["singleFile"] = false;
      return true;
    }
  }
  return false;
}
