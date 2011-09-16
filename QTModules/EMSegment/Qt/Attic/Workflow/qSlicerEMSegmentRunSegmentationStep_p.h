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

#ifndef __qSlicerEMSegmentRunSegmentationStep_p_h
#define __qSlicerEMSegmentRunSegmentationStep_p_h

// EMSegment includes
#include "qSlicerEMSegmentRunSegmentationStep.h"
#include "ui_qSlicerEMSegmentRunSegmentationStep.h"

// EMSegment/MRML includes
class vtkMRMLAnnotationROINode;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentRunSegmentationStepPrivate : public QObject,
                                                   public Ui_qSlicerEMSegmentRunSegmentationStep
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qSlicerEMSegmentRunSegmentationStep)
protected:
  qSlicerEMSegmentRunSegmentationStep* const q_ptr;

public:
  qSlicerEMSegmentRunSegmentationStepPrivate(qSlicerEMSegmentRunSegmentationStep& object);

  void setupUi(qSlicerEMSegmentWorkflowWidgetStep *step);

  void setMRMLROINode(vtkMRMLAnnotationROINode* newROINode);

  // Delete any ROI nodes named "SegmentationROI", and create a new ROI node
  vtkMRMLAnnotationROINode* createROINode();

  // Propagate changes in ROINode MRML to EMSegmentRunSegmentationStep ROI MRML
  void updateMRMLROINodeUsingInputVolume();

  void updateWidgetFromMRML();

  void updateMRMLFromWidget();

  vtkMRMLAnnotationROINode* ROINode;

private slots:

  void display2DVOI(bool show);
  void initializeNode(vtkMRMLNode* node);

};

#endif
