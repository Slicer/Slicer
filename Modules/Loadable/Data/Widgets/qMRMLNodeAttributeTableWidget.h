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

// DataWidgets includes
#include "qSlicerDataModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkMRMLNode;
class qMRMLNodeAttributeTableWidgetPrivate;
class qMRMLNodeAttributeTableView;

class Q_SLICER_MODULE_DATA_WIDGETS_EXPORT qMRMLNodeAttributeTableWidget : public QWidget
{
  Q_OBJECT

public:
  /// Constructors
  explicit qMRMLNodeAttributeTableWidget(QWidget* parent = 0);
  virtual ~qMRMLNodeAttributeTableWidget();

  /// Get node attribute table view
  qMRMLNodeAttributeTableView* tableView();

public slots:
  /// Set the inspected MRML node
  void setMRMLNode(vtkMRMLNode* node);

protected:
  QScopedPointer<qMRMLNodeAttributeTableWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLNodeAttributeTableWidget);
  Q_DISABLE_COPY(qMRMLNodeAttributeTableWidget);
};

#endif
