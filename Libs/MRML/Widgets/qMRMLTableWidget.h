/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef __qMRMLTableWidget_h
#define __qMRMLTableWidget_h

// Qt includes
#include <QWidget>
class QResizeEvent;

// qMRMLWidget includes
#include "qMRMLWidget.h"
class qMRMLTableViewControllerWidget;
class qMRMLTableView;
class qMRMLTableWidgetPrivate;

// MRML includes
class vtkMRMLTableViewNode;
class vtkMRMLScene;

/// \brief qMRMLTableWidget is the toplevel table widget that can be
/// packed in a layout.
///
/// qMRMLTableWidget provides tabling capabilities with a display
/// canvas for the table and a controller widget to control the
/// content and properties of the table.
class QMRML_WIDGETS_EXPORT qMRMLTableWidget : public qMRMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLTableWidget(QWidget* parent = nullptr);
  ~qMRMLTableWidget() override;

  /// Get the tabl node observed by view.
  vtkMRMLTableViewNode* mrmlTableViewNode()const;

  /// Get a reference to the underlying Table View
  /// Becareful if you change the TableView, you might
  /// unsynchronize the view from the nodes/logics.
  Q_INVOKABLE qMRMLTableView* tableView()const;

  /// Get table view controller widget
  Q_INVOKABLE qMRMLTableViewControllerWidget* tableController()const;

  /// Get the view label for the table.
  /// \sa qMRMLTableControllerWidget::tableViewLabel()
  /// \sa setTableViewLabel()
  QString viewLabel()const;

  /// Set the view label for the table.
  /// \sa qMRMLTableControllerWidget::tableViewLabel()
  /// \sa tableViewLabel()
  void setViewLabel(const QString& newTableViewLabel);

public slots:
  /// Set the current \a viewNode to observe
  void setMRMLTableViewNode(vtkMRMLTableViewNode* newTableViewNode);

protected:
  QScopedPointer<qMRMLTableWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLTableWidget);
  Q_DISABLE_COPY(qMRMLTableWidget);
};

#endif
