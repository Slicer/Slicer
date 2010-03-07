#ifndef __qMRMLWindowLevelWidget_h
#define __qMRMLWindowLevelWidget_h


/// qVTK includes
#include <qVTKObject.h>

/// qCTK includes
#include <qCTKPimpl.h>

/// QT includes
#include <QWidget>

#include "qMRMLWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLScalarVolumeDisplayNode;
class vtkMRMLScalarVolumeNode;
class qMRMLWindowLevelWidgetPrivate;

class QMRML_WIDGETS_EXPORT qMRMLWindowLevelWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

  Q_PROPERTY(int autoWindowLevel READ autoWindowLevel WRITE setAutoWindowLevel)
  Q_PROPERTY(double window READ window WRITE setWindow)
  Q_PROPERTY(double level READ level WRITE setLevel)

public:
  /// Constructors
  typedef QWidget Superclass;
  explicit qMRMLWindowLevelWidget(QWidget* parent);
  virtual ~qMRMLWindowLevelWidget(){}

  int autoWindowLevel() const;

  /// 
  /// Get window
  double window()const;

  /// 
  /// Get level
  double level()const;

  /// 
  /// Return the current MRML node of interest
  vtkMRMLScalarVolumeNode* mrmlVolumeNode()const
    { return this->VolumeNode; };

signals:
  /// 
  /// Signal sent if the window/level value is updated
  void windowLevelValuesChanged(double window, double level);
  /// 
  /// Signal sent if the auto/manual value is updated
  void autoWindowLevelValueChanged(int value);

public slots:
  ///
  /// Set Auto/Manual mode
  void setAutoWindowLevel(int autoWindowLevel);

  /// 
  /// Set window
  void setWindow(double window);

  /// 
  /// Set level
  void setLevel(double level);

  /// 
  /// Sset window/level in once
  void setWindowLevel(double window, double level);
  // TODO remove when range becomes double
  void setWindowLevel(int window, int level);

  /// 
  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLScalarVolumeNode* displayNode);
  void setMRMLVolumeNode(vtkMRMLNode* node);


protected slots:

  /// update widget GUI from MRML node
  void updateWidgetFromMRML();

protected:
  /// 
  /// Return the current MRML display node
  vtkMRMLScalarVolumeDisplayNode* mrmlDisplayNode()const 
    { return this->VolumeDisplayNode;};

  /// 
  /// Set current MRML display node
  void setMRMLVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode* displayNode);

  /// 
  /// Set sliders range
  void setMinimum(double min);
  void setMaximum(double max);


private:
  QCTK_DECLARE_PRIVATE(qMRMLWindowLevelWidget);

  vtkMRMLScalarVolumeNode* VolumeNode;
  vtkMRMLScalarVolumeDisplayNode* VolumeDisplayNode;
};

#endif
