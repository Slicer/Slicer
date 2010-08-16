#ifndef __qMRMLThreeDView_h
#define __qMRMLThreeDView_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKRenderView.h>

#include "qMRMLWidgetsExport.h"

class qMRMLThreeDViewPrivate;
class vtkMRMLScene;
class vtkMRMLViewNode;

class QMRML_WIDGETS_EXPORT qMRMLThreeDView : public ctkVTKRenderView
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkVTKRenderView Superclass;
  
  /// Constructors
  explicit qMRMLThreeDView(QWidget* parent = 0);
  virtual ~qMRMLThreeDView(){}

  /// Register DisplayableManagers
  /// \a scriptedDisplayableManagerDirectory is the based directory from which
  /// scripted DisplayableManager should be sourced from.
  void registerDisplayableManagers(const QString& scriptedDisplayableManagerDirectory);

public slots:

  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* newScene);

  /// Set/Get \a viewNode
  void setMRMLViewNode(vtkMRMLViewNode* newViewNode);
  vtkMRMLViewNode* mrmlViewNode()const;
  
private:
  CTK_DECLARE_PRIVATE(qMRMLThreeDView);
};

#endif
