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

#ifndef __qSlicerTransformsModuleWidget_h
#define __qSlicerTransformsModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

// Transforms includes
#include "qSlicerTablesModuleExport.h"

class vtkMatrix4x4;
class vtkMRMLNode;
class qSlicerTablesModuleWidgetPrivate;

class Q_SLICER_QTMODULES_TABLES_EXPORT qSlicerTablesModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTablesModuleWidget(QWidget *parent=0);
  virtual ~qSlicerTablesModuleWidget();

public slots:
  /// Select the specified node as the current node in the user interface
  void setCurrentTableNode(vtkMRMLNode*);

protected:

  virtual void setup();

protected slots:

  void onMRMLTableNodeModified(vtkObject* caller);
  void onNodeSelected(vtkMRMLNode* node);
  void onLockTableButtonClicked();

protected:
  ///
  /// Convenient method to return the coordinate system currently selected
  //int coordinateReference()const;

protected:
  QScopedPointer<qSlicerTablesModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTablesModuleWidget);
  Q_DISABLE_COPY(qSlicerTablesModuleWidget);
};

#endif
