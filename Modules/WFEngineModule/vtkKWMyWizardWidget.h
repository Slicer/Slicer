#ifndef VTKKWMYWIZARDWIDGET_H_
#define VTKKWMYWIZARDWIDGET_H_

#include <vtkKWWizardWidget.h>

#include <map>
#include <string>

class vtkKWWizardWorkflow;
class vtkKWWizardWorkflow;
class vtkKWProgressGauge;
class vtkKWComboBoxWithLabel;
class vtkKWWizardStep;

class vtkKWMyWizardWidget : public vtkKWWizardWidget
{
public:
    static vtkKWMyWizardWidget *New();
    
    vtkTypeRevisionMacro(vtkKWMyWizardWidget,vtkKWWizardWidget);
    
//    vtkKWWizardWorkflow *GetWizardWorkflow();
//    vtkKWMyWizardWorkflow *GetMyWizardWorkflow();
    
    void SetNumberOfUnprocessedSteps(int steps);
    int GetNumberOfUnprocessedSteps();
    void SetNumberOfProcessedSteps(int steps);
    //BTX
    enum{
        nextButtonClicked = 10000,
        backButtonClicked,
        comboBoxEntryChanged
    };
    //ETX
    
    void UpdateNavigationGUI();
    
    vtkKWProgressGauge *ProgressGauge;
    vtkKWComboBoxWithLabel *ComboBox;
    
    void RemoveAllObservers();
    void ComboBoxEntryChanged(const char*);
    
    const char* GetCurrentComboBoxValue();
    int GetCurrentComboBoxIndex();
    vtkKWWizardStep *GetGotoWFStep();
protected:        
    vtkKWMyWizardWidget();
    virtual ~vtkKWMyWizardWidget();
    // Description:
    // Create the widget.
    virtual void CreateWidget();
    
    static void NextButtonClicked(vtkObject* obj, unsigned long,void*, void*);
    static void BackButtonClicked(vtkObject* obj, unsigned long,void*, void*);
    
    static void NavigationStackChanged(vtkObject* obj, unsigned long,void*, void*);
        
private:
//    vtkKWMyWizardWorkflow *WizardWorkflow;     
    
    int m_numberOfUnprocessedSteps;
    int m_numberOfProcessedSteps;
    int m_currentSelectedIndex;
    
    //BTX
    std::map<int, vtkKWWizardStep*> *m_itemToStepMap;    
    //ETX
    
    vtkKWMyWizardWidget(const vtkKWMyWizardWidget&);   // Not implemented.
    void operator=(const vtkKWMyWizardWidget&);  // Not implemented.
};

#endif /*VTKKWMYWIZARDWIDGET_H_*/
