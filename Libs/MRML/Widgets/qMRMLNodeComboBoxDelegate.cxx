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

  This file was originally developed by Benjamin Long, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt include
#include <QAbstractItemView>
#include <QComboBox>
#include <QPainter>
#include <QStandardItemModel>

//include
#include "qMRMLNodeComboBoxDelegate.h"

// --------------------------------------------------------------------------
qMRMLNodeComboBoxDelegate::qMRMLNodeComboBoxDelegate(QObject* parent,
                                                     QComboBox* comboBox)
  : QItemDelegate(parent)
  , mCombo(comboBox)
{

}

// --------------------------------------------------------------------------
bool qMRMLNodeComboBoxDelegate::isSeparator(const QModelIndex &index)
{
  return index.data(Qt::AccessibleDescriptionRole).toString() ==
                    QLatin1String("separator");
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxDelegate::setSeparator(QAbstractItemModel *model,
                                             const QModelIndex &index)
{
  model->setData(index, QString::fromLatin1("separator"),
                 Qt::AccessibleDescriptionRole);
  if (QStandardItemModel *m = qobject_cast<QStandardItemModel*>(model))
      if (QStandardItem *item = m->itemFromIndex(index))
          item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxDelegate::paint(QPainter *painter,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    if (this->isSeparator(index))
      {
      QRect rect = option.rect;
      if (const QAbstractItemView *view =
          qobject_cast<const QAbstractItemView*>(option.widget))
        {
        rect.setWidth(view->viewport()->width());
        }
      QStyleOption opt;
      opt.rect = rect;
      this->mCombo->style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator,
                                           &opt, painter, this->mCombo);
      }
    else
      {
      this->Superclass::paint(painter, option, index);
      }
}

// --------------------------------------------------------------------------
QSize qMRMLNodeComboBoxDelegate::sizeHint(const QStyleOptionViewItem &option,
                       const QModelIndex &index) const
{
  if (this->isSeparator(index))
    {
    int pm = this->mCombo->style()->pixelMetric(QStyle::PM_DefaultFrameWidth,
                                                nullptr, this->mCombo);
    return QSize(pm, pm);
    }
  return this->Superclass::sizeHint(option, index);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxDelegate::drawDisplay(QPainter *painter,
                                            const QStyleOptionViewItem &option,
                                            const QRect &rect,
                                            const QString &text) const
{
   QStyleOptionViewItem option2 = option;
   if (option.state & QStyle::State_HasFocus &&
       !(option.state & QStyle::State_Selected))
     {
     option2.state |= QStyle::State_Selected;
     QColor unselectedHighlight = option2.palette.color(QPalette::Highlight);
     unselectedHighlight.setHsv(unselectedHighlight.hue(),
                                qMax(0, unselectedHighlight.saturation() - 50),
                                qMin(255,unselectedHighlight.value() + 15));
     QColor unselectedHighlightedText =
         option2.palette.color(QPalette::HighlightedText).darker(30);
     option2.palette.setColor(QPalette::Highlight, unselectedHighlight);
     option2.palette.setColor(QPalette::HighlightedText, unselectedHighlightedText);
     }
   this->Superclass::drawDisplay(painter, option2, rect, text);
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxDelegate::drawFocus(QPainter *painter,
                                          const QStyleOptionViewItem &option,
                                          const QRect &rect) const
{
  Q_UNUSED(painter);
  Q_UNUSED(option);
  Q_UNUSED(rect);
  return;
}
