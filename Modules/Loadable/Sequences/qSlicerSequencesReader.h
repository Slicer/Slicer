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

#ifndef __qSlicerSequencesReader_h
#define __qSlicerSequencesReader_h

// Slicer includes
#include "qSlicerFileReader.h"
class qSlicerSequencesReaderPrivate;

// Slicer includes
class vtkSlicerSequencesLogic;

//-----------------------------------------------------------------------------
class qSlicerSequencesReader : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerSequencesReader(vtkSlicerSequencesLogic* sequencesLogic = 0, QObject* parent = 0);
  ~qSlicerSequencesReader() override;

  void setSequencesLogic(vtkSlicerSequencesLogic* sequencesLogic);
  vtkSlicerSequencesLogic* sequencesLogic() const;

  QString description() const override;
  IOFileType fileType() const override;
  QStringList extensions() const override;

  /// Returns a positive number (>0) if the reader can load this file.
  /// In case the file uses a generic file extension (such as .nrrd) then the confidence value is adjusted based on
  /// the file content: if the file contains a sequence nrrd file then confidence is increased to 0.6,
  /// otherwise the confidence is decreased to 0.4.
  double canLoadFileConfidence(const QString& file) const override;

  bool load(const IOProperties& properties) override;

protected:
  QScopedPointer<qSlicerSequencesReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSequencesReader);
  Q_DISABLE_COPY(qSlicerSequencesReader);
};

#endif
