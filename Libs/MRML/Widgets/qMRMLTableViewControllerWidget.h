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

#ifndef __qMRMLTableViewControllerWidget_h
#define __qMRMLTableViewControllerWidget_h

// CTK includes
#include <ctkVTKObject.h>

// qMRMLWidget includes
#include "qMRMLViewControllerBar.h"
class qMRMLTableViewControllerWidgetPrivate;
class qMRMLTableView;

// MRML includes
class vtkMRMLTableViewNode;

///
/// qMRMLTableViewControllerWidget offers controls to a table view
/// (vtkMRMLTableViewNode and vtkMRMLTableNode). This controller
/// allows for the content (data) and style (properties) of a table to
/// be defined.
class QMRML_WIDGETS_EXPORT qMRMLTableViewControllerWidget
  : public qMRMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Superclass typedef
  typedef qMRMLViewControllerBar Superclass;

  /// Constructors
  explicit qMRMLTableViewControllerWidget(QWidget* parent = nullptr);
  ~qMRMLTableViewControllerWidget() override;

  /// Set the label for the table view (abbreviation for the view
  /// name)
  void setViewLabel(const QString& newViewLabel);

  /// Get the label for the view (abbreviation for the view name)
  QString viewLabel()const;

public slots:
  /// Set the scene
  void setMRMLScene(vtkMRMLScene* newScene) override;

  /// Set the TableView with which this controller interacts
  void setTableView(qMRMLTableView* TableView);

  /// Set the TableViewNode associated with this TableViewController.
  /// TableViewNodes are 1-to-1 with TableViews
  void setMRMLTableViewNode(vtkMRMLTableViewNode* tableViewNode);

protected slots:
  void updateWidgetFromMRML();

private:
  Q_DECLARE_PRIVATE(qMRMLTableViewControllerWidget);
  Q_DISABLE_COPY(qMRMLTableViewControllerWidget);
};

#endif
