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

#ifndef __qMRMLColorListView_h
#define __qMRMLColorListView_h

// Qt includes
#include <QListView>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class QSortFilterProxyModel;
class qMRMLColorListViewPrivate;
class qMRMLColorModel;
class vtkMRMLColorNode;
class vtkMRMLNode;

class QMRML_WIDGETS_EXPORT qMRMLColorListView : public QListView
{
  Q_OBJECT
  Q_PROPERTY(bool showOnlyNamedColors READ showOnlyNamedColors WRITE setShowOnlyNamedColors)
public:
  qMRMLColorListView(QWidget *parent=nullptr);
  ~qMRMLColorListView() override;

  vtkMRMLColorNode* mrmlColorNode()const;
  qMRMLColorModel* colorModel()const;
  QSortFilterProxyModel* sortFilterProxyModel()const;

  bool showOnlyNamedColors()const;

public slots:
  void setMRMLColorNode(vtkMRMLColorNode* colorNode);
  /// Utility function to simply connect signals/slots with Qt Designer
  void setMRMLColorNode(vtkMRMLNode* colorNode);

  void setShowOnlyNamedColors(bool);

signals:
  /// Colors are selected when there is a new current item
  /// (not fired on the activated signal).
  void colorSelected(int index);
  void colorSelected(const QColor& color);
  void colorSelected(const QString& name);

protected slots:
  void currentChanged(const QModelIndex&, const QModelIndex&) override;

protected:
  QScopedPointer<qMRMLColorListViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLColorListView);
  Q_DISABLE_COPY(qMRMLColorListView);
};

#endif
