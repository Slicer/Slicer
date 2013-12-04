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

#ifndef __qSlicerDoubleArraysReader
#define __qSlicerDoubleArraysReader

// SlicerQt includes
#include "qSlicerFileReader.h"
class qSlicerDoubleArraysReaderPrivate;
class vtkSlicerDoubleArraysLogic;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_DoubleArray
class qSlicerDoubleArraysReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerDoubleArraysReader(QObject* parent = 0);
  qSlicerDoubleArraysReader(vtkSlicerDoubleArraysLogic* logic,
                       QObject* parent = 0);
  virtual ~qSlicerDoubleArraysReader();

  vtkSlicerDoubleArraysLogic* logic()const;
  void setLogic(vtkSlicerDoubleArraysLogic* logic);

  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QStringList extensions()const;

  virtual bool load(const IOProperties& properties);
protected:
  QScopedPointer<qSlicerDoubleArraysReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDoubleArraysReader);
  Q_DISABLE_COPY(qSlicerDoubleArraysReader);
};

#endif
