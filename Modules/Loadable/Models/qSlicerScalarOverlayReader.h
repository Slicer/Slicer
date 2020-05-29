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

#ifndef __qSlicerScalarOverlayReader_h
#define __qSlicerScalarOverlayReader_h

// Slicer includes
#include "qSlicerFileReader.h"
class qSlicerScalarOverlayReaderPrivate;

// Slicer includes
class vtkSlicerModelsLogic;

//-----------------------------------------------------------------------------
class qSlicerScalarOverlayReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerScalarOverlayReader(vtkSlicerModelsLogic* modelsLogic, QObject* parent = nullptr);
  ~qSlicerScalarOverlayReader() override;

  void setModelsLogic(vtkSlicerModelsLogic* modelsLogic);
  vtkSlicerModelsLogic* modelsLogic()const;

  QString description()const override;
  IOFileType fileType()const override;
  QStringList extensions()const override;
  qSlicerIOOptions* options()const override;

  bool load(const IOProperties& properties) override;

protected:
  QScopedPointer<qSlicerScalarOverlayReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScalarOverlayReader);
  Q_DISABLE_COPY(qSlicerScalarOverlayReader);
};

#endif
