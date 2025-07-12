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
#include <QRegularExpression>
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
  return QStringList() //
         << tr("Sequence") + " (*.seq.mrb *.mrb)" << tr("Volume Sequence") + " (*.seq.nrrd *.seq.nhdr)" << tr("Volume Sequence") + " (*.nrrd *.nhdr)";
}

//----------------------------------------------------------------------------
double qSlicerSequencesReader::canLoadFileConfidence(const QString& fileName) const
{
  double confidence = Superclass::canLoadFileConfidence(fileName);

  // Confidence for .nrrd and .nhdr file is 0.55 (5 characters in the file extension matched),
  // for composite file extensions (.seq.nhdr) it would be 0.59.
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
        // The dimension and kinds fields are usually found at around position 500, but we
        // read a bit more just in case there are some extra fields.
        QString line = in.read(800);

        bool looksLikeSequence = false;

        // Supported 4D NRRD files contain "dimension: 4" line.
        // 2D+t and 3D+color+t files are not yet supported.
        QRegularExpression dimensionRe("dimension:([^\\n]+)");
        QRegularExpressionMatch dimensionMatch = dimensionRe.match(line);
        if (dimensionMatch.hasMatch())
        {
          QString dimensionStr = dimensionMatch.captured(1);
          bool ok = false;
          int dimension = dimensionStr.toInt(&ok);
          if (ok && dimension == 4)
          {
            // Supported 4D NRRD files "kinds" field contain "time" or "list" axis.
            // We don't want to load 3D+color images or displacement field volumes.
            // For example: "kinds: space space space list".
            QRegularExpression kindsRe("kinds:([^\\n]+)");
            QRegularExpressionMatch kindsMatch = kindsRe.match(line);
            if (kindsMatch.hasMatch())
            {
              QString kindsStr = kindsMatch.captured(1);
              if (kindsStr.contains("list") || kindsStr.contains("time"))
              {
                looksLikeSequence = true;
              }
            }
          }
        }
        // If it looks like sequence then we need to set a confidence value that is larger than 0.55.
        // However, if we get a 4D sequence it may be some other 4D data set, such as .seg.nrrd.
        // We would not want a .seg.nrrd file to be recognized as sequence by default, so we need to set
        // the confidence value to smaller than 0.59. Therefore, if it looks like a sequence then we
        // use confidence of 0.58.
        confidence = (looksLikeSequence ? 0.58 : 0.4);
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
    browserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(this->mrmlScene()->AddNewNodeByClass("vtkMRMLSequenceBrowserNode", browserCustomName));
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
          appLogic->PropagateVolumeSelection(); // includes FitSliceToBackground by default
        }
      }
    }
  }

  this->setLoadedNodes(loadedNodeIDs);
  return true;
}
