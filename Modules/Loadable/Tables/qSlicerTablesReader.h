/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef qSlicerTablesReader_h
#define qSlicerTablesReader_h

// Slicer includes
#include "qSlicerFileReader.h"

class qSlicerTablesReaderPrivate;
class vtkSlicerTablesLogic;

//-----------------------------------------------------------------------------
class qSlicerTablesReader : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerTablesReader(QObject* parent = nullptr);
  qSlicerTablesReader(vtkSlicerTablesLogic* logic, QObject* parent = nullptr);
  ~qSlicerTablesReader() override;

  vtkSlicerTablesLogic* logic() const;
  void setLogic(vtkSlicerTablesLogic* logic);

  QString description() const override;
  IOFileType fileType() const override;
  QStringList extensions() const override;

  /// Returns a positive number (>0) if the reader can load this file.
  /// It only differs from the default confidence (based on file extension matching)
  /// that .txt files are recognized with a reduced confidence of 0.4, because
  /// .txt files more likely store simple text than a table.
  double canLoadFileConfidence(const QString& file) const override;

  bool load(const IOProperties& properties) override;

protected:
  QScopedPointer<qSlicerTablesReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTablesReader);
  Q_DISABLE_COPY(qSlicerTablesReader);
};

#endif
