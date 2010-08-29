#ifndef __qMRMLLabelComboBox_h
#define __qMRMLLabelComboBox_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"
#include "qMRMLWidget.h"

class vtkMRMLNode;
class vtkMRMLColorNode;
class qMRMLLabelComboBoxPrivate;

class QMRML_WIDGETS_EXPORT qMRMLLabelComboBox : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(bool noneEnabled READ noneEnabled WRITE setNoneEnabled)
  Q_PROPERTY(int currentColor READ currentColor WRITE setCurrentColor USER true)
  Q_PROPERTY(int maximumColorCount READ maximumColorCount WRITE setMaximumColorCount)

public:

  typedef qMRMLWidget Superclass;

  /// Construct an empty qMRMLColorTableComboBox with a null scene,
  /// no nodeType, where the hidden nodes are not forced on display.
  explicit qMRMLLabelComboBox(QWidget* newParent = 0);
  virtual ~qMRMLLabelComboBox(){}

  /// Set/Get NoneEnabled flags
  /// An additional item is added into the menu list, where the user can select "None".
  bool noneEnabled()const;
  void setNoneEnabled(bool enable);

  virtual void printAdditionalInfo();

  vtkMRMLColorNode* mrmlColorNode()const;

  int currentColor()const;

  int maximumColorCount()const;
  void setMaximumColorCount(int maximum);

public slots:

  void setMRMLColorNode(vtkMRMLNode * newMRMLColorNode);

  void setCurrentColor(int index);

  void updateWidgetFromMRML();
  
signals:

  void currentColorChanged(const QColor& color);
  void currentColorChanged(const QString& name);
  void currentColorChanged(int index);

private slots:

  void onCurrentIndexChanged(int index);

private:

  CTK_DECLARE_PRIVATE(qMRMLLabelComboBox);
};

#endif
