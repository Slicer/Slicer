#ifndef __qMRMLSliceViewWidget_h
#define __qMRMLSliceViewWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// qMRMLWidget includes
#include "qMRMLWidget.h"

#include "qMRMLWidgetsExport.h"

class qMRMLSliceViewWidgetPrivate;
class vtkMRMLScene;
class vtkMRMLNode;
class vtkMRMLSliceNode;
class vtkMRMLSliceCompositeNode;
class vtkImageData;
class ctkVTKSliceView;

class QMRML_WIDGETS_EXPORT qMRMLSliceViewWidget : public qMRMLWidget
{
  Q_OBJECT
  Q_PROPERTY(QString sliceOrientation READ sliceOrientation WRITE setSliceOrientation)
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;
  
  /// Constructors
  explicit qMRMLSliceViewWidget(const QString& sliceViewName, QWidget* parent = 0);
  virtual ~qMRMLSliceViewWidget(){}

  ///
  /// Get slice orientation
  /// \sa setSliceOrientation(QString);
  QString sliceOrientation();

  ///
  /// Get \a sliceNode
  vtkMRMLSliceNode* mrmlSliceNode()const;

  ///
  /// Get imageData
  vtkImageData* imageData()const;

  ///
  /// Set/Get \a sliceCompositeNode
  void setMRMLSliceCompositeNode(vtkMRMLSliceCompositeNode* sliceCompositeNode);
  vtkMRMLSliceCompositeNode* mrmlSliceCompositeNode()const;

  ///
  /// Set slice offset range
  void setSliceOffsetRange(double min, double max);

  /// Set slice offset increment
  void setSliceOffsetResolution(double resolution);

public slots:

  /// 
  /// Set the MRML \a scene that should be listened for events
  virtual void setMRMLScene(vtkMRMLScene* scene);

  ///
  /// Set \a sliceNode
  virtual void setMRMLSliceNode(vtkMRMLSliceNode* sliceNode);

  ///
  /// Set imageData
  virtual void setImageData(vtkImageData* newImageData);

  ///
  /// Set slice orientation
  /// Orientation could be either "Axial, "Sagital", "Coronal" or "Reformat"
  void setSliceOrientation(const QString& orienation);

  ///
  /// \sa ctkVTKSliceView::scheduleRender()
  void scheduleRender();

protected slots:
  ///
  /// Triggered after the slice offset slider position is changed
  virtual void onSliceOffsetValueChanged(double value);

protected:

  /// Get a reference to the underlying Slice View
  ctkVTKSliceView * sliceView()const;

private:
  CTK_DECLARE_PRIVATE(qMRMLSliceViewWidget);
};

#endif
