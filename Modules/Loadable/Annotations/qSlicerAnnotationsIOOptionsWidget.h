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

// Slicer includes
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
  qSlicerAnnotationsIOOptionsWidget(QWidget *parent=nullptr);
  ~qSlicerAnnotationsIOOptionsWidget() override;

  QButtonGroup* FileTypeButtonGroup;

public slots:
  void setFileName(const QString& fileName) override;
  void setFileNames(const QStringList& fileNames) override;

protected slots:
  void updateProperties();

private:
  Q_DECLARE_PRIVATE_D(qGetPtrHelper(qSlicerIOOptions::d_ptr),
                      qSlicerAnnotationsIOOptionsWidget);
  Q_DISABLE_COPY(qSlicerAnnotationsIOOptionsWidget);
};

#endif
