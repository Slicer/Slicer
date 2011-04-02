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

#ifndef __qSlicerEMSegmentEditRegistrationParameterStep_p_h
#define __qSlicerEMSegmentEditRegistrationParameterStep_p_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

// EMSegment includes
#include "qSlicerEMSegmentEditRegistrationParametersStep.h"
#include "ui_qSlicerEMSegmentEditRegistrationParametersStep.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentEditRegistrationParametersStepPrivate : public QObject,
                                                              public Ui_qSlicerEMSegmentEditRegistrationParametersStep
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qSlicerEMSegmentEditRegistrationParametersStep)
protected:
  qSlicerEMSegmentEditRegistrationParametersStep* const q_ptr;
public:
  qSlicerEMSegmentEditRegistrationParametersStepPrivate(qSlicerEMSegmentEditRegistrationParametersStep& object);

  void updateWidgetFromMRML();

  void updateMRMLFromWidget();

  void setupUi(qSlicerEMSegmentWorkflowWidgetStep *step);

  void updateAtlasScansToInputChannelsLayoutFromMRML();

  void updateMRMLFromAtlasScansToInputChannelsLayout();

private slots:

  void onCurrentAffineRegistrationIndexChanged(int index);
  void onCurrentDeformableRegistrationIndexChanged(int index);
  void onCurrentInterpolationIndexChanged(int index);

};

#endif
