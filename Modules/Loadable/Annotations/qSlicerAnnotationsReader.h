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

#ifndef __qSlicerAnnotationsReader_h
#define __qSlicerAnnotationsReader_h

// Slicer includes
#include "qSlicerFileReader.h"

class qSlicerAnnotationsReaderPrivate;
class vtkSlicerAnnotationModuleLogic;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotations
class qSlicerAnnotationsReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerAnnotationsReader(QObject* parent = nullptr);
  qSlicerAnnotationsReader(vtkSlicerAnnotationModuleLogic* logic, QObject* parent = nullptr);
  ~qSlicerAnnotationsReader() override;

  vtkSlicerAnnotationModuleLogic* annotationLogic()const;
  void setAnnotationLogic(vtkSlicerAnnotationModuleLogic* logic);

  QString description()const override;
  IOFileType fileType()const override;
  QStringList extensions()const override;
  qSlicerIOOptions* options()const override;

  bool load(const IOProperties& properties) override;
protected:
  QScopedPointer<qSlicerAnnotationsReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerAnnotationsReader);
  Q_DISABLE_COPY(qSlicerAnnotationsReader);

};

#endif
