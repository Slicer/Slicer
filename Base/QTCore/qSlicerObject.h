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

#ifndef __qSlicerObject_h
#define __qSlicerObject_h

// Qt includes
#include <QScopedPointer>

// CTK includes
#include "qSlicerBaseQTCoreExport.h"

class vtkMRMLScene;
class qSlicerObjectPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerObject
{
public:
  qSlicerObject();
  virtual ~qSlicerObject();

  /// Return a pointer on the MRML scene
  vtkMRMLScene* mrmlScene() const;

  /// Set the current MRML scene to the widget
  virtual void setMRMLScene(vtkMRMLScene*);

protected:
  QScopedPointer<qSlicerObjectPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerObject);
  Q_DISABLE_COPY(qSlicerObject);
};

#endif
