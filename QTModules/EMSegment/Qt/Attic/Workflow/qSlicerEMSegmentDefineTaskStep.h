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

#ifndef __qSlicerEMSegmentDefineTaskStep_h
#define __qSlicerEMSegmentDefineTaskStep_h

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentWorkflowWidgetStep.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentDefineTaskStepPrivate;

/// \ingroup Slicer_QtModules_EMSegment
class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentDefineTaskStep : public qSlicerEMSegmentWorkflowWidgetStep
{
  Q_OBJECT

public:

  const static QString StepId;

  typedef qSlicerEMSegmentWorkflowWidgetStep Superclass;
  explicit qSlicerEMSegmentDefineTaskStep(ctkWorkflow* newWorkflow, QWidget* parent = 0);
  virtual ~qSlicerEMSegmentDefineTaskStep();

public slots:

  virtual void createUserInterface();

protected:
  QScopedPointer<qSlicerEMSegmentDefineTaskStepPrivate> d_ptr;

  void showDefaultTasksList();
  int loadDefaultTask(int index);

private:
  Q_DECLARE_PRIVATE(qSlicerEMSegmentDefineTaskStep);
  Q_DISABLE_COPY(qSlicerEMSegmentDefineTaskStep);

  //BTX:
  std::vector<std::string> pssDefaultTasksName;
  std::vector<std::string> pssDefaultTasksFile;
  std::vector<std::string> DefinePreprocessingTasksName;
  std::vector<std::string> DefinePreprocessingTasksFile;
 //ETX:

};

#endif
