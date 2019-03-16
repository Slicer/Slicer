#ifndef __qMRMLExpandingWebView_p_h
#define __qMRMLExpandingWebView_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// VTK includes
#include <vtkWeakPointer.h>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLExpandingWebView.h"

class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLExpandingWebViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLExpandingWebView);
protected:
  qMRMLExpandingWebView* const q_ptr;
public:
  qMRMLExpandingWebViewPrivate(qMRMLExpandingWebView& object);
  ~qMRMLExpandingWebViewPrivate() override;

  virtual void init();

  void setMRMLScene(vtkMRMLScene* scene);
  vtkMRMLScene *mrmlScene();

public slots:
  /// Handle MRML scene events
  void startProcessing();
  void endProcessing();

protected:

  vtkMRMLScene*                      MRMLScene;
};

#endif
