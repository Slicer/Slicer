/*=========================================================================

  Program: 3D Slicer

  Copyright (c) Seattle Children's Hospital d/b/a Seattle Children's Research Institute.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, EBATINCA, S.L.
  and was funded by by Murat Maga (Seattle Children's Research Institute).

=========================================================================*/

// Colors includes
#include "qSlicerTerminologyEditorButton.h"
#include "qSlicerTerminologyEditorDialog.h"

// Terminology includes
#include "qSlicerTerminologyNavigatorWidget.h"
#include "vtkSlicerTerminologyEntry.h"

// Qt includes
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStylePainter>

class qSlicerTerminologyEditorButtonPrivate
{
  Q_DECLARE_PUBLIC(qSlicerTerminologyEditorButton);

protected:
  qSlicerTerminologyEditorButton* const q_ptr;

public:
  qSlicerTerminologyEditorButtonPrivate(qSlicerTerminologyEditorButton& object);
  void init();
  QString text() const;

  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle TerminologyInfo;
  mutable QSize CachedSizeHint;
};

//-----------------------------------------------------------------------------
qSlicerTerminologyEditorButtonPrivate::qSlicerTerminologyEditorButtonPrivate(qSlicerTerminologyEditorButton& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorButtonPrivate::init()
{
  Q_Q(qSlicerTerminologyEditorButton);
  q->setCheckable(true);
  QObject::connect(q, SIGNAL(toggled(bool)), q, SLOT(onToggled(bool)));
}

//-----------------------------------------------------------------------------
qSlicerTerminologyEditorButton::qSlicerTerminologyEditorButton(QWidget* _parent)
  : QPushButton(_parent)
  , d_ptr(new qSlicerTerminologyEditorButtonPrivate(*this))
{
  Q_D(qSlicerTerminologyEditorButton);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerTerminologyEditorButton::~qSlicerTerminologyEditorButton() = default;

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorButton::changeTerminology()
{
  Q_D(qSlicerTerminologyEditorButton);
  if (qSlicerTerminologyEditorDialog::getTerminology(d->TerminologyInfo, this))
  {
    this->update();
    emit terminologyChanged();
  }
  else
  {
    emit canceled();
  }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorButton::onToggled(bool change)
{
  if (change)
  {
    this->changeTerminology();
    this->setChecked(false);
  }
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorButton::terminologyInfo(
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle& terminologyInfo)
{
  Q_D(qSlicerTerminologyEditorButton);
  terminologyInfo = d->TerminologyInfo;
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorButton::setTerminologyInfo(
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle& terminologyInfo)
{
  Q_D(qSlicerTerminologyEditorButton);

  d->TerminologyInfo = terminologyInfo;

  this->update();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorButton::paintEvent(QPaintEvent*)
{
  Q_D(qSlicerTerminologyEditorButton);
  QStylePainter p(this);
  QStyleOptionButton option;
  this->initStyleOption(&option);
  option.text = d->TerminologyInfo.Name;
  p.drawControl(QStyle::CE_PushButton, option);
}
