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

  This file was originally developed by Simon Drouin, Brigham and Women's
  Hospital, Boston, MA.

==============================================================================*/

#ifndef __qSlicerShaderPropertyReader_h
#define __qSlicerShaderPropertyReader_h

// Slicer includes
#include <qSlicerFileReader.h>

// Volume Rendering includes
class qSlicerShaderPropertyReaderPrivate;
class vtkSlicerVolumeRenderingLogic;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerShaderPropertyReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerShaderPropertyReader(QObject* parent = nullptr);
  qSlicerShaderPropertyReader(vtkSlicerVolumeRenderingLogic* logic, QObject* parent = nullptr);
  ~qSlicerShaderPropertyReader() override;

  void setVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* logic);
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic()const;

  // Reimplemented for IO specific description
  QString description() const override;
  IOFileType fileType() const override;
  QStringList extensions() const override;

  bool load(const IOProperties& properties) override ;

protected:
  QScopedPointer<qSlicerShaderPropertyReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerShaderPropertyReader);
  Q_DISABLE_COPY(qSlicerShaderPropertyReader);
};

#endif
