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

#ifndef __qSlicerSceneWriter_h
#define __qSlicerSceneWriter_h

// QtCore includes
#include "qSlicerFileWriter.h"

// Data includes
#include "qSlicerDataModuleExport.h"

class Q_SLICER_QTMODULES_DATA_EXPORT qSlicerSceneWriter
  : public qSlicerFileWriter
{
  Q_OBJECT
public:
  typedef qSlicerFileWriter Superclass;
  qSlicerSceneWriter(QObject* parent = nullptr);
  ~qSlicerSceneWriter() override;

  QString description()const override;
  IOFileType fileType()const override;

  /// Return true if the object is handled by the writer.
  bool canWriteObject(vtkObject* object)const override;

  /// Return  a list of the supported extensions for a particuliar object.
  /// Please read QFileDialog::nameFilters for the allowed formats
  /// Example: "Image (*.jpg *.png *.tiff)", "Model (*.vtk)"
  QStringList extensions(vtkObject* object)const override;

  /// Write the node identified by nodeID into the fileName file.
  /// Returns true on success.
  bool write(const qSlicerIO::IOProperties& properties) override;

protected:
  bool writeToMRML(const qSlicerIO::IOProperties& properties);
  bool writeToMRB(const qSlicerIO::IOProperties& properties);
  bool writeToDirectory(const qSlicerIO::IOProperties& properties);

private:
  Q_DISABLE_COPY(qSlicerSceneWriter);
};

#endif
