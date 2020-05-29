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
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// Qt includes
#include <QDebug>

// MRML includes
#include <vtkMRMLScene.h>
#include "vtkMRMLTextNode.h"
#include "vtkMRMLTextStorageNode.h"

// std includes
#include <iostream>
#include <string>
#include <cctype>

// Slicer includes
#include "qSlicerTextsReader.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

//-----------------------------------------------------------------------------
class qSlicerTextsReaderPrivate
{
};

//-----------------------------------------------------------------------------
qSlicerTextsReader::qSlicerTextsReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTextsReaderPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTextsReader::~qSlicerTextsReader() = default;

//-----------------------------------------------------------------------------
QString qSlicerTextsReader::description() const
{
  return "Text file";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTextsReader::fileType() const
{
  return "TextFile";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTextsReader::extensions() const
{
  QStringList supportedExtensions = QStringList();
  supportedExtensions << "Text file (*.txt)";
  supportedExtensions << "XML document (*.xml)";
  supportedExtensions << "JSON document (*.json)";
  return supportedExtensions;
}

//-----------------------------------------------------------------------------
bool qSlicerTextsReader::load(const IOProperties& properties)
{
  Q_D(qSlicerTextsReader);
  if (!properties.contains("fileName"))
    {
    qCritical() << Q_FUNC_INFO << " did not receive fileName property";
    return false;
    }
  std::string fileName = properties["fileName"].toString().toStdString();

  std::string textNodeName = vtksys::SystemTools::GetFilenameWithoutLastExtension(fileName);
  vtkSmartPointer<vtkMRMLTextNode> textNode = vtkMRMLTextNode::SafeDownCast(this->mrmlScene()->AddNewNodeByClass("vtkMRMLTextNode", textNodeName));
  if (!textNode)
    {
    return false;
    }

  vtkSmartPointer<vtkMRMLTextStorageNode> storageNode = vtkMRMLTextStorageNode::SafeDownCast(this->mrmlScene()->AddNewNodeByClass("vtkMRMLTextStorageNode"));
  if (!storageNode)
    {
    this->mrmlScene()->RemoveNode(textNode);
    return false;
    }

  textNode->SetAndObserveStorageNodeID(storageNode->GetID());
  storageNode->SetFileName(fileName.c_str());
  int retval = storageNode->ReadData(textNode);
  if (retval != 1)
    {
    qCritical() << Q_FUNC_INFO << "load: error reading " << fileName.c_str();
    this->mrmlScene()->RemoveNode(textNode);
    this->mrmlScene()->RemoveNode(storageNode);
    return false;
    }

  this->setLoadedNodes(QStringList(QString(textNode->GetID())));

  return true;
}
