#ifndef __qSlicermiAnnotationModuleAnnotationPropertyDialog_h
#define __qSlicermiAnnotationModuleAnnotationPropertyDialog_h

#include <QLineEdit>
#include "ui_qSlicermiAnnotationModuleAnnotationPropertyDialog.h"

#include "qSlicerAbstractModuleWidget.h"

class vtkSlicermiAnnotationModuleLogic;
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


class qSlicermiAnnotationModuleAnnotationPropertyDialog : public QDialog
{
  Q_OBJECT

public:
  qSlicermiAnnotationModuleAnnotationPropertyDialog(vtkMRMLNode* node, vtkSlicermiAnnotationModuleLogic* logic);
  ~qSlicermiAnnotationModuleAnnotationPropertyDialog();

  void Initialize(vtkMRMLNode* node);
  void updateTextFromTable(QString text);
  void updateValue(QString valueString);
  void updateCoordinates(double* pos, int id);
  void SaveStateForUndo(vtkMRMLNode* node);
  void Undo(vtkMRMLNode* node);
  void SetButtonText(int type);
  void UpdateLockUnlockStatus(bool isLock);

  static void TurnColorArrayToQColor(double* color, QColor &qcolor);
  static void TurnQColorToColorArray(double* color, QColor &qcolor);
  static void FormatValueToChar(const char* format, double value, QString &valueString );


protected:
  
protected slots:

    void onCoordinateChanged(QString text);
    void onTextChanged();
    void onDialogRejected();
    void onDialogAccepted();
    void onTextColorChanged(QColor qcolor);
    void onTextSelectedColorChanged(QColor qcolor);
    void onTextScaleChanged(double value);
    void onPointColorChanged(QColor qcolor);
    void onPointSelectedColorChanged(QColor qcolor);
    void onPointSizeChanged(double value);
    void onPointOpacityChanged(double value);
    void onPointAmbientChanged(double value);
    void onPointDiffuseChanged(double value);
    void onPointSpecularChanged(double value);
    void onLineColorChanged(QColor qcolor);
    void onLineSelectedColorChanged(QColor qcolor);
    void onLineWidthChanged(double value);
    void onLineOpacityChanged(double value);
    void onLineAmbientChanged(double value);
    void onLineDiffuseChanged(double value);
    void onLineSpecularChanged(double value);


signals:
    void coordinateChanged(QString, char*);
    void textChanged(QString text, char*);
    void dialogRejected(char*);
    void dialogAccepted(char*, QString);
    void doublePropertyChanged(double, char*, int);
    void colorPropertyChanged(QColor, char*, int);


private:

    Ui::qSlicermiAnnotationModuleAnnotationPropertyDialog ui;
    QVector<QLineEdit*> m_lineEditList;
    QGridLayout* m_gridLayout;

    vtkSlicermiAnnotationModuleLogic* m_logic;

    vtkMRMLAnnotationRulerNode* m_rulerCopy;
    vtkMRMLAnnotationAngleNode* m_angleCopy;
    vtkMRMLAnnotationDisplayNode* m_textDispCopy;
    vtkMRMLAnnotationLineDisplayNode* m_lineDispCopy;
    vtkMRMLAnnotationPointDisplayNode* m_pointDispCopy;

    char* m_nodeId;
    bool m_isUpdated;

    void createConnection();

    void SaveLinesNode(vtkMRMLAnnotationLinesNode* node);
    void SaveControlPoints(vtkMRMLAnnotationControlPointsNode* node);
    void SaveAnnotationNode(vtkMRMLAnnotationNode* node);
    void UndoLinesNode(vtkMRMLAnnotationLinesNode* node);
    void UndoControlPoints(vtkMRMLAnnotationControlPointsNode* node);
    void UndoAnnotationNode(vtkMRMLAnnotationNode* node);

};

#endif
