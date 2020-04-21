/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// qMRML includes
#include "qMRMLSequenceBrowserPlayWidget.h"
#include "ui_qMRMLSequenceBrowserPlayWidget.h"

// MRML includes
#include <vtkMRMLSequenceBrowserNode.h>
#include <vtkMRMLSequenceNode.h>

// Qt includes
#include <QDebug>
#include <QShortcut>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markups
class qMRMLSequenceBrowserPlayWidgetPrivate
: public Ui_qMRMLSequenceBrowserPlayWidget
{
  Q_DECLARE_PUBLIC(qMRMLSequenceBrowserPlayWidget);
protected:
  qMRMLSequenceBrowserPlayWidget* const q_ptr;
public:
  qMRMLSequenceBrowserPlayWidgetPrivate(qMRMLSequenceBrowserPlayWidget& object);
  void init();

  vtkWeakPointer<vtkMRMLSequenceBrowserNode> SequenceBrowserNode;
};

//-----------------------------------------------------------------------------
// qMRMLSequenceBrowserPlayWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLSequenceBrowserPlayWidgetPrivate::qMRMLSequenceBrowserPlayWidgetPrivate(qMRMLSequenceBrowserPlayWidget& object)
: q_ptr(&object)
{
  this->SequenceBrowserNode = nullptr;
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidgetPrivate::init()
{
  Q_Q(qMRMLSequenceBrowserPlayWidget);
  this->setupUi(q);

  QObject::connect( this->pushButton_VcrFirst, SIGNAL(clicked()), q, SLOT(onVcrFirst()) );
  QObject::connect( this->pushButton_VcrPrevious, SIGNAL(clicked()), q, SLOT(onVcrPrevious()) );
  QObject::connect( this->pushButton_VcrNext, SIGNAL(clicked()), q, SLOT(onVcrNext()) );
  QObject::connect( this->pushButton_VcrLast, SIGNAL(clicked()), q, SLOT(onVcrLast()) );
  QObject::connect( this->pushButton_VcrPlayPause, SIGNAL(toggled(bool)), q, SLOT(setPlaybackEnabled(bool)) );
  QObject::connect(this->pushButton_VcrLoop, SIGNAL(toggled(bool)), q, SLOT(setPlaybackLoopEnabled(bool)));
  QObject::connect( this->doubleSpinBox_VcrPlaybackRate, SIGNAL(valueChanged(double)), q, SLOT(setPlaybackRateFps(double)) );
  QObject::connect(this->pushButton_VcrRecord, SIGNAL(toggled(bool)), q, SLOT(setRecordingEnabled(bool)));
  QObject::connect(this->pushButton_Snapshot, SIGNAL(clicked()), q, SLOT(onRecordSnapshot()));

  q->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
// qMRMLSequenceBrowserPlayWidget methods

//-----------------------------------------------------------------------------
qMRMLSequenceBrowserPlayWidget::qMRMLSequenceBrowserPlayWidget(QWidget *newParent)
: Superclass(newParent)
, d_ptr(new qMRMLSequenceBrowserPlayWidgetPrivate(*this))
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLSequenceBrowserPlayWidget::~qMRMLSequenceBrowserPlayWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::setMRMLSequenceBrowserNode(vtkMRMLNode* browserNode)
{
  setMRMLSequenceBrowserNode(vtkMRMLSequenceBrowserNode::SafeDownCast(browserNode));
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::setMRMLSequenceBrowserNode(vtkMRMLSequenceBrowserNode* browserNode)
{
  Q_D(qMRMLSequenceBrowserPlayWidget);

  qvtkReconnect(d->SequenceBrowserNode, browserNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));

  d->SequenceBrowserNode = browserNode;
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLSequenceBrowserPlayWidget);

  vtkMRMLSequenceNode* sequenceNode = d->SequenceBrowserNode.GetPointer() ? d->SequenceBrowserNode->GetMasterSequenceNode() : nullptr;
  this->setEnabled(sequenceNode != nullptr);
  if (!sequenceNode)
    {
    return;
    }

  QObjectList vcrPlaybackControls; // Note we don't include the recording, because we want to be able to record when no data is available
  vcrPlaybackControls
    << d->pushButton_VcrFirst << d->pushButton_VcrLast << d->pushButton_VcrLoop
    << d->pushButton_VcrNext << d->pushButton_VcrPlayPause << d->pushButton_VcrPrevious;
  bool vcrControlsEnabled=false;

  int numberOfDataNodes=sequenceNode->GetNumberOfDataNodes();
  if (numberOfDataNodes>0 && !d->SequenceBrowserNode->GetRecordingActive())
  {
    vcrControlsEnabled=true;

    bool pushButton_VcrPlayPauseBlockSignals = d->pushButton_VcrPlayPause->blockSignals(true);
    d->pushButton_VcrPlayPause->setChecked(d->SequenceBrowserNode->GetPlaybackActive());
    d->pushButton_VcrPlayPause->blockSignals(pushButton_VcrPlayPauseBlockSignals);

    bool pushButton_VcrLoopBlockSignals = d->pushButton_VcrLoop->blockSignals(true);
    d->pushButton_VcrLoop->setChecked(d->SequenceBrowserNode->GetPlaybackLooped());
    d->pushButton_VcrLoop->blockSignals(pushButton_VcrLoopBlockSignals);
  }

  bool signalsBlocked = d->doubleSpinBox_VcrPlaybackRate->blockSignals(true);
  d->doubleSpinBox_VcrPlaybackRate->setValue(d->SequenceBrowserNode->GetPlaybackRateFps());
  d->doubleSpinBox_VcrPlaybackRate->blockSignals(signalsBlocked);

  bool pushButton_VcrRecordingBlockSignals = d->pushButton_VcrRecord->blockSignals(true);
  d->pushButton_VcrRecord->setChecked(d->SequenceBrowserNode->GetRecordingActive());
  d->pushButton_VcrRecord->blockSignals(pushButton_VcrRecordingBlockSignals);

  bool recordingAllowed = d->SequenceBrowserNode->IsAnySequenceNodeRecording();
  bool playbackActive = d->SequenceBrowserNode->GetPlaybackActive();
  bool recordingActive = d->SequenceBrowserNode->GetRecordingActive();
  d->pushButton_VcrRecord->setVisible(recordingAllowed);
  d->pushButton_VcrRecord->setEnabled(!playbackActive);
  d->pushButton_Snapshot->setVisible(recordingAllowed);
  d->pushButton_Snapshot->setEnabled(!playbackActive && !recordingActive);

  foreach( QObject*w, vcrPlaybackControls ) { w->setProperty( "enabled", vcrControlsEnabled ); }
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::onVcrFirst()
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrFirst failed: no active browser node is selected";
    updateWidgetFromMRML();
    return;
    }
  d->SequenceBrowserNode->SelectFirstItem();
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::onVcrLast()
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrLast failed: no active browser node is selected";
    updateWidgetFromMRML();
    return;
    }
  d->SequenceBrowserNode->SelectLastItem();
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::onVcrPrevious()
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrPrevious failed: no active browser node is selected";
    updateWidgetFromMRML();
    return;
    }
  d->SequenceBrowserNode->SelectNextItem(-1);
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::onVcrNext()
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrNext failed: no active browser node is selected";
    updateWidgetFromMRML();
    return;
    }
  d->SequenceBrowserNode->SelectNextItem(1);
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::onVcrPlayPause()
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode == nullptr)
    {
    qDebug() << "onVcrPlayPause failed: no active browser node is selected";
    updateWidgetFromMRML();
    return;
    }
  d->SequenceBrowserNode->SetRecordingActive(false);
  d->SequenceBrowserNode->SetPlaybackActive(!d->SequenceBrowserNode->GetPlaybackActive());
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::onRecordSnapshot()
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode == nullptr)
    {
    qDebug() << "onRecordSnapshot failed: no active browser node is selected";
    updateWidgetFromMRML();
    return;
    }
  d->SequenceBrowserNode->SaveProxyNodesState();
}


//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::setPlaybackEnabled(bool play)
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrPlayPauseStateChanged failed: no active browser node is selected";
    updateWidgetFromMRML();
    return;
    }
  d->SequenceBrowserNode->SetRecordingActive(false);
  if (play!=d->SequenceBrowserNode->GetPlaybackActive())
    {
    d->SequenceBrowserNode->SetPlaybackActive(play);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::setRecordingEnabled(bool record)
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrRecordStateChanged failed: no active browser node is selected";
    updateWidgetFromMRML();
    return;
    }
  d->SequenceBrowserNode->SetPlaybackActive(false);
  if (record!=d->SequenceBrowserNode->GetRecordingActive())
    {
    d->SequenceBrowserNode->SetRecordingActive(record);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::setPlaybackLoopEnabled(bool loopEnabled)
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "onVcrPlaybackLoopStateChanged failed: no active browser node is selected";
    this->updateWidgetFromMRML();
    return;
    }
  if (loopEnabled!=d->SequenceBrowserNode->GetPlaybackLooped())
    {
    d->SequenceBrowserNode->SetPlaybackLooped(loopEnabled);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::setPlaybackRateFps(double playbackRateFps)
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  if (d->SequenceBrowserNode==nullptr)
    {
    qDebug() << "setPlaybackRateFps failed: no active browser node is selected";
    this->updateWidgetFromMRML();
    return;
    }
  if (playbackRateFps!=d->SequenceBrowserNode->GetPlaybackRateFps())
    {
    d->SequenceBrowserNode->SetPlaybackRateFps(playbackRateFps);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::setPlayPauseShortcut(QString keySequence)
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  QObject::connect(new QShortcut(QKeySequence(keySequence), this), SIGNAL(activated()), SLOT(onVcrPlayPause()));
  d->pushButton_VcrPlayPause->setToolTip(d->pushButton_VcrPlayPause->toolTip()+" ("+keySequence+")");
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::setPreviousFrameShortcut(QString keySequence)
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  QObject::connect(new QShortcut(QKeySequence(keySequence), this), SIGNAL(activated()), SLOT(onVcrPrevious()));
  d->pushButton_VcrPrevious->setToolTip(d->pushButton_VcrPrevious->toolTip() + " (" + keySequence + ")");
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserPlayWidget::setNextFrameShortcut(QString keySequence)
{
  Q_D(qMRMLSequenceBrowserPlayWidget);
  QObject::connect(new QShortcut(QKeySequence(keySequence), this), SIGNAL(activated()), SLOT(onVcrNext()));
  d->pushButton_VcrNext->setToolTip(d->pushButton_VcrNext->toolTip() + " (" + keySequence + ")");
}
