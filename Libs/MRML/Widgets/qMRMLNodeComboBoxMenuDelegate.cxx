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
#include <QApplication>
#include <QComboBox>
#include <QPainter>
#include <QStyledItemDelegate>

//include
#include "qMRMLNodeComboBoxMenuDelegate.h"
#include "qMRMLNodeComboBoxDelegate.h"

// --------------------------------------------------------------------------
qMRMLNodeComboBoxMenuDelegate::qMRMLNodeComboBoxMenuDelegate(QObject* parent,
                                                             QComboBox* comboBox)
  : QAbstractItemDelegate(parent)
  , mCombo(comboBox)
{
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBoxMenuDelegate::paint(QPainter *painter,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    QStyleOptionMenuItem opt = this->getStyleOption(option, index);
#ifndef Q_WS_S60
    painter->fillRect(option.rect, opt.palette.background());
#endif
    this->mCombo->style()->drawControl(QStyle::CE_MenuItem, &opt, painter,
                                       this->mCombo);
}

// --------------------------------------------------------------------------
QSize qMRMLNodeComboBoxMenuDelegate::sizeHint(const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    QStyleOptionMenuItem opt = getStyleOption(option, index);
    return this->mCombo->style()->sizeFromContents(
        QStyle::CT_MenuItem, &opt, option.rect.size(), this->mCombo);
}

QStyleOptionMenuItem qMRMLNodeComboBoxMenuDelegate::getStyleOption(
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionMenuItem menuOption;

    QPalette resolvedpalette =
        option.palette.resolve(QApplication::palette("QMenu"));
    QVariant value = index.data(Qt::ForegroundRole);
    if (value.canConvert(QMetaType::QBrush))
      {
      resolvedpalette.setBrush(QPalette::WindowText, qvariant_cast<QBrush>(value));
      resolvedpalette.setBrush(QPalette::ButtonText, qvariant_cast<QBrush>(value));
      resolvedpalette.setBrush(QPalette::Text, qvariant_cast<QBrush>(value));
      }
    menuOption.palette = resolvedpalette;
    menuOption.state = QStyle::State_None;
    if (this->mCombo->window()->isActiveWindow())
      {
      menuOption.state = QStyle::State_Active;
      }
    if ((option.state & QStyle::State_Enabled) &&
        (index.model()->flags(index) & Qt::ItemIsEnabled))
      {
      menuOption.state |= QStyle::State_Enabled;
      }
    else
      {
      menuOption.palette.setCurrentColorGroup(QPalette::Disabled);
      }
    if (option.state & QStyle::State_Selected)
      {
      menuOption.state |= QStyle::State_Selected;
      }

    /// Hash to show unselectable items highlighted when the mouse is
    /// over them to give a feeling of selectable item.
    /// The highlight color is slightly more transparent than the style palette
    /// highlight color to indicates that the item is not really selectable
    /// but correspond to an action.
    if (option.state & QStyle::State_HasFocus &&
        !(option.state & QStyle::State_Selected))
      {
      menuOption.state |= QStyle::State_Selected;
      QColor unselectedHighlight = menuOption.palette.color(QPalette::Highlight);
      unselectedHighlight.setHsv(unselectedHighlight.hue(),
                                 qMax(0, unselectedHighlight.saturation() - 50),
                                 qMin(255,unselectedHighlight.value() + 15));
      QColor unselectedHighlightedText =
          menuOption.palette.color(QPalette::HighlightedText);
      menuOption.palette.setColor(QPalette::Highlight, unselectedHighlight);
      menuOption.palette.setColor(QPalette::HighlightedText,
                                  unselectedHighlightedText);
      }
    /// End of the highlight

    menuOption.checkType = QStyleOptionMenuItem::NonExclusive;
    menuOption.checked = this->mCombo->currentIndex() == index.row();
    if (qMRMLNodeComboBoxDelegate::isSeparator(index))
      {
      menuOption.menuItemType = QStyleOptionMenuItem::Separator;
      }
    else
      {
      menuOption.menuItemType = QStyleOptionMenuItem::Normal;
      }
    QVariant variant = index.model()->data(index, Qt::DecorationRole);
    switch (variant.type())
      {
      case QVariant::Icon:
          menuOption.icon = qvariant_cast<QIcon>(variant);
          break;
      case QVariant::Color:
          {
          static QPixmap pixmap(option.decorationSize);
          pixmap.fill(qvariant_cast<QColor>(variant));
          menuOption.icon = pixmap;
          break;
          }
      default:
          menuOption.icon = qvariant_cast<QPixmap>(variant);
          break;
      }
    if (index.data(Qt::BackgroundRole).canConvert(QMetaType::QBrush))
      {
      menuOption.palette.setBrush(QPalette::All, QPalette::Background,
                             qvariant_cast<QBrush>(index.data(Qt::BackgroundRole)));
      }
    menuOption.text = index.model()->data(index, Qt::DisplayRole).toString()
                           .replace(QLatin1Char('&'), QLatin1String("&&"));
    menuOption.tabWidth = 0;
    menuOption.maxIconWidth =  option.decorationSize.width() + 4;
    menuOption.menuRect = option.rect;
    menuOption.rect = option.rect;

    // Make sure fonts set on the combo box also overrides the font for the popup menu.
    if (this->mCombo->testAttribute(Qt::WA_SetFont) ||
        this->mCombo->testAttribute(Qt::WA_MacSmallSize) ||
        this->mCombo->testAttribute(Qt::WA_MacMiniSize) ||
        this->mCombo->font() != QFont())
      {
      menuOption.font = this->mCombo->font();
      }
    else
      {
      menuOption.font = this->mCombo->font();
      }

    menuOption.fontMetrics = QFontMetrics(menuOption.font);

    return menuOption;
}
