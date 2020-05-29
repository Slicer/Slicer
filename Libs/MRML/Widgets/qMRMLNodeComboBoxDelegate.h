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

#ifndef __qMRMLNodeComboBoxDelegate_h
#define __qMRMLNodeComboBoxDelegate_h

// Qt includes
#include <QItemDelegate>

// MRMLWidgets includes
#include "qMRMLWidgetsExport.h"

/// Item delegate to show unselectable items highlighted when the mouse is
/// over them to give a feeling of selectable item.
/// This is used in qMRMLNodeComboBox for the extra items:
/// e.g. "Add/Remove/Edit node" actions.
/// The highlight color is slightly more transparent than the style palette
/// highlight color to indicates that the item is not really selectable
/// but correspond to an action.

/// We reimplemente the code form Qt "QComboBoxDelegate" used in the class QComboBox.

class QComboBox;

class QMRML_WIDGETS_EXPORT qMRMLNodeComboBoxDelegate : public QItemDelegate
{
  Q_OBJECT
public :
  typedef QItemDelegate Superclass;
  qMRMLNodeComboBoxDelegate(QObject* parent, QComboBox* comboBox);

  static bool isSeparator(const QModelIndex &index);
  static void setSeparator(QAbstractItemModel *model, const QModelIndex &index);

protected:
  /// Reimplemented to force the highlight in case the item is not selectable
  /// but current. The highlight color used is then slightly different from
  /// the default color
  void paint(QPainter *painter,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) const override;

  QSize sizeHint(const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;

  void drawDisplay(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QRect &rect,
                           const QString &text) const override;

  void drawFocus(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QRect &rect) const override;

private:
  QComboBox* mCombo;
  Q_DISABLE_COPY(qMRMLNodeComboBoxDelegate);
};

#endif // __qMRMLNodeComboBoxDelegate_h
