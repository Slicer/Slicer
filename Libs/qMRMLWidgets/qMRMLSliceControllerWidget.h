#ifndef __qMRMLSliceControllerWidget_h
#define __qMRMLSliceControllerWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// qMRMLWidget includes
#include "qMRMLWidget.h"

#include "qMRMLWidgetsExport.h"

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

public slots:

  /// Set \a newSliceNode
  void setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode);

  /// Set imageData
  void setImageData(vtkImageData* newImageData);

  /// \sa fitSliceToBackground();
  void setSliceViewSize(const QSize& newSize);

  /// Fit slices to background. A No-op if no SliceView has been set
  /// \sa setSliceView();
  void fitSliceToBackground();

  /// Set slice orientation
  /// Orientation could be either "Axial, "Sagittal", "Coronal" or "Reformat"
  void setSliceOrientation(const QString& orienation);

  /// Called after the slice offset slider position is changed
  void setSliceOffsetValue(double value);

signals:
  void imageDataModified(vtkImageData * imageData);

private:
  CTK_DECLARE_PRIVATE(qMRMLSliceControllerWidget);
};

#endif
