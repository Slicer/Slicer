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

// CTK includes
#include <ctkExpandButton.h>
#include <ctkFittedTextBrowser.h>

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

  bool isVisible(qSlicerCLIProgressBar::Visibility visibility)const;

private:

  QGridLayout *  GridLayout;
  QLabel *       NameLabel;
  QLabel *       StatusLabelLabel;
  QLabel *       StatusLabel;
  ctkExpandButton * DetailsTextExpandButton;
  ctkFittedTextBrowser * DetailsTextBrowser;
  QProgressBar * ProgressBar;
  QProgressBar * StageProgressBar;

  vtkMRMLCommandLineModuleNode* CommandLineModuleNode;
  qSlicerCLIProgressBar::Visibility NameVisibility;
  qSlicerCLIProgressBar::Visibility StatusVisibility;
  qSlicerCLIProgressBar::Visibility ProgressVisibility;
  qSlicerCLIProgressBar::Visibility StageProgressVisibility;
};

//-----------------------------------------------------------------------------
// qSlicerCLIProgressBarPrivate methods

//-----------------------------------------------------------------------------
qSlicerCLIProgressBarPrivate::qSlicerCLIProgressBarPrivate(qSlicerCLIProgressBar& object)
  :q_ptr(&object)
{
  this->CommandLineModuleNode = nullptr;
  this->NameVisibility = qSlicerCLIProgressBar::AlwaysHidden;
  this->StatusVisibility = qSlicerCLIProgressBar::AlwaysVisible;
  this->ProgressVisibility = qSlicerCLIProgressBar::VisibleAfterCompletion;
  this->StageProgressVisibility = qSlicerCLIProgressBar::HiddenWhenIdle;
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBarPrivate::init()
{
  Q_Q(qSlicerCLIProgressBar);
  // Create widget .. layout
  this->GridLayout = new QGridLayout(this->q_ptr);
  this->GridLayout->setObjectName(QString::fromUtf8("gridLayout"));
  this->GridLayout->setContentsMargins(0,0,0,0);

  this->NameLabel = new QLabel();
  this->NameLabel->setObjectName(QString::fromUtf8("NameLabel"));

  this->GridLayout->addWidget(NameLabel, 1, 0, 1, 1);

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

  this->DetailsTextExpandButton = new ctkExpandButton();
  this->DetailsTextExpandButton->setObjectName(QString::fromUtf8("DetailsTextExpandButton"));
  this->DetailsTextExpandButton->setToolTip(qSlicerCLIProgressBar::tr("Show details"));
  this->DetailsTextExpandButton->setOrientation(Qt::Vertical);

  this->GridLayout->addWidget(DetailsTextExpandButton, 1, 2, 1, 1);

  this->DetailsTextBrowser = new ctkFittedTextBrowser();
  this->DetailsTextBrowser->setObjectName(QString::fromUtf8("DetailsTextBrowser"));
  this->DetailsTextBrowser->setVisible(false);

  this->GridLayout->addWidget(DetailsTextBrowser, 2, 0, 1, 3);

  this->ProgressBar = new QProgressBar();
  this->ProgressBar->setObjectName(QString::fromUtf8("ProgressBar"));
  this->ProgressBar->setMaximum(100);
  this->ProgressBar->setValue(0);

  this->GridLayout->addWidget(ProgressBar, 3, 0, 1, 3);

  this->StageProgressBar = new QProgressBar();
  this->StageProgressBar->setObjectName(QString::fromUtf8("StageProgressBar"));
  this->StageProgressBar->setValue(0);
  this->GridLayout->addWidget(StageProgressBar, 4, 0, 1, 3);

  this->NameLabel->setText("");
  this->StatusLabelLabel->setText(qSlicerCLIProgressBar::tr("Status:"));
  this->StatusLabel->setText(qSlicerCLIProgressBar::tr("Idle"));

  QObject::connect(this->DetailsTextExpandButton, SIGNAL(toggled(bool)),
    q, SLOT(showDetails(bool)));

  q->updateUiFromCommandLineModuleNode(this->CommandLineModuleNode);
}

//-----------------------------------------------------------------------------
bool qSlicerCLIProgressBarPrivate
::isVisible(qSlicerCLIProgressBar::Visibility visibility)const
{
  if (visibility == qSlicerCLIProgressBar::AlwaysHidden)
    {
    return false;
    }
  if (visibility == qSlicerCLIProgressBar::AlwaysVisible)
    {
    return true;
    }
  if (visibility == qSlicerCLIProgressBar::HiddenWhenIdle)
    {
    return this->CommandLineModuleNode ? this->CommandLineModuleNode->IsBusy() : false;
    }
  if (visibility == qSlicerCLIProgressBar::VisibleAfterCompletion)
    {
    return this->CommandLineModuleNode ?
      (this->CommandLineModuleNode->IsBusy() ||
       this->CommandLineModuleNode->GetStatus() == vtkMRMLCommandLineModuleNode::Completed ||
       this->CommandLineModuleNode->GetStatus() == vtkMRMLCommandLineModuleNode::CompletedWithErrors) : false;
    }
  return true;
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
qSlicerCLIProgressBar::~qSlicerCLIProgressBar() = default;

//-----------------------------------------------------------------------------
vtkMRMLCommandLineModuleNode * qSlicerCLIProgressBar::commandLineModuleNode()const
{
  Q_D(const qSlicerCLIProgressBar);
  return d->CommandLineModuleNode;
}

//-----------------------------------------------------------------------------
qSlicerCLIProgressBar::Visibility qSlicerCLIProgressBar::nameVisibility()const
{
  Q_D(const qSlicerCLIProgressBar);
  return d->NameVisibility;
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBar::setNameVisibility(qSlicerCLIProgressBar::Visibility visibility)
{
  Q_D(qSlicerCLIProgressBar);
  if (visibility == d->NameVisibility)
    {
    return;
    }

  d->NameVisibility = visibility;
  this->updateUiFromCommandLineModuleNode(d->CommandLineModuleNode);
}

//-----------------------------------------------------------------------------
qSlicerCLIProgressBar::Visibility qSlicerCLIProgressBar::statusVisibility()const
{
  Q_D(const qSlicerCLIProgressBar);
  return d->StatusVisibility;
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBar::setStatusVisibility(qSlicerCLIProgressBar::Visibility visibility)
{
  Q_D(qSlicerCLIProgressBar);
  if (visibility == d->StatusVisibility)
    {
    return;
    }

  d->StatusVisibility = visibility;
  this->updateUiFromCommandLineModuleNode(d->CommandLineModuleNode);
}

//-----------------------------------------------------------------------------
qSlicerCLIProgressBar::Visibility qSlicerCLIProgressBar::progressVisibility()const
{
  Q_D(const qSlicerCLIProgressBar);
  return d->ProgressVisibility;
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBar::setProgressVisibility(qSlicerCLIProgressBar::Visibility visibility)
{
  Q_D(qSlicerCLIProgressBar);
  if (visibility == d->ProgressVisibility)
    {
    return;
    }

  d->ProgressVisibility = visibility;
  this->updateUiFromCommandLineModuleNode(d->CommandLineModuleNode);
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
void qSlicerCLIProgressBar::updateUiFromCommandLineModuleNode(
  vtkObject* commandLineModuleNode)
{
  Q_D(qSlicerCLIProgressBar);
  Q_ASSERT(commandLineModuleNode == d->CommandLineModuleNode);
  vtkMRMLCommandLineModuleNode * node =
    vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);

  d->NameLabel->setVisible(d->isVisible(d->NameVisibility));
  d->StatusLabelLabel->setVisible(d->isVisible(d->StatusVisibility));
  d->StatusLabel->setVisible(d->isVisible(d->StatusVisibility));
  d->ProgressBar->setVisible(d->isVisible(d->ProgressVisibility));
  d->StageProgressBar->setVisible(d->isVisible(d->StageProgressVisibility));

  if (!node)
    {
    d->StatusLabel->setText("");
    d->ProgressBar->setMaximum(0);
    d->StageProgressBar->setMaximum(0);
    d->DetailsTextBrowser->setVisible(d->DetailsTextExpandButton->isChecked());
    return;
    }

  // Update progress
  d->StatusLabel->setText(node->GetStatusString());
  d->NameLabel->setText(node->GetName());

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

  // If user chose to show details then all
  std::string errorText = node->GetErrorText();
  bool showDetails = (d->DetailsTextExpandButton->isChecked()
    || (node->GetStatus() == vtkMRMLCommandLineModuleNode::CompletedWithErrors
    && !errorText.empty()));
  d->DetailsTextBrowser->setVisible(showDetails);
  if (showDetails)
    {
    std::string outputText;
    int maxNumberOfLinesShown = 5;
    if (d->DetailsTextExpandButton->isChecked())
      {
      outputText = node->GetOutputText();
      maxNumberOfLinesShown = 15;
      }
    // Limit number of text lines shown (more shown if user clicked to show more details)
    int lineSpacing = QFontMetrics(d->DetailsTextBrowser->document()->defaultFont()).lineSpacing();
    d->DetailsTextBrowser->setMaximumHeight(lineSpacing * maxNumberOfLinesShown);

    QString detailsText;
    detailsText = "<pre>";
    if (!errorText.empty())
      {
      detailsText += "<span style = \"color:#FF0000;\">";
      detailsText += errorText.c_str();
      detailsText += "</span>";
      }
    if (!errorText.empty() && !outputText.empty())
      {
      detailsText += "\n";
      }
    if (!outputText.empty())
      {
      detailsText += node->GetOutputText().c_str();
      }
    detailsText += "</pre>";
    d->DetailsTextBrowser->setText(detailsText);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBar::showDetails(bool)
{
  Q_D(qSlicerCLIProgressBar);
  this->updateUiFromCommandLineModuleNode(d->CommandLineModuleNode);
}
