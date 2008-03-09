#ifndef __vtkSlicerDataTransferWidget_h
#define __vtkSlicerDataTransferWidget_h

#include "vtkSlicerWidget.h"
#include "vtkDataTransfer.h"

//widgets
class vtkKWIcon;
class vtkKWLabel;
class vtkKWPushButton;
class vtkKWFrame;
class vtkKWTopLevel;
class vtkSlicerDataTransferIcons;
class vtkKWTextWithScrollbars;


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerDataTransferWidget : public vtkSlicerWidget
  {
  public:

    // Description:
    // Usual vtk class functions.
    static vtkSlicerDataTransferWidget* New();
    vtkTypeRevisionMacro(vtkSlicerDataTransferWidget,vtkSlicerWidget);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Get/Set on members
    vtkGetObjectMacro (DataTransferFrame, vtkKWFrame );
    vtkGetObjectMacro (URILabel, vtkKWLabel );
    vtkGetObjectMacro (TransferTypeLabel, vtkKWLabel );
    vtkGetObjectMacro (TransferStatusLabel, vtkKWLabel );
    vtkGetObjectMacro (CancelButton, vtkKWPushButton );
    vtkGetObjectMacro (DeleteButton, vtkKWPushButton );
    vtkGetObjectMacro (InformationButton, vtkKWPushButton );
    vtkGetObjectMacro (InformationTopLevel, vtkKWTopLevel );
    vtkGetObjectMacro (InformationFrame, vtkKWFrame );
    vtkGetObjectMacro (InformationText, vtkKWTextWithScrollbars );
    vtkGetObjectMacro (DataTransferIcons, vtkSlicerDataTransferIcons );
    vtkGetObjectMacro (InformationCloseButton, vtkKWPushButton );
    vtkGetObjectMacro ( DataTransfer, vtkDataTransfer);
    vtkSetObjectMacro ( DataTransfer, vtkDataTransfer);

    // Description:
    // Add/Remove observers on widgets in the GUI.
    virtual void AddWidgetObservers();
    virtual void RemoveWidgetObservers();

    // Description:
    // Method to propagate events generated in GUI to logic / mrml.
    void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData);

    // Description:
    // Methods to update content in the information text widget
    // and to display and hide it.
    virtual void DisplayInformationWindow();
    virtual void HideInformationWindow();
    virtual void UpdateInformationText();
    
    // Description:
    // Method to update the widget when a new node is loaded.
    // Or when a Data Transfer's state may have changed.
    void UpdateWidget();

  protected:
    vtkSlicerDataTransferWidget(void);
    virtual ~vtkSlicerDataTransferWidget(void);

    // Description:
    // Method to create the widget.
    virtual void CreateWidget();

    //widgets (GUI)
    vtkKWFrame *DataTransferFrame;
    vtkKWLabel *URILabel;
    vtkKWLabel *TransferTypeLabel;
    vtkKWLabel *TransferStatusLabel;
    vtkKWPushButton *CancelButton;
    vtkKWPushButton *InformationButton;
    vtkKWPushButton *DeleteButton;
    vtkSlicerDataTransferIcons *DataTransferIcons;
    vtkKWTopLevel *InformationTopLevel;
    vtkKWFrame *InformationFrame;
    vtkKWTextWithScrollbars *InformationText;
    vtkKWPushButton *InformationCloseButton;
    vtkDataTransfer *DataTransfer;

  private:
    vtkSlicerDataTransferWidget (const vtkSlicerDataTransferWidget&); // Not implemented.
    void operator = (const vtkSlicerDataTransferWidget&); //Not implemented.
  };

#endif 
