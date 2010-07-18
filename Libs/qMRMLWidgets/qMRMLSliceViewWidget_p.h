#ifndef __qMRMLSliceViewWidget_p_h
#define __qMRMLSliceViewWidget_p_h

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLSliceViewWidget.h"
#include "ui_qMRMLSliceViewWidget.h"

class QResizeEvent;

//-----------------------------------------------------------------------------
class qMRMLSliceViewWidgetPrivate: public QObject,
                                   public ctkPrivate<qMRMLSliceViewWidget>,
                                   public Ui_qMRMLSliceViewWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qMRMLSliceViewWidget);
  qMRMLSliceViewWidgetPrivate();
  ~qMRMLSliceViewWidgetPrivate();

public slots:

  /// Handle MRML scene event
  void onSceneAboutToBeClosedEvent();
  void onSceneClosedEvent();
  void onSceneAboutToBeImportedEvent();
  void onSceneImportedEvent();
  void onSceneRestoredEvent();

  /// Connected with the imageDataModified signal invoked by qMRMLSliceControllerWidget
  /// \sa qMRMLSliceControllerWidget::imageDataModified
  void onImageDataModified(vtkImageData * imageData);
};

#endif
