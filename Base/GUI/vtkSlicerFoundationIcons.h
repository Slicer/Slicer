#ifndef __vtkSlicerFoundationIcons_h
#define __vtkSlicerFoundationIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerFoundation_ImageData.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerFoundationIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerFoundationIcons* New ( );
    vtkTypeRevisionMacro (vtkSlicerFoundationIcons, vtkSlicerIcons );
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( SlicerMoreOptionsIcon, vtkKWIcon);
    vtkGetObjectMacro ( SlicerGoIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerGoing0Icon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerGoing1Icon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerGoing2Icon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerGoing3Icon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerGoing4Icon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerGoing5Icon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerGoing6Icon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerGoing7Icon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerCameraIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerBlankIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerCancelIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerCancelDisabledIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerCancelledIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerCancelRequestedIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerCleanUpIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerColorsIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerPlayerCycleIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerDecrementIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerDeleteIcon, vtkKWIcon);
    vtkGetObjectMacro ( SlicerDeleteDisabledIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerDoneIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerErrorIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerGlyphIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerPlayerGoToFirstIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerPlayerGoToLastIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerIncrementIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerInformationIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerLoadIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerDownloadIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerUploadIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerPlayerPauseIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerPlayerPingPongIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerPlayerBackwardIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerPlayerForwardIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerPreparingIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerPlayerRecordIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerSaveIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerPlayerStopRecordingIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerTimedOutIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerTinyHelpIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerWaitIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerMagnifyIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerMinifyIcon, vtkKWIcon );    
    vtkGetObjectMacro ( SlicerNextIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerPreviousIcon, vtkKWIcon );    
    vtkGetObjectMacro ( SlicerGoToEndIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerGoToStartIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerUndoIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerRedoIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerUnlinkIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerLinkIcon, vtkKWIcon );    
    vtkGetObjectMacro ( SlicerVisibleIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerInvisibleIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerRefreshIcon, vtkKWIcon );
    vtkGetObjectMacro ( SlicerVolumeIcon, vtkKWIcon );

    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerFoundationIcons ( );
    virtual ~vtkSlicerFoundationIcons ( );
    vtkKWIcon *SlicerGoIcon;
    vtkKWIcon *SlicerGoing0Icon;
    vtkKWIcon *SlicerGoing1Icon;
    vtkKWIcon *SlicerGoing2Icon;
    vtkKWIcon *SlicerGoing3Icon;
    vtkKWIcon *SlicerGoing4Icon;
    vtkKWIcon *SlicerGoing5Icon;
    vtkKWIcon *SlicerGoing6Icon;
    vtkKWIcon *SlicerGoing7Icon;
    vtkKWIcon *SlicerCameraIcon;
    vtkKWIcon *SlicerBlankIcon;
    vtkKWIcon *SlicerCancelIcon;
    vtkKWIcon *SlicerCancelDisabledIcon;
    vtkKWIcon *SlicerCancelledIcon;
    vtkKWIcon *SlicerCancelRequestedIcon;
    vtkKWIcon *SlicerCleanUpIcon;
    vtkKWIcon *SlicerColorsIcon;
    vtkKWIcon *SlicerPlayerCycleIcon;
    vtkKWIcon *SlicerDecrementIcon;
    vtkKWIcon *SlicerDeleteIcon;
    vtkKWIcon *SlicerDeleteDisabledIcon;
    vtkKWIcon *SlicerDoneIcon;
    vtkKWIcon *SlicerErrorIcon;
    vtkKWIcon *SlicerGlyphIcon;
    vtkKWIcon *SlicerPlayerGoToFirstIcon;
    vtkKWIcon *SlicerPlayerGoToLastIcon;
    vtkKWIcon *SlicerIncrementIcon;
    vtkKWIcon *SlicerInformationIcon;
    vtkKWIcon *SlicerLoadIcon;
    vtkKWIcon *SlicerUploadIcon;
    vtkKWIcon *SlicerDownloadIcon;
    vtkKWIcon *SlicerPlayerPauseIcon;
    vtkKWIcon *SlicerPlayerPingPongIcon;
    vtkKWIcon *SlicerPlayerBackwardIcon;
    vtkKWIcon *SlicerPlayerForwardIcon;
    vtkKWIcon *SlicerPreparingIcon;
    vtkKWIcon *SlicerPlayerRecordIcon;
    vtkKWIcon *SlicerSaveIcon;
    vtkKWIcon *SlicerPlayerStopRecordingIcon;
    vtkKWIcon *SlicerTimedOutIcon;
    vtkKWIcon *SlicerTinyHelpIcon;
    vtkKWIcon *SlicerWaitIcon;
    vtkKWIcon *SlicerMagnifyIcon;
    vtkKWIcon *SlicerMinifyIcon;
    vtkKWIcon *SlicerNextIcon;
    vtkKWIcon *SlicerPreviousIcon;
    vtkKWIcon *SlicerGoToEndIcon;
    vtkKWIcon *SlicerGoToStartIcon;
    vtkKWIcon *SlicerUndoIcon;
    vtkKWIcon *SlicerRedoIcon;
    vtkKWIcon *SlicerUnlinkIcon;
    vtkKWIcon *SlicerLinkIcon;    
    vtkKWIcon *SlicerVisibleIcon;
    vtkKWIcon *SlicerInvisibleIcon;
    vtkKWIcon *SlicerRefreshIcon;
    vtkKWIcon *SlicerVolumeIcon;
    vtkKWIcon *SlicerMoreOptionsIcon;
    
 private:
    vtkSlicerFoundationIcons ( const vtkSlicerFoundationIcons&); // Not implemented
    void operator = (const vtkSlicerFoundationIcons& ); // not implemented.
    
};
#endif
