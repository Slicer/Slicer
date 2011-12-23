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

#ifndef __qSlicerAnnotationsIOOptionsWidget_h
#define __qSlicerAnnotationsIOOptionsWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerIOOptionsWidget.h"

// Annotations includes
#include "qSlicerAnnotationsModuleExport.h"

class qSlicerAnnotationsIOOptionsWidgetPrivate;
class QButtonGroup;
/// \ingroup Slicer_QtModules_Annotations
class qSlicerAnnotationsIOOptionsWidget :
  public qSlicerIOOptionsWidget
{
  Q_OBJECT
public:
  qSlicerAnnotationsIOOptionsWidget(QWidget *parent=0);
  virtual ~qSlicerAnnotationsIOOptionsWidget();

  QButtonGroup* FileTypeButtonGroup;
                                   
public slots:
  virtual void setFileName(const QString& fileName);
  virtual void setFileNames(const QStringList& fileNames);

protected slots:
  void updateProperties();
protected:
  QScopedPointer<qSlicerAnnotationsIOOptionsWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerAnnotationsIOOptionsWidget);
  Q_DISABLE_COPY(qSlicerAnnotationsIOOptionsWidget);
};

#endif
