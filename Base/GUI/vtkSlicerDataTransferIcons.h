#ifndef __vtkSlicerDataTransferIcons_h
#define __vtkSlicerDataTransferIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerDataTransfer_ImageData.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerDataTransferIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerDataTransferIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerDataTransferIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( TransferTypeUnspecifiedIcon, vtkKWIcon);
    vtkGetObjectMacro ( TransferTypeLoadIcon, vtkKWIcon);
    vtkGetObjectMacro ( TransferTypeSaveIcon, vtkKWIcon);
    vtkGetObjectMacro ( TransferTypeRemoteLoadIcon, vtkKWIcon);
    vtkGetObjectMacro ( TransferTypeRemoteSaveIcon, vtkKWIcon);

    vtkGetObjectMacro ( TransferStatusGoingIcon, vtkKWIcon);
    vtkGetObjectMacro ( TransferStatusGoing0Icon, vtkKWIcon);
    vtkGetObjectMacro ( TransferStatusGoing1Icon, vtkKWIcon);
    vtkGetObjectMacro ( TransferStatusGoing2Icon, vtkKWIcon);
    vtkGetObjectMacro ( TransferStatusGoing3Icon, vtkKWIcon);
    vtkGetObjectMacro ( TransferStatusGoing4Icon, vtkKWIcon);
    vtkGetObjectMacro ( TransferStatusGoing5Icon, vtkKWIcon);
    vtkGetObjectMacro ( TransferStatusGoing6Icon, vtkKWIcon);
    vtkGetObjectMacro ( TransferStatusGoing7Icon, vtkKWIcon);

    vtkGetObjectMacro ( TransferStatusCancelledIcon, vtkKWIcon);
    vtkGetObjectMacro ( TransferStatusDoneIcon, vtkKWIcon);
    vtkGetObjectMacro ( TransferStatusTimedOutIcon, vtkKWIcon);
    vtkGetObjectMacro ( TransferStatusIdleIcon, vtkKWIcon );
    vtkGetObjectMacro ( TransferStatusWaitIcon, vtkKWIcon );
    vtkGetObjectMacro ( TransferStatusReadyIcon, vtkKWIcon);
    vtkGetObjectMacro ( TransferStatusErrorIcon, vtkKWIcon );
    vtkGetObjectMacro ( TransferStatusCancelRequestedIcon, vtkKWIcon );

    vtkGetObjectMacro ( TransferCancelIcon, vtkKWIcon);
    vtkGetObjectMacro ( TransferInformationIcon, vtkKWIcon);
    vtkGetObjectMacro ( DeleteFromCacheIcon, vtkKWIcon);
    vtkGetObjectMacro ( TransferCancelDisabledIcon, vtkKWIcon );
    vtkGetObjectMacro ( DeleteFromCacheDisabledIcon, vtkKWIcon );
    vtkGetObjectMacro ( RefreshSettingsIcon, vtkKWIcon );


    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerDataTransferIcons ( );
    ~vtkSlicerDataTransferIcons ( );

    vtkKWIcon *TransferTypeUnspecifiedIcon;
    vtkKWIcon *TransferTypeRemoteLoadIcon;
    vtkKWIcon *TransferTypeRemoteSaveIcon;
    vtkKWIcon *TransferTypeLoadIcon;
    vtkKWIcon *TransferTypeSaveIcon;


    vtkKWIcon *TransferStatusGoingIcon;
    vtkKWIcon *TransferStatusGoing0Icon;
    vtkKWIcon *TransferStatusGoing1Icon;
    vtkKWIcon *TransferStatusGoing2Icon;
    vtkKWIcon *TransferStatusGoing3Icon;
    vtkKWIcon *TransferStatusGoing4Icon;
    vtkKWIcon *TransferStatusGoing5Icon;
    vtkKWIcon *TransferStatusGoing6Icon;
    vtkKWIcon *TransferStatusGoing7Icon;

    vtkKWIcon *TransferStatusReadyIcon;
    vtkKWIcon *TransferStatusCancelledIcon;
    vtkKWIcon *TransferStatusDoneIcon;
    vtkKWIcon *TransferStatusTimedOutIcon;
    vtkKWIcon *TransferStatusIdleIcon;
    vtkKWIcon *TransferStatusWaitIcon;
    vtkKWIcon *TransferStatusCancelRequestedIcon;
    vtkKWIcon *TransferStatusErrorIcon;

    vtkKWIcon *TransferCancelIcon;
    vtkKWIcon *TransferInformationIcon;
    vtkKWIcon *DeleteFromCacheIcon;
    vtkKWIcon *TransferCancelDisabledIcon;
    vtkKWIcon *DeleteFromCacheDisabledIcon;
    vtkKWIcon *RefreshSettingsIcon;
    
 private:
    vtkSlicerDataTransferIcons (const vtkSlicerDataTransferIcons&); // Not implemented
    void operator = ( const vtkSlicerDataTransferIcons& ); // Not implemented
    
};

#endif
