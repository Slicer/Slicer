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
#include "ui_qSlicerEMSegmentDefineTaskPanel.h"

// MRML includes
class vtkMRMLNode;

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefineTaskStepPrivate : public QObject,
                                              public ctkPrivate<qSlicerEMSegmentDefineTaskStep>,
                                              public Ui_qSlicerEMSegmentDefineTaskPanel
{
  Q_OBJECT

public:
  CTK_DECLARE_PUBLIC(qSlicerEMSegmentDefineTaskStep)
  qSlicerEMSegmentDefineTaskStepPrivate();

  QSignalMapper Mapper;

  void setupUi(qSlicerEMSegmentWorkflowWidgetStep* step);

signals:
  void modeChanged(const QString& branchId);

protected slots:
  void selectTask(vtkMRMLNode* mrmlNode);
};

#endif
