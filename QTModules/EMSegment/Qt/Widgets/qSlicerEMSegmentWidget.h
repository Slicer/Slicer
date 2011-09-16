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

#ifndef __qSlicerEMSegmentWidget_h
#define __qSlicerEMSegmentWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerWidget.h"

#include "qSlicerEMSegmentModuleWidgetsExport.h"

class qSlicerEMSegmentWidgetPrivate;
class vtkEMSegmentMRMLManager;

/// \ingroup Slicer_QtModules_EMSegment
class Q_SLICER_MODULE_EMSEGMENT_WIDGET_EXPORT qSlicerEMSegmentWidget : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerWidget Superclass;
  explicit qSlicerEMSegmentWidget(QWidget *newParent = 0);
  virtual ~qSlicerEMSegmentWidget();

  vtkEMSegmentMRMLManager* mrmlManager() const;

public slots:
  virtual void setMRMLManager(vtkEMSegmentMRMLManager * newMRMLManager);

signals:
  void mrmlManagerChanged(vtkEMSegmentMRMLManager* newMRMLManager);

protected:
  QScopedPointer<qSlicerEMSegmentWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerEMSegmentWidget);
  Q_DISABLE_COPY(qSlicerEMSegmentWidget);

};

#endif
