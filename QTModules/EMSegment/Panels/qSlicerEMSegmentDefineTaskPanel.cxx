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

// Qt includes
#include <QDebug>
#include <QSignalMapper>

// EMSegment includes
#include "qSlicerEMSegmentDefineTaskPanel.h" 
#include "ui_qSlicerEMSegmentDefineTaskPanel.h"
#include "qSlicerEMSegmentModuleWidget.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSNode.h>

//-----------------------------------------------------------------------------
class qSlicerEMSegmentDefineTaskPanelPrivate :
    public ctkPrivate<qSlicerEMSegmentDefineTaskPanel>,
    public Ui_qSlicerEMSegmentDefineTaskPanel
{
public:
  qSlicerEMSegmentDefineTaskPanelPrivate()
    {
    }
  QSignalMapper Mapper;
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentDefineTaskPanel::qSlicerEMSegmentDefineTaskPanel(QWidget *newParent):
Superclass(newParent)
{
  CTK_INIT_PRIVATE(qSlicerEMSegmentDefineTaskPanel);
  CTK_D(qSlicerEMSegmentDefineTaskPanel);
  d->setupUi(this);

  // Connect TaskSelector
  connect(d->TaskSelectorComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          SLOT(selectTask(vtkMRMLNode*)));

  // Each button is mapped with the corresponding branch name
  d->Mapper.setMapping(d->SimpleModeButton, qSlicerEMSegmentModuleWidget::SimpleMode);
  d->Mapper.setMapping(d->AdvancedModeButton, qSlicerEMSegmentModuleWidget::AdvancedMode);

  // Connect buttons
  connect(d->SimpleModeButton, SIGNAL(clicked()), &d->Mapper, SLOT(map()));
  connect(d->AdvancedModeButton, SIGNAL(clicked()), &d->Mapper,SLOT(map()));

  connect(&d->Mapper, SIGNAL(mapped(const QString)), SIGNAL(modeChanged(const QString&)));
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentDefineTaskPanel::selectTask(vtkMRMLNode* mrmlNode)
{
  this->mrmlManager()->SetNode(vtkMRMLEMSNode::SafeDownCast(mrmlNode));
}
