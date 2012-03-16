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

#ifndef __qSlicerIO_h
#define __qSlicerIO_h

// Qt includes
#include <QMap>
#include <QMetaType>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QStringList>
#include <QVariant>

// QtCore includes
#include "qSlicerBaseQTCoreExport.h"

class vtkMRMLScene;
class qSlicerIOOptions;
class qSlicerIOPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerIO : public QObject
{
  Q_OBJECT
  Q_ENUMS(IOFileType)
public:
  explicit qSlicerIO(QObject* parent = 0);
  virtual ~qSlicerIO();

  //typedef int IOFileType;

  enum IOFileType
  {
    NoFile = 0,
    SceneFile = 1, 
    VolumeFile = 2,
    TransformFile = 3,
    ModelFile = 4,
    ScalarOverlayFile = 5,
    ColorTableFile = 7,
    FiducialListFile = 8,
    FiberBundleFile = 9,
    TransferFunctionFile = 10,
    AnnotationFile = 11,
    UserFile = 32,
  };

  typedef QVariantMap IOProperties;

  /// Unique name of a reader
  virtual QString description()const = 0;
  /// Multiple readers can share the same file type
  virtual IOFileType fileType()const = 0;
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
  QStringList supportedNameFilters(const QString& fileName)const;

  /// Returns a list of options for the reader. qSlicerIOOptions can be
  /// derived and have a UI associated to it (i.e. qSlicerIOOptionsWidget).
  /// Warning: you are responsible for freeing the memory of the returned
  /// options
  virtual qSlicerIOOptions* options()const;

  // TBD: Derive from qSlicerObject instead of reimplementing setMRMLScene ?
  void setMRMLScene(vtkMRMLScene* scene);
  vtkMRMLScene* mrmlScene()const;
  
  /// Properties availables : fileMode, multipleFiles, fileType.
  virtual bool load(const IOProperties& properties);
  virtual bool save(const IOProperties& properties);

  QStringList loadedNodes()const;
  QStringList savedNodes()const;
  
protected:
  void setLoadedNodes(const QStringList& nodes);
  void setSavedNodes(const QStringList& nodes);
  
protected:
  QScopedPointer<qSlicerIOPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerIO);
  Q_DISABLE_COPY(qSlicerIO);
};

Q_DECLARE_METATYPE(qSlicerIO::IOFileType)
Q_DECLARE_METATYPE(qSlicerIO::IOProperties)

#endif
