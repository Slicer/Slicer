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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLColorTableView_h
#define __qMRMLColorTableView_h

// Qt includes
#include <QTableView>
#include <QItemDelegate>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class QSortFilterProxyModel;
class qMRMLColorTableViewPrivate;
class qMRMLColorModel;
class vtkMRMLColorNode;
class vtkMRMLNode;

/// \brief Table view for color table nodes.
/// Edition of color (opening dialog) and opacity (slider popup) is handled by
/// qMRMLItemDelegate.
class QMRML_WIDGETS_EXPORT qMRMLColorTableView : public QTableView
{
  Q_OBJECT
  /// This property show/hides the colors whose name are (none).
  /// false by default.
  /// \sa showOnlyNamedColors(), setShowOnlyNamedColors()
  Q_PROPERTY(bool showOnlyNamedColors READ showOnlyNamedColors WRITE setShowOnlyNamedColors)
public:
  qMRMLColorTableView(QWidget *parent=nullptr);
  ~qMRMLColorTableView() override;

  Q_INVOKABLE vtkMRMLColorNode* mrmlColorNode()const;
  Q_INVOKABLE qMRMLColorModel* colorModel()const;
  Q_INVOKABLE QSortFilterProxyModel* sortFilterProxyModel()const;

  /// Return the row of the color with name \a colorName.
  /// \sa rowFromColorIndex()
  Q_INVOKABLE int rowFromColorName(const QString& colorName)const;
  /// Return the row of the color of index \a colorIndex.
  /// \sa rowFromColorIndex()
  Q_INVOKABLE int rowFromColorIndex(int colorIndex)const;

  bool showOnlyNamedColors()const;

public slots:
  void setMRMLColorNode(vtkMRMLColorNode* colorNode);
  /// Utility function to simply connect signals/slots with Qt Designer
  void setMRMLColorNode(vtkMRMLNode* colorNode);

  void setShowOnlyNamedColors(bool);

protected:
  QScopedPointer<qMRMLColorTableViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLColorTableView);
  Q_DISABLE_COPY(qMRMLColorTableView);
};

#endif
