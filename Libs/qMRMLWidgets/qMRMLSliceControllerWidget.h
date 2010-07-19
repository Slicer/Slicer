#ifndef __qMRMLSliceControllerWidget_h
#define __qMRMLSliceControllerWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// qMRMLWidget includes
#include "qMRMLWidget.h"

#include "qMRMLWidgetsExport.h"

class QButtonGroup;
class qMRMLSliceControllerWidgetPrivate;
class vtkMRMLScene;
class vtkMRMLNode;
class vtkMRMLSliceNode;
class vtkMRMLSliceCompositeNode;
class vtkImageData;
class ctkVTKSliceView;

class vtkMRMLSliceLogic;

class QMRML_WIDGETS_EXPORT qMRMLSliceControllerWidget : public qMRMLWidget
{
  Q_OBJECT
  Q_PROPERTY(QString sliceViewName READ sliceViewName WRITE setSliceViewName)
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;
  
  /// Constructors
  explicit qMRMLSliceControllerWidget(QWidget* parent = 0);
  virtual ~qMRMLSliceControllerWidget(){}

  /// Get slice orientation
  /// \sa setSliceOrientation(QString);
  QString sliceOrientation();

  /// Get imageData
  vtkImageData* imageData()const;

  /// Get \a sliceNode
  /// \sa setMRMLSliceCompositeNode();
  vtkMRMLSliceNode* mrmlSliceNode()const;

  /// Get sliceCompositeNode
  /// \sa vtkMRMLSliceLogic::GetSliceCompositeNode();
  vtkMRMLSliceCompositeNode* mrmlSliceCompositeNode()const;

  /// Set slice view name
  /// \note SliceViewName should be set before setMRMLSliceNode() is called
  void setSliceViewName(const QString& newSliceViewName);

  /// Get slice view name
  QString sliceViewName()const;

  /// Set slice offset range
  void setSliceOffsetRange(double min, double max);

  /// Set slice offset \a resolution (increment)
  void setSliceOffsetResolution(double resolution);

  /// Get SliceLogic
  vtkMRMLSliceLogic* sliceLogic()const;

  /// Set \a newSliceLogic
  /// Use if two instances of the controller need to observe the same logic.
  /// \note setSliceLogic() should be called before setMRMLSliceNode() otherwise it's a no-op.
  void setSliceLogic(vtkMRMLSliceLogic * newSliceLogic);

  /// Set controller widget group
  /// All controllers of a same group will be set visible or hidden if at least
  /// one of the sliceCollapsibleButton of the group is clicked.
  void setControllerButtonGroup(QButtonGroup* group);

public slots:

  /// Set a new SliceNode.
  void setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode);

  /// Set a new imageData.
  void setImageData(vtkImageData* newImageData);

  /// \sa fitSliceToBackground();
  void setSliceViewSize(const QSize& newSize);

  /// Fit slices to background. A No-op if no SliceView has been set.
  /// \sa setSliceView();
  void fitSliceToBackground();

  /// Set slice orientation.
  /// \note Orientation could be either "Axial, "Sagittal", "Coronal" or "Reformat".
  void setSliceOrientation(const QString& orientation);

  /// Set slice \a offset.
  void setSliceOffsetValue(double offset);

signals:

  /// This signal is emitted when the giben \a imageData is modified.
  void imageDataModified(vtkImageData * imageData);

private:
  CTK_DECLARE_PRIVATE(qMRMLSliceControllerWidget);
};

#endif
