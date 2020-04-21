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
#include <QFileInfo>

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
/// \ingroup Slicer_QtModules_Sequences
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
vtkSlicerSequencesLogic* qSlicerSequencesReader::sequencesLogic()const
{
  Q_D(const qSlicerSequencesReader);
  return d->SequencesLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerSequencesReader::description()const
{
  return "Sequence";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSequencesReader::fileType()const
{
  return QString("SequenceFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSequencesReader::extensions()const
{
  return QStringList()
    << "Sequence (*.seq.mrb *.mrb)"
    << "Volume Sequence (*.seq.nrrd *.seq.nhdr)" << "Volume Sequence (*.nrrd *.nhdr)";
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
    return false;
    }
  vtkMRMLSequenceNode* node = d->SequencesLogic->AddSequence(fileName.toLatin1());
  if (!node)
    {
    return false;
    }

  if (properties.contains("name"))
    {
    std::string customName = this->mrmlScene()->GetUniqueNameByString(
      properties["name"].toString().toLatin1());
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
