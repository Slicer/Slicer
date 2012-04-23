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

#ifndef __qSlicerFileWriter_h
#define __qSlicerFileWriter_h

// QtCore includes
#include "qSlicerIO.h"
class qSlicerFileWriterPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerFileWriter : public qSlicerIO
{
  Q_OBJECT
public:
  qSlicerFileWriter(QObject* parent = 0);
  virtual ~qSlicerFileWriter();

  virtual QStringList nodeTags() const = 0;

  /// Based on the file extensions, returns true if the file can be saved,
  /// false otherwise.
  /// This function is relatively fast as it doesn't try to access the file.
  bool canWriteFile(const QString& file)const;

  /// Properties availables : fileMode, multipleFiles, fileType.
  virtual bool write(const qSlicerIO::IOProperties& properties);
  QStringList writtenNodes()const;

protected:
  void setWrittenNodes(const QStringList& nodes);

protected:
  QScopedPointer<qSlicerFileWriterPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerFileWriter);
  Q_DISABLE_COPY(qSlicerFileWriter);
};

#endif
