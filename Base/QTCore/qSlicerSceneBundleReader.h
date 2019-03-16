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

#ifndef __qSlicerSceneBundleReader_h
#define __qSlicerSceneBundleReader_h

// QtCore includes
#include "qSlicerFileReader.h"

///
/// qSlicerSceneBundleReader is the IO class that handle MRML scene
/// embedded in a zip file (called a Slicer Data Bundle).  The extension
/// is mrb (for Medical Reality Bundle)
/// It internally calls vtkMRMLScene::Connect() or vtkMRMLScene::Import()
/// depending on the clear flag.
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerSceneBundleReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerSceneBundleReader(QObject* _parent = nullptr);

  QString description()const override;
  /// Support QString("SceneFile")
  qSlicerIO::IOFileType fileType()const override;

  /// Support only .mrb files
  QStringList extensions()const override;

  /// the supported properties are:
  /// QString fileName: the path of the mrml scene to load
  /// bool clear: whether the current should be cleared or not
  bool load(const qSlicerIO::IOProperties& properties) override;
};


#endif
