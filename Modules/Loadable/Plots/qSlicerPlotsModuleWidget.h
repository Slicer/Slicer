/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef qSlicerPlotsModuleWidget_h
#define qSlicerPlotsModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

// Transforms includes
#include "qSlicerPlotsModuleExport.h"

class vtkMatrix4x4;
class vtkMRMLNode;
class qSlicerPlotsModuleWidgetPrivate;

class Q_SLICER_QTMODULES_PLOTS_EXPORT qSlicerPlotsModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerPlotsModuleWidget(QWidget* parent = nullptr);
  ~qSlicerPlotsModuleWidget() override;

  bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString()) override;

public slots:
  /// Select the specified node as the current node in the user interface
  void setCurrentPlotNode(vtkMRMLNode*);

protected:
  void setup() override;

protected slots:

  void onMRMLPlotChartNodeModified(vtkObject* caller);
  void onNodeSelected(vtkMRMLNode* node);
  void onLockPlotButtonClicked();
  void onCopyPlotSeriesNodeClicked();
  void onShowChartButtonClicked();
  void onSeriesNodeAddedByUser(vtkMRMLNode*);

protected:
  ///
  /// Convenient method to return the coordinate system currently selected
  // int coordinateReference() const;

protected:
  QScopedPointer<qSlicerPlotsModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerPlotsModuleWidget);
  Q_DISABLE_COPY(qSlicerPlotsModuleWidget);
};

#endif
