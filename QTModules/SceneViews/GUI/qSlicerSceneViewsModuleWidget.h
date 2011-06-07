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

/// \ingroup Slicer_QtModules_SceneViews
class Q_SLICER_QTMODULES_SCENEVIEWS_EXPORT qSlicerSceneViewsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
    typedef qSlicerAbstractModuleWidget Superclass;
    qSlicerSceneViewsModuleWidget(QWidget *parent=0);
    ~qSlicerSceneViewsModuleWidget();

public slots:
    /// a public slot allowing other modules to open up the scene view capture
    /// dialog (get the module manager, get the module sceneviews, get the
    /// widget representation, then invoke this method, see qSlicerIOManager openSceneViewsDialog
    void showSceneViewDialog();

    /// User clicked on restore button
    void restoreSceneView(const QString& mrmlId);

    /// User clicked on property edit button
    void editSceneView(const QString& mrmlId);

    /// Refresh the tree from MRML
    void refreshTree();
                                       
protected slots:

  void moveDownSelected();
  void moveUpSelected();

protected:
  QScopedPointer<qSlicerSceneViewsModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerSceneViewsModuleWidget);
  Q_DISABLE_COPY(qSlicerSceneViewsModuleWidget);

};

#endif
