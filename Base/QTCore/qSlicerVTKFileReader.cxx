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

==============================================================================*/

// Qt includes
#include <QDebug>

// QtCore includes
#include "qSlicerVTKFileReader.h"

// CTK includes
#include <ctkUtils.h>

// Slicer includes
#include <vtkMRMLFileReader.h>
#include <vtkMRMLIOProperties.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerVTKFileReaderPrivate
{
public:
  vtkSmartPointer<vtkMRMLFileReader> FileReader;
};

//----------------------------------------------------------------------------
qSlicerVTKFileReader::qSlicerVTKFileReader(vtkMRMLFileReader* reader, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVTKFileReaderPrivate)
{
  this->setFileReader(reader);
}

//----------------------------------------------------------------------------
qSlicerVTKFileReader::~qSlicerVTKFileReader() = default;

//----------------------------------------------------------------------------
vtkMRMLFileReader* qSlicerVTKFileReader::fileReader() const
{
  Q_D(const qSlicerVTKFileReader);
  return d->FileReader;
}

//----------------------------------------------------------------------------
void qSlicerVTKFileReader::setFileReader(vtkMRMLFileReader* reader)
{
  Q_D(qSlicerVTKFileReader);
  d->FileReader = reader;
  if (reader && this->mrmlScene())
  {
    reader->SetScene(this->mrmlScene());
  }
}

//----------------------------------------------------------------------------
vtkMRMLFileIOHandler* qSlicerVTKFileReader::ioHandler() const
{
  return this->fileReader();
}

//----------------------------------------------------------------------------
QString qSlicerVTKFileReader::description() const
{
  vtkMRMLFileReader* reader = this->fileReader();
  if (!reader)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file reader";
    return QString();
  }
  return QString::fromStdString(reader->GetDescription());
}

//----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerVTKFileReader::fileType() const
{
  vtkMRMLFileReader* reader = this->fileReader();
  if (!reader)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file reader";
    return IOFileType();
  }
  return IOFileType(QString::fromStdString(reader->GetFileType()));
}

//----------------------------------------------------------------------------
vtkMRMLMessageCollection* qSlicerVTKFileReader::userMessages() const
{
  vtkMRMLFileReader* reader = this->fileReader();
  if (!reader)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file reader";
    return this->Superclass::userMessages();
  }
  return reader->GetUserMessages();
}

//----------------------------------------------------------------------------
void qSlicerVTKFileReader::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  vtkMRMLFileReader* reader = this->fileReader();
  if (reader)
  {
    reader->SetScene(scene);
  }
}

//----------------------------------------------------------------------------
QStringList qSlicerVTKFileReader::extensions() const
{
  vtkMRMLFileReader* reader = this->fileReader();
  if (!reader)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file reader";
    return QStringList();
  }
  QStringList nameFilters;
  ctk::stlVectorToQList(reader->GetNameFilters(), nameFilters);
  return nameFilters;
}

//----------------------------------------------------------------------------
bool qSlicerVTKFileReader::canLoadFile(const QString& fileName) const
{
  vtkMRMLFileReader* reader = this->fileReader();
  if (!reader)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file reader";
    return false;
  }
  return reader->CanLoadFile(fileName.toStdString());
}

//----------------------------------------------------------------------------
double qSlicerVTKFileReader::canLoadFileConfidence(const QString& fileName) const
{
  vtkMRMLFileReader* reader = this->fileReader();
  if (!reader)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file reader";
    return 0.0;
  }
  return reader->CanLoadFileConfidence(fileName.toStdString());
}

//----------------------------------------------------------------------------
bool qSlicerVTKFileReader::load(const IOProperties& properties)
{
  vtkMRMLFileReader* reader = this->fileReader();
  if (!reader)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file reader";
    return false;
  }
  reader->SetScene(this->mrmlScene());
  vtkNew<vtkMRMLIOProperties> vtkProperties;
  qSlicerIO::toVTKProperties(properties, vtkProperties);
  reader->ClearLoadedNodeIDs();
  return reader->Load(vtkProperties);
}

//----------------------------------------------------------------------------
void qSlicerVTKFileReader::setLoadedNodes(const QStringList& nodes)
{
  vtkMRMLFileReader* reader = this->fileReader();
  if (!reader)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file reader";
    return;
  }
  std::vector<std::string> nodeIDs;
  ctk::qListToSTLVector(nodes, nodeIDs);
  reader->SetLoadedNodeIDs(nodeIDs);
}

//----------------------------------------------------------------------------
QStringList qSlicerVTKFileReader::loadedNodes() const
{
  vtkMRMLFileReader* reader = this->fileReader();
  if (!reader)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file reader";
    return QStringList();
  }
  QStringList loadedNodes;
  ctk::stlVectorToQList(reader->GetLoadedNodeIDs(), loadedNodes);
  return loadedNodes;
}

//----------------------------------------------------------------------------
bool qSlicerVTKFileReader::examineFileInfoList(QFileInfoList& fileInfoList, QFileInfo& archetypeFileInfo, qSlicerIO::IOProperties& ioProperties) const
{
  vtkMRMLFileReader* reader = this->fileReader();
  if (!reader)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file reader";
    return false;
  }
  std::vector<std::string> fileList;
  for (const QFileInfo& fileInfo : fileInfoList)
  {
    fileList.push_back(fileInfo.absoluteFilePath().toStdString());
  }
  vtkNew<vtkMRMLIOProperties> vtkProperties;
  qSlicerIO::toVTKProperties(ioProperties, vtkProperties);
  std::string archetypeFile = reader->ExamineFileList(fileList, vtkProperties);
  if (archetypeFile.empty())
  {
    return false;
  }
  archetypeFileInfo = QFileInfo(QString::fromStdString(archetypeFile));
  // Remove files from the list that the reader removed
  QStringList remainingFiles;
  ctk::stlVectorToQList(fileList, remainingFiles);
  QMutableListIterator<QFileInfo> fileInfoIterator(fileInfoList);
  while (fileInfoIterator.hasNext())
  {
    if (!remainingFiles.contains(fileInfoIterator.next().absoluteFilePath()))
    {
      fileInfoIterator.remove();
    }
  }
  ioProperties = qSlicerIO::fromVTKProperties(vtkProperties);
  return true;
}
