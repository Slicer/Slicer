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

#ifndef __qMRMLClipNodeWidget_h
#define __qMRMLClipNodeWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"
#include "vtkMRMLClipModelsNode.h"

class qMRMLClipNodeWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLClipModelsNode;

class QMRML_WIDGETS_EXPORT qMRMLClipNodeWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  qMRMLClipNodeWidget(QWidget *parent=nullptr);
  ~qMRMLClipNodeWidget() override;

  vtkMRMLClipModelsNode* mrmlClipNode()const;

  int clipType()const;
  int redSliceClipState()const;
  int yellowSliceClipState()const;
  int greenSliceClipState()const;
  vtkMRMLClipModelsNode::ClippingMethodType clippingMethod()const;

  void setClipType(int);
  void setRedSliceClipState(int);
  void setYellowSliceClipState(int);
  void setGreenSliceClipState(int);
  void setClippingMethod(vtkMRMLClipModelsNode::ClippingMethodType);

public slots:
  /// Set the clip node to represent
  void setMRMLClipNode(vtkMRMLClipModelsNode *node);
  /// Utility function to be connected to signals/slots
  void setMRMLClipNode(vtkMRMLNode *node);

protected slots:
  void updateWidgetFromMRML();

  void updateNodeClipType();
  void updateNodeRedClipState();
  void updateNodeYellowClipState();
  void updateNodeGreenClipState();
  void updateNodeClippingMethod();

protected:
  QScopedPointer<qMRMLClipNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLClipNodeWidget);
  Q_DISABLE_COPY(qMRMLClipNodeWidget);
};

#endif
