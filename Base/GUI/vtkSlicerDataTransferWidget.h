#ifndef __vtkSlicerDataTransferWidget_h
#define __vtkSlicerDataTransferWidget_h

#include "vtkSlicerWidget.h"

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
    vtkGetObjectMacro (InformationButton, vtkKWPushButton );
    vtkGetObjectMacro (InformationTopLevel, vtkKWTopLevel );
    vtkGetObjectMacro (InformationFrame, vtkKWFrame );
    vtkGetObjectMacro (InformationText, vtkKWTextWithScrollbars );
    vtkGetObjectMacro (DataTransferIcons, vtkSlicerDataTransferIcons );
    vtkGetObjectMacro (InformationCloseButton, vtkKWPushButton );
    vtkGetMacro ( TransferType, int );
    vtkSetMacro ( TransferType, int );
    vtkGetMacro ( Status, int );
    vtkSetMacro ( Status, int );

    // Description:
    // Add/Remove observers on widgets in the GUI.
    virtual void AddWidgetObservers();
    virtual void RemoveWidgetObservers();

    // Description:
    // Method to propagate events generated in GUI to logic / mrml.
    void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData);

    // Description:
    // Method to update the widget when a new node is loaded.
    void UpdateWidget();

  protected:
    vtkSlicerDataTransferWidget(void);
    virtual ~vtkSlicerDataTransferWidget(void);

    // Description:
    // Method to create the widget.
    virtual void CreateWidget();

    // Description:
    virtual void InvokeTransferCancelEvent();

    virtual void PopulateInformationDisplay ( );

    // Description:
    // Method to update the data transfer information
    void UpdateInfo();

    //widgets (GUI)
    vtkKWFrame *DataTransferFrame;
    vtkKWLabel *URILabel;
    vtkKWLabel *TransferTypeLabel;
    vtkKWLabel *TransferStatusLabel;
    vtkKWPushButton *CancelButton;
    vtkKWPushButton *InformationButton;
    vtkSlicerDataTransferIcons *DataTransferIcons;
    vtkKWTopLevel *InformationTopLevel;
    vtkKWFrame *InformationFrame;
    vtkKWTextWithScrollbars *InformationText;
    vtkKWPushButton *InformationCloseButton;
    int TransferType;
    int Status;

    //BTX
    enum {
      StatusUnspecified = 0,
      StatusGoing,
      StatusCancelled,
      StatusDone,
    };
    enum {
      Unspecified = 0,
      Download,
      Upload,
      Load,
      Save,
      DownloadToCache,
      UploadFromCache,
      LoadFromCache,
      SaveToCache,
      LoadFromDisk,
      SaveToDisk,
    };
    //ETX

  private:
    vtkSlicerDataTransferWidget (const vtkSlicerDataTransferWidget&); // Not implemented.
    void operator = (const vtkSlicerDataTransferWidget&); //Not implemented.
  };

#endif 
