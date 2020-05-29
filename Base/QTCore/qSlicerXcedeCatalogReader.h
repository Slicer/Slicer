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

#ifndef __qSlicerXcedeCatalogReader
#define __qSlicerXcedeCatalogReader

// Slicer includes
#include "qSlicerFileReader.h"

class qSlicerXcedeCatalogReaderPrivate;

// Slicer Logic
class vtkMRMLColorLogic;

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerXcedeCatalogReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerXcedeCatalogReader(QObject* parent = nullptr);
  qSlicerXcedeCatalogReader(vtkMRMLColorLogic* logic, QObject* parent = nullptr);
  ~qSlicerXcedeCatalogReader() override;

  vtkMRMLColorLogic* colorLogic()const;
  void setColorLogic(vtkMRMLColorLogic* logic);

  QString description()const override;
  IOFileType fileType()const override;
  QStringList extensions()const override;

  bool load(const IOProperties& properties) override;
protected:
  QScopedPointer<qSlicerXcedeCatalogReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerXcedeCatalogReader);
  Q_DISABLE_COPY(qSlicerXcedeCatalogReader);
};

#endif
