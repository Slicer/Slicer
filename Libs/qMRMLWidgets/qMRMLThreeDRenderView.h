#ifndef __qMRMLThreeDRenderView_h
#define __qMRMLThreeDRenderView_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKRenderView.h>

#include "qMRMLWidgetsExport.h"

class qMRMLThreeDRenderViewPrivate;
class vtkMRMLScene;
class vtkMRMLViewNode;

class QMRML_WIDGETS_EXPORT qMRMLThreeDRenderView : public ctkVTKRenderView
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkVTKRenderView Superclass;
  
  /// Constructors
  explicit qMRMLThreeDRenderView(QWidget* parent = 0);
  virtual ~qMRMLThreeDRenderView(){}  
  
  /// Set axis label color
  void setAxisLabelColor( double r, double g, double b);

public slots:

  /// 
  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* scene);

  ///
  /// Set/Get \a viewNode
  void setMRMLViewNode(vtkMRMLViewNode* viewNode);
  vtkMRMLViewNode* mrmlViewNode()const;
  
private:
  CTK_DECLARE_PRIVATE(qMRMLThreeDRenderView);
};

#endif
