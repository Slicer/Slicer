#ifndef __qMRMLNodeTreeSelector_h
#define __qMRMLNodeTreeSelector_h

// CTK includes
#include <ctkPimpl.h>

/// qMRML includes
#include "qMRMLNodeComboBox.h"

#include "qMRMLWidgetsExport.h"

class qMRMLNodeTreeSelectorPrivate;

class QMRML_WIDGETS_EXPORT qMRMLNodeTreeSelector : public qMRMLNodeComboBox
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLNodeComboBox Superclass;
  
  /// Constructors
  explicit qMRMLNodeTreeSelector(QWidget* parent = 0);
  virtual ~qMRMLNodeTreeSelector(){}

protected:
  virtual void addNodeInternal(vtkMRMLNode* mrmlNode);
  
private:
  CTK_DECLARE_PRIVATE(qMRMLNodeTreeSelector);
};

#endif
