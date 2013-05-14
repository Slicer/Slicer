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

#ifndef __qSlicerColorsModuleWidget_h
#define __qSlicerColorsModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

// Colors includes
#include "qSlicerColorsModuleExport.h"

class qSlicerColorsModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_COLORS_EXPORT qSlicerColorsModuleWidget
  : public qSlicerAbstractModuleWidget
{
  Q_OBJECT
public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerColorsModuleWidget(QWidget *parent=0);
  virtual ~qSlicerColorsModuleWidget();


public slots:
  void setCurrentColorNode(vtkMRMLNode*);
  void setNumberOfColors();
  void setLookupTableRange(double min, double max);
  void copyCurrentColorNode();

protected slots:
  void onMRMLColorNodeChanged(vtkMRMLNode* newColorNode);

protected:
  virtual void setup();
  virtual void setMRMLScene(vtkMRMLScene *scene);

protected:
  QScopedPointer<qSlicerColorsModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerColorsModuleWidget);
  Q_DISABLE_COPY(qSlicerColorsModuleWidget);
};

#endif
