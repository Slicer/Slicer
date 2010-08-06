#ifndef __qSlicerLayoutManager_h
#define __qSlicerLayoutManager_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

#include "qSlicerBaseQTGUIExport.h"

class qMRMLThreeDView;
class qMRMLSliceWidget;
class qSlicerLayoutManagerPrivate;
class vtkCollection;
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

  /// Get SliceViewWidget identified by \a name
  qMRMLSliceWidget* sliceView(const QString& name);

  /// Return the number of instantiated ThreeDRenderView
  int threeDViewCount();
  
  /// Get ThreeDRenderView identified by \a id
  /// where \a id is an integer ranging from 0 to N-1 with N being the number
  /// of instantiated qMRMLThreeDView (that should also be equal to the number
  /// of vtkMRMLViewNode)
  qMRMLThreeDView* threeDView(int id);

  vtkCollection* mrmlSliceLogics()const;

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
