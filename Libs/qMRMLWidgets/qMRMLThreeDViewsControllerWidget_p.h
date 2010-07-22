#ifndef __qMRMLThreeDViewsControllerWidget_p_h
#define __qMRMLThreeDViewsControllerWidget_p_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLThreeDViewsControllerWidget.h"
#include "ui_qMRMLThreeDViewsControllerWidget.h"

class QAction;

//-----------------------------------------------------------------------------
class qMRMLThreeDViewsControllerWidgetPrivate: public QObject,
                                   public ctkPrivate<qMRMLThreeDViewsControllerWidget>,
                                   public Ui_qMRMLThreeDViewsControllerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qMRMLThreeDViewsControllerWidget);
  qMRMLThreeDViewsControllerWidgetPrivate();
  ~qMRMLThreeDViewsControllerWidgetPrivate();

  void setupUi(qMRMLWidget* widget);

public:

  QAction* actionFitToWindow;
  
};

#endif
