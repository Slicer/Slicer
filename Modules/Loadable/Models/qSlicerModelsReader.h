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

#ifndef __qSlicerModelsReader_h
#define __qSlicerModelsReader_h

// Slicer includes
#include "qSlicerFileReader.h"
class qSlicerModelsReaderPrivate;

// Slicer includes
class vtkSlicerModelsLogic;

//-----------------------------------------------------------------------------
class qSlicerModelsReader : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerModelsReader(vtkSlicerModelsLogic* modelsLogic = nullptr, QObject* parent = nullptr);
  ~qSlicerModelsReader() override;

  void setModelsLogic(vtkSlicerModelsLogic* modelsLogic);
  vtkSlicerModelsLogic* modelsLogic() const;

  QString description() const override;
  IOFileType fileType() const override;
  QStringList extensions() const override;
  qSlicerIOOptions* options() const override;

  /// Returns a positive number (>0) if the reader can load this file.
  /// It uses default confidence value except for .vtk files. For .vtk image files it returns
  /// lower than default (0.0, because this reader cannot read images) and
  /// for mesh files it returns higher than default (0.6).
  double canLoadFileConfidence(const QString& file) const override;

  bool load(const IOProperties& properties) override;

protected:
  QScopedPointer<qSlicerModelsReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerModelsReader);
  Q_DISABLE_COPY(qSlicerModelsReader);
};

#endif
