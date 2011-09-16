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

  This file was originally developed by
    Danielle Pace and Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerEMSegmentDefineAtlasStep_h
#define __qSlicerEMSegmentDefineAtlasStep_h

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidgetStep.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentDefineAtlasStepPrivate;

/// \ingroup Slicer_QtModules_EMSegment
class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentDefineAtlasStep : public qSlicerEMSegmentWorkflowWidgetStep
{
  Q_OBJECT

public:

  const static QString StepId;

  typedef qSlicerEMSegmentWorkflowWidgetStep Superclass;
  explicit qSlicerEMSegmentDefineAtlasStep(ctkWorkflow* newWorkflow, QWidget* parent = 0);
  virtual ~qSlicerEMSegmentDefineAtlasStep();

public slots:

  virtual void createUserInterface();

  virtual void onEntry(const ctkWorkflowStep* comingFrom,
                       const ctkWorkflowInterstepTransition::InterstepTransitionType transitionType);

  virtual void showUserInterface();

protected:
  QScopedPointer<qSlicerEMSegmentDefineAtlasStepPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerEMSegmentDefineAtlasStep);
  Q_DISABLE_COPY(qSlicerEMSegmentDefineAtlasStep);

};



#endif
