#ifndef __qSlicerAnnotationModulePropertyDialog_h
#define __qSlicerAnnotationModulePropertyDialog_h

#include <QLineEdit>
#include "ui_qSlicerAnnotationModulePropertyDialog.h"

#include "qSlicerAbstractModuleWidget.h"

#include "vtkStdString.h"

class vtkSlicerAnnotationModuleLogic;
class vtkMRMLAnnotationLinesNode;
class vtkMRMLAnnotationControlPointsNode;
class vtkMRMLAnnotationNode;
class vtkMRMLNode;
class vtkMRMLAnnotationDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationRulerNode;
class vtkMRMLAnnotationAngleNode;
class vtkMRMLAnnotationFiducialNode;

/// \ingroup Slicer_QtModules_Annotation
class qSlicerAnnotationModulePropertyDialog : public QDialog
{
  Q_OBJECT

public:
  qSlicerAnnotationModulePropertyDialog(const char * id, vtkSlicerAnnotationModuleLogic* logic);
  ~qSlicerAnnotationModulePropertyDialog() override;

  const char * GetID() { return this->m_id; }


  void updateTextFromTable(QString text);
  void updateValue(QString valueString);

  void saveStateForUndo(vtkMRMLNode* node);
  void undo(vtkMRMLNode* node);
  void SetButtonText(int type);
  void updateLockUnlockStatus(bool isLock);

  static void formatValueToChar(const char* format, std::vector<double> vv, QString &valueString );

  /// for the current m_id node, if all display nodes have the same unselected
  /// colour, return it, otherwise return black.
  void getAllColor(QColor &qcolor);
  /// use the GetAllColor method and set the all color button
  void updateAllColorButton();

  /// look for the annotation display nodes for the node with id and set the
  /// unselected color on each one. If it's the currently displayed node,
  /// update the buttons as well.
  void setColorOnAnnotationDisplayNodes(const char *id, QColor qcolor);

protected:
  /// update the ui.typeLabel from the node with m_id
  void updateTypeLabelText();
  /// update the ui.idLabel from the node with m_id
  void updateIDLabelText();
  /// update the ui.nameLineEdit from the node with m_id
  void updateNameText();
protected slots:

  void onNameLineEditChanged();
  /// change the color on the hierarchy display node as well as on all display
  /// nodes for all children
  void onAllColorChanged(QColor qcolor);
  /// change point size for all annotations in this hierarchy
  void onHierarchyPointSizeChanged(double value);
  /// reset the hierarchy point size slider to default value
  void onHierarchyPointSizeDefaultButtonClicked();
  /// change the text size for all annotations in this hierarchy
  void onHierarchyTextScaleChanged(double value);
  /// reset the hierarchy text scale slider to default value
  void onHierarchyTextScaleDefaultButtonClicked();
  /// change the glyph type for all annotations in this hierarchy
  void onHierarchyPointGlyphChanged(QString value);
  /// reset the hierarchy point glyph combo box to default glyph type
  void onHierarchyPointGlyphTypeDefaultButtonClicked();


  void onSizeSmallPushButtonClicked();
  void onSizeMediumPushButtonClicked();
  void onSizeLargePushButtonClicked();

    void onCoordinateChanged(QString text);

    void onPointsTableWidgetChanged(QTableWidgetItem *tableItem);
    void onPointColorChanged(QColor qcolor);
    void onPointSelectedColorChanged(QColor qcolor);
    void onPointSizeChanged(double value);
    void onPointOpacityChanged(double value);
    void onPointAmbientChanged(double value);
    void onPointDiffuseChanged(double value);
    void onPointSpecularChanged(double value);
    void onPointGlyphChanged(QString value);
    void onLineColorChanged(QColor qcolor);
    void onLineSelectedColorChanged(QColor qcolor);
    void onLineWidthChanged(double value);
    void onLineLabelPositionChanged(double value);
    void onLineLabelVisibilityStateChanged(int state);
    void onLineTickSpacingChanged();
    void onLineMaxTicksChanged(double value);
    void onLineOpacityChanged(double value);
    void onLineAmbientChanged(double value);
    void onLineDiffuseChanged(double value);
    void onLineSpecularChanged(double value);

  void onDescriptionTextChanged();
  void onTextChanged();
  void onRASCoordinatesChanged(double *coords);
  void onDialogRejected();
  void onDialogAccepted();
  void onTextUnselectedColorChanged(QColor qcolor);
  void onTextSelectedColorChanged(QColor qcolor);
  void onTextScaleChanged(double value);
  void onTextOpacityChanged(double value);
  void onTextVisibilityChanged(bool value);

  void onLockUnlockButtonClicked();
  void onVisibleInvisibleButtonClicked();


signals:
    void coordinateChanged(QString, char*);
    void textChanged();
    void textChanged(const QString &);
    void dialogRejected();
    void dialogAccepted();
    void doublePropertyChanged(double, char*, int);
    void colorPropertyChanged(QColor, char*, int);
    void itemChanged(QTableWidgetItem *);
  void coordinatesChanged(double*);

private:

    void saveLinesNode(vtkMRMLAnnotationLinesNode* node);
    void saveControlPoints(vtkMRMLAnnotationControlPointsNode* node);
    void saveAnnotationNode(vtkMRMLAnnotationNode* node);
    void undoLinesNode(vtkMRMLAnnotationLinesNode* node);
    void undoControlPoints(vtkMRMLAnnotationControlPointsNode* node);
    void undoAnnotationNode(vtkMRMLAnnotationNode* node);

    // approved code starts here
    Ui::qSlicerAnnotationModulePropertyDialog ui;

    vtkStdString m_id;
    vtkSlicerAnnotationModuleLogic* m_logic;

    // create the slot and signal connections
    void createConnection();

    // initialize the GUI in respect to values from the logic
    void initialize();

    // lock/unlock the complete interface
    void lockUnlockInterface(bool lock);
};

#endif
