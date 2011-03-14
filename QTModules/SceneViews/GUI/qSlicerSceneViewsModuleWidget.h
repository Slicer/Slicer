#ifndef __qSlicerSceneViewsModuleWidget_h
#define __qSlicerSceneViewsModuleWidget_h

// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerSceneViewsModuleExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>


class qSlicerSceneViewsModuleDialog;
class vtkMRMLSceneViewNode;
class qSlicerSceneViewsModuleWidgetPrivate;

class vtkMRMLNode;


class Q_SLICER_QTMODULES_SCENEVIEWS_EXPORT qSlicerSceneViewsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
    typedef qSlicerAbstractModuleWidget Superclass;
    qSlicerSceneViewsModuleWidget(QWidget *parent=0);
    ~qSlicerSceneViewsModuleWidget();

    void refreshTree();

    /// User clicked on property edit button
    void propertyEditButtonClicked(QString mrmlId);

    /// User clicked on restore button
    void restoreButtonClicked(QString mrmlId);
public slots:
    /// a public slot allowing other modules to open up the scene view capture
    /// dialog (get the module manager, get the module sceneviews, get the
    /// widget representation, then invoke this method, see qSlicerIOManager openSceneViewsDialog
    void showSceneViewDialog();

protected:

protected slots:

  void moveDownSelected();
  void moveUpSelected();

  /// Delete selected Annotations
  void deleteSelectedButtonClicked();

  // Scene view dialog
  void sceneViewRejected();
  void sceneViewAccepted();
  void onSceneViewButtonClicked();

protected:
  QScopedPointer<qSlicerSceneViewsModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSceneViewsModuleWidget);
  Q_DISABLE_COPY(qSlicerSceneViewsModuleWidget);

  virtual void setup();

  qSlicerSceneViewsModuleDialog* m_SceneViewDialog;

};

#endif
