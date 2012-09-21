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

#ifndef __qSlicerDataModuleWidget_h
#define __qSlicerDataModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerDataModuleExport.h"

class vtkMRMLNode;
class qSlicerDataModuleWidgetPrivate;
class QTableWidgetItem;

class Q_SLICER_QTMODULES_DATA_EXPORT qSlicerDataModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:
  qSlicerDataModuleWidget(QWidget *parentWidget = 0);
  virtual ~qSlicerDataModuleWidget();

public slots:
  void loadScene();
  void addScene();
  void addData();
  void addVolumes();
  void addModels();
  void addScalarOverlay();
  void addTransformation();
  void addFiducialList();
  void addColorTable();
  void addFiberBundle();

  void setMRMLIDsVisible(bool visible);

  /// Reimplemented for internal reasons
  virtual void setMRMLScene(vtkMRMLScene* scene);

protected slots:
  void onCurrentNodeChanged(vtkMRMLNode* newCurrentNode);
  void onSceneModelChanged(const QString& modelType);

public slots:
  void insertTransformNode();
  void hardenTransformOnCurrentNode();
  void printObject();

protected:
  virtual void setup();
  
protected:
  QScopedPointer<qSlicerDataModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDataModuleWidget);
  Q_DISABLE_COPY(qSlicerDataModuleWidget);
};

#endif
