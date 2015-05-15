/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qSlicerSimpleMarkupsWidget_h
#define __qSlicerSimpleMarkupsWidget_h

// Qt includes
#include "qSlicerWidget.h"

#include "qMRMLUtils.h"

// Markups Widgets includes
#include "qSlicerMarkupsModuleWidgetsExport.h"
#include "ui_qSlicerSimpleMarkupsWidget.h"


class qSlicerSimpleMarkupsWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT 
qSlicerSimpleMarkupsWidget : public qSlicerWidget
{
  Q_OBJECT

public:
  typedef qSlicerWidget Superclass;
  qSlicerSimpleMarkupsWidget(QWidget *parent=0);
  virtual ~qSlicerSimpleMarkupsWidget();

  Q_INVOKABLE vtkMRMLNode* getCurrentNode();
  
public slots:
  void setCurrentNode(vtkMRMLNode* currentNode);
  void setNodeBaseName(QString newNodeBaseName);

  void getNodeColor(QColor color);
  void setNodeColor(QColor color);
  void setDefaultNodeColor(QColor color);

  void highlightNthFiducial(int n);
  void activate();

protected slots:
  void onColorButtonChanged(QColor);
  void onVisibilityButtonClicked();
  void onLockButtonClicked();
  void onDeleteButtonClicked();
  void onPlaceButtonClicked();
  void onActiveButtonClicked();

  void onMarkupsFiducialNodeChanged();
  void onMarkupsFiducialNodeAdded(vtkMRMLNode*);
  void onMarkupsFiducialTableContextMenu(const QPoint& position);

  void onMarkupsFiducialEdited(int row, int column);

  void updateWidget();

signals:
  void markupsFiducialNodeChanged();
  void markupsFiducialActivated();
  void markupsFiducialPlaceModeChanged();
  void updateFinished();

protected:
  QScopedPointer<qSlicerSimpleMarkupsWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();

  void connectInteractionAndSelectionNodes();

private:
  Q_DECLARE_PRIVATE(qSlicerSimpleMarkupsWidget);
  Q_DISABLE_COPY(qSlicerSimpleMarkupsWidget);

};

#endif
