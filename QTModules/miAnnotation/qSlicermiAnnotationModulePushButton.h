#ifndef __qSlicermiAnnotationModulePushButton_h
#define __qSlicermiAnnotationModulePushButton_h

// QT includes
#include <QPushButton>
#include <QString>

class qSlicermiAnnotationModulePushButton : public QPushButton
{
  Q_OBJECT
public:
  qSlicermiAnnotationModulePushButton(int index, QWidget *parent=0);
  ~qSlicermiAnnotationModulePushButton(){};
  void setButtonIndex(int index);

protected:
  
protected slots:
    void mybuttonClicked();

signals:
    void buttonClickedWithText(QString);
    void buttonClickedWithIndex(int);


private:

    QString m_text;
    int m_index;

};

#endif
