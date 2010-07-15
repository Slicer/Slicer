#ifndef __qSlicerLayoutManager_h
#define __qSlicerLayoutManager_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

#include "qSlicerBaseQTGUIExport.h"

class qMRMLThreeDRenderView;
class qSlicerLayoutManagerPrivate;
class vtkMRMLScene;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerLayoutManager : public QObject
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QObject Superclass;
  
  /// Constructors
  explicit qSlicerLayoutManager(QWidget* widget);
  virtual ~qSlicerLayoutManager(){}

  /// Return the number of instantiated ThreeDRenderView
  int threeDRenderViewCount();
  
  /// Get ThreeDRenderView identified by \a id
  /// where \a id is an integer ranging from 0 to N-1 with N being the number
  /// of instantiated qMRMLThreeDRenderView (that should also be equal to the number
  /// of vtkMRMLViewNode)
  qMRMLThreeDRenderView* threeDRenderView(int id);

public slots:

  /// 
  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* scene);

  /// Switch to the different layout
  /// TODO A better way would be to register layout classes with the manager
  void switchToConventionalView();
  void switchToOneUp3DView();
  void switchToOneUpRedSliceView();
  void switchToOneUpYellowSliceView();
  void switchToOneUpGreenSliceView();
  void switchToOneUpSliceView(const QString& sliceViewName);
  void switchToFourUpView();
  void switchToTabbed3DView();
  void switchToTabbedSliceView();
  void switchToLightboxView();
  void switchToCompareView();
  void switchToSideBySideCompareView();
  void switchToDual3DView();
  void switchToNone();
  
private:
  CTK_DECLARE_PRIVATE(qSlicerLayoutManager);
};

#endif
