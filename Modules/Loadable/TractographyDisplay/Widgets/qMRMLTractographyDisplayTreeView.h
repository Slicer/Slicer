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

#ifndef __qMRMLTractographyDisplayTreeView_h
#define __qMRMLTractographyDisplayTreeView_h

// Qt includes
#include <QTreeView>
// TractographyDisplay QT includes
#include "qSlicerTractographyDisplayModuleWidget.h"

// CTK includes
#include <ctkPimpl.h>

// qMRML includes
#include "qMRMLTreeView.h"

#include "qSlicerTractographyDisplayModuleWidgetsExport.h"

// Logic includes

class qMRMLSortFilterProxyModel;
class qMRMLTractographyDisplayTreeViewPrivate;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkSlicerFiberBundleLogic;

/// \ingroup Slicer_QtModules_TractographyDisplay
class Q_SLICER_MODULE_TRACTOGRAPHYDISPLAY_WIDGETS_EXPORT qMRMLTractographyDisplayTreeView : public qMRMLTreeView
{
  Q_OBJECT

public:
  typedef qMRMLTreeView Superclass;

  qMRMLTractographyDisplayTreeView(QWidget *parent=0);
  virtual ~qMRMLTractographyDisplayTreeView();

  virtual void setMRMLScene(vtkMRMLScene* scene);

  // Register the logic
  void setLogic(vtkSlicerFiberBundleLogic* logic);

  virtual bool clickDecoration(const QModelIndex& index);


signals:
  void onPropertyEditButtonClicked(QString id);
  // type 0-line 1-tube 2-glyph
  void visibilityChanged(int type);
public slots:


protected:
  QScopedPointer<qMRMLTractographyDisplayTreeViewPrivate> d_ptr;
  #ifndef QT_NO_CURSOR
    bool viewportEvent(QEvent* e);
  #endif

protected slots:
  void dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight );

private:
  Q_DECLARE_PRIVATE(qMRMLTractographyDisplayTreeView);
  Q_DISABLE_COPY(qMRMLTractographyDisplayTreeView);

  vtkSlicerFiberBundleLogic* m_Logic;
  
  // toggle the visibility of an TractographyDisplay
  void onVisibilityColumnClicked(vtkMRMLNode* node);

};

#endif
