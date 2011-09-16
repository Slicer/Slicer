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

  This file was originally developed by Danielle Pace, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerEMSegmentSpecifyIntensityDistributionStep_p_h
#define __qSlicerEMSegmentSpecifyIntensityDistributionStep_p_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

// Qt includes
#include <QSignalMapper>
class QString;

// EMSegment includes
#include "qSlicerEMSegmentDefineTaskStep.h"
#include "ui_qSlicerEMSegmentDefineTaskStep.h"

// MRML includes
class vtkMRMLNode;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentDefineTaskStepPrivate : public QObject,
                                              public Ui_qSlicerEMSegmentDefineTaskStep
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qSlicerEMSegmentDefineTaskStep)
protected:
  qSlicerEMSegmentDefineTaskStep* const q_ptr;
public:
  qSlicerEMSegmentDefineTaskStepPrivate(qSlicerEMSegmentDefineTaskStep& object);

  QSignalMapper Mapper;

  void setupUi(qSlicerEMSegmentWorkflowWidgetStep* step);

signals:
  void modeChanged(const QString& branchId);

protected slots:
  void selectTask();
};

#endif
