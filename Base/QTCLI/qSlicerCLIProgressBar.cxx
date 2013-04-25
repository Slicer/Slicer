/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
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
#include <QApplication>
#include <QDebug>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>

// Slicer includes
#include "qSlicerCLIProgressBar.h"

// SlicerExecutionModel includes
#include <ModuleDescription.h>
#include <ModuleProcessInformation.h>

// MRML includes
#include <vtkMRMLCommandLineModuleNode.h>

//-----------------------------------------------------------------------------
// qSlicerCLIProgressBarPrivate methods

//-----------------------------------------------------------------------------
class qSlicerCLIProgressBarPrivate
{
  Q_DECLARE_PUBLIC(qSlicerCLIProgressBar);
protected:
  qSlicerCLIProgressBar* const q_ptr;
public:
  typedef qSlicerCLIProgressBarPrivate Self;
  qSlicerCLIProgressBarPrivate(qSlicerCLIProgressBar& object);

  void init();

private:

  QGridLayout *  GridLayout;
  QLabel *       StatusLabelLabel;
  QLabel *       StatusLabel;
  QProgressBar * ProgressBar;
  QProgressBar * StageProgressBar;

  vtkMRMLCommandLineModuleNode* CommandLineModuleNode;
  bool VisibleAfterExecution;

};

//-----------------------------------------------------------------------------
// qSlicerCLIProgressBarPrivate methods

//-----------------------------------------------------------------------------
qSlicerCLIProgressBarPrivate::qSlicerCLIProgressBarPrivate(qSlicerCLIProgressBar& object)
  :q_ptr(&object)
{
  this->CommandLineModuleNode = 0;
  this->VisibleAfterExecution = true;
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBarPrivate::init()
{
  // Create widget .. layout
  this->GridLayout = new QGridLayout(this->q_ptr);
  this->GridLayout->setObjectName(QString::fromUtf8("gridLayout"));

  this->StatusLabelLabel = new QLabel();
  this->StatusLabelLabel->setObjectName(QString::fromUtf8("StatusLabelLabel"));
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(StatusLabelLabel->sizePolicy().hasHeightForWidth());
  this->StatusLabelLabel->setSizePolicy(sizePolicy);
  this->StatusLabelLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

  this->GridLayout->addWidget(StatusLabelLabel, 1, 0, 1, 1);

  this->StatusLabel = new QLabel();
  this->StatusLabel->setObjectName(QString::fromUtf8("StatusLabel"));

  this->GridLayout->addWidget(StatusLabel, 1, 1, 1, 1);

  this->ProgressBar = new QProgressBar();
  this->ProgressBar->setObjectName(QString::fromUtf8("ProgressBar"));
  this->ProgressBar->setMaximum(100);
  this->ProgressBar->setValue(0);

  this->GridLayout->addWidget(ProgressBar, 2, 0, 1, 2);

  this->StageProgressBar = new QProgressBar();
  this->StageProgressBar->setObjectName(QString::fromUtf8("StageProgressBar"));
  this->StageProgressBar->setValue(0);
  this->GridLayout->addWidget(StageProgressBar, 3, 0, 1, 2);

  this->StatusLabelLabel->setText(QObject::tr("Status:"));
  this->StatusLabel->setText(QObject::tr("Idle"));
}

//-----------------------------------------------------------------------------
// qSlicerCLIProgressBar methods

//-----------------------------------------------------------------------------
qSlicerCLIProgressBar::qSlicerCLIProgressBar(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCLIProgressBarPrivate(*this))
{
  Q_D(qSlicerCLIProgressBar);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerCLIProgressBar::~qSlicerCLIProgressBar()
{
}

//-----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode * qSlicerCLIProgressBar::commandLineModuleNode()const
{
  Q_D(const qSlicerCLIProgressBar);
  return d->CommandLineModuleNode;
}

//-----------------------------------------------------------------------------
bool qSlicerCLIProgressBar::isVisibleAfterExecution()const
{
  Q_D(const qSlicerCLIProgressBar);
  return d->VisibleAfterExecution;
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBar::setCommandLineModuleNode(
  vtkMRMLCommandLineModuleNode* commandLineModuleNode)
{
  Q_D(qSlicerCLIProgressBar);
  if (commandLineModuleNode == d->CommandLineModuleNode)
    {
    return;
    }

  // Connect node modified event to updateUi that synchronize the values of the
  // nodes with the Ui
  this->qvtkReconnect(d->CommandLineModuleNode, commandLineModuleNode,
    vtkCommand::ModifiedEvent,
    this, SLOT(updateUiFromCommandLineModuleNode(vtkObject*)));

  d->CommandLineModuleNode = commandLineModuleNode;
  this->updateUiFromCommandLineModuleNode(d->CommandLineModuleNode);
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBar::setVisibleAfterExecution(bool visible)
{
  Q_D(qSlicerCLIProgressBar);
  if (visible == d->VisibleAfterExecution)
    {
    return;
    }

  d->VisibleAfterExecution = visible;
  this->updateUiFromCommandLineModuleNode(d->CommandLineModuleNode);
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBar::updateUiFromCommandLineModuleNode(
  vtkObject* commandLineModuleNode)
{
  Q_D(qSlicerCLIProgressBar);
  vtkMRMLCommandLineModuleNode * node =
    vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);
  if (!node)
    {
    d->StatusLabel->setText("No CLI");
    d->ProgressBar->setVisible(false);
    d->StageProgressBar->setVisible(false);
    d->ProgressBar->setMaximum(0);
    d->StageProgressBar->setMaximum(0);
    return;
    }

  // Update progress
  d->StatusLabel->setText(node->GetStatusString());

  // Update visibility
  bool showProgressBar = node->IsBusy();
  if (d->VisibleAfterExecution)
    {
    showProgressBar |=
      node->GetStatus() == vtkMRMLCommandLineModuleNode::Completed;
    showProgressBar |=
      node->GetStatus() == vtkMRMLCommandLineModuleNode::CompletedWithErrors;
    }
  d->ProgressBar->setVisible(showProgressBar);
  d->StageProgressBar->setVisible(node->IsBusy());

  // Update Progress
  ModuleProcessInformation* info = node->GetModuleDescription().GetProcessInformation();
  switch (node->GetStatus())
    {
    case vtkMRMLCommandLineModuleNode::Cancelled:
      d->ProgressBar->setMaximum(0);
      break;
    case vtkMRMLCommandLineModuleNode::Scheduled:
      d->ProgressBar->setMaximum(0);
      break;
    case vtkMRMLCommandLineModuleNode::Running:
      d->ProgressBar->setMaximum(info->Progress != 0.0 ? 100 : 0);
      d->ProgressBar->setValue(info->Progress * 100.);
      if (info->ElapsedTime != 0.)
        {
        d->StatusLabel->setText(QString("%1 (%2)").arg(node->GetStatusString()).arg(info->ElapsedTime));
        }
      d->StageProgressBar->setMaximum(info->StageProgress != 0.0 ? 100 : 0);
      d->StageProgressBar->setFormat(info->ProgressMessage);
      d->StageProgressBar->setValue(info->StageProgress * 100.);
      break;
    case vtkMRMLCommandLineModuleNode::Completed:
    case vtkMRMLCommandLineModuleNode::CompletedWithErrors:
      d->ProgressBar->setMaximum(100);
      d->ProgressBar->setValue(100);
      break;
    default:
    case vtkMRMLCommandLineModuleNode::Idle:
      break;
    }
}
