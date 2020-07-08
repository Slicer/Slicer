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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerScriptedFileWriter_h
#define __qSlicerScriptedFileWriter_h

// Slicer includes
#include "qSlicerFileWriter.h"
#include "qSlicerBaseQTCoreExport.h"

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif
class qSlicerScriptedFileWriterPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerScriptedFileWriter
  : public qSlicerFileWriter
{
  Q_OBJECT

  /// This property allows the writer to report back what nodes it was able to write
  Q_PROPERTY(QStringList writtenNodes READ writtenNodes WRITE setWrittenNodes)

public:
  typedef qSlicerFileWriter Superclass;
  qSlicerScriptedFileWriter(QObject* parent = nullptr);
  ~qSlicerScriptedFileWriter() override;

  QString pythonSource()const;

  /// \warning Setting the source is a no-op. See detailed comment in the source code.
  /// If missingClassIsExpected is true (default) then missing class is expected and not treated as an error.
  bool setPythonSource(const QString& newPythonSource, const QString& className = QLatin1String(""), bool missingClassIsExpected = true);

  /// Convenience method allowing to retrieve the associated scripted instance
  Q_INVOKABLE PyObject* self() const;

  /// Reimplemented to propagate to python methods
  /// \sa qSlicerIO::description()
  QString description()const override;

  /// Reimplemented to propagate to python methods
  /// \sa qSlicerIO::fileType()
  IOFileType fileType()const override;

  /// Reimplemented to propagate to python methods
  /// \sa qSlicerFileWriter::canWriteObject()
  bool canWriteObject(vtkObject* object)const override;

  /// Reimplemented to propagate to python methods
  /// \sa qSlicerFileWriter::extensions()
  QStringList extensions(vtkObject* object)const override;

  /// Reimplemented to propagate to python methods
  /// \sa qSlicerFileWriter::write()
  bool write(const qSlicerIO::IOProperties& properties) override;

  /// Added so node writers can report back written nodes
  /// \sa qSlicerFileWriter::writtenNodex()
  void addWrittenNode(const QString& writtenNode);

  /// Reimplemented to support python methods and q_property
  /// Exposes setWrittenNodes, which is protected in superclass
  /// \sa qSlicerFileWriter::writtenNodes()
  /// \sa qSlicerFileReader::loadedNodes()
  QStringList writtenNodes()const override {
    return Superclass::writtenNodes();
  };
  void setWrittenNodes(const QStringList& nodes) override {
    Superclass::setWrittenNodes(nodes);
  };

protected:
  QScopedPointer<qSlicerScriptedFileWriterPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScriptedFileWriter);
  Q_DISABLE_COPY(qSlicerScriptedFileWriter);
};

#endif
