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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qMRMLNodeAttributeTableWidget_h
#define __qMRMLNodeAttributeTableWidget_h

// Qt includes
#include <QWidget>

// MRMLWidgets includes
#include "qMRMLWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkMRMLNode;
class qMRMLNodeAttributeTableWidgetPrivate;
class qMRMLNodeAttributeTableView;

class QMRML_WIDGETS_EXPORT qMRMLNodeAttributeTableWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructors
  explicit qMRMLNodeAttributeTableWidget(QWidget* parent = nullptr);
  ~qMRMLNodeAttributeTableWidget() override;

  /// Get node attribute table view
  qMRMLNodeAttributeTableView* tableView();

  /// Get the inspected MRML node
  vtkMRMLNode* mrmlNode()const;

public slots:
  /// Set the inspected MRML node
  void setMRMLNode(vtkMRMLNode* node);

  /// Refreshes the widget contents (useful for keeping displayed contents up-to-date without invoking node modified event)
  void updateWidgetFromMRML();

protected:
  void showEvent(QShowEvent *) override;

  QScopedPointer<qMRMLNodeAttributeTableWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLNodeAttributeTableWidget);
  Q_DISABLE_COPY(qMRMLNodeAttributeTableWidget);
};

#endif
