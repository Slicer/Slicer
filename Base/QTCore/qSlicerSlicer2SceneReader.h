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

#ifndef __qSlicerSlicer2SceneReader
#define __qSlicerSlicer2SceneReader

// Slicer includes
#include "qSlicerFileReader.h"
class qSlicerSlicer2SceneReaderPrivate;

// SlicerLogic includes
class vtkSlicerApplicationLogic;

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerSlicer2SceneReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerSlicer2SceneReader(QObject* parent = nullptr);
  qSlicerSlicer2SceneReader(vtkSlicerApplicationLogic* logic, QObject* parent = nullptr);
  ~qSlicerSlicer2SceneReader() override;

  vtkSlicerApplicationLogic* applicationLogic()const;
  void setApplicationLogic(vtkSlicerApplicationLogic* logic);

  QString description()const override;
  IOFileType fileType()const override;
  QStringList extensions()const override;

  bool load(const IOProperties& properties) override;
protected:
  QScopedPointer<qSlicerSlicer2SceneReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSlicer2SceneReader);
  Q_DISABLE_COPY(qSlicerSlicer2SceneReader);
};

#endif
