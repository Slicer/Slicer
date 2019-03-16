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
#include <QVariant>

// QtCore includes
#include "qSlicerBaseQTCoreExport.h"
#include "qSlicerObject.h"

class qSlicerIOOptions;

/// Base class for qSlicerFileReader and qSlicerFileWriter
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerIO
  : public QObject
  , public qSlicerObject
{
  Q_OBJECT

public:
  typedef QObject Superclass;
  explicit qSlicerIO(QObject* parent = nullptr);
  ~qSlicerIO() override;

  typedef QString     IOFileType;
  typedef QVariantMap IOProperties;

  /// Unique name of the reader/writer
  virtual QString description()const = 0;

  /// Multiple readers can share the same file type
  virtual IOFileType fileType()const = 0;

  /// Returns a list of options for the reader. qSlicerIOOptions can be
  /// derived and have a UI associated to it (i.e. qSlicerIOOptionsWidget).
  /// Warning: you are responsible for freeing the memory of the returned
  /// options
  virtual qSlicerIOOptions* options()const;

private:
  Q_DISABLE_COPY(qSlicerIO);
};

Q_DECLARE_METATYPE(qSlicerIO::IOFileType)
Q_DECLARE_METATYPE(qSlicerIO::IOProperties)

#endif
