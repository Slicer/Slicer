#ifndef __qMRMLColorTableComboBox_h
#define __qMRMLColorTableComboBox_h

// CTK includes
#include <ctkPimpl.h>

// qMRML includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLWidgetsExport.h"

class qMRMLColorTableComboBoxPrivate;

class QMRML_WIDGETS_EXPORT qMRMLColorTableComboBox : public qMRMLNodeComboBox
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLNodeComboBox Superclass;

  /// Construct an empty qMRMLColorTableComboBox with a null scene,
  /// no nodeType, where the hidden nodes are not forced on display.
  explicit qMRMLColorTableComboBox(QWidget* parent = 0);
  virtual ~qMRMLColorTableComboBox();
  virtual void setMRMLScene(vtkMRMLScene* scene);

protected:
  QAbstractItemModel* createSceneModel();

private:
  CTK_DECLARE_PRIVATE(qMRMLColorTableComboBox);
};

#endif
