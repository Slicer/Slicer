/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qMRMLItemModelUtils_h
#define __qMRMLItemModelUtils_h

// Qt includes
#include <QList>

#include "qMRMLWidgetsExport.h"

class QStandardItem;

/// \brief Utility functions for item models of MRML widgets.
namespace qMRMLItemModelUtils
{
/// Insert multiple rows (each row is a list of items, one for each column) under \a parent at \a row,
/// with a single row insertion notification (rowsAboutToBeInserted and rowsInserted signals).
///
/// The rows may already have children (items can be appended to the detached row items before calling
/// this method). Objects that are notified about the row insertion (proxy models, views, other observers)
/// can access all the inserted items, including their children.
///
/// This is much faster than inserting the rows one by one, because proxy models and views
/// process each row insertion notification separately (with a cost proportional to the number of
/// rows under the parent).
///
/// QStandardItem does not provide a method for this: QStandardItem::insertRows(int, QList<QStandardItem*>)
/// inserts single-column rows and it does not set the model in the children of the inserted items.
QMRML_WIDGETS_EXPORT void insertRows(QStandardItem* parent, int row, const QList<QList<QStandardItem*>>& rows);
} // namespace qMRMLItemModelUtils

#endif
