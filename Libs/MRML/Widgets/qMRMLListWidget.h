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

#ifndef __qMRMLListWidget_h
#define __qMRMLListWidget_h

// Qt includes
#include <QListView>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

class qMRMLListWidgetPrivate;
class vtkMRMLScene;

class QMRML_WIDGETS_EXPORT qMRMLListWidget : public QListView
{
  Q_OBJECT
public:
  qMRMLListWidget(QWidget *parent=nullptr);
  ~qMRMLListWidget() override;

  vtkMRMLScene* mrmlScene()const;

public slots:
  void setMRMLScene(vtkMRMLScene* scene);

protected:
  QScopedPointer<qMRMLListWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLListWidget);
  Q_DISABLE_COPY(qMRMLListWidget);
};

#endif
