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

#ifndef __qSlicerTablesReader
#define __qSlicerTablesReader

// Slicer includes
#include "qSlicerFileReader.h"

class qSlicerTablesReaderPrivate;
class vtkSlicerTablesLogic;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_DoubleArray
class qSlicerTablesReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerTablesReader(QObject* parent = nullptr);
  qSlicerTablesReader(vtkSlicerTablesLogic* logic,
                       QObject* parent = nullptr);
  ~qSlicerTablesReader() override;

  vtkSlicerTablesLogic* logic()const;
  void setLogic(vtkSlicerTablesLogic* logic);

  QString description()const override;
  IOFileType fileType()const override;
  QStringList extensions()const override;

  bool load(const IOProperties& properties) override;
protected:
  QScopedPointer<qSlicerTablesReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTablesReader);
  Q_DISABLE_COPY(qSlicerTablesReader);
};

#endif
