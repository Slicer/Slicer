#ifndef __qSlicerAnnotationModulePushButton_h
#define __qSlicerAnnotationModulePushButton_h

#include <QPushButton>
#include <QString>

class qSlicerAnnotationModulePushButton : public QPushButton
{
  Q_OBJECT
public:
  qSlicerAnnotationModulePushButton(int index, QWidget *parent=0);
  ~qSlicerAnnotationModulePushButton(){};
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
