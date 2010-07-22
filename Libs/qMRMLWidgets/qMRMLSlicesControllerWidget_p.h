#ifndef __qMRMLSlicesControllerWidget_p_h
#define __qMRMLSlicesControllerWidget_p_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLSlicesControllerWidget.h"
#include "ui_qMRMLSlicesControllerWidget.h"

class QAction;

//-----------------------------------------------------------------------------
class qMRMLSlicesControllerWidgetPrivate: public QObject,
                                   public ctkPrivate<qMRMLSlicesControllerWidget>,
                                   public Ui_qMRMLSlicesControllerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qMRMLSlicesControllerWidget);
  qMRMLSlicesControllerWidgetPrivate();
  ~qMRMLSlicesControllerWidgetPrivate();

  void setupUi(qMRMLWidget* widget);

public:

  QAction* actionFitToWindow;
  
};

#endif
