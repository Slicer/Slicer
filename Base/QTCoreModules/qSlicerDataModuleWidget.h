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

#ifndef __qSlicerDataModuleWidget_h
#define __qSlicerDataModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerBaseQTCoreModulesExport.h"

class vtkMRMLNode;
class qSlicerDataModuleWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerDataModuleWidget :
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
  void addDTI();

  void setMRMLIDsVisible(bool visible);

  /// Reimplemented for internal reasons
  virtual void setMRMLScene(vtkMRMLScene* scene);
/* HIDDEN to the UI
  void setCurrentNodeName(const QString& name);

protected slots:
  void onMRMLNodeChanged(vtkMRMLNode* node);
  void onMRMLNodeModified();
  void validateNodeName();
*/
public slots:
  void insertTransformNode();
  void hardenTransformOnCurrentNode();

protected slots:
  void onCurrentNodeChanged(vtkMRMLNode* newCurrentNode);
  void onSceneModelChanged(const QString& modelType);
  
protected:
  QScopedPointer<qSlicerDataModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerDataModuleWidget);
  Q_DISABLE_COPY(qSlicerDataModuleWidget);
};

#endif
