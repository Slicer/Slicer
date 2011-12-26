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

#ifndef __qSlicerTransformsIO_h
#define __qSlicerTransformsIO_h

// SlicerQt includes
#include "qSlicerIO.h"
class qSlicerTransformsIOPrivate;

// Slicer includes
class vtkSlicerTransformLogic;

//-----------------------------------------------------------------------------
class qSlicerTransformsIO: public qSlicerIO
{
  Q_OBJECT
public:
  qSlicerTransformsIO(vtkSlicerTransformLogic* transformLogic, QObject* parent = 0);
  virtual ~qSlicerTransformsIO();

  void setTransformLogic(vtkSlicerTransformLogic* transformLogic);
  vtkSlicerTransformLogic* transformLogic()const;

  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QStringList extensions()const;

  virtual bool load(const IOProperties& properties);

protected:
  QScopedPointer<qSlicerTransformsIOPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTransformsIO);
  Q_DISABLE_COPY(qSlicerTransformsIO);
};

#endif
