#ifndef __qMRMLSliceInformationWidget_p_h
#define __qMRMLSliceInformationWidget_p_h

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLSliceInformationWidget.h"
#include "ui_qMRMLSliceInformationWidget.h"

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>

/// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkImageData.h>

class QAction;
class ctkVTKSliceView;
class vtkMRMLSliceNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLSliceInformationWidgetPrivate: public QObject,
                                   public ctkPrivate<qMRMLSliceInformationWidget>,
                                   public Ui_qMRMLSliceInformationWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qMRMLSliceInformationWidget);
  qMRMLSliceInformationWidgetPrivate();
  ~qMRMLSliceInformationWidgetPrivate();

  void setupUi(qMRMLWidget* widget);

public slots:
  /// Update widget state using the associated MRML slice node
  void updateWidgetFromMRMLSliceNode();



public:
  vtkMRMLSliceNode*                   MRMLSliceNode;
  QButtonGroup*                       SliceSpacingModeGroup;
  
};

#endif
