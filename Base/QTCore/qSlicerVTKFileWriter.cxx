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

// STD includes
#include <typeinfo>

// Qt includes
#include <QDebug>

// QtCore includes
#include "qSlicerVTKFileWriter.h"

// CTK includes
#include <ctkUtils.h>

// Slicer includes
#include <vtkMRMLFileWriter.h>
#include <vtkMRMLIOProperties.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerVTKFileWriterPrivate
{
public:
  vtkSmartPointer<vtkMRMLFileWriter> FileWriter;
  /// Class that set the writer. If the actual class of the object is a subclass of this
  /// then methods may be overridden and so the writer cannot be used directly.
  const QMetaObject* FileWriterOwnerMetaObject{ nullptr };
  const std::type_info* FileWriterOwnerType{ nullptr };
};

//----------------------------------------------------------------------------
qSlicerVTKFileWriter::qSlicerVTKFileWriter(vtkMRMLFileWriter* writer, QObject* parentObject)
  : Superclass(parentObject)
  , d_ptr(new qSlicerVTKFileWriterPrivate)
{
  this->setFileWriter(writer);
}

//----------------------------------------------------------------------------
qSlicerVTKFileWriter::~qSlicerVTKFileWriter() = default;

//----------------------------------------------------------------------------
vtkMRMLFileWriter* qSlicerVTKFileWriter::fileWriter() const
{
  Q_D(const qSlicerVTKFileWriter);
  return d->FileWriter;
}

//----------------------------------------------------------------------------
void qSlicerVTKFileWriter::setFileWriter(vtkMRMLFileWriter* writer)
{
  Q_D(qSlicerVTKFileWriter);
  d->FileWriter = writer;
  // This method is called from the constructor of the class that delegates all tasks
  // to the writer, therefore metaObject() and typeid() return the type of that class.
  // The C++ type is checked as well, because subclasses may not use the Q_OBJECT macro.
  d->FileWriterOwnerMetaObject = this->metaObject();
  d->FileWriterOwnerType = &typeid(*this);
  if (writer && this->mrmlScene())
  {
    writer->SetScene(this->mrmlScene());
  }
}

//----------------------------------------------------------------------------
bool qSlicerVTKFileWriter::isFileWriterUsableDirectly() const
{
  Q_D(const qSlicerVTKFileWriter);
  return d->FileWriter && d->FileWriterOwnerMetaObject == this->metaObject() //
         && d->FileWriterOwnerType && *d->FileWriterOwnerType == typeid(*this);
}

//----------------------------------------------------------------------------
vtkMRMLFileIOHandler* qSlicerVTKFileWriter::ioHandler() const
{
  return this->fileWriter();
}

//----------------------------------------------------------------------------
QString qSlicerVTKFileWriter::description() const
{
  vtkMRMLFileWriter* writer = this->fileWriter();
  if (!writer)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file writer";
    return QString();
  }
  return QString::fromStdString(writer->GetDescription());
}

//----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerVTKFileWriter::fileType() const
{
  vtkMRMLFileWriter* writer = this->fileWriter();
  if (!writer)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file writer";
    return IOFileType();
  }
  return IOFileType(QString::fromStdString(writer->GetFileType()));
}

//----------------------------------------------------------------------------
vtkMRMLMessageCollection* qSlicerVTKFileWriter::userMessages() const
{
  vtkMRMLFileWriter* writer = this->fileWriter();
  if (!writer)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file writer";
    return this->Superclass::userMessages();
  }
  return writer->GetUserMessages();
}

//----------------------------------------------------------------------------
void qSlicerVTKFileWriter::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  vtkMRMLFileWriter* writer = this->fileWriter();
  if (writer)
  {
    writer->SetScene(scene);
  }
}

//----------------------------------------------------------------------------
bool qSlicerVTKFileWriter::canWriteObject(vtkObject* object) const
{
  vtkMRMLFileWriter* writer = this->fileWriter();
  if (!writer)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file writer";
    return false;
  }
  return writer->CanWriteObject(object);
}

//----------------------------------------------------------------------------
double qSlicerVTKFileWriter::canWriteObjectConfidence(vtkObject* object) const
{
  vtkMRMLFileWriter* writer = this->fileWriter();
  if (!writer)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file writer";
    return 0.0;
  }
  if (this->isFileWriterUsableDirectly())
  {
    return writer->CanWriteObjectConfidence(object);
  }
  // A subclass (for example, a qSlicerNodeWriter subclass in an extension) may override canWriteObject(),
  // therefore it must be used for computing the confidence.
  return this->canWriteObject(object) ? writer->GetConfidenceForMatchingClass() : 0.0;
}

//----------------------------------------------------------------------------
QStringList qSlicerVTKFileWriter::extensions(vtkObject* object) const
{
  vtkMRMLFileWriter* writer = this->fileWriter();
  if (!writer)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file writer";
    return QStringList();
  }
  QStringList nameFilters;
  ctk::stlVectorToQList(writer->GetNameFiltersForObject(object), nameFilters);
  return nameFilters;
}

//----------------------------------------------------------------------------
bool qSlicerVTKFileWriter::write(const qSlicerIO::IOProperties& properties)
{
  vtkMRMLFileWriter* writer = this->fileWriter();
  if (!writer)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file writer";
    return false;
  }
  writer->SetScene(this->mrmlScene());
  vtkNew<vtkMRMLIOProperties> vtkProperties;
  qSlicerIO::toVTKProperties(properties, vtkProperties);
  writer->ClearWrittenNodeIDs();
  return writer->Write(vtkProperties);
}

//----------------------------------------------------------------------------
void qSlicerVTKFileWriter::setWrittenNodes(const QStringList& nodes)
{
  vtkMRMLFileWriter* writer = this->fileWriter();
  if (!writer)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file writer";
    return;
  }
  std::vector<std::string> nodeIDs;
  ctk::qListToSTLVector(nodes, nodeIDs);
  writer->SetWrittenNodeIDs(nodeIDs);
}

//----------------------------------------------------------------------------
QStringList qSlicerVTKFileWriter::writtenNodes() const
{
  vtkMRMLFileWriter* writer = this->fileWriter();
  if (!writer)
  {
    qCritical() << Q_FUNC_INFO << "failed: invalid file writer";
    return QStringList();
  }
  QStringList writtenNodes;
  ctk::stlVectorToQList(writer->GetWrittenNodeIDs(), writtenNodes);
  return writtenNodes;
}
