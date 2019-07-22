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

#ifndef __qMRMLNodeAttributeTableView_h
#define __qMRMLNodeAttributeTableView_h

// Qt includes
#include <QWidget>

// MRMLWidgets includes
#include "qMRMLWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkMRMLNode;
class qMRMLNodeAttributeTableViewPrivate;
class QTableWidgetItem;
class QItemSelectionModel;

class QMRML_WIDGETS_EXPORT qMRMLNodeAttributeTableView : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructor
  explicit qMRMLNodeAttributeTableView(QWidget* parent = nullptr);
  /// Destructor
  ~qMRMLNodeAttributeTableView() override;

  /// Set the inspected MRML node
  void setInspectedNode(vtkMRMLNode* node);
  /// Get the inspected MRML node
  vtkMRMLNode* inspectedNode();

  /// Return number of attributes (rows) in the table
  int attributeCount() const;

  /// Return lost of names of all attributes in the table
  QStringList attributes() const;

  /// Get value of an attribute with a given name
  QString attributeValue(const QString& attributeName) const;

  /// Make an item range selected (for testing purposes)
  void selectItemRange(const int topRow, const int leftColumn, const int bottomRow, const int rightColumn);

  /// Get item selection model (for testing purposes)
  QItemSelectionModel* selectionModel();

public slots:
  /// Add empty attribute row. The name of the added attribute is unique,
  /// NewAttributeName for the first added name, NewAttributeName1 if
  /// attribute NewAttributeName already exists, NewAttributeName2 if attribute
  /// NewAttributeName1 already exists, etc.
  void addAttribute();

  /// Remove selected attributes
  void removeSelectedAttributes();

  /// Set value of an attribute with a given name
  void setAttribute(const QString& attributeName, const QString& attributeValue);

  /// Rename an attribute with a given name
  void renameAttribute(const QString& oldName, const QString& newName);

protected:
  /// Get table widget item of an attribute name cell containing the given attribute name
  QTableWidgetItem* findAttributeNameItem(const QString& attributeName) const;

  /// Generate the next new attribute name that is added
  QString generateNewAttributeName() const;

protected slots:
  /// Populate attribute table according to the inspected node
  void populateAttributeTable();

  /// Handles changing of text in a cell (attribute name or value)
  void onAttributeChanged(QTableWidgetItem* changedItem);

protected:
  QScopedPointer<qMRMLNodeAttributeTableViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLNodeAttributeTableView);
  Q_DISABLE_COPY(qMRMLNodeAttributeTableView);
};

#endif
