#ifndef VTKKWMYWIZARDWIDGET_H_
#define VTKKWMYWIZARDWIDGET_H_

#include <vtkKWWizardWidget.h>

class vtkKWWizardWorkflow;
class vtkKWMyWizardWorkflow;

class vtkKWMyWizardWidget : public vtkKWWizardWidget
{
public:
    static vtkKWMyWizardWidget *New();
    
    vtkTypeRevisionMacro(vtkKWMyWizardWidget,vtkKWWizardWidget);
    
    vtkKWWizardWorkflow *GetWizardWorkflow();
    vtkKWMyWizardWorkflow *GetMyWizardWorkflow();
    
    //BTX
    enum{
        nextButtonClicked = 10000,
        backButtonClicked
    };
    //ETX
protected:
    vtkKWMyWizardWidget();
    virtual ~vtkKWMyWizardWidget();
    // Description:
    // Create the widget.
    virtual void CreateWidget();
    
    static void NextButtonClicked(vtkObject* obj, unsigned long,void*, void*);
    static void BackButtonClicked(vtkObject* obj, unsigned long,void*, void*);
private:
    vtkKWMyWizardWorkflow *WizardWorkflow;
    
    vtkKWMyWizardWidget(const vtkKWMyWizardWidget&);   // Not implemented.
    void operator=(const vtkKWMyWizardWidget&);  // Not implemented.
};

#endif /*VTKKWMYWIZARDWIDGET_H_*/
