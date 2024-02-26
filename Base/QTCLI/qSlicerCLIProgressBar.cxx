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
#include <QElapsedTimer>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QScrollBar>
#include <QTextBrowser>

// CTK includes
#include <ctkExpandButton.h>

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

  bool isVisible(qSlicerCLIProgressBar::Visibility visibility) const;

  QString getLastNLines(const std::string& str, int numberOfLines, unsigned int maxLength = 5000);

private:
  QGridLayout* GridLayout;
  QLabel* NameLabel;
  QLabel* StatusLabelLabel;
  QLabel* StatusLabel;
  ctkExpandButton* DetailsTextExpandButton;
  QTextBrowser* DetailsTextBrowser;
  QElapsedTimer DetailsLastUpdateTime;
  QProgressBar* ProgressBar;
  QProgressBar* StageProgressBar;

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
  : q_ptr(&object)
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
  this->GridLayout->setContentsMargins(0, 0, 0, 0);

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
  this->StatusLabelLabel->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

  this->GridLayout->addWidget(StatusLabelLabel, 1, 0, 1, 1);

  this->StatusLabel = new QLabel();
  this->StatusLabel->setObjectName(QString::fromUtf8("StatusLabel"));

  this->GridLayout->addWidget(StatusLabel, 1, 1, 1, 1);

  this->DetailsTextExpandButton = new ctkExpandButton();
  this->DetailsTextExpandButton->setObjectName(QString::fromUtf8("DetailsTextExpandButton"));
  this->DetailsTextExpandButton->setToolTip(qSlicerCLIProgressBar::tr("Show details"));
  this->DetailsTextExpandButton->setOrientation(Qt::Vertical);

  this->GridLayout->addWidget(DetailsTextExpandButton, 1, 2, 1, 1);

  this->DetailsTextBrowser = new QTextBrowser();
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
  this->StageProgressBar->setMaximum(100);
  this->StageProgressBar->setValue(0);
  this->GridLayout->addWidget(StageProgressBar, 4, 0, 1, 3);

  this->NameLabel->setText("");
  this->StatusLabelLabel->setText(qSlicerCLIProgressBar::tr("Status:"));
  this->StatusLabel->setText(qSlicerCLIProgressBar::tr("Idle"));

  QObject::connect(this->DetailsTextExpandButton, SIGNAL(toggled(bool)), q, SLOT(showDetails(bool)));

  q->updateUiFromCommandLineModuleNode(this->CommandLineModuleNode);
}

//-----------------------------------------------------------------------------
bool qSlicerCLIProgressBarPrivate ::isVisible(qSlicerCLIProgressBar::Visibility visibility) const
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
    return this->CommandLineModuleNode
             ? (this->CommandLineModuleNode->IsBusy()
                || this->CommandLineModuleNode->GetStatus() == vtkMRMLCommandLineModuleNode::Completed
                || this->CommandLineModuleNode->GetStatus() == vtkMRMLCommandLineModuleNode::CompletedWithErrors)
             : false;
  }
  return true;
}

//-----------------------------------------------------------------------------
QString qSlicerCLIProgressBarPrivate::getLastNLines(const std::string& str, int numberOfLines, unsigned int maxLength)
{
  if (numberOfLines < 1 || str.size() < 1)
  {
    return QString();
  }
  const char lineSeparator = '\n';
  size_t linesStartPosition = str.size() - 1;
  for (int line = 0; line <= numberOfLines; ++line)
  {
    linesStartPosition = str.find_last_of(lineSeparator, linesStartPosition - 1);
    if (linesStartPosition == std::string::npos || linesStartPosition == 0)
    {
      // we need the full string
      if (str.size() <= maxLength)
      {
        return QString::fromStdString(str);
      }
      else
      {
        return QStringLiteral("...") + QString::fromStdString(str.substr(str.size() - maxLength, maxLength)).trimmed();
      }
    }
  }
  if (str.size() - linesStartPosition > maxLength)
  {
    linesStartPosition = str.size() - maxLength;
  }
  return QStringLiteral("...")
         + QString::fromStdString(str.substr(linesStartPosition + 1, str.size() - linesStartPosition)).trimmed();
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
vtkMRMLCommandLineModuleNode* qSlicerCLIProgressBar::commandLineModuleNode() const
{
  Q_D(const qSlicerCLIProgressBar);
  return d->CommandLineModuleNode;
}

//-----------------------------------------------------------------------------
qSlicerCLIProgressBar::Visibility qSlicerCLIProgressBar::nameVisibility() const
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
qSlicerCLIProgressBar::Visibility qSlicerCLIProgressBar::statusVisibility() const
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
qSlicerCLIProgressBar::Visibility qSlicerCLIProgressBar::progressVisibility() const
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
void qSlicerCLIProgressBar::setCommandLineModuleNode(vtkMRMLCommandLineModuleNode* commandLineModuleNode)
{
  Q_D(qSlicerCLIProgressBar);
  if (commandLineModuleNode == d->CommandLineModuleNode)
  {
    return;
  }

  // Connect node modified event to updateUi that synchronize the values of the
  // nodes with the Ui
  this->qvtkReconnect(d->CommandLineModuleNode,
                      commandLineModuleNode,
                      vtkCommand::ModifiedEvent,
                      this,
                      SLOT(updateUiFromCommandLineModuleNode(vtkObject*)));

  if (d->CommandLineModuleNode)
  {
    // UpdateOutputTextDuringExecution was enabled because the details button was open
    if (d->DetailsTextExpandButton->isChecked())
    {
      d->CommandLineModuleNode->EndContinuousOutputUpdate();
    }
  }

  d->CommandLineModuleNode = commandLineModuleNode;

  if (d->CommandLineModuleNode)
  {
    if (d->DetailsTextExpandButton->isChecked())
    {
      d->CommandLineModuleNode->StartContinuousOutputUpdate();
    }
  }

  this->updateUiFromCommandLineModuleNode(d->CommandLineModuleNode);
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBar::updateUiFromCommandLineModuleNode(vtkObject* commandLineModuleNode)
{
  Q_D(qSlicerCLIProgressBar);
  Q_ASSERT(commandLineModuleNode == d->CommandLineModuleNode);
  vtkMRMLCommandLineModuleNode* node = vtkMRMLCommandLineModuleNode::SafeDownCast(commandLineModuleNode);

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
    d->DetailsLastUpdateTime.invalidate();
    return;
  }

  // Update progress
  d->StatusLabel->setText(QString::fromStdString(node->GetDisplayableStatusString()));
  d->NameLabel->setText(node->GetName());

  // Update Progress
  ModuleProcessInformation* info = node->GetModuleDescription().GetProcessInformation();
  QString statusLabelFormat = tr("%1 (%2s)");
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
        d->StatusLabel->setText(statusLabelFormat.arg(QString::fromStdString(node->GetDisplayableStatusString()))
                                  .arg(info->ElapsedTime, 0, 'f', 1));
      }
      // We keep StageProgressBar maximum at 100, because if it was set to 0
      // then the progress message would not be displayed.
      d->StageProgressBar->setMaximum(100);
      d->StageProgressBar->setFormat(info->ProgressMessage);
      d->StageProgressBar->setValue(info->StageProgress * 100.);
      break;
    case vtkMRMLCommandLineModuleNode::Completed:
    case vtkMRMLCommandLineModuleNode::CompletedWithErrors:
      if (info->ElapsedTime != 0.)
      {
        d->StatusLabel->setText(statusLabelFormat.arg(QString::fromStdString(node->GetDisplayableStatusString()))
                                  .arg(info->ElapsedTime, 0, 'f', 1));
      }
      d->ProgressBar->setMaximum(100);
      d->ProgressBar->setValue(100);
      break;
    default:
    case vtkMRMLCommandLineModuleNode::Idle:
      break;
  }

  // If user chose to show details then all
  std::string errorText = node->GetErrorText();
  bool showDetails =
    (d->DetailsTextExpandButton->isChecked()
     || (node->GetStatus() == vtkMRMLCommandLineModuleNode::CompletedWithErrors && !errorText.empty()));
  d->DetailsTextBrowser->setVisible(showDetails);

  // While the module is running, avoid too frequent updates of the process output
  // as long output can cause slowdowns.
  const double minRefreshTimeMsec = 3000.0;
  bool updateDetails =
    showDetails
    && ((node->GetStatus() != vtkMRMLCommandLineModuleNode::Running)
        || (!d->DetailsLastUpdateTime.isValid() || d->DetailsLastUpdateTime.elapsed() > minRefreshTimeMsec));

  if (showDetails && updateDetails)
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
    d->DetailsTextBrowser->setMinimumHeight(lineSpacing * maxNumberOfLinesShown);
    d->DetailsTextBrowser->setMaximumHeight(lineSpacing * maxNumberOfLinesShown);

    QString detailsText;
    detailsText = "<pre>";
    if (!outputText.empty())
    {
      if (node->GetStatus() == vtkMRMLCommandLineModuleNode::Running)
      {
        // Limit output text while running, to reduce time spent with updating the GUI and reduce need for scrolling.
        detailsText += d->getLastNLines(node->GetOutputText(), 30).toHtmlEscaped();
      }
      else
      {
        detailsText += QString::fromStdString(node->GetOutputText()).toHtmlEscaped();
      }
    }
    if (!errorText.empty() && !outputText.empty())
    {
      detailsText += "<hr/>";
    }
    if (!errorText.empty())
    {
      detailsText += "<span style = \"color:#FF0000;\">";
      if (node->GetStatus() == vtkMRMLCommandLineModuleNode::Running)
      {
        // Limit output text while running, to reduce time spent with updating the GUI and reduce need for scrolling.
        detailsText += d->getLastNLines(errorText, 10).toHtmlEscaped();
      }
      else
      {
        detailsText += QString::fromStdString(errorText).toHtmlEscaped();
      }
      detailsText += "</span>";
    }

    detailsText += "</pre>";
    d->DetailsTextBrowser->setText(detailsText);
    QScrollBar* vScrollBar = d->DetailsTextBrowser->verticalScrollBar();
    if (vScrollBar)
    {
      vScrollBar->setValue(vScrollBar->maximum());
    }
    d->DetailsLastUpdateTime.restart();
  }
}

//-----------------------------------------------------------------------------
void qSlicerCLIProgressBar::showDetails(bool show)
{
  Q_D(qSlicerCLIProgressBar);
  if (d->CommandLineModuleNode)
  {
    if (show)
    {
      d->CommandLineModuleNode->StartContinuousOutputUpdate();
    }
    else
    {
      d->CommandLineModuleNode->EndContinuousOutputUpdate();
    }
  }
  this->updateUiFromCommandLineModuleNode(d->CommandLineModuleNode);
}
