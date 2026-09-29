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

#ifndef __qSlicerVTKFileWriter_h
#define __qSlicerVTKFileWriter_h

// QtCore includes
#include "qSlicerFileWriter.h"
#include "qSlicerBaseQTCoreExport.h"

class qSlicerVTKFileWriterPrivate;
class vtkMRMLFileWriter;

/// Qt interface of a VTK-based file writer: all methods are delegated to the VTK-based writer (see fileWriter()).
/// Subclasses may override methods (for example, canWriteObject() or write()).
///
/// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#qt-interface
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerVTKFileWriter : public qSlicerFileWriter
{
  Q_OBJECT
public:
  typedef qSlicerFileWriter Superclass;
  explicit qSlicerVTKFileWriter(vtkMRMLFileWriter* writer, QObject* parent = nullptr);
  ~qSlicerVTKFileWriter() override;

  /// VTK-based writer that performs all the tasks of this class.
  Q_INVOKABLE vtkMRMLFileWriter* fileWriter() const;

  /// Returns the VTK-based writer (same as fileWriter()).
  vtkMRMLFileIOHandler* ioHandler() const override;

  QString description() const override;
  IOFileType fileType() const override;
  vtkMRMLMessageCollection* userMessages() const override;

  /// Set the scene of this object and of the VTK-based writer.
  void setMRMLScene(vtkMRMLScene* scene) override;

  bool canWriteObject(vtkObject* object) const override;

  /// Returns the confidence of the VTK-based writer.
  /// If a subclass overrides canWriteObject() then the confidence is computed from canWriteObject() instead,
  /// to respect the override.
  double canWriteObjectConfidence(vtkObject* object) const override;

  QStringList extensions(vtkObject* object) const override;
  bool write(const qSlicerIO::IOProperties& properties) override;
  QStringList writtenNodes() const override;

protected:
  void setWrittenNodes(const QStringList& nodes) override;

  /// Set the VTK-based writer that performs all the tasks of this class.
  /// The current scene is set in the writer.
  /// It must be called from the constructor of the class that delegates all tasks to the VTK-based writer
  /// (the class of the object is recorded to detect subclasses that may override methods).
  void setFileWriter(vtkMRMLFileWriter* writer);

protected:
  QScopedPointer<qSlicerVTKFileWriterPrivate> d_ptr;

private:
  /// Returns true if the VTK-based writer can be used directly, without calling canWriteObject(),
  /// i.e., this object is of the class that set the writer and not a subclass that may override methods.
  bool isFileWriterUsableDirectly() const;

  Q_DECLARE_PRIVATE(qSlicerVTKFileWriter);
  Q_DISABLE_COPY(qSlicerVTKFileWriter);
};

#endif
