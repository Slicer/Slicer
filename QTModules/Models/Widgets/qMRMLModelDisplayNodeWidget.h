/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

#ifndef __qMRMLModelDisplayNodeWidget_h
#define __qMRMLModelDisplayNodeWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qSlicerModelsModuleExport.h"

class qMRMLModelDisplayNodeWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLModelDisplayNode;
class vtkMRMLColorTableNode;

class Q_SLICER_QTMODULES_MODELS_EXPORT qMRMLModelDisplayNodeWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  qMRMLModelDisplayNodeWidget(QWidget *parent=0);
  virtual ~qMRMLModelDisplayNodeWidget();
  
  vtkMRMLModelDisplayNode* mrmlModelDisplayNode()const;

  bool scalarsVisibility()const;
  QString activeScalarName()const;
  vtkMRMLColorTableNode* scalarsColorTable()const;

public slots:
  /// Set the volume node to display
  void setMRMLModelDisplayNode(vtkMRMLModelDisplayNode *node);
  /// Utility function to be connected with generic signals
  void setMRMLModelDisplayNode(vtkMRMLNode *node);
  /// Utility function to be connected with generic signals,
  /// it internally shows the 1st display node.
  void setMRMLNode(vtkMRMLNode* modelNode);
  
  void setScalarsVisibility(bool);
  void setActiveScalarName(const QString&);
  void setScalarsColorTable(vtkMRMLNode*);
  void setScalarsColorTable(vtkMRMLColorTableNode*);

protected slots:
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qMRMLModelDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLModelDisplayNodeWidget);
  Q_DISABLE_COPY(qMRMLModelDisplayNodeWidget);
};

#endif
