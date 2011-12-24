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

#ifndef __qSlicerColorsIO_h
#define __qSlicerColorsIO_h

// SlicerQt includes
#include "qSlicerIO.h"
class qSlicerColorsIOPrivate;

// Slicer includes
class vtkSlicerColorLogic;

//-----------------------------------------------------------------------------
class qSlicerColorsIO: public qSlicerIO
{
  Q_OBJECT
public:
  qSlicerColorsIO(vtkSlicerColorLogic* colorLogic = 0, QObject* parent = 0);

  void setColorLogic(vtkSlicerColorLogic* colorLogic);
  vtkSlicerColorLogic* colorLogic()const;

  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QStringList extensions()const;

  virtual bool load(const IOProperties& properties);
protected:
  QScopedPointer<qSlicerColorsIOPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerColorsIO);
  Q_DISABLE_COPY(qSlicerColorsIO);
};

#endif
