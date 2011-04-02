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

// EMSegment includes
#include "qSlicerEMSegmentWidget.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentWidgetPrivate
{
public:
  qSlicerEMSegmentWidgetPrivate();

  vtkEMSegmentMRMLManager *  MRMLManager;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentWidgetPrivate::qSlicerEMSegmentWidgetPrivate()
{
  this->MRMLManager = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentWidget methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentWidget::qSlicerEMSegmentWidget(QWidget *newParent) :
    Superclass(newParent)
  , d_ptr(new qSlicerEMSegmentWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerEMSegmentWidget::~qSlicerEMSegmentWidget()
{
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerEMSegmentWidget, vtkEMSegmentMRMLManager*, mrmlManager, MRMLManager);

//-----------------------------------------------------------------------------
void qSlicerEMSegmentWidget::setMRMLManager(vtkEMSegmentMRMLManager* newMRMLManager)
{
  Q_D(qSlicerEMSegmentWidget);
  if (d->MRMLManager == newMRMLManager)
    {
    return;
    }
  d->MRMLManager = newMRMLManager;
  this->setMRMLScene(newMRMLManager ? newMRMLManager->GetMRMLScene() : 0);
  emit mrmlManagerChanged(newMRMLManager);
}
