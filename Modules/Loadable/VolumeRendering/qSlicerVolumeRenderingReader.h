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

#ifndef __qSlicerVolumeRenderingReader_h
#define __qSlicerVolumeRenderingReader_h

// Slicer includes
#include <qSlicerFileReader.h>

// Volume Rendering includes
class qSlicerVolumeRenderingReaderPrivate;
class vtkSlicerVolumeRenderingLogic;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerVolumeRenderingReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerVolumeRenderingReader(QObject* parent = nullptr);
  qSlicerVolumeRenderingReader(vtkSlicerVolumeRenderingLogic* logic, QObject* parent = nullptr);
  ~qSlicerVolumeRenderingReader() override;

  void setVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* logic);
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic()const;

  // Reimplemented for IO specific description
  QString description()const override;
  IOFileType fileType()const override;
  QStringList extensions()const override;

  bool load(const IOProperties& properties) override;

protected:
  QScopedPointer<qSlicerVolumeRenderingReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeRenderingReader);
  Q_DISABLE_COPY(qSlicerVolumeRenderingReader);
};

#endif
