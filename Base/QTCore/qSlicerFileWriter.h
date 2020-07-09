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

class vtkObject;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerFileWriter
  : public qSlicerIO
{
  Q_OBJECT
public:
  qSlicerFileWriter(QObject* parent = nullptr);
  ~qSlicerFileWriter() override;

  /// Return true if the object is handled by the writer.
  virtual bool canWriteObject(vtkObject* object)const;

  /// Return  a list of the supported extensions for a particuliar object.
  /// Please read QFileDialog::nameFilters for the allowed formats
  /// Example: "Image (*.jpg *.png *.tiff)", "Model (*.vtk)"
  virtual QStringList extensions(vtkObject* object)const = 0;

  /// Write the node identified by nodeID into the fileName file.
  /// Returns true on success
  /// Properties availables:
  /// * QString nodeID
  /// * QString fileName
  /// * QString fileFormat
  /// * bool compressed
  /// ...
  virtual bool write(const qSlicerIO::IOProperties& properties);

  /// Return the list of saved nodes from writing the file(s) in write().
  /// Empty list if write() failed
  /// \sa setWrittenNodes(), write()
  virtual QStringList writtenNodes()const;

protected:
  virtual void setWrittenNodes(const QStringList& nodes);

protected:
  QScopedPointer<qSlicerFileWriterPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerFileWriter);
  Q_DISABLE_COPY(qSlicerFileWriter);
};

#endif
