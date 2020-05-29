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

#ifndef __qSlicerTransformsReader_h
#define __qSlicerTransformsReader_h

// Slicer includes
#include "qSlicerFileReader.h"
class qSlicerTransformsReaderPrivate;

// Slicer includes
class vtkSlicerTransformLogic;

//-----------------------------------------------------------------------------
class qSlicerTransformsReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerTransformsReader(vtkSlicerTransformLogic* transformLogic, QObject* parent = nullptr);
  ~qSlicerTransformsReader() override;

  void setTransformLogic(vtkSlicerTransformLogic* transformLogic);
  vtkSlicerTransformLogic* transformLogic()const;

  QString description()const override;
  IOFileType fileType()const override;
  QStringList extensions()const override;

  bool load(const IOProperties& properties) override;

protected:
  QScopedPointer<qSlicerTransformsReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTransformsReader);
  Q_DISABLE_COPY(qSlicerTransformsReader);
};

#endif
