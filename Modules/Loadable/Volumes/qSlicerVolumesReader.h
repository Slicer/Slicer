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

#ifndef __qSlicerVolumesReader_h
#define __qSlicerVolumesReader_h

// SlicerQt includes
#include "qSlicerFileReader.h"
class qSlicerVolumesReaderPrivate;
class vtkSlicerVolumesLogic;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Volumes
class qSlicerVolumesReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerVolumesReader(QObject* parent = 0);
  qSlicerVolumesReader(vtkSlicerVolumesLogic* logic, QObject* parent = 0);
  virtual ~qSlicerVolumesReader();

  vtkSlicerVolumesLogic* logic()const;
  void setLogic(vtkSlicerVolumesLogic* logic);

  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QStringList extensions()const;
  virtual qSlicerIOOptions* options()const;

  virtual bool load(const IOProperties& properties);
protected:
  QScopedPointer<qSlicerVolumesReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumesReader);
  Q_DISABLE_COPY(qSlicerVolumesReader);
};

#endif
