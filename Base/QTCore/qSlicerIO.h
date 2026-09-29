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
#include <QString>
#include <QStringList>
#include <QImage>
#include <QVariant>

// QtCore includes
#include "qSlicerBaseQTCoreExport.h"
#include "qSlicerObject.h"

class qSlicerIOOptions;
class qSlicerIOPrivate;
class vtkImageData;
class vtkMRMLMessageCollection;
class vtkMRMLFileIOHandler;
class vtkMRMLIOProperties;

/// Base class for qSlicerFileReader and qSlicerFileWriter
///
/// \deprecated Reading and writing is implemented in VTK-based classes (vtkMRMLFileReader, vtkMRMLFileWriter)
/// and managed by vtkMRMLFileIOManager. This class is kept for backward compatibility and to provide Qt options widgets.
/// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#qt-interface
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerIO
  : public QObject
  , public qSlicerObject
{
  Q_OBJECT

public:
  typedef QObject Superclass;
  explicit qSlicerIO(QObject* parent = nullptr);
  ~qSlicerIO() override;

  typedef QString IOFileType;
  typedef QVariantMap IOProperties;

  /// Unique name of the reader/writer
  Q_INVOKABLE virtual QString description() const = 0;

  /// Multiple readers can share the same file type
  Q_INVOKABLE virtual qSlicerIO::IOFileType fileType() const = 0;

  /// Returns a list of options for the reader. qSlicerIOOptions can be
  /// derived and have a UI associated to it (i.e. qSlicerIOOptionsWidget).
  /// Warning: you are responsible for freeing the memory of the returned
  /// options
  Q_INVOKABLE virtual qSlicerIOOptions* options() const;

  /// Additional warning or error messages occurred during IO operation.
  Q_INVOKABLE virtual vtkMRMLMessageCollection* userMessages() const;

  /// VTK-based reader or writer that performs all the tasks of this class.
  /// Returns nullptr (legacy Qt-based readers and writers implement reading and writing themselves).
  /// It is overridden in qSlicerVTKFileReader and qSlicerVTKFileWriter.
  Q_INVOKABLE virtual vtkMRMLFileIOHandler* ioHandler() const;

  /// Set the scene of this object.
  void setMRMLScene(vtkMRMLScene* scene) override;

  /// Convert Qt properties to VTK properties.
  /// Supported value types: bool, integer (including 64-bit), floating-point, QString, QStringList,
  /// QVariantList (of bool, integer, floating-point, and QString values), QVariantMap and QVariantHash
  /// (converted to nested properties), QImage and QPixmap (converted to vtkImageData).
  /// Properties of other types are ignored and an error is logged.
  static void toVTKProperties(const IOProperties& properties, vtkMRMLIOProperties* vtkProperties);
  /// Convert VTK properties to Qt properties. Lists are converted to QVariantList, nested properties
  /// to QVariantMap, vtkImageData values to QImage.
  /// Objects that cannot be converted to Qt types are ignored and an error is logged.
  static IOProperties fromVTKProperties(vtkMRMLIOProperties* vtkProperties);

  /// Convert Qt image to VTK image (RGBA, unsigned char).
  static bool qImageToVTKImageData(const QImage& image, vtkImageData* imageData);
  /// Convert VTK image (unsigned char, 1, 3, or 4 components) to Qt image.
  static QImage vtkImageDataToQImage(vtkImageData* imageData);

protected:
  QScopedPointer<qSlicerIOPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerIO);
  Q_DISABLE_COPY(qSlicerIO);
};

Q_DECLARE_METATYPE(qSlicerIO::IOFileType)
Q_DECLARE_METATYPE(qSlicerIO::IOProperties)

#endif
