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

#ifndef __qSlicerSceneReader_h
#define __qSlicerSceneReader_h

// QtCore includes
#include "qSlicerDataModuleExport.h"
#include "qSlicerFileReader.h"

// Logic includes
class vtkSlicerCamerasModuleLogic;
class qSlicerSceneReaderPrivate;

///
/// qSlicerSceneReader is the IO class that handle MRML scene
/// It internally call vtkMRMLScene::Connect() or vtkMRMLScene::Import()
/// depending on the clear flag.
class Q_SLICER_QTMODULES_DATA_EXPORT qSlicerSceneReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerSceneReader(vtkSlicerCamerasModuleLogic* camerasLogic, QObject* _parent = nullptr);
  ~qSlicerSceneReader() override;

  QString description()const override;
  /// Support QString("SceneFile")
  qSlicerIO::IOFileType fileType()const override;

  /// Support only .mrml files
  QStringList extensions()const override;

  /// Options to control scene loading
  qSlicerIOOptions* options()const override;

  /// the supported properties are:
  /// QString fileName: the path of the mrml scene to load
  /// bool clear: whether the current should be cleared or not
  bool load(const qSlicerIO::IOProperties& properties) override;
protected:
  QScopedPointer<qSlicerSceneReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSceneReader);
  Q_DISABLE_COPY(qSlicerSceneReader);
};


#endif
