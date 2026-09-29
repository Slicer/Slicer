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

#ifndef __qSlicerVTKFileReader_h
#define __qSlicerVTKFileReader_h

// QtCore includes
#include "qSlicerFileReader.h"
#include "qSlicerBaseQTCoreExport.h"

class qSlicerVTKFileReaderPrivate;
class vtkMRMLFileReader;

/// Qt interface of a VTK-based file reader: all methods are delegated to the VTK-based reader (see fileReader()).
///
/// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#qt-interface
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerVTKFileReader : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  explicit qSlicerVTKFileReader(vtkMRMLFileReader* reader, QObject* parent = nullptr);
  ~qSlicerVTKFileReader() override;

  /// VTK-based reader that performs all the tasks of this class.
  Q_INVOKABLE vtkMRMLFileReader* fileReader() const;

  /// Returns the VTK-based reader (same as fileReader()).
  vtkMRMLFileIOHandler* ioHandler() const override;

  QString description() const override;
  IOFileType fileType() const override;
  vtkMRMLMessageCollection* userMessages() const override;

  /// Set the scene of this object and of the VTK-based reader.
  void setMRMLScene(vtkMRMLScene* scene) override;

  QStringList extensions() const override;
  bool canLoadFile(const QString& file) const override;
  double canLoadFileConfidence(const QString& file) const override;
  bool load(const IOProperties& properties) override;
  QStringList loadedNodes() const override;
  bool examineFileInfoList(QFileInfoList& fileInfoList, QFileInfo& archetypeFileInfo, qSlicerIO::IOProperties& ioProperties) const override;

protected:
  void setLoadedNodes(const QStringList& nodes) override;

  /// Set the VTK-based reader that performs all the tasks of this class.
  /// The current scene is set in the reader.
  void setFileReader(vtkMRMLFileReader* reader);

protected:
  QScopedPointer<qSlicerVTKFileReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVTKFileReader);
  Q_DISABLE_COPY(qSlicerVTKFileReader);
};

#endif
