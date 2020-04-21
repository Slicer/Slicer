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
#include "qMRMLSequenceBrowserSeekWidget.h"
#include "ui_qMRMLSequenceBrowserSeekWidget.h"

// MRML includes
#include <vtkMRMLSequenceBrowserNode.h>
#include <vtkMRMLSequenceNode.h>

// Qt includes
#include <QDebug>
#include <QFontDatabase>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markups
class qMRMLSequenceBrowserSeekWidgetPrivate
  : public Ui_qMRMLSequenceBrowserSeekWidget
{
  Q_DECLARE_PUBLIC(qMRMLSequenceBrowserSeekWidget);
protected:
  qMRMLSequenceBrowserSeekWidget* const q_ptr;
public:
  qMRMLSequenceBrowserSeekWidgetPrivate(qMRMLSequenceBrowserSeekWidget& object);
  void init();

  vtkWeakPointer<vtkMRMLSequenceBrowserNode> SequenceBrowserNode;
};

//-----------------------------------------------------------------------------
// qMRMLSequenceBrowserSeekWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLSequenceBrowserSeekWidgetPrivate::qMRMLSequenceBrowserSeekWidgetPrivate(qMRMLSequenceBrowserSeekWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserSeekWidgetPrivate::init()
{
  Q_Q(qMRMLSequenceBrowserSeekWidget);
  this->setupUi(q);
  this->label_IndexValue->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  QObject::connect(this->slider_IndexValue, SIGNAL(valueChanged(int)), q, SLOT(setSelectedItemNumber(int)));
  q->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
// qMRMLSequenceBrowserSeekWidget methods

//-----------------------------------------------------------------------------
qMRMLSequenceBrowserSeekWidget::qMRMLSequenceBrowserSeekWidget(QWidget *newParent)
  : Superclass(newParent)
  , d_ptr(new qMRMLSequenceBrowserSeekWidgetPrivate(*this))
{
  Q_D(qMRMLSequenceBrowserSeekWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLSequenceBrowserSeekWidget::~qMRMLSequenceBrowserSeekWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserSeekWidget::setMRMLSequenceBrowserNode(vtkMRMLNode* browserNode)
{
  setMRMLSequenceBrowserNode(vtkMRMLSequenceBrowserNode::SafeDownCast(browserNode));
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserSeekWidget::setMRMLSequenceBrowserNode(vtkMRMLSequenceBrowserNode* browserNode)
{
  Q_D(qMRMLSequenceBrowserSeekWidget);

  qvtkReconnect(d->SequenceBrowserNode, browserNode, vtkMRMLSequenceBrowserNode::IndexDisplayFormatModifiedEvent,
    this, SLOT(onIndexDisplayFormatModified()));
  qvtkReconnect(d->SequenceBrowserNode, browserNode, vtkCommand::ModifiedEvent,
    this, SLOT(updateWidgetFromMRML()));

  d->SequenceBrowserNode = browserNode;
  this->onIndexDisplayFormatModified();
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserSeekWidget::setSelectedItemNumber(int itemNumber)
{
  Q_D(qMRMLSequenceBrowserSeekWidget);
  if (d->SequenceBrowserNode == nullptr)
    {
    qCritical("setSelectedItemNumber failed: browser node is invalid");
    this->updateWidgetFromMRML();
    return;
    }
  int selectedItemNumber = -1;
  vtkMRMLSequenceNode* sequenceNode = d->SequenceBrowserNode->GetMasterSequenceNode();
  if (sequenceNode != nullptr && itemNumber >= 0)
    {
    if (itemNumber < sequenceNode->GetNumberOfDataNodes())
      {
      selectedItemNumber = itemNumber;
      }
    }
  d->SequenceBrowserNode->SetSelectedItemNumber(selectedItemNumber);
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserSeekWidget::onIndexDisplayFormatModified()
{
  Q_D(qMRMLSequenceBrowserSeekWidget);
  // Reset the fixed width of the label
  QFontMetrics fontMetrics = QFontMetrics(d->label_IndexValue->font());
  d->label_IndexValue->setFixedWidth(fontMetrics.width(d->label_IndexValue->text()));
}

//-----------------------------------------------------------------------------
void qMRMLSequenceBrowserSeekWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLSequenceBrowserSeekWidget);
  vtkMRMLSequenceNode* sequenceNode = d->SequenceBrowserNode.GetPointer() ? d->SequenceBrowserNode->GetMasterSequenceNode() : nullptr;
  this->setEnabled(sequenceNode != nullptr);
  if (!sequenceNode)
    {
    d->label_IndexName->setText("");
    d->label_IndexUnit->setText("");
    d->label_IndexValue->setText("");
    return;
    }

  d->label_IndexName->setText(sequenceNode->GetIndexName().c_str());

  // Setting the min/max could trigger an index change (if current index is out of the new range),
  // therefore we have to block signals.
  bool sliderBlockSignals = d->slider_IndexValue->blockSignals(true);
  int numberOfDataNodes = sequenceNode->GetNumberOfDataNodes();
  if (numberOfDataNodes > 0 && !d->SequenceBrowserNode->GetRecordingActive())
    {
    d->slider_IndexValue->setEnabled(true);
    d->slider_IndexValue->setMinimum(0);
    d->slider_IndexValue->setMaximum(numberOfDataNodes - 1);
    }
  else
    {
    d->slider_IndexValue->setEnabled(false);
    }
  d->slider_IndexValue->blockSignals(sliderBlockSignals);

  int selectedItemNumber = d->SequenceBrowserNode->GetSelectedItemNumber();
  if (selectedItemNumber >= 0)
    {
    QString indexValue;
    QString indexUnit;

    if (d->SequenceBrowserNode->GetIndexDisplayMode() == vtkMRMLSequenceBrowserNode::IndexDisplayAsIndexValue)
      {
      // display as formatted index value (12.34sec)
      indexValue = QString::fromStdString(d->SequenceBrowserNode->GetFormattedIndexValue(d->SequenceBrowserNode->GetSelectedItemNumber()));
      indexUnit = QString::fromStdString(sequenceNode->GetIndexUnit());
      if (indexValue.length() == 0)
        {
        qWarning() << "Item " << selectedItemNumber << " has no index value defined";
        }
      }
    else
      {
      // display index as item index number (23/37)
      indexValue = QString::number(selectedItemNumber + 1) + "/" + QString::number(sequenceNode->GetNumberOfDataNodes());
      indexUnit = "";
      }

    QFontMetrics fontMetrics = QFontMetrics(d->label_IndexValue->font());

    d->label_IndexValue->setText(indexValue);
    d->label_IndexUnit->setText(indexUnit);

    d->label_IndexValue->setFixedWidth(std::max(fontMetrics.width(indexValue), d->label_IndexValue->width()));
    d->slider_IndexValue->setValue(selectedItemNumber);
    }
  else
    {
    d->label_IndexValue->setFixedWidth(0);
    d->label_IndexValue->setText("");
    d->label_IndexUnit->setText("");
    d->slider_IndexValue->setValue(0);
    }
}

//-----------------------------------------------------------------------------
QSlider* qMRMLSequenceBrowserSeekWidget::slider() const
{
  Q_D(const qMRMLSequenceBrowserSeekWidget);
  return d->slider_IndexValue;
}
