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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women�s Hospital through NIH grant R01MH112748.

==============================================================================*/

// Qt includes
#include <QDebug>

// QtGUI includes
#include "qSlicerMarkupsWriter.h"

// QTCore includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSceneViewNode.h>
#include <vtkMRMLMarkupsJsonStorageNode.h>
#include <vtkMRMLMarkupsFiducialStorageNode.h>
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkStdString.h>
#include <vtkStringArray.h>

//----------------------------------------------------------------------------
qSlicerMarkupsWriter::qSlicerMarkupsWriter(QObject* parentObject)
  : qSlicerNodeWriter("Markups", QString("MarkupsFile"), QStringList() << "vtkMRMLMarkupsNode", true, parentObject)
{
}

//----------------------------------------------------------------------------
qSlicerMarkupsWriter::~qSlicerMarkupsWriter() = default;

//----------------------------------------------------------------------------
QStringList qSlicerMarkupsWriter::extensions(vtkObject* object)const
{
  QStringList supportedExtensions;

  vtkNew<vtkMRMLMarkupsJsonStorageNode> jsonStorageNode;
  const int formatCount = jsonStorageNode->GetSupportedWriteFileTypes()->GetNumberOfValues();
  for (int formatIt = 0; formatIt < formatCount; ++formatIt)
    {
    vtkStdString format = jsonStorageNode->GetSupportedWriteFileTypes()->GetValue(formatIt);
    supportedExtensions << QString::fromStdString(format);
    }

  vtkNew<vtkMRMLMarkupsFiducialStorageNode> fcsvStorageNode;
  const int fidsFormatCount = fcsvStorageNode->GetSupportedWriteFileTypes()->GetNumberOfValues();
  for (int formatIt = 0; formatIt < fidsFormatCount; ++formatIt)
    {
    vtkStdString format = fcsvStorageNode->GetSupportedWriteFileTypes()->GetValue(formatIt);
    supportedExtensions << QString::fromStdString(format);
    }

  return supportedExtensions;
}

//----------------------------------------------------------------------------
void qSlicerMarkupsWriter::setStorageNodeClass(vtkMRMLStorableNode* storableNode, const QString& storageNodeClassName)
{
  if (!storableNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: invalid storable node";
    return;
    }
  vtkMRMLScene* scene = storableNode->GetScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << " failed: invalid scene";
    return;
    }

  vtkMRMLStorageNode* currentStorageNode = storableNode->GetStorageNode();
  std::string storageNodeClassNameStr = storageNodeClassName.toStdString();
  if (currentStorageNode != nullptr && currentStorageNode->IsA(storageNodeClassNameStr.c_str()))
    {
    // requested storage node class is the same as current class, there is nothing to do
    return;
    }

  // Create and use new storage node of the correct class
  vtkMRMLStorageNode* newStorageNode = vtkMRMLStorageNode::SafeDownCast(scene->AddNewNodeByClass(storageNodeClassNameStr));
  if (!newStorageNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: cannot create new storage noed of class " << storageNodeClassName;
    return;
    }
  storableNode->SetAndObserveStorageNodeID(newStorageNode->GetID());

  // Remove old storage node
  if (currentStorageNode)
    {
    scene->RemoveNode(currentStorageNode);
    }
}

//----------------------------------------------------------------------------
bool qSlicerMarkupsWriter::write(const qSlicerIO::IOProperties& properties)
{
  vtkMRMLStorableNode* node = vtkMRMLStorableNode::SafeDownCast(this->getNodeByID(properties["nodeID"].toString().toUtf8().data()));
  std::string fileName = properties["fileName"].toString().toStdString();

  vtkNew<vtkMRMLMarkupsFiducialStorageNode> fcsvStorageNode;
  std::string fcsvCompatibleFileExtension = fcsvStorageNode->GetSupportedFileExtension(fileName.c_str(), false, true);
  if (!fcsvCompatibleFileExtension.empty())
    {
    // fcsv file needs to be written
    this->setStorageNodeClass(node, "vtkMRMLMarkupsFiducialStorageNode");
    }
  else
    {
    // json file needs to be written
    this->setStorageNodeClass(node, "vtkMRMLMarkupsJsonStorageNode");
    }

  return Superclass::write(properties);
}
