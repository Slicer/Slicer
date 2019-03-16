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

#ifndef __qMRMLModelInfoWidget_h
#define __qMRMLModelInfoWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class qMRMLModelInfoWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLModelNode;

class QMRML_WIDGETS_EXPORT qMRMLModelInfoWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  qMRMLModelInfoWidget(QWidget *parent=nullptr);
  ~qMRMLModelInfoWidget() override;

  vtkMRMLModelNode* mrmlModelNode()const;

public slots:
  /// Utility function to be connected with generic signals
  void setMRMLModelNode(vtkMRMLNode *node);
  /// Set the Model node to display
  void setMRMLModelNode(vtkMRMLModelNode *node);

protected slots:
  void updateWidgetFromMRML();

protected:
  void showEvent(QShowEvent *) override;
  QScopedPointer<qMRMLModelInfoWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLModelInfoWidget);
  Q_DISABLE_COPY(qMRMLModelInfoWidget);
};

#endif
