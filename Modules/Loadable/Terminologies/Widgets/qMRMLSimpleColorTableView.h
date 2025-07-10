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

  This file was originally developed by Csaba Pinter, EBATINCA, S.L.
  and was funded by by Murat Maga (Seattle Children's Research Institute).

==============================================================================*/

#ifndef __qMRMLSimpleColorTableView_h
#define __qMRMLSimpleColorTableView_h

// Qt includes
#include <QTableView>

// Colors includes
#include "qSlicerTerminologiesModuleWidgetsExport.h"

class qMRMLSortFilterColorProxyModel;
class qMRMLSimpleColorTableViewPrivate;
class qMRMLColorModel;
class vtkMRMLColorNode;
class vtkMRMLNode;

/// \brief Table view for color table nodes.
/// Edition of color (opening dialog) and opacity (slider popup) is handled by
/// qMRMLItemDelegate.
class Q_SLICER_MODULE_TERMINOLOGIES_WIDGETS_EXPORT qMRMLSimpleColorTableView : public QTableView
{
  Q_OBJECT
public:
  qMRMLSimpleColorTableView(QWidget* parent = nullptr);
  ~qMRMLSimpleColorTableView() override;

  Q_INVOKABLE vtkMRMLColorNode* mrmlColorNode()const;
  Q_INVOKABLE qMRMLColorModel* colorModel()const;
  Q_INVOKABLE qMRMLSortFilterColorProxyModel* sortFilterProxyModel()const;

  Q_INVOKABLE int selectedColorIndex()const;

public slots:
  void setMRMLColorNode(vtkMRMLColorNode* colorNode);
  /// Utility function to simply connect signals/slots with Qt Designer
  void setMRMLColorNode(vtkMRMLNode* colorNode);
  /// Select row in table by color index and scroll to it.
  /// Returns true on success.
  bool selectColorByIndex(int colorIndex);

protected:
  QScopedPointer<qMRMLSimpleColorTableViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSimpleColorTableView);
  Q_DISABLE_COPY(qMRMLSimpleColorTableView);
};

#endif
