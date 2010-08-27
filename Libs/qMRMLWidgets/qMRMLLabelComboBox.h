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
  Q_PROPERTY( DisplayOption option READ displayOption WRITE setDisplayOption USER true)
  Q_ENUMS( DisplayOption )

public:

  typedef qMRMLWidget Superclass;

  /// Construct an empty qMRMLColorTableComboBox with a null scene,
  /// no nodeType, where the hidden nodes are not forced on display.
  explicit qMRMLLabelComboBox(QWidget* newParent = 0);
  virtual ~qMRMLLabelComboBox(){}

  enum DisplayOption { WithNone, OnlyColors };

  void setMRMLColorNode(vtkMRMLColorNode *newNode);
  
  void setCurrentColor(int index); 

  void setDisplayOption(qMRMLLabelComboBox::DisplayOption newOption);
  DisplayOption displayOption();

public slots:

  void setMRMLColorNode(vtkMRMLNode *newNode);
  
signals:

  void currentlabelChanged(QColor color);
  void currentlabelChanged(const QString &name);
  void currentlabelChanged(int index);

protected slots:

  void colorSelected(int index); 
  void updateWidgetFromMRML();

private:

  CTK_DECLARE_PRIVATE(qMRMLLabelComboBox);
};

#endif
