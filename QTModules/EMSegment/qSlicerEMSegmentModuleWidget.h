/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerEMSegmentModuleWidget_h
#define __qSlicerEMSegmentModuleWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentModuleWidgetPrivate;
class vtkMRMLNode;
class ctkWorkflowStep;

/// \ingroup Slicer_QtModules_EMSegment
class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  const static QString SimpleMode;
  const static QString AdvancedMode;

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerEMSegmentModuleWidget(QWidget *parent=0);
  virtual ~qSlicerEMSegmentModuleWidget();

protected:
  virtual void setup();

protected:
  QScopedPointer<qSlicerEMSegmentModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerEMSegmentModuleWidget);
  Q_DISABLE_COPY(qSlicerEMSegmentModuleWidget);
};

#endif
