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
  7
  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QFileInfo>
#include <QTextStream>

// Slicer includes
#include "qSlicerSequencesReader.h"
#include "qSlicerSequencesModule.h"

// Logic includes
#include "vtkSlicerSequencesLogic.h"

// MRML includes
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLLabelMapVolumeNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLSequenceBrowserNode.h"
#include "vtkMRMLSequenceStorageNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeNode.h"

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerSequencesReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerSequencesLogic> SequencesLogic;
};

//-----------------------------------------------------------------------------
qSlicerSequencesReader::qSlicerSequencesReader(vtkSlicerSequencesLogic* sequencesLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSequencesReaderPrivate)
{
  this->setSequencesLogic(sequencesLogic);
}

//-----------------------------------------------------------------------------
qSlicerSequencesReader::~qSlicerSequencesReader() = default;

//-----------------------------------------------------------------------------
void qSlicerSequencesReader::setSequencesLogic(vtkSlicerSequencesLogic* newSequencesLogic)
{
  Q_D(qSlicerSequencesReader);
  d->SequencesLogic = newSequencesLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerSequencesLogic* qSlicerSequencesReader::sequencesLogic() const
{
  Q_D(const qSlicerSequencesReader);
  return d->SequencesLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerSequencesReader::description() const
{
  return tr("Sequence");
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSequencesReader::fileType() const
{
  return QString("SequenceFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSequencesReader::extensions() const
{
  return QStringList() << tr("Sequence") + " (*.seq.mrb *.mrb)" << tr("Volume Sequence") + " (*.seq.nrrd *.seq.nhdr)"
                       << tr("Volume Sequence") + " (*.nrrd *.nhdr)";
}

//----------------------------------------------------------------------------
double qSlicerSequencesReader::canLoadFileConfidence(const QString& fileName) const
{
  double confidence = Superclass::canLoadFileConfidence(fileName);

  // Confidence for .json file is 0.55 (5 characters in the file extension matched),
  // for composite file extensions (.mrk.json) it would be 0.59.
  // Therefore, confidence below 0.56 means that we got a generic file extension
  // that we need to inspect further.
  if (confidence > 0 && confidence < 0.56)
  {
    // Not a composite file extension, inspect the content
    // Unzipping the mrb file to inspect if it looks like a sequence would be too time-consuming,
    // therefore we only check NRRD files for now.
    QString upperCaseFileName = fileName.toUpper();
    if (upperCaseFileName.endsWith("NRRD") || upperCaseFileName.endsWith("NHDR"))
    {
      QFile file(fileName);
      if (file.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        QTextStream in(&file);
        // Markups json files contain a custom field specifying the index type as
        // "axis 0 index type:=" or "axis 3 index type:=" around position 500,
        // read a bit further to account for slight variations in the header.
        QString line = in.read(800);
        bool looksLikeSequence = line.contains("axis 0 index type:=") || line.contains("axis 3 index type:=");
        confidence = (looksLikeSequence ? 0.6 : 0.4);
      }
    }
  }
  return confidence;
}

//-----------------------------------------------------------------------------
bool qSlicerSequencesReader::load(const IOProperties& properties)
{
  Q_D(qSlicerSequencesReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  this->setLoadedNodes(QStringList());
  if (d->SequencesLogic.GetPointer() == 0)
  {
    qCritical() << Q_FUNC_INFO << (" failed: Sequences logic is invalid.");
    return false;
  }
  vtkMRMLSequenceNode* node = d->SequencesLogic->AddSequence(fileName.toUtf8(), this->userMessages());
  if (!node)
  {
    // errors are already logged and userMessages contain details that can be displayed to users
    return false;
  }

  if (properties.contains("name"))
  {
    std::string customName = this->mrmlScene()->GetUniqueNameByString(properties["name"].toString().toLatin1());
    node->SetName(customName.c_str());
  }

  QStringList loadedNodeIDs;
  loadedNodeIDs << QString::fromUtf8(node->GetID());

  bool show = true; // show volume node in viewers
  if (properties.contains("show"))
  {
    show = properties["show"].toBool();
  }
  vtkMRMLSequenceBrowserNode* browserNode = nullptr;
  if (show)
  {
    std::string browserCustomName = std::string(node->GetName()) + " browser";
    browserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(
      this->mrmlScene()->AddNewNodeByClass("vtkMRMLSequenceBrowserNode", browserCustomName));
  }
  if (browserNode)
  {
    loadedNodeIDs << QString::fromUtf8(browserNode->GetID());
    browserNode->SetAndObserveMasterSequenceNodeID(node->GetID());
    qSlicerSequencesModule::showSequenceBrowser(browserNode);

    d->SequencesLogic->UpdateProxyNodesFromSequences(browserNode);
    vtkMRMLNode* proxyNode = browserNode->GetProxyNode(node);

    // Associate color node
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(proxyNode);
    if (displayableNode)
    {
      if (properties.contains("colorNodeID"))
      {
        QString colorNodeID = properties["colorNodeID"].toString();
        if (displayableNode->GetDisplayNode())
        {
          displayableNode->GetDisplayNode()->SetAndObserveColorNodeID(colorNodeID.toUtf8());
        }
      }
    }

    // Propagate volume selection
    vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(browserNode->GetProxyNode(node));
    if (volumeNode)
    {
      vtkSlicerApplicationLogic* appLogic = d->SequencesLogic->GetApplicationLogic();
      vtkMRMLSelectionNode* selectionNode = appLogic ? appLogic->GetSelectionNode() : nullptr;
      if (selectionNode)
      {
        if (vtkMRMLLabelMapVolumeNode::SafeDownCast(volumeNode))
        {
          selectionNode->SetActiveLabelVolumeID(volumeNode->GetID());
        }
        else
        {
          selectionNode->SetActiveVolumeID(volumeNode->GetID());
        }
        if (appLogic)
        {
          appLogic->PropagateVolumeSelection(); // includes FitSliceToAll by default
        }
      }
    }
  }

  this->setLoadedNodes(loadedNodeIDs);
  return true;
}
