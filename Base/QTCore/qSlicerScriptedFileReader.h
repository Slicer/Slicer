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

#ifndef __qSlicerScriptedFileReader_h
#define __qSlicerScriptedFileReader_h

// Slicer includes
#include "qSlicerFileReader.h"
#include "qSlicerBaseQTCoreExport.h"

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif
class qSlicerScriptedFileReaderPrivate;
class vtkObject;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerScriptedFileReader
  : public qSlicerFileReader
{
  Q_OBJECT

  /// This property allows the reader to report back what nodes it was able to load
  Q_PROPERTY(QStringList loadedNodes READ loadedNodes WRITE setLoadedNodes)

public:
  typedef qSlicerFileReader Superclass;
  qSlicerScriptedFileReader(QObject* parent = nullptr);
  ~qSlicerScriptedFileReader() override;

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
  /// \sa qSlicerFileReader::extensions()
  QStringList extensions()const override;

  /// Reimplemented to propagate to python methods
  /// \sa qSlicerFileReader::canLoadFile()
  bool canLoadFile(const QString& file)const override;

  /// Reimplemented to propagate to python methods
  /// \sa qSlicerFileReader::write()
  bool load(const qSlicerIO::IOProperties& properties) override;

  /// Reimplemented to support python methods and q_property
  /// Exposes setLoadedNodes, which is protected in superclass
  /// \sa qSlicerFileReader::loadedNodes()
  /// \sa qSlicerFileWriter::writtenNodes()
  QStringList loadedNodes()const override {
    return Superclass::loadedNodes();
  };
  void setLoadedNodes(const QStringList& nodes) override {
    Superclass::setLoadedNodes(nodes);
  };

protected:
  QScopedPointer<qSlicerScriptedFileReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScriptedFileReader);
  Q_DISABLE_COPY(qSlicerScriptedFileReader);
};

#endif
