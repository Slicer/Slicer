#ifndef __qSlicerSliceViewWidget_p_h
#define __qSlicerSliceViewWidget_p_h

// Qt includes

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qSlicerSliceViewWidget.h"

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qSlicerSliceViewWidgetPrivate: public QObject,
                                     public ctkPrivate<qSlicerSliceViewWidget>
{
  Q_OBJECT
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qSlicerSliceViewWidget);
  qSlicerSliceViewWidgetPrivate();
  ~qSlicerSliceViewWidgetPrivate();

public slots:

  /// Triggered after the SliceLogic is modified
  void onSliceLogicModifiedEvent();

  /// Triggered after the ImageData associated with the SliceLogic is modified
  void onImageDataModifiedEvent();

public:
  vtkSmartPointer<vtkMRMLSliceLogic>   SliceLogic;
  vtkWeakPointer<vtkImageData>         ImageData;

};

#endif
