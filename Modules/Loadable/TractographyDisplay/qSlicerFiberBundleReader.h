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

#ifndef __qSlicerFiberBundleReader_h
#define __qSlicerFiberBundleReader_h

// SlicerQt includes
#include "qSlicerFileReader.h"
class qSlicerFiberBundleReaderPrivate;

// Slicer includes
class vtkSlicerFiberBundleLogic;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TractographyDisplay
class qSlicerFiberBundleReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerFiberBundleReader(vtkSlicerFiberBundleLogic* fiberBundleLogic, QObject* parent = 0);
  virtual ~qSlicerFiberBundleReader();

  void setFiberBundleLogic(vtkSlicerFiberBundleLogic* fiberBundleLogic);
  vtkSlicerFiberBundleLogic* fiberBundleLogic()const;

  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QStringList extensions()const;

  virtual bool load(const IOProperties& properties);

protected:
  QScopedPointer<qSlicerFiberBundleReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerFiberBundleReader);
  Q_DISABLE_COPY(qSlicerFiberBundleReader);
};

#endif
