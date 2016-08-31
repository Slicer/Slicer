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

// QtCore includes
#include "qSlicerIO.h"
#include "qSlicerBaseQTCoreExport.h"

class qSlicerFileReaderOptions;
class qSlicerFileReaderPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerFileReader
  : public qSlicerIO
{
  Q_OBJECT
public:
  typedef qSlicerIO Superclass;
  explicit qSlicerFileReader(QObject* parent = 0);
  virtual ~qSlicerFileReader();

  /// Return  a list of the supported extensions. Please read
  /// QFileDialog::nameFilters for the allowed formats
  /// Example: "Image (*.jpg *.png *.tiff)", "Model (*.vtk)"
  virtual QStringList extensions()const;

  /// Based on the file extensions, returns true if the file can be read,
  /// false otherwise.
  /// This function is relatively fast as it doesn't try to access the file.
  bool canLoadFile(const QString& file)const;

  /// Return the matching name filters -> if the fileName is "myimage.nrrd"
  /// and the supported extensions are "Volumes (*.mha *.nrrd *.raw)",
  /// "Images (*.png" *.jpg")", "DICOM (*)" then it returns
  /// "Volumes (*.mha *.nrrd *.raw), DICOM (*)"
  /// \param longestExtensionMatchPtr If non-zero then the method returns
  /// the length of the longest matched extension length in this argument.
  /// It can be used to determine how specifically extension matched.
  QStringList supportedNameFilters(const QString& fileName, int* longestExtensionMatchPtr = NULL)const;

  /// Properties availables : fileMode, multipleFiles, fileType.
  virtual bool load(const IOProperties& properties);

  /// Return the list of generated nodes from loading the file(s) in load().
  /// Empty list of load() failed
  /// \sa setLoadedNodes(), load()
  QStringList loadedNodes()const;

protected:
  /// Must be called in load() on success with the list of nodes added into the
  /// scene.
  void setLoadedNodes(const QStringList& nodes);

protected:
  QScopedPointer<qSlicerFileReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerFileReader);
  Q_DISABLE_COPY(qSlicerFileReader);
};

#endif
