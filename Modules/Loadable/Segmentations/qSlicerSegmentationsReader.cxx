/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Adam Rankin, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Segmentations includes
#include "qSlicerSegmentationsReader.h"
#include "qSlicerSegmentationsIOOptionsWidget.h"

// Qt includes
#include <QFileInfo>
#include <QTextStream>

// Logic includes
#include "vtkSlicerSegmentationsModuleLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLSegmentationDisplayNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelStorageNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerSegmentationsReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerSegmentationsModuleLogic> SegmentationsLogic;
};

//-----------------------------------------------------------------------------
qSlicerSegmentationsReader::qSlicerSegmentationsReader(vtkSlicerSegmentationsModuleLogic* segmentationsLogic,
                                                       QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSegmentationsReaderPrivate)
{
  this->setSegmentationsLogic(segmentationsLogic);
}

//-----------------------------------------------------------------------------
qSlicerSegmentationsReader::~qSlicerSegmentationsReader() = default;

//-----------------------------------------------------------------------------
void qSlicerSegmentationsReader::setSegmentationsLogic(vtkSlicerSegmentationsModuleLogic* newSegmentationsLogic)
{
  Q_D(qSlicerSegmentationsReader);
  d->SegmentationsLogic = newSegmentationsLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerSegmentationsModuleLogic* qSlicerSegmentationsReader::segmentationsLogic() const
{
  Q_D(const qSlicerSegmentationsReader);
  return d->SegmentationsLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerSegmentationsReader::description() const
{
  return tr("Segmentation");
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSegmentationsReader::fileType() const
{
  return QString(/*no tr*/ "SegmentationFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSegmentationsReader::extensions() const
{
  QString extensionText = tr("Segmentation");
  return QStringList() << extensionText + " (*.seg.nrrd)" << extensionText + " (*.seg.nhdr)"
                       << extensionText + " (*.seg.vtm)" << extensionText + " (*.nrrd)" << extensionText + " (*.nhdr)"
                       << extensionText + " (*.vtm)" << extensionText + " (*.nii.gz)" << extensionText + " (*.nii)"
                       << extensionText + " (*.hdr)" << extensionText + " (*.stl)" << extensionText + " (*.obj)";
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerSegmentationsReader::options() const
{
  qSlicerIOOptionsWidget* options = new qSlicerSegmentationsIOOptionsWidget;
  options->setMRMLScene(this->mrmlScene());
  return options;
}

//----------------------------------------------------------------------------
double qSlicerSegmentationsReader::canLoadFileConfidence(const QString& fileName) const
{
  double confidence = Superclass::canLoadFileConfidence(fileName);

  // Confidence for .nrrd file is 0.55 (5 characters in the file extension matched),
  // .vtm is 0.54; for composite file extensions (.seg.nrrd, .seg.vtm) it would be >0.58.
  // Therefore, confidence below 0.56 means that we got a generic file extension
  // that we need to inspect further.
  if (confidence > 0 && confidence < 0.56)
  {
    // Not a composite file extension, inspect the content (for now, only nrrd).
    QString upperCaseFileName = fileName.toUpper();
    if (upperCaseFileName.endsWith("NRRD") || upperCaseFileName.endsWith("NHDR"))
    {
      QFile file(fileName);
      if (file.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        QTextStream in(&file);
        // Segmentation NRRD files contain ID for each segment (such as Segment0_ID:=...)
        // around position 500, read a bit further to account for slight variations in the header.
        QString line = in.read(800);
        // If this appears in the file header then declare higher confidence value.
        confidence = (line.contains("Segment0_ID:=") ? 0.6 : 0.4);
      }
    }
  }
  return confidence;
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentationsReader::load(const IOProperties& properties)
{
  Q_D(qSlicerSegmentationsReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  this->setLoadedNodes(QStringList());
  if (d->SegmentationsLogic.GetPointer() == nullptr)
  {
    return false;
  }

  QString name;
  if (properties.contains("name"))
  {
    name = properties["name"].toString();
  }

  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fileName.toStdString());

  if (extension.compare(".stl") == 0 || extension.compare(".obj") == 0)
  {
    vtkSmartPointer<vtkPolyData> closedSurfaceRepresentation;
    vtkNew<vtkMRMLModelStorageNode> modelStorageNode;
    modelStorageNode->SetFileName(fileName.toStdString().c_str());
    vtkNew<vtkMRMLModelNode> modelNode;
    if (!modelStorageNode->ReadData(modelNode))
    {
      return false;
    }
    closedSurfaceRepresentation = modelNode->GetPolyData();

    // Remove all arrays, because they could slow down all further processing
    // and consume significant amount of memory.
    if (closedSurfaceRepresentation != nullptr && closedSurfaceRepresentation->GetPointData() != nullptr)
    {
      vtkPointData* pointData = closedSurfaceRepresentation->GetPointData();
      while (pointData->GetNumberOfArrays() > 0)
      {
        pointData->RemoveArray(0);
      }
    }

    if (closedSurfaceRepresentation == nullptr)
    {
      return false;
    }

    if (name.isEmpty())
    {
      name = QFileInfo(fileName).completeBaseName();
    }

    vtkNew<vtkSegment> segment;
    segment->SetName(name.toUtf8().constData());
    segment->AddRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(),
                               closedSurfaceRepresentation);

    vtkMRMLSegmentationNode* segmentationNode =
      vtkMRMLSegmentationNode::SafeDownCast(this->mrmlScene()->AddNewNodeByClass(
        "vtkMRMLSegmentationNode", this->mrmlScene()->GetUniqueNameByString(name.toUtf8())));
    segmentationNode->SetSourceRepresentationToClosedSurface();
    segmentationNode->CreateDefaultDisplayNodes();
    vtkMRMLSegmentationDisplayNode* displayNode =
      vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
    if (displayNode)
    {
      // Show slice intersections using closed surface representation (don't create binary labelmap for display)
      displayNode->SetPreferredDisplayRepresentationName2D(
        vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
    }

    segmentationNode->GetSegmentation()->AddSegment(segment.GetPointer());

    this->setLoadedNodes(QStringList(QString(segmentationNode->GetID())));
  }
  else
  {
    // Non-STL file, load using segmentation storage node
    bool autoOpacities = true;
    if (properties.contains("autoOpacities"))
    {
      autoOpacities = properties["autoOpacities"].toBool();
    }

    vtkMRMLColorTableNode* colorTableNode = nullptr;
    if (properties.contains("colorNodeID"))
    {
      std::string nodeID = properties["colorNodeID"].toString().toStdString();
      colorTableNode = vtkMRMLColorTableNode::SafeDownCast(this->mrmlScene()->GetNodeByID(nodeID));
    }

    vtkMRMLSegmentationNode* node = d->SegmentationsLogic->LoadSegmentationFromFile(
      fileName.toUtf8().constData(), autoOpacities, name.toUtf8(), colorTableNode);
    if (!node)
    {
      this->setLoadedNodes(QStringList());
      return false;
    }

    this->setLoadedNodes(QStringList(QString(node->GetID())));
  }

  return true;
}
