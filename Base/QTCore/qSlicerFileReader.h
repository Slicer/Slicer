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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerFileReader_h
#define __qSlicerFileReader_h

// Qt includes
#include <QFileInfo>

// QtCore includes
#include "qSlicerIO.h"
#include "qSlicerBaseQTCoreExport.h"

class qSlicerFileReaderOptions;
class qSlicerFileReaderPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerFileReader : public qSlicerIO
{
  Q_OBJECT
public:
  typedef qSlicerIO Superclass;
  explicit qSlicerFileReader(QObject* parent = nullptr);
  ~qSlicerFileReader() override;

  /// Return  a list of the supported extensions. Please read
  /// QFileDialog::nameFilters for the allowed formats
  /// Example: "Image (*.jpg *.png *.tiff)", "Model (*.vtk)"
  Q_INVOKABLE virtual QStringList extensions() const;

  /// Returns true if the reader can load this file.
  /// Default implementation is a simple and fast, it just checks
  /// if file extension matches any of the wildcards returned by extensions() method.
  /// This method is kept for backward compatibility, readers should override
  /// canLoadFileConfidence method instead of this method to indicate if they can
  /// read a file.
  Q_INVOKABLE virtual bool canLoadFile(const QString& file) const;

  /// Returns a positive number (>0) if the reader can load this file.
  /// The higher the returned value is the more confident the reader it is
  /// the most suitable class to load the file.
  /// By default, the method calls canLoadFile and if it returns true then
  /// the returned confidence value is 0.5 + 0.01 * matchedFileExtensionLength.
  /// The additional confidence for longer matched file extensions allow prioritization of
  /// more specific readers. For example, "*.seg.nrrd" is more specific than "*.nrrd";
  /// "*.nrrd" is more specific than "*.*".
  Q_INVOKABLE virtual double canLoadFileConfidence(const QString& file) const;

  /// Return the matching name filters -> if the fileName is "my_image.nrrd"
  /// and the supported extensions are "Volumes (*.mha *.nrrd *.raw)",
  /// "Images (*.png" *.jpg")", "DICOM (*)" then it returns
  /// "Volumes (*.mha *.nrrd *.raw), DICOM (*)"
  /// \param longestExtensionMatchPtr If non-zero then the method returns
  /// the length of the longest matched extension length in this argument.
  /// It can be used to determine how specifically extension matched.
  Q_INVOKABLE QStringList supportedNameFilters(const QString& fileName, int* longestExtensionMatchPtr = nullptr) const;

  /// Properties available: fileMode, multipleFiles, fileType.
  Q_INVOKABLE virtual bool load(const IOProperties& properties);

  /// Return the list of generated nodes from loading the file(s) in load().
  /// Empty list if load() failed
  /// \sa setLoadedNodes(), load()
  Q_INVOKABLE virtual QStringList loadedNodes() const;

  /// Implements the file list examination for the corresponding method in the core
  /// IO manager.
  /// \sa qSlicerCoreIOManager
  Q_INVOKABLE virtual bool examineFileInfoList(QFileInfoList& fileInfoList,
                                               QFileInfo& archetypeFileInfo,
                                               qSlicerIO::IOProperties& ioProperties) const;

protected:
  /// Must be called in load() on success with the list of nodes added into the
  /// scene.
  virtual void setLoadedNodes(const QStringList& nodes);

protected:
  QScopedPointer<qSlicerFileReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerFileReader);
  Q_DISABLE_COPY(qSlicerFileReader);
};

#endif
