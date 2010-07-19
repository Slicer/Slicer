#ifndef __qMRMLSliceInformationWidget_h
#define __qMRMLSliceInformationWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// qMRMLWidget includes
#include "qMRMLWidget.h"

#include "qMRMLWidgetsExport.h"

class qMRMLSliceInformationWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLSliceNode;

class vtkMRMLSliceLogic;

class QMRML_WIDGETS_EXPORT qMRMLSliceInformationWidget : public qMRMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;
  
  /// Constructors
  explicit qMRMLSliceInformationWidget(QWidget* parent = 0);
  virtual ~qMRMLSliceInformationWidget(){}

  /// Get \a sliceNode
  /// \sa setMRMLSliceCompositeNode();
  vtkMRMLSliceNode* mrmlSliceNode()const;

public slots:

  /// Set a new SliceNode.
  void setMRMLSliceNode(vtkMRMLNode* newNode);

  /// Set a new SliceNode.
  void setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode);

  /// Set slice orientation.
  /// \note Orientation could be either "Axial, "Sagittal", "Coronal" or "Reformat".
  void setSliceOrientation(const QString& orientation);

  /// Set slice visible.
  void setSliceVisible(bool visible);

  /// Set widget visible.
  void setWidgetVisible(bool visible);

  /// Set lightbox layout row count
  void setLightboxLayoutRows(int rowCount);

  /// Set lightbox layout column count
  void setLightboxLayoutColumns(int columnCount);

  /// Specify whether the slice spacing is automatically determined or prescribed
  /// \sa vtkMRMLSliceNode::AutomaticSliceSpacingMode vtkMRMLSliceNode::PrescribedSliceSpacingMode
  void setSliceSpacingMode(int spacingMode);

  /// Set prescribed spacing
  void setPrescribedSliceSpacing(double spacing);

private:
  CTK_DECLARE_PRIVATE(qMRMLSliceInformationWidget);
};

#endif
