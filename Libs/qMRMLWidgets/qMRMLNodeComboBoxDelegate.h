/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

//QT include
#include <QItemDelegate>

// qMRMLWidgets includes
#include "qMRMLWidgetsExport.h"

/// Item delegate to show unselectable items highlighted when the mouse is
/// over them to give a feeling of selectable item.
/// This is used in qMRMLNodeComboBox for the extra items:
/// e.g. "Add/Remove/Edit node" actions.
/// The highlight color is slightly more transparent than the style palette
/// highlight color to indicates that the item is not really selectable
/// but correspond to an action. 
class QMRML_WIDGETS_EXPORT qMRMLNodeComboBoxDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  explicit qMRMLNodeComboBoxDelegate(QObject* parent = 0);

  /// Reimplemented to force the highlight in case the item is not selectable
  /// but current. The highlight color used is then slightly different from
  /// the default color
  virtual void drawDisplay(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QRect &rect, const QString &text) const;

  virtual void drawFocus(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QRect &rect) const;
private:
  Q_DISABLE_COPY(qMRMLNodeComboBoxDelegate);
};

#endif // __qMRMLNodeComboBoxDelegate_h
