

#include "vtkObjectFactory.h"
#include "vtkSlicerFoundationIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerFoundationIcons );
vtkCxxRevisionMacro ( vtkSlicerFoundationIcons, "$Revision$");

//---------------------------------------------------------------------------
vtkSlicerFoundationIcons::vtkSlicerFoundationIcons ( )
{
  this->SlicerFiducialsDeleteAllInListIcon = vtkKWIcon::New();
  this->SlicerLockOrUnlockIcon = vtkKWIcon::New();
  this->SlicerVisibleOrInvisibleIcon = vtkKWIcon::New();
  this->SlicerSelectAllIcon = vtkKWIcon::New();
  this->SlicerDeselectAllIcon = vtkKWIcon::New();  
  this->SlicerTableIcon = vtkKWIcon::New();
  this->SlicerMoreOptionsIcon = vtkKWIcon::New();
  this->SlicerGoIcon = vtkKWIcon::New();
  this->SlicerGoing0Icon = vtkKWIcon::New();
  this->SlicerGoing1Icon = vtkKWIcon::New();
  this->SlicerGoing2Icon = vtkKWIcon::New();
  this->SlicerGoing3Icon = vtkKWIcon::New();
  this->SlicerGoing4Icon = vtkKWIcon::New();
  this->SlicerGoing5Icon = vtkKWIcon::New();
  this->SlicerGoing6Icon = vtkKWIcon::New();
  this->SlicerGoing7Icon = vtkKWIcon::New();
  this->SlicerCameraIcon = vtkKWIcon::New();
  this->SlicerBlankIcon = vtkKWIcon::New();
  this->SlicerCancelIcon = vtkKWIcon::New();
  this->SlicerCancelDisabledIcon = vtkKWIcon::New();
  this->SlicerCancelledIcon = vtkKWIcon::New();
  this->SlicerCancelRequestedIcon = vtkKWIcon::New();
  this->SlicerCleanUpIcon = vtkKWIcon::New();
  this->SlicerColorsIcon = vtkKWIcon::New();
  this->SlicerPlayerCycleIcon = vtkKWIcon::New();
  this->SlicerDecrementIcon = vtkKWIcon::New();
  this->SlicerDeleteIcon = vtkKWIcon::New();
  this->SlicerDeleteDisabledIcon = vtkKWIcon::New();
  this->SlicerDoneIcon = vtkKWIcon::New();
  this->SlicerErrorIcon = vtkKWIcon::New();
  this->SlicerGlyphIcon = vtkKWIcon::New();
  this->SlicerPlayerGoToFirstIcon = vtkKWIcon::New();
  this->SlicerPlayerGoToLastIcon = vtkKWIcon::New();
  this->SlicerIncrementIcon = vtkKWIcon::New();
  this->SlicerInformationIcon = vtkKWIcon::New();
  this->SlicerLoadIcon = vtkKWIcon::New();
  this->SlicerDownloadIcon = vtkKWIcon::New();
  this->SlicerUploadIcon = vtkKWIcon::New();
  this->SlicerPlayerPauseIcon = vtkKWIcon::New();
  this->SlicerPlayerPingPongIcon = vtkKWIcon::New();
  this->SlicerPlayerBackwardIcon = vtkKWIcon::New();
  this->SlicerPlayerForwardIcon = vtkKWIcon::New();
  this->SlicerPreparingIcon = vtkKWIcon::New();
  this->SlicerPlayerRecordIcon = vtkKWIcon::New();
  this->SlicerSaveIcon = vtkKWIcon::New();
  this->SlicerPlayerStopRecordingIcon = vtkKWIcon::New();
  this->SlicerTimedOutIcon = vtkKWIcon::New();
  this->SlicerTinyHelpIcon = vtkKWIcon::New();
  this->SlicerWaitIcon = vtkKWIcon::New();
  this->SlicerMagnifyIcon = vtkKWIcon::New();
  this->SlicerMinifyIcon = vtkKWIcon::New();
  this->SlicerNextIcon = vtkKWIcon::New();
  this->SlicerPreviousIcon = vtkKWIcon::New();
  this->SlicerGoToEndIcon = vtkKWIcon::New();
  this->SlicerGoToStartIcon = vtkKWIcon::New();
  this->SlicerUndoIcon = vtkKWIcon::New();
  this->SlicerRedoIcon = vtkKWIcon::New();
  this->SlicerUnlinkIcon = vtkKWIcon::New();
  this->SlicerLinkIcon = vtkKWIcon::New();    
  this->SlicerCheckedVisibleIcon = vtkKWIcon::New();
  this->SlicerVisibleNoFrameIcon = vtkKWIcon::New();
  this->SlicerInvisibleNoFrameIcon = vtkKWIcon::New();
  this->SlicerVisibleIcon = vtkKWIcon::New();
  this->SlicerInvisibleIcon = vtkKWIcon::New();
  this->SlicerRefreshIcon = vtkKWIcon::New();
  this->SlicerVolumeIcon = vtkKWIcon::New();
  this->SlicerCenterOnFiducialIcon = vtkKWIcon::New();
  this->SlicerFiducialsAddNewIcon = vtkKWIcon::New();
  this->SlicerFiducialsDeleteAllIcon = vtkKWIcon::New();
  this->SlicerFiducialsDeleteLastClickedIcon = vtkKWIcon::New();
  this->SlicerFiducialsSelectAllIcon = vtkKWIcon::New();
  this->SlicerFiducialsSelectNoneIcon = vtkKWIcon::New();
  this->SlicerFiducialsSelectAllInListIcon = vtkKWIcon::New();
  this->SlicerFiducialsSelectNoneInListIcon = vtkKWIcon::New();
  this->SlicerFiducialsUpIcon = vtkKWIcon::New();
  this->SlicerFiducialsDownIcon = vtkKWIcon::New();
  this->SlicerFiducialsDeleteListIcon = vtkKWIcon::New();
  this->SlicerFiducialsLockListIcon = vtkKWIcon::New();
  this->SlicerFiducialsUnlockListIcon = vtkKWIcon::New();
  this->SlicerFiducialsHideListIcon = vtkKWIcon::New();
  this->SlicerFiducialsExposeListIcon = vtkKWIcon::New();
  this->SlicerFiducialsHideExposeAllListsIcon = vtkKWIcon::New();  
  this->SlicerFiducialsHideAllListsIcon = vtkKWIcon::New();  
  this->SlicerFiducialsExposeAllListsIcon = vtkKWIcon::New();  
  this->SlicerCompositeIcon = vtkKWIcon::New();
  this->SlicerLockIcon = vtkKWIcon::New();
  this->SlicerUnlockIcon = vtkKWIcon::New();
  this->SlicerFoundOnDiskIcon = vtkKWIcon::New();
  this->SlicerNotFoundOnDiskIcon = vtkKWIcon::New();
  this->SlicerImportSceneIcon = vtkKWIcon::New();
  this->SlicerLoadSceneIcon = vtkKWIcon::New();
  this->SlicerLoadDicomVolumeIcon = vtkKWIcon::New();
  this->SlicerLoadVolumeIcon = vtkKWIcon::New();
  this->SlicerLoadDirectoryIcon = vtkKWIcon::New();
  this->SlicerLoadFiducialsIcon = vtkKWIcon::New();
  this->SlicerLoadModelIcon = vtkKWIcon::New();
  this->SlicerLoadTransformIcon = vtkKWIcon::New();
  this->SlicerLoadColorLUTIcon = vtkKWIcon::New();
  this->SlicerLoadFiberBundleIcon = vtkKWIcon::New();
  this->SlicerLoadScalarOverlayIcon = vtkKWIcon::New();
  this->SlicerCloseIcon = vtkKWIcon::New();
  this->SlicerExtensionsIcon = vtkKWIcon::New();
  this->SlicerWWWIcon = vtkKWIcon::New();
  this->SlicerRotateToPixelSpaceIcon = vtkKWIcon::New();
  this->SlicerAnnotationRulerEndpoint1Icon= vtkKWIcon::New();
  this->SlicerAnnotationRulerEndpoint2Icon= vtkKWIcon::New();
  this->SlicerAnnotationAngleEndpoint1Icon= vtkKWIcon::New();
  this->SlicerAnnotationAngleEndpoint2Icon= vtkKWIcon::New();
  this->SlicerAnnotationAngleCenterpointIcon= vtkKWIcon::New();
  
  this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkSlicerFoundationIcons::~vtkSlicerFoundationIcons ( )
{

  if (this->SlicerFiducialsDeleteAllInListIcon)
    {
    this->SlicerFiducialsDeleteAllInListIcon->Delete();
    this->SlicerFiducialsDeleteAllInListIcon = NULL;    
    }
  if (this->SlicerLockOrUnlockIcon)
    {
    this->SlicerLockOrUnlockIcon->Delete();
    this->SlicerLockOrUnlockIcon = NULL;
    }
  if (this->SlicerVisibleOrInvisibleIcon)
    {
    this->SlicerVisibleOrInvisibleIcon->Delete();
    this->SlicerVisibleOrInvisibleIcon = NULL;
    }
  if (this->SlicerSelectAllIcon)
    {
    this->SlicerSelectAllIcon->Delete();
    this->SlicerSelectAllIcon = NULL;    
    }
  if (this->SlicerDeselectAllIcon )
    {
    this->SlicerDeselectAllIcon->Delete();
    this->SlicerDeselectAllIcon = NULL;    
    }
  if ( this->SlicerTableIcon )
    {
    this->SlicerTableIcon->Delete();
    this->SlicerTableIcon = NULL;
    }
  if ( this->SlicerMoreOptionsIcon )
    {
    this->SlicerMoreOptionsIcon->Delete();
    this->SlicerMoreOptionsIcon = NULL;
    }
  if ( this->SlicerGoIcon)
    {
    this->SlicerGoIcon->Delete();
    this->SlicerGoIcon = NULL;
    }
  if ( this->SlicerGoing0Icon )
    {
    this->SlicerGoing0Icon->Delete();
    this->SlicerGoing0Icon = NULL;
    }
  if ( this->SlicerGoing1Icon )
    {
    this->SlicerGoing1Icon->Delete();
    this->SlicerGoing1Icon = NULL;
    }
  if ( this->SlicerGoing2Icon )
    {
    this->SlicerGoing2Icon->Delete();
    this->SlicerGoing2Icon = NULL;
    }
  if ( this->SlicerGoing3Icon )
    {
    this->SlicerGoing3Icon->Delete();
    this->SlicerGoing3Icon = NULL;
    }
  if ( this->SlicerGoing4Icon )
    {
    this->SlicerGoing4Icon->Delete();
    this->SlicerGoing4Icon = NULL;
    }
  if ( this->SlicerGoing5Icon )
    {
    this->SlicerGoing5Icon->Delete();
    this->SlicerGoing5Icon = NULL;
    }
  if ( this->SlicerGoing6Icon )
    {
    this->SlicerGoing6Icon->Delete();
    this->SlicerGoing6Icon = NULL;
    }
  if ( this->SlicerGoing7Icon )
    {
    this->SlicerGoing7Icon->Delete();
    this->SlicerGoing7Icon = NULL;
    }
  if ( this->SlicerCameraIcon)
    {
    this->SlicerCameraIcon->Delete();
    this->SlicerCameraIcon = NULL;
    }
  if ( this->SlicerBlankIcon)
    {
    this->SlicerBlankIcon->Delete();
    this->SlicerBlankIcon = NULL;
    }
  if ( this->SlicerCancelIcon)
    {
    this->SlicerCancelIcon->Delete();
    this->SlicerCancelIcon = NULL;
    }
  if ( this->SlicerCancelDisabledIcon)
    {
    this->SlicerCancelDisabledIcon->Delete();
    this->SlicerCancelDisabledIcon = NULL;
    }
  if ( this->SlicerCancelledIcon)
    {
    this->SlicerCancelledIcon->Delete();
    this->SlicerCancelDisabledIcon = NULL;
    }
  if ( this->SlicerCancelRequestedIcon)
    {
    this->SlicerCancelRequestedIcon->Delete();
    this->SlicerCancelRequestedIcon = NULL;
    }
  if ( this->SlicerCleanUpIcon)
    {
    this->SlicerCleanUpIcon->Delete();
    this->SlicerCleanUpIcon = NULL;
    }
  if ( this->SlicerColorsIcon)
    {
    this->SlicerColorsIcon->Delete();
    this->SlicerColorsIcon = NULL;
    }
  if ( this->SlicerPlayerCycleIcon)
    {
    this->SlicerPlayerCycleIcon->Delete();
    this->SlicerPlayerCycleIcon = NULL;
    }
  if ( this->SlicerDecrementIcon)
    {
    this->SlicerDecrementIcon->Delete();
    this->SlicerDecrementIcon = NULL;
    }
  if ( this->SlicerDeleteIcon )
    {
    this->SlicerDeleteIcon->Delete();
    this->SlicerDeleteIcon = NULL;
    }
  if ( this->SlicerDeleteDisabledIcon)
    {
    this->SlicerDeleteDisabledIcon->Delete();
    this->SlicerDeleteDisabledIcon = NULL;
    }
  if ( this->SlicerDoneIcon)
    {
    this->SlicerDoneIcon->Delete();
    this->SlicerDoneIcon  = NULL;
    }
  if ( this->SlicerErrorIcon)
    {
    this->SlicerErrorIcon->Delete();
    this->SlicerErrorIcon = NULL;
    }
  if ( this->SlicerGlyphIcon)
    {
    this->SlicerGlyphIcon->Delete();
    this->SlicerGlyphIcon = NULL;
    }
  if ( this->SlicerPlayerGoToFirstIcon)
    {
    this->SlicerPlayerGoToFirstIcon->Delete();
    this->SlicerPlayerGoToFirstIcon = NULL;
    }
  if ( this->SlicerPlayerGoToLastIcon)
    {
    this->SlicerPlayerGoToLastIcon->Delete();
    this->SlicerPlayerGoToLastIcon = NULL;
    }
  if ( this->SlicerIncrementIcon)
    {
    this->SlicerIncrementIcon->Delete();
    this->SlicerIncrementIcon = NULL;
    }
  if ( this->SlicerInformationIcon)
    {
    this->SlicerInformationIcon->Delete();
    this->SlicerInformationIcon = NULL;    
    }
  if ( this->SlicerDownloadIcon)
    {
    this->SlicerDownloadIcon->Delete();
    this->SlicerDownloadIcon = NULL;
    }
  if ( this->SlicerUploadIcon)
    {
    this->SlicerUploadIcon->Delete();
    this->SlicerUploadIcon = NULL;
    }
  if ( this->SlicerLoadIcon)
    {
    this->SlicerLoadIcon->Delete();
    this->SlicerLoadIcon = NULL;    
    }
  if ( this->SlicerPlayerPauseIcon)
    {
    this->SlicerPlayerPauseIcon->Delete();
    this->SlicerPlayerPauseIcon = NULL;    
    }
  if ( this->SlicerPlayerPingPongIcon)
    {
    this->SlicerPlayerPingPongIcon->Delete();
    this->SlicerPlayerPingPongIcon = NULL;    
    }      
  if ( this->SlicerPlayerBackwardIcon)
    {
    this->SlicerPlayerBackwardIcon->Delete();
    this->SlicerPlayerBackwardIcon = NULL;    
    }
  if ( this->SlicerPlayerForwardIcon)
    {
    this->SlicerPlayerForwardIcon->Delete();
    this->SlicerPlayerForwardIcon = NULL;    
    }
  if ( this->SlicerPreparingIcon)
    {
    this->SlicerPreparingIcon->Delete();
    this->SlicerPreparingIcon = NULL;    
    }
  if ( this->SlicerPlayerRecordIcon)
    {
    this->SlicerPlayerRecordIcon->Delete();
    this->SlicerPlayerRecordIcon = NULL;    
    }
  if ( this->SlicerSaveIcon)
    {
    this->SlicerSaveIcon->Delete();
    this->SlicerSaveIcon = NULL;    
    }
  if ( this->SlicerPlayerStopRecordingIcon)
    {
    this->SlicerPlayerStopRecordingIcon->Delete();
    this->SlicerPlayerStopRecordingIcon = NULL;    
    }
  if ( this->SlicerTimedOutIcon)
    {
    this->SlicerTimedOutIcon->Delete();
    this->SlicerTimedOutIcon = NULL;    
    }
  if ( this->SlicerTinyHelpIcon)
    {
    this->SlicerTinyHelpIcon->Delete();
    this->SlicerTinyHelpIcon = NULL;    
    }
  if ( this->SlicerWaitIcon)
    {
    this->SlicerWaitIcon->Delete();
    this->SlicerWaitIcon = NULL;    
    }
  if ( this->SlicerMagnifyIcon)
    {
    this->SlicerMagnifyIcon->Delete();
    this->SlicerMagnifyIcon = NULL;    
    }
  if ( this->SlicerMinifyIcon)    
    {
    this->SlicerMinifyIcon->Delete();
    this->SlicerMinifyIcon = NULL;    
    }
  if ( this->SlicerNextIcon)
    {
    this->SlicerNextIcon->Delete();
    this->SlicerNextIcon = NULL;    
    }
  if  (this->SlicerGoToEndIcon)
    {
    this->SlicerGoToEndIcon->Delete();
    this->SlicerGoToEndIcon = NULL;
    }
  if (this->SlicerGoToStartIcon )
    {
    this->SlicerGoToStartIcon->Delete();
    this->SlicerGoToStartIcon = NULL;
    }
  if ( this->SlicerPreviousIcon)
    {
    this->SlicerPreviousIcon->Delete();
    this->SlicerPreviousIcon = NULL;    
    }
  if ( this->SlicerUndoIcon)
    {
    this->SlicerUndoIcon->Delete();
    this->SlicerUndoIcon = NULL;    
    }
  if ( this->SlicerRedoIcon)
    {
    this->SlicerRedoIcon->Delete();
    this->SlicerRedoIcon = NULL;    
    }
  if ( this->SlicerUnlinkIcon)
    {
    this->SlicerUnlinkIcon->Delete();
    this->SlicerUnlinkIcon = NULL;    
    }
  if ( this->SlicerLinkIcon)    
    {
    this->SlicerLinkIcon->Delete();
    this->SlicerLinkIcon = NULL;    
    }

  if ( this->SlicerCheckedVisibleIcon)
    {
    this->SlicerCheckedVisibleIcon->Delete();
    this->SlicerCheckedVisibleIcon = NULL;    
    }
  if ( this->SlicerVisibleNoFrameIcon)
    {
    this->SlicerVisibleNoFrameIcon->Delete();
    this->SlicerVisibleNoFrameIcon = NULL;
    }
  if ( this->SlicerInvisibleNoFrameIcon )
    {
    this->SlicerInvisibleNoFrameIcon->Delete();
    this->SlicerInvisibleNoFrameIcon = NULL;
    }
  if ( this->SlicerVisibleIcon)
    {
    this->SlicerVisibleIcon->Delete();
    this->SlicerVisibleIcon = NULL;    
    }
  if ( this->SlicerInvisibleIcon)
    {
    this->SlicerInvisibleIcon->Delete();
    this->SlicerInvisibleIcon = NULL;    
    }
  if ( this->SlicerRefreshIcon)
    {
    this->SlicerRefreshIcon->Delete();
    this->SlicerRefreshIcon = NULL;    
    }
  if ( this->SlicerVolumeIcon)
    {
    this->SlicerVolumeIcon->Delete();
    this->SlicerVolumeIcon = NULL;    
    }
  if ( this->SlicerCenterOnFiducialIcon )
    {
    this->SlicerCenterOnFiducialIcon->Delete();
    this->SlicerCenterOnFiducialIcon = NULL;
    }
  if ( this->SlicerFiducialsAddNewIcon )
    {
    this->SlicerFiducialsAddNewIcon->Delete();
    this->SlicerFiducialsAddNewIcon = NULL;
    }
  if ( this->SlicerFiducialsDeleteAllIcon )
    {
    this->SlicerFiducialsDeleteAllIcon->Delete();
    this->SlicerFiducialsDeleteAllIcon = NULL;
    }
  if ( this->SlicerFiducialsDeleteLastClickedIcon )
    {
    this->SlicerFiducialsDeleteLastClickedIcon->Delete();
    this->SlicerFiducialsDeleteLastClickedIcon = NULL;
    }
  if ( this->SlicerFiducialsSelectAllIcon )
    {
    this->SlicerFiducialsSelectAllIcon->Delete();
    this->SlicerFiducialsSelectAllIcon = NULL;
    }
  if ( this->SlicerFiducialsSelectNoneIcon )
    {
    this->SlicerFiducialsSelectNoneIcon->Delete();
    this->SlicerFiducialsSelectNoneIcon = NULL;
    }
  if ( this->SlicerFiducialsSelectAllInListIcon )
    {
    this->SlicerFiducialsSelectAllInListIcon->Delete();
    this->SlicerFiducialsSelectAllInListIcon = NULL;
    }
  if ( this->SlicerFiducialsSelectNoneInListIcon )
    {
    this->SlicerFiducialsSelectNoneInListIcon->Delete();
    this->SlicerFiducialsSelectNoneInListIcon = NULL;
    }
  if ( this->SlicerFiducialsDownIcon)
    {
    this->SlicerFiducialsDownIcon->Delete();
    this->SlicerFiducialsDownIcon = NULL;    
    }
  if ( this->SlicerFiducialsUpIcon)
    {
    this->SlicerFiducialsUpIcon->Delete();
    this->SlicerFiducialsUpIcon = NULL;    
    }
  if ( this->SlicerFiducialsDeleteListIcon )
    {
    this->SlicerFiducialsDeleteListIcon->Delete();
    this->SlicerFiducialsDeleteListIcon = NULL;
    }
  if ( this->SlicerFiducialsLockListIcon )
    {
    this->SlicerFiducialsLockListIcon->Delete();
    this->SlicerFiducialsLockListIcon = NULL;
    }
  if ( this->SlicerFiducialsUnlockListIcon )
    {
    this->SlicerFiducialsUnlockListIcon->Delete();
    this->SlicerFiducialsUnlockListIcon = NULL;
    }
  if ( this->SlicerFiducialsHideListIcon )
    {
    this->SlicerFiducialsHideListIcon->Delete();
    this->SlicerFiducialsHideListIcon = NULL;
    }
  if ( this->SlicerFiducialsExposeListIcon )
    {
    this->SlicerFiducialsExposeListIcon->Delete();
    this->SlicerFiducialsExposeListIcon = NULL;
    }
  if ( this->SlicerFiducialsHideExposeAllListsIcon )  
    {
    this->SlicerFiducialsHideExposeAllListsIcon->Delete();
    this->SlicerFiducialsHideExposeAllListsIcon = NULL;    
    }
  if ( this->SlicerFiducialsHideAllListsIcon )  
    {
    this->SlicerFiducialsHideAllListsIcon->Delete();
    this->SlicerFiducialsHideAllListsIcon = NULL;    
    }
  if ( this->SlicerFiducialsExposeAllListsIcon )  
    {
    this->SlicerFiducialsExposeAllListsIcon->Delete();
    this->SlicerFiducialsExposeAllListsIcon = NULL;    
    }
  if ( this->SlicerCompositeIcon )
    {
    this->SlicerCompositeIcon->Delete();
    this->SlicerCompositeIcon = NULL;
    }
  if ( this->SlicerLockIcon)
    {
    this->SlicerLockIcon->Delete();
    this->SlicerLockIcon = NULL;
    }
  if ( this->SlicerUnlockIcon)
    {
    this->SlicerUnlockIcon->Delete();
    this->SlicerUnlockIcon = NULL;
    }
  if ( this->SlicerFoundOnDiskIcon )
    {
    this->SlicerFoundOnDiskIcon->Delete();
    this->SlicerFoundOnDiskIcon = NULL;    
    }
  if ( this->SlicerNotFoundOnDiskIcon )
    {
    this->SlicerNotFoundOnDiskIcon->Delete();
    this->SlicerNotFoundOnDiskIcon = NULL;    
    }
  if (SlicerImportSceneIcon)
    {
    this->SlicerImportSceneIcon->Delete();
    this->SlicerImportSceneIcon = NULL;
    }
  if (SlicerLoadSceneIcon)
    {
    this->SlicerLoadSceneIcon->Delete();
    this->SlicerLoadSceneIcon = NULL;
    }
  if (SlicerLoadDicomVolumeIcon)
    {
    this->SlicerLoadDicomVolumeIcon->Delete();
    this->SlicerLoadDicomVolumeIcon = NULL;
    }
  if (SlicerLoadVolumeIcon)
    {
    this->SlicerLoadVolumeIcon->Delete();
    this->SlicerLoadVolumeIcon = NULL;
    }
  if (SlicerLoadDirectoryIcon)
    {
    this->SlicerLoadDirectoryIcon->Delete();
    this->SlicerLoadDirectoryIcon = NULL;
    }
  if (SlicerLoadFiducialsIcon)
    {
    this->SlicerLoadFiducialsIcon->Delete();
    this->SlicerLoadFiducialsIcon = NULL;
    }
  if (SlicerLoadModelIcon)
    {
    this->SlicerLoadModelIcon->Delete();
    this->SlicerLoadModelIcon = NULL;
    }
  if (SlicerLoadTransformIcon)
    {
    this->SlicerLoadTransformIcon->Delete();
    this->SlicerLoadTransformIcon = NULL;
    }
  if ( SlicerCloseIcon )
    {
    this->SlicerCloseIcon->Delete();
    this->SlicerCloseIcon = NULL;
    }
  if ( SlicerLoadColorLUTIcon )
    {
    this->SlicerLoadColorLUTIcon->Delete();
    this->SlicerLoadColorLUTIcon = NULL;
    }
  if ( SlicerLoadFiberBundleIcon )
    {
    this->SlicerLoadFiberBundleIcon->Delete();
    this->SlicerLoadFiberBundleIcon = NULL;    
    }
  if ( SlicerLoadScalarOverlayIcon )
    {
    this->SlicerLoadScalarOverlayIcon->Delete();
    this->SlicerLoadScalarOverlayIcon = NULL;    
    }
  if (this->SlicerExtensionsIcon )
    {
    this->SlicerExtensionsIcon->Delete();
    this->SlicerExtensionsIcon = NULL;
    }
  if ( this->SlicerWWWIcon )
    {
    this->SlicerWWWIcon->Delete();
    this->SlicerWWWIcon = NULL;
    }

  if ( this->SlicerAnnotationRulerEndpoint1Icon)
    {
    this->SlicerAnnotationRulerEndpoint1Icon->Delete();
    this->SlicerAnnotationRulerEndpoint1Icon = NULL;    
    }
  if ( this->SlicerAnnotationRulerEndpoint2Icon)
    {
    this->SlicerAnnotationRulerEndpoint2Icon->Delete();
    this->SlicerAnnotationRulerEndpoint2Icon = NULL;    
    }
  if ( this->SlicerAnnotationAngleEndpoint1Icon)
    {
    this->SlicerAnnotationAngleEndpoint1Icon->Delete();
    this->SlicerAnnotationAngleEndpoint1Icon = NULL;    
    }
  if ( this->SlicerAnnotationAngleEndpoint2Icon)
    {
    this->SlicerAnnotationAngleEndpoint2Icon->Delete();
    this->SlicerAnnotationAngleEndpoint2Icon = NULL;    
    }
  if ( this->SlicerAnnotationAngleCenterpointIcon)
    { 
    this->SlicerAnnotationAngleCenterpointIcon->Delete();
    this->SlicerAnnotationAngleCenterpointIcon = NULL;    
    }
  if (this->SlicerRotateToPixelSpaceIcon )
    {
    this->SlicerRotateToPixelSpaceIcon->Delete();
    this->SlicerRotateToPixelSpaceIcon = NULL;    
    }
}


//---------------------------------------------------------------------------
void vtkSlicerFoundationIcons::AssignImageDataToIcons ( )
{

  this->SlicerLockOrUnlockIcon->SetImage ( image_SlicerLockOrUnlock,
                                           image_SlicerLockOrUnlock_width,
                                           image_SlicerLockOrUnlock_height,
                                           image_SlicerLockOrUnlock_pixel_size,
                                           image_SlicerLockOrUnlock_length, 0 );
  this->SlicerVisibleOrInvisibleIcon->SetImage (image_SlicerVisibleOrInvisible,
                                                image_SlicerVisibleOrInvisible_width,
                                                image_SlicerVisibleOrInvisible_height,
                                                image_SlicerVisibleOrInvisible_pixel_size,
                                                image_SlicerVisibleOrInvisible_length, 0);
  this->SlicerSelectAllIcon->SetImage (image_SlicerSelectAll,
                                       image_SlicerSelectAll_width,
                                       image_SlicerSelectAll_height,
                                       image_SlicerSelectAll_pixel_size,
                                       image_SlicerSelectAll_length, 0);
  this->SlicerDeselectAllIcon->SetImage (image_SlicerDeselectAll,
                                       image_SlicerDeselectAll_width,
                                       image_SlicerDeselectAll_height,
                                       image_SlicerDeselectAll_pixel_size,
                                       image_SlicerDeselectAll_length, 0);

  this->SlicerTableIcon->SetImage ( image_SlicerTable,
                                    image_SlicerTable_width,
                                    image_SlicerTable_height,
                                    image_SlicerTable_pixel_size,
                                    image_SlicerTable_length, 0);
  this->SlicerMoreOptionsIcon->SetImage (image_SlicerMoreOptions,
                                         image_SlicerMoreOptions_width,
                                         image_SlicerMoreOptions_height,
                                         image_SlicerMoreOptions_pixel_size,
                                         image_SlicerMoreOptions_length, 0);
  this->SlicerGoIcon->SetImage ( image_SlicerGo,
                           image_SlicerGo_width,
                           image_SlicerGo_height,
                           image_SlicerGo_pixel_size,
                           image_SlicerGo_length, 0);
  this->SlicerGoing0Icon->SetImage ( image_SlicerGoing0,
                           image_SlicerGoing0_width,
                           image_SlicerGoing0_height,
                           image_SlicerGoing0_pixel_size,
                           image_SlicerGoing0_length, 0);
  this->SlicerGoing1Icon->SetImage ( image_SlicerGoing1,
                           image_SlicerGoing1_width,
                           image_SlicerGoing1_height,
                           image_SlicerGoing1_pixel_size,
                           image_SlicerGoing1_length, 0);
  this->SlicerGoing2Icon->SetImage ( image_SlicerGoing2,
                           image_SlicerGoing2_width,
                           image_SlicerGoing2_height,
                           image_SlicerGoing2_pixel_size,
                           image_SlicerGoing2_length, 0);
  this->SlicerGoing3Icon->SetImage ( image_SlicerGoing3,
                           image_SlicerGoing3_width,
                           image_SlicerGoing3_height,
                           image_SlicerGoing3_pixel_size,
                           image_SlicerGoing3_length, 0);
  this->SlicerGoing4Icon->SetImage ( image_SlicerGoing4,
                           image_SlicerGoing4_width,
                           image_SlicerGoing4_height,
                           image_SlicerGoing4_pixel_size,
                           image_SlicerGoing4_length, 0);
  this->SlicerGoing5Icon->SetImage ( image_SlicerGoing5,
                           image_SlicerGoing5_width,
                           image_SlicerGoing5_height,
                           image_SlicerGoing5_pixel_size,
                           image_SlicerGoing5_length, 0);
  this->SlicerGoing6Icon->SetImage ( image_SlicerGoing6,
                           image_SlicerGoing6_width,
                           image_SlicerGoing6_height,
                           image_SlicerGoing6_pixel_size,
                           image_SlicerGoing6_length, 0);
  this->SlicerGoing7Icon->SetImage ( image_SlicerGoing7,
                           image_SlicerGoing7_width,
                           image_SlicerGoing7_height,
                           image_SlicerGoing7_pixel_size,
                           image_SlicerGoing7_length, 0);
  this->SlicerCameraIcon->SetImage ( image_SlicerCamera,
                               image_SlicerCamera_width,
                               image_SlicerCamera_height,
                               image_SlicerCamera_pixel_size,
                               image_SlicerCamera_length, 0);                               
  this->SlicerBlankIcon->SetImage ( image_SlicerBlank,
                                    image_SlicerBlank_width,
                                    image_SlicerBlank_height,
                                    image_SlicerBlank_pixel_size,
                                    image_SlicerBlank_length, 0);                                    
  this->SlicerCancelIcon->SetImage ( image_SlicerCancel,
                                     image_SlicerCancel_width,
                                     image_SlicerCancel_height,
                                     image_SlicerCancel_pixel_size,
                                     image_SlicerCancel_length, 0);
  this->SlicerCancelDisabledIcon->SetImage ( image_SlicerCancelDisabled,
                                             image_SlicerCancelDisabled_width,
                                             image_SlicerCancelDisabled_height,
                                             image_SlicerCancelDisabled_pixel_size,
                                             image_SlicerCancelDisabled_length, 0);
  this->SlicerCancelledIcon->SetImage ( image_SlicerCancelled,
                                        image_SlicerCancelled_width,
                                        image_SlicerCancelled_height,
                                        image_SlicerCancelled_pixel_size,
                                        image_SlicerCancelled_length, 0);
  this->SlicerCancelRequestedIcon->SetImage ( image_SlicerCancelRequested,
                                              image_SlicerCancelRequested_width,
                                              image_SlicerCancelRequested_height,
                                              image_SlicerCancelRequested_pixel_size,
                                              image_SlicerCancelRequested_length, 0);
  this->SlicerCleanUpIcon->SetImage ( image_SlicerCleanUp,
                                      image_SlicerCleanUp_width,
                                      image_SlicerCleanUp_height,
                                      image_SlicerCleanUp_pixel_size,
                                      image_SlicerCleanUp_length, 0);
  this->SlicerColorsIcon->SetImage ( image_SlicerColors,
                                     image_SlicerColors_width,
                                     image_SlicerColors_height,
                                     image_SlicerColors_pixel_size,
                                     image_SlicerColors_length, 0);
  this->SlicerPlayerCycleIcon->SetImage ( image_SlicerCycle,
                                          image_SlicerCycle_width,
                                          image_SlicerCycle_height,
                                          image_SlicerCycle_pixel_size,
                                          image_SlicerCycle_length, 0);
  this->SlicerDecrementIcon->SetImage ( image_SlicerDecrement,
                                        image_SlicerDecrement_width,
                                        image_SlicerDecrement_height,
                                        image_SlicerDecrement_pixel_size,
                                        image_SlicerDecrement_length, 0);
  this->SlicerDeleteIcon->SetImage ( image_SlicerDelete,
                                     image_SlicerDelete_width,
                                     image_SlicerDelete_height,
                                     image_SlicerDelete_pixel_size,
                                     image_SlicerDelete_length, 0);
  this->SlicerDeleteDisabledIcon->SetImage ( image_SlicerDeleteDisabled,
                                             image_SlicerDeleteDisabled_width,
                                             image_SlicerDeleteDisabled_height,
                                             image_SlicerDeleteDisabled_pixel_size,
                                             image_SlicerDeleteDisabled_length, 0);
  this->SlicerDoneIcon->SetImage ( image_SlicerDone,
                                   image_SlicerDone_width,
                                   image_SlicerDone_height,
                                   image_SlicerDone_pixel_size,
                                   image_SlicerDone_length, 0);
  this->SlicerErrorIcon->SetImage ( image_SlicerError,
                                    image_SlicerError_width,
                                    image_SlicerError_height,
                                    image_SlicerError_pixel_size,
                                    image_SlicerError_length, 0);
  this->SlicerGlyphIcon->SetImage ( image_SlicerGlyphType,
                                    image_SlicerGlyphType_width,
                                    image_SlicerGlyphType_height,
                                    image_SlicerGlyphType_pixel_size,
                                    image_SlicerGlyphType_length, 0);
  this->SlicerPlayerGoToFirstIcon->SetImage ( image_SlicerGoToFirst,
                                              image_SlicerGoToFirst_width,
                                              image_SlicerGoToFirst_height,
                                              image_SlicerGoToFirst_pixel_size,
                                              image_SlicerGoToFirst_length, 0);
  this->SlicerGoToStartIcon->SetImage (image_SlicerGoToStart,
                               image_SlicerGoToStart_width,
                               image_SlicerGoToStart_height,
                               image_SlicerGoToStart_pixel_size,
                               image_SlicerGoToStart_length, 0);
  this->SlicerGoToEndIcon->SetImage ( image_SlicerGoToEnd,
                                      image_SlicerGoToEnd_width,
                                      image_SlicerGoToEnd_height,
                                      image_SlicerGoToEnd_pixel_size,
                                      image_SlicerGoToEnd_length, 0);
  this->SlicerPlayerGoToLastIcon->SetImage ( image_SlicerGoToLast,
                                             image_SlicerGoToLast_width,
                                             image_SlicerGoToLast_height,
                                             image_SlicerGoToLast_pixel_size,
                                             image_SlicerGoToLast_length, 0);
  this->SlicerIncrementIcon->SetImage ( image_SlicerIncrement,
                                        image_SlicerIncrement_width,
                                        image_SlicerIncrement_height,
                                        image_SlicerIncrement_pixel_size,
                                        image_SlicerIncrement_length, 0);
  this->SlicerInformationIcon->SetImage ( image_SlicerInformation,
                                          image_SlicerInformation_width,
                                          image_SlicerInformation_height,
                                          image_SlicerInformation_pixel_size,
                                          image_SlicerInformation_length, 0);
  this->SlicerDownloadIcon->SetImage ( image_SlicerDownload,
                                       image_SlicerDownload_width,
                                       image_SlicerDownload_height,
                                       image_SlicerDownload_pixel_size,
                                       image_SlicerDownload_length, 0);
  this->SlicerUploadIcon->SetImage ( image_SlicerUpload,
                                     image_SlicerUpload_width,
                                     image_SlicerUpload_height,
                                     image_SlicerUpload_pixel_size,
                                     image_SlicerUpload_length, 0);
  this->SlicerLoadIcon->SetImage ( image_SlicerLoad,
                                   image_SlicerLoad_width,
                                   image_SlicerLoad_height,
                                   image_SlicerLoad_pixel_size,
                                   image_SlicerLoad_length, 0);
  this->SlicerPlayerPauseIcon->SetImage ( image_SlicerPause,
                                          image_SlicerPause_width,
                                          image_SlicerPause_height,
                                          image_SlicerPause_pixel_size,
                                          image_SlicerPause_length, 0);
  this->SlicerPlayerPingPongIcon->SetImage ( image_SlicerPingPong,
                                             image_SlicerPingPong_width,
                                             image_SlicerPingPong_height,
                                             image_SlicerPingPong_pixel_size,
                                             image_SlicerPingPong_length, 0);
  this->SlicerPlayerBackwardIcon->SetImage ( image_SlicerPlayBackward,
                                             image_SlicerPlayBackward_width,
                                             image_SlicerPlayBackward_height,
                                             image_SlicerPlayBackward_pixel_size,
                                             image_SlicerPlayBackward_length, 0);
  this->SlicerPlayerForwardIcon->SetImage ( image_SlicerPlayForward,
                                            image_SlicerPlayForward_width,
                                            image_SlicerPlayForward_height,
                                            image_SlicerPlayForward_pixel_size,
                                            image_SlicerPlayForward_length, 0);
  this->SlicerPreparingIcon->SetImage ( image_SlicerPreparing,
                                        image_SlicerPreparing_width,
                                        image_SlicerPreparing_height,
                                        image_SlicerPreparing_pixel_size,
                                        image_SlicerPreparing_length, 0);
  this->SlicerPlayerRecordIcon->SetImage ( image_SlicerRecord,
                                           image_SlicerRecord_width,
                                           image_SlicerRecord_height,
                                           image_SlicerRecord_pixel_size,
                                           image_SlicerRecord_length, 0);
  this->SlicerSaveIcon->SetImage ( image_SlicerSave,
                                   image_SlicerSave_width,
                                   image_SlicerSave_height,
                                   image_SlicerSave_pixel_size,
                                   image_SlicerSave_length, 0);
  this->SlicerPlayerStopRecordingIcon->SetImage ( image_SlicerStopRecording,
                                                  image_SlicerStopRecording_width,
                                                  image_SlicerStopRecording_height,
                                                  image_SlicerStopRecording_pixel_size,
                                                  image_SlicerStopRecording_length, 0);
  this->SlicerTimedOutIcon->SetImage ( image_SlicerTimedOut,
                                       image_SlicerTimedOut_width,
                                       image_SlicerTimedOut_height,
                                       image_SlicerTimedOut_pixel_size,
                                       image_SlicerTimedOut_length, 0);
  this->SlicerTinyHelpIcon->SetImage ( image_SlicerTinyHelp,
                                       image_SlicerTinyHelp_width,
                                       image_SlicerTinyHelp_height,
                                       image_SlicerTinyHelp_pixel_size,
                                       image_SlicerTinyHelp_length, 0);
  this->SlicerWaitIcon->SetImage ( image_SlicerWait,
                                   image_SlicerWait_width,
                                   image_SlicerWait_height,
                                   image_SlicerWait_pixel_size,
                                   image_SlicerWait_length, 0);
  this->SlicerMagnifyIcon->SetImage ( image_SlicerZoomIn,
                                      image_SlicerZoomIn_width,
                                      image_SlicerZoomIn_height,
                                      image_SlicerZoomIn_pixel_size,
                                      image_SlicerZoomIn_length, 0);
  this->SlicerMinifyIcon->SetImage ( image_SlicerZoomOut,
                                     image_SlicerZoomOut_width,
                                     image_SlicerZoomOut_height,
                                     image_SlicerZoomOut_pixel_size,
                                     image_SlicerZoomOut_length, 0);
  this->SlicerNextIcon->SetImage ( image_SlicerNext,
                                   image_SlicerNext_width,
                                   image_SlicerNext_height,
                                   image_SlicerNext_pixel_size,
                                   image_SlicerNext_length, 0);
  this->SlicerPreviousIcon->SetImage ( image_SlicerPrevious,
                                       image_SlicerPrevious_width,
                                       image_SlicerPrevious_height,
                                       image_SlicerPrevious_pixel_size,
                                       image_SlicerPrevious_length, 0);
  this->SlicerUndoIcon->SetImage ( image_SlicerUnDo,
                                   image_SlicerUnDo_width,
                                   image_SlicerUnDo_height,
                                   image_SlicerUnDo_pixel_size,
                                   image_SlicerUnDo_length, 0);
  this->SlicerRedoIcon->SetImage ( image_SlicerReDo,
                                   image_SlicerReDo_width,
                                   image_SlicerReDo_height,
                                   image_SlicerReDo_pixel_size,
                                   image_SlicerReDo_length, 0);
  this->SlicerUnlinkIcon->SetImage ( image_SlicerUnlink,
                                     image_SlicerUnlink_width,
                                     image_SlicerUnlink_height,
                                     image_SlicerUnlink_pixel_size,
                                     image_SlicerUnlink_length, 0);
  this->SlicerLinkIcon->SetImage ( image_SlicerLink,
                                   image_SlicerLink_width,
                                   image_SlicerLink_height,
                                   image_SlicerLink_pixel_size,
                                   image_SlicerLink_length, 0);
  this->SlicerVisibleNoFrameIcon->SetImage (image_SlicerVisibleNoFrame,
                                            image_SlicerVisibleNoFrame_width,
                                            image_SlicerVisibleNoFrame_height,
                                            image_SlicerVisibleNoFrame_pixel_size,
                                            image_SlicerVisibleNoFrame_length, 0);
  this->SlicerInvisibleNoFrameIcon->SetImage (image_SlicerInvisibleNoFrame,
                                            image_SlicerInvisibleNoFrame_width,
                                            image_SlicerInvisibleNoFrame_height,
                                            image_SlicerInvisibleNoFrame_pixel_size,
                                            image_SlicerInvisibleNoFrame_length, 0);
  this->SlicerVisibleIcon->SetImage ( image_SlicerVisible,
                                      image_SlicerVisible_width,
                                      image_SlicerVisible_height,
                                      image_SlicerVisible_pixel_size,
                                      image_SlicerVisible_length, 0);
  this->SlicerCheckedVisibleIcon->SetImage ( image_SlicerCheckedVisible,
                                      image_SlicerCheckedVisible_width,
                                      image_SlicerCheckedVisible_height,
                                      image_SlicerCheckedVisible_pixel_size,
                                      image_SlicerCheckedVisible_length, 0);
  this->SlicerInvisibleIcon->SetImage ( image_SlicerInvisible,
                                        image_SlicerInvisible_width,
                                        image_SlicerInvisible_height,
                                        image_SlicerInvisible_pixel_size,
                                        image_SlicerInvisible_length, 0);
  this->SlicerRefreshIcon->SetImage ( image_SlicerRefresh,
                                      image_SlicerRefresh_width,
                                      image_SlicerRefresh_height,
                                      image_SlicerRefresh_pixel_size,
                                      image_SlicerRefresh_length, 0);
  this->SlicerVolumeIcon->SetImage ( image_SlicerVolume,
                                     image_SlicerVolume_width,
                                     image_SlicerVolume_height,
                                     image_SlicerVolume_pixel_size,
                                     image_SlicerVolume_length, 0);
  this->SlicerCenterOnFiducialIcon->SetImage (image_SlicerCenterOnFiducial,
                                              image_SlicerCenterOnFiducial_width,
                                              image_SlicerCenterOnFiducial_height,
                                              image_SlicerCenterOnFiducial_pixel_size,
                                              image_SlicerCenterOnFiducial_length, 0);
  this->SlicerFiducialsAddNewIcon->SetImage (image_SlicerAddFiducial,
                                             image_SlicerAddFiducial_width,
                                             image_SlicerAddFiducial_height,
                                             image_SlicerAddFiducial_pixel_size,
                                             image_SlicerAddFiducial_length, 0);
  this->SlicerFiducialsDeleteAllIcon->SetImage(image_SlicerFiducialListsDeleteAll,
                                               image_SlicerFiducialListsDeleteAll_width,
                                               image_SlicerFiducialListsDeleteAll_height,
                                               image_SlicerFiducialListsDeleteAll_pixel_size,
                                               image_SlicerFiducialListsDeleteAll_length, 0);
  this->SlicerFiducialsDeleteLastClickedIcon->SetImage (image_SlicerFiducialsDeleteLastClicked,
                                                        image_SlicerFiducialsDeleteLastClicked_width,
                                                        image_SlicerFiducialsDeleteLastClicked_height,
                                                        image_SlicerFiducialsDeleteLastClicked_pixel_size,
                                                        image_SlicerFiducialsDeleteLastClicked_length, 0);
  this->SlicerFiducialsSelectAllIcon->SetImage ( image_SlicerFiducialListsSelectAll,
                                                 image_SlicerFiducialListsSelectAll_width,
                                                 image_SlicerFiducialListsSelectAll_height,
                                                 image_SlicerFiducialListsSelectAll_pixel_size,
                                                 image_SlicerFiducialListsSelectAll_length, 0);
  this->SlicerFiducialsSelectNoneIcon->SetImage (image_SlicerFiducialListsSelectNone,
                                                 image_SlicerFiducialListsSelectNone_width,
                                                 image_SlicerFiducialListsSelectNone_height,
                                                 image_SlicerFiducialListsSelectNone_pixel_size,
                                                 image_SlicerFiducialListsSelectNone_length, 0);
  this->SlicerFiducialsSelectAllInListIcon->SetImage ( image_SlicerFiducialsSelectAllInList,
                                                 image_SlicerFiducialsSelectAllInList_width,
                                                 image_SlicerFiducialsSelectAllInList_height,
                                                 image_SlicerFiducialsSelectAllInList_pixel_size,
                                                 image_SlicerFiducialsSelectAllInList_length, 0);
  this->SlicerFiducialsDeleteAllInListIcon->SetImage ( image_SlicerFiducialsDeleteAll,
                                                 image_SlicerFiducialsDeleteAll_width,
                                                 image_SlicerFiducialsDeleteAll_height,
                                                 image_SlicerFiducialsDeleteAll_pixel_size,
                                                 image_SlicerFiducialsDeleteAll_length, 0);
  this->SlicerFiducialsSelectNoneInListIcon->SetImage (image_SlicerFiducialsSelectNoneInList,
                                                 image_SlicerFiducialsSelectNoneInList_width,
                                                 image_SlicerFiducialsSelectNoneInList_height,
                                                 image_SlicerFiducialsSelectNoneInList_pixel_size,
                                                 image_SlicerFiducialsSelectNoneInList_length, 0);
  this->SlicerFiducialsDownIcon->SetImage ( image_SlicerFiducialsDown,
                                   image_SlicerFiducialsDown_width,
                                   image_SlicerFiducialsDown_height,
                                   image_SlicerFiducialsDown_pixel_size,
                                   image_SlicerFiducialsDown_length, 0);
  this->SlicerFiducialsUpIcon->SetImage ( image_SlicerFiducialsUp,
                                   image_SlicerFiducialsUp_width,
                                   image_SlicerFiducialsUp_height,
                                   image_SlicerFiducialsUp_pixel_size,
                                   image_SlicerFiducialsUp_length, 0);
  this->SlicerFiducialsDeleteListIcon->SetImage ( image_SlicerDeleteFiducialList,
                                                  image_SlicerDeleteFiducialList_width,
                                                  image_SlicerDeleteFiducialList_height,
                                                  image_SlicerDeleteFiducialList_pixel_size,
                                                  image_SlicerDeleteFiducialList_length, 0);
  this->SlicerFiducialsLockListIcon->SetImage ( image_SlicerLockFiducialList,
                                               image_SlicerLockFiducialList_width,
                                               image_SlicerLockFiducialList_height,
                                               image_SlicerLockFiducialList_pixel_size,
                                               image_SlicerLockFiducialList_length, 0);
  this->SlicerFiducialsUnlockListIcon->SetImage (image_SlicerUnlockFiducialList,
                                                image_SlicerUnlockFiducialList_width,
                                                image_SlicerUnlockFiducialList_height,
                                                image_SlicerUnlockFiducialList_pixel_size,
                                                image_SlicerUnlockFiducialList_length, 0);
  this->SlicerFiducialsHideListIcon->SetImage (image_SlicerHideFiducialList,
                                              image_SlicerHideFiducialList_width,
                                              image_SlicerHideFiducialList_height,
                                              image_SlicerHideFiducialList_pixel_size,
                                              image_SlicerHideFiducialList_length, 0);
  this->SlicerFiducialsExposeListIcon->SetImage (image_SlicerExposeFiducialList,
                                                image_SlicerExposeFiducialList_width,
                                                image_SlicerExposeFiducialList_height,
                                                image_SlicerExposeFiducialList_pixel_size,
                                                image_SlicerExposeFiducialList_length, 0);
  this->SlicerFiducialsHideExposeAllListsIcon->SetImage ( image_SlicerHideExposeAllFiducialLists,
                                               image_SlicerHideExposeAllFiducialLists_width,
                                               image_SlicerHideExposeAllFiducialLists_height,
                                               image_SlicerHideExposeAllFiducialLists_pixel_size,
                                               image_SlicerHideExposeAllFiducialLists_length, 0);
  this->SlicerFiducialsHideAllListsIcon->SetImage ( image_SlicerHideAllFiducialLists,
                                               image_SlicerHideAllFiducialLists_width,
                                               image_SlicerHideAllFiducialLists_height,
                                               image_SlicerHideAllFiducialLists_pixel_size,
                                               image_SlicerHideAllFiducialLists_length, 0);
  this->SlicerFiducialsExposeAllListsIcon->SetImage ( image_SlicerExposeAllFiducialLists,
                                               image_SlicerExposeAllFiducialLists_width,
                                               image_SlicerExposeAllFiducialLists_height,
                                               image_SlicerExposeAllFiducialLists_pixel_size,
                                               image_SlicerExposeAllFiducialLists_length, 0);
  this->SlicerCompositeIcon->SetImage ( image_SlicerComposite,
                                        image_SlicerComposite_width,
                                        image_SlicerComposite_height,
                                        image_SlicerComposite_pixel_size,
                                        image_SlicerComposite_length, 0);
  this->SlicerLockIcon->SetImage (image_SlicerLock,
                                  image_SlicerLock_width,
                                  image_SlicerLock_height,
                                  image_SlicerLock_pixel_size,
                                  image_SlicerLock_length, 0);
  this->SlicerUnlockIcon->SetImage (image_SlicerUnlock,
                                    image_SlicerUnlock_width,
                                    image_SlicerUnlock_height,
                                    image_SlicerUnlock_pixel_size,
                                    image_SlicerUnlock_length, 0);
  this->SlicerFoundOnDiskIcon->SetImage (image_SlicerFoundOnDisk,
                                         image_SlicerFoundOnDisk_width,
                                         image_SlicerFoundOnDisk_height,
                                         image_SlicerFoundOnDisk_pixel_size,
                                         image_SlicerFoundOnDisk_length, 0);
  this->SlicerNotFoundOnDiskIcon->SetImage (image_SlicerNotFoundOnDisk,
                                         image_SlicerNotFoundOnDisk_width,
                                         image_SlicerNotFoundOnDisk_height,
                                         image_SlicerNotFoundOnDisk_pixel_size,
                                         image_SlicerNotFoundOnDisk_length, 0);
  this->SlicerImportSceneIcon->SetImage (image_SlicerImportScene,
                                         image_SlicerImportScene_width,
                                         image_SlicerImportScene_height,
                                         image_SlicerImportScene_pixel_size,
                                         image_SlicerImportScene_length, 0);
  this->SlicerLoadSceneIcon->SetImage (image_SlicerLoadScene,
                                       image_SlicerLoadScene_width,
                                       image_SlicerLoadScene_height,
                                       image_SlicerLoadScene_pixel_size,
                                       image_SlicerLoadScene_length, 0);
  this->SlicerLoadDicomVolumeIcon->SetImage (image_SlicerLoadDICOMVolume,
                                             image_SlicerLoadDICOMVolume_width,
                                             image_SlicerLoadDICOMVolume_height,
                                             image_SlicerLoadDICOMVolume_pixel_size,
                                             image_SlicerLoadDICOMVolume_length, 0);
  this->SlicerLoadVolumeIcon->SetImage (image_SlicerLoadVolume,
                                        image_SlicerLoadVolume_width,
                                        image_SlicerLoadVolume_height,
                                        image_SlicerLoadVolume_pixel_size,
                                        image_SlicerLoadVolume_length, 0);
  this->SlicerLoadDirectoryIcon->SetImage (image_SlicerLoadDirectory,
                                           image_SlicerLoadDirectory_width,
                                           image_SlicerLoadDirectory_height,
                                           image_SlicerLoadDirectory_pixel_size,
                                           image_SlicerLoadDirectory_length, 0);
  this->SlicerLoadFiducialsIcon->SetImage (image_SlicerLoadFiducials,
                                           image_SlicerLoadFiducials_width,
                                           image_SlicerLoadFiducials_height,
                                           image_SlicerLoadFiducials_pixel_size,
                                           image_SlicerLoadFiducials_length, 0);
  this->SlicerLoadModelIcon->SetImage (image_SlicerLoadModel,
                                       image_SlicerLoadModel_width,
                                       image_SlicerLoadModel_height,
                                       image_SlicerLoadModel_pixel_size,
                                       image_SlicerLoadModel_length, 0);
  this->SlicerLoadTransformIcon->SetImage (image_SlicerLoadTransform,
                                       image_SlicerLoadTransform_width,
                                       image_SlicerLoadTransform_height,
                                       image_SlicerLoadTransform_pixel_size,
                                       image_SlicerLoadTransform_length, 0);
  this->SlicerLoadColorLUTIcon->SetImage ( image_SlicerLoadColorLUT,
                                           image_SlicerLoadColorLUT_width,
                                           image_SlicerLoadColorLUT_height,
                                           image_SlicerLoadColorLUT_pixel_size,
                                           image_SlicerLoadColorLUT_length, 0);
  this->SlicerLoadFiberBundleIcon->SetImage ( image_SlicerLoadFiberBundle,
                                              image_SlicerLoadFiberBundle_width,
                                              image_SlicerLoadFiberBundle_height,
                                              image_SlicerLoadFiberBundle_pixel_size,
                                              image_SlicerLoadFiberBundle_length, 0);
  this->SlicerLoadScalarOverlayIcon->SetImage ( image_SlicerLoadScalarOverlay,
                                                image_SlicerLoadScalarOverlay_width,
                                                image_SlicerLoadScalarOverlay_height,
                                                image_SlicerLoadScalarOverlay_pixel_size,
                                                image_SlicerLoadScalarOverlay_length, 0);
  this->SlicerCloseIcon->SetImage (image_SlicerClose,
                                   image_SlicerClose_width,
                                   image_SlicerClose_height,
                                   image_SlicerClose_pixel_size,
                                   image_SlicerClose_length, 0);
  this->SlicerExtensionsIcon->SetImage (image_SlicerExtensions,
                                        image_SlicerExtensions_width,
                                        image_SlicerExtensions_height,
                                        image_SlicerExtensions_pixel_size,
                                        image_SlicerExtensions_length, 0);
  this->SlicerWWWIcon->SetImage (image_SlicerWWW,
                                 image_SlicerWWW_width,
                                 image_SlicerWWW_height,
                                 image_SlicerWWW_pixel_size,
                                 image_SlicerWWW_length, 0);
  this->SlicerAnnotationRulerEndpoint1Icon->SetImage ( image_AnnotationRulerEndpoint1,
                                                   image_AnnotationRulerEndpoint1_width,
                                                   image_AnnotationRulerEndpoint1_height,
                                                   image_AnnotationRulerEndpoint1_pixel_size,
                                                   image_AnnotationRulerEndpoint1_length, 0);
  this->SlicerAnnotationRulerEndpoint2Icon->SetImage ( image_AnnotationRulerEndpoint2,
                                                   image_AnnotationRulerEndpoint2_width,
                                                   image_AnnotationRulerEndpoint2_height,
                                                   image_AnnotationRulerEndpoint2_pixel_size,
                                                   image_AnnotationRulerEndpoint2_length, 0);
  this->SlicerAnnotationAngleEndpoint1Icon->SetImage ( image_AnnotationAngleEndpoint1,
                                                   image_AnnotationAngleEndpoint1_width,
                                                   image_AnnotationAngleEndpoint1_height,
                                                   image_AnnotationAngleEndpoint1_pixel_size,
                                                   image_AnnotationAngleEndpoint1_length, 0);
  this->SlicerAnnotationAngleEndpoint2Icon->SetImage (image_AnnotationAngleEndpoint2,
                                                  image_AnnotationAngleEndpoint2_width,
                                                  image_AnnotationAngleEndpoint2_height,
                                                  image_AnnotationAngleEndpoint2_pixel_size,
                                                  image_AnnotationAngleEndpoint2_length, 0);
  this->SlicerAnnotationAngleCenterpointIcon->SetImage (image_AnnotationAngleCenterPoint,
                                                    image_AnnotationAngleCenterPoint_width,
                                                    image_AnnotationAngleCenterPoint_height,
                                                    image_AnnotationAngleCenterPoint_pixel_size,
                                                    image_AnnotationAngleCenterPoint_length, 0);

  this->SlicerRotateToPixelSpaceIcon->SetImage (image_SlicerRotateToPixelSpace,
                                            image_SlicerRotateToPixelSpace_width,
                                            image_SlicerRotateToPixelSpace_height,
                                            image_SlicerRotateToPixelSpace_pixel_size,
                                            image_SlicerRotateToPixelSpace_length, 0);                                            
}



//---------------------------------------------------------------------------
void vtkSlicerFoundationIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerFoundationIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "SlicerSelectAllIcon: " << this->GetSlicerSelectAllIcon() << "\n";
    os << indent << "SlicerDeselectAllIcon: " << this->GetSlicerDeselectAllIcon() << "\n";
    os << indent << "SlicerTableIcon: " << this->GetSlicerTableIcon() << "\n";
    os << indent << "SlicerMoreOptionsIcon: " << this->GetSlicerMoreOptionsIcon() << "\n";
    os << indent << "SlicerGoIcon: " << this->GetSlicerGoIcon() << "\n";
    os << indent << "SlicerGoing0Icon: " << this->GetSlicerGoing0Icon() << "\n";
    os << indent << "SlicerGoing1Icon: " << this->GetSlicerGoing1Icon() << "\n";
    os << indent << "SlicerGoing2Icon: " << this->GetSlicerGoing2Icon() << "\n";
    os << indent << "SlicerGoing3Icon: " << this->GetSlicerGoing3Icon() << "\n";
    os << indent << "SlicerGoing4Icon: " << this->GetSlicerGoing4Icon() << "\n";
    os << indent << "SlicerGoing5Icon: " << this->GetSlicerGoing5Icon() << "\n";
    os << indent << "SlicerGoing6Icon: " << this->GetSlicerGoing6Icon() << "\n";
    os << indent << "SlicerGoing7Icon: " << this->GetSlicerGoing7Icon() << "\n";
    os << indent << "SlicerCameraIcon: " << this->GetSlicerCameraIcon() << "\n";
    os << indent << "SlicerBlankIcon: " << this->GetSlicerBlankIcon() << "\n";
    os << indent << "SlicerCancelIcon: " << this->GetSlicerCancelIcon() << "\n";
    os << indent << "SlicerCancelDisabledIcon: " << this->GetSlicerCancelDisabledIcon() << "\n";
    os << indent << "SlicerCancelledIcon: " << this->GetSlicerCancelledIcon() << "\n";
    os << indent << "SlicerCancelRequestedIcon: " << this->GetSlicerCancelRequestedIcon() << "\n";
    os << indent << "SlicerCleanUpIcon: " << this->GetSlicerCleanUpIcon() << "\n";
    os << indent << "SlicerColorsIcon: " << this->GetSlicerColorsIcon() << "\n";
    os << indent << "SlicerPlayerCycleIcon: " << this->GetSlicerPlayerCycleIcon() << "\n";
    os << indent << "SlicerDecrementIcon: " << this->GetSlicerDecrementIcon() << "\n";
    os << indent << "SlicerDeleteIcon: " << this->GetSlicerDeleteIcon() << "\n";
    os << indent << "SlicerDeleteDisabledIcon: " << this->GetSlicerDeleteDisabledIcon() << "\n";
    os << indent << "SlicerDoneIcon: " << this->GetSlicerDoneIcon() << "\n";
    os << indent << "SlicerErrorIcon: " << this->GetSlicerErrorIcon() << "\n";
    os << indent << "SlicerGlyphIcon: " << this->GetSlicerGlyphIcon() << "\n";
    os << indent << "SlicerPlayerGoToFirstIcon: " << this->GetSlicerPlayerGoToFirstIcon() << "\n";
    os << indent << "SlicerGoToEndIcon: " << this->GetSlicerPlayerGoToLastIcon() << "\n";
    os << indent << "SlicerGoToStartIcon: " << this->GetSlicerGoToStartIcon() << "\n";
    os << indent << "SlicerPlayerGoToLastIcon: " << this->GetSlicerGoToEndIcon() << "\n";
    os << indent << "SlicerIncrementIcon: " << this->GetSlicerIncrementIcon() << "\n";
    os << indent << "SlicerInformationIcon: " << this->GetSlicerInformationIcon() << "\n";
    os << indent << "SlicerDownloadIcon: " << this->GetSlicerDownloadIcon() << "\n";
    os << indent << "SlicerUploadIcon: " << this->GetSlicerUploadIcon() << "\n";
    os << indent << "SlicerLoadIcon: " << this->GetSlicerLoadIcon() << "\n";
    os << indent << "SlicerPlayerPauseIcon: " << this->GetSlicerPlayerPauseIcon() << "\n";
    os << indent << "SlicerPlayerPingPongIcon: " << this->GetSlicerPlayerPingPongIcon() << "\n";
    os << indent << "SlicerPlayerBackwardIcon: " << this->GetSlicerPlayerBackwardIcon() << "\n";
    os << indent << "SlicerPlayerForwardIcon: " << this->GetSlicerPlayerForwardIcon() << "\n";
    os << indent << "SlicerPreparingIcon: " << this->GetSlicerPreparingIcon() << "\n";
    os << indent << "SlicerPlayerRecordIcon: " << this->GetSlicerPlayerRecordIcon() << "\n";
    os << indent << "SlicerSaveIcon: " << this->GetSlicerSaveIcon() << "\n";
    os << indent << "SlicerPlayerStopRecordingIcon: " << this->GetSlicerPlayerStopRecordingIcon() << "\n";
    os << indent << "SlicerTimedOutIcon: " << this->GetSlicerTimedOutIcon() << "\n";
    os << indent << "SlicerTinyHelpIcon: " << this->GetSlicerTinyHelpIcon() << "\n";
    os << indent << "SlicerWaitIcon: " << this->GetSlicerWaitIcon() << "\n";
    os << indent << "SlicerMagnifyIcon: " << this->GetSlicerMagnifyIcon() << "\n";
    os << indent << "SlicerMinifyIcon: " << this->GetSlicerMinifyIcon() << "\n";
    os << indent << "SlicerNextIcon: " << this->GetSlicerNextIcon() << "\n";
    os << indent << "SlicerPreviousIcon: " << this->GetSlicerPreviousIcon() << "\n";
    os << indent << "SlicerUndoIcon: " << this->GetSlicerUndoIcon() << "\n";
    os << indent << "SlicerRedoIcon: " << this->GetSlicerRedoIcon() << "\n";
    os << indent << "SlicerUnlinkIcon: " << this->GetSlicerUnlinkIcon() << "\n";
    os << indent << "SlicerLinkIcon: " << this->GetSlicerLinkIcon() << "\n";
    os << indent << "SlicerCheckedVisibleIcon: " << this->GetSlicerCheckedVisibleIcon() << "\n";
    os << indent << "SlicerVisibleNoFrameIcon: " << this->GetSlicerVisibleNoFrameIcon() << "\n";
    os << indent << "SlicerInvisibleNoFrameIcon: " << this->GetSlicerInvisibleNoFrameIcon() << "\n";
    os << indent << "SlicerVisibleIcon: " << this->GetSlicerVisibleIcon() << "\n";
    os << indent << "SlicerInvisibleIcon: " << this->GetSlicerInvisibleIcon() << "\n";
    os << indent << "SlicerRefreshIcon: " << this->GetSlicerRefreshIcon() << "\n";
    os << indent << "SlicerVolumeIcon: " << this->GetSlicerVolumeIcon() << "\n";
    os << indent << "SlicerCenterOnFiducialIcon: " << this->GetSlicerCenterOnFiducialIcon() << "\n";
    os << indent << "SlicerFiducialsAddNewIcon: " << this->GetSlicerFiducialsAddNewIcon() << "\n";
    os << indent << "SlicerFiducialsDeleteAllIcon: " << this->GetSlicerFiducialsDeleteAllIcon() << "\n";
    os << indent << "SlicerFiducialsDeleteLastClickedIcon: " << this->GetSlicerFiducialsDeleteLastClickedIcon() << "\n";
    os << indent << "SlicerFiducialsSelectAllIcon: " << this->GetSlicerFiducialsSelectAllIcon() << "\n";
    os << indent << "SlicerFiducialsSelectNoneIcon: " << this->GetSlicerFiducialsSelectNoneIcon() << "\n";
    os << indent << "SlicerFiducialsSelectAllInListIcon: " << this->GetSlicerFiducialsSelectAllIcon() << "\n";
    os << indent << "SlicerFiducialsSelectNoneInListIcon: " << this->GetSlicerFiducialsSelectNoneIcon() << "\n";
    os << indent << "SlicerFiducialsDownIcon: " << this->GetSlicerFiducialsDownIcon() << "\n";
    os << indent << "SlicerFiducialsUpIcon: " << this->GetSlicerFiducialsUpIcon() << "\n";
    os << indent << "SlicerFiducialsDeleteListIcon: " << this->GetSlicerFiducialsDeleteListIcon() << "\n";
    os << indent << "SlicerFiducialsLockListIcon: " << this->GetSlicerFiducialsLockListIcon() << "\n";
    os << indent << "SlicerFiducialsUnlockListIcon: " << this->GetSlicerFiducialsUnlockListIcon() << "\n";
    os << indent << "SlicerFiducialsHideListIcon: " << this->GetSlicerFiducialsHideListIcon() << "\n";
    os << indent << "SlicerFiducialsExposeListIcon: " << this->GetSlicerFiducialsExposeListIcon() << "\n";
    os << indent << "SlicerFiducialsHideExposeAllListsIcon: " << this->GetSlicerFiducialsHideExposeAllListsIcon() << "\n";
    os << indent << "SlicerFiducialsHideAllListsIcon: " << this->GetSlicerFiducialsHideAllListsIcon() << "\n";
    os << indent << "SlicerFiducialsExposeAllListsIcon: " << this->GetSlicerFiducialsExposeAllListsIcon() << "\n";    
    os << indent << "SlicerCompositeIcon: " << this->GetSlicerCompositeIcon() << "\n";
    os << indent << "SlicerLockIcon: " << this->GetSlicerLockIcon() << "\n";
    os << indent << "SlicerUnlockIcon: " << this->GetSlicerUnlockIcon() << "\n";
    os << indent << "SlicerFoundOnDiskIcon: " << this->GetSlicerFoundOnDiskIcon() << "\n";
    os << indent << "SlicerNotFoundOnDiskIcon: " << this->GetSlicerNotFoundOnDiskIcon() << "\n";
    os << indent << "SlicerLockOrUnlockIcon: " << this->GetSlicerLockOrUnlockIcon() << "\n";
    os << indent << "SlicerVisibleOrInvisibleIcon: " << this->GetSlicerVisibleOrInvisibleIcon() << "\n";
    os << indent << "SlicerFiducialsDeleteAllInListIcon: " << this->GetSlicerFiducialsDeleteAllInListIcon() << "\n";
    os << indent << "SlicerImportSceneIcon: " << this->GetSlicerImportSceneIcon() << "\n";
    os << indent << "SlicerLoadSceneIcon: " << this->GetSlicerLoadSceneIcon() << "\n";
    os << indent << "SlicerLoadDicomVolumeIcon: " << this->GetSlicerLoadDicomVolumeIcon() << "\n";
    os << indent << "SlicerLoadVolumeIcon: " << this->GetSlicerLoadVolumeIcon() << "\n";
    os << indent << "SlicerLoadDirectoryIcon: " << this->GetSlicerLoadDirectoryIcon() << "\n";
    os << indent << "SlicerLoadFiducialsIcon: " << this->GetSlicerLoadFiducialsIcon() << "\n";
    os << indent << "SlicerLoadModelIcon: " << this->GetSlicerLoadModelIcon() << "\n";
    os << indent << "SlicerLoadTransformIcon: " << this->GetSlicerLoadTransformIcon() << "\n";
    os << indent << "SlicerLoadColorLUTIcon: " << this->GetSlicerLoadColorLUTIcon() << "\n";
    os << indent << "SlicerLoadFiberBundleIcon: " << this->GetSlicerLoadFiberBundleIcon() << "\n";
    os << indent << "SlicerLoadScalarOverlayIcon: " << this->GetSlicerLoadScalarOverlayIcon() << "\n";
    os << indent << "SlicerCloseIcon: " << this->GetSlicerCloseIcon() << "\n";
    os << indent << "SlicerExtensionsIcon: " << this->GetSlicerExtensionsIcon() << "\n";
    os << indent << "SlicerWWWIcon: " << this->GetSlicerWWWIcon() << "\n";
    os << indent << "SlicerAnnotationRulerEndpoint1Icon: " << this->GetSlicerAnnotationRulerEndpoint1Icon() << "\n";
    os << indent << "SlicerAnnotationRulerEndpoint2Icon: " << this->GetSlicerAnnotationRulerEndpoint2Icon() << "\n";
    os << indent << "SlicerAnnotationAngleEndpoint1Icon: " << this->GetSlicerAnnotationAngleEndpoint1Icon() << "\n";
    os << indent << "SlicerAnnotationAngleEndpoint2Icon: " << this->GetSlicerAnnotationAngleEndpoint2Icon() << "\n";
    os << indent << "SlicerAnnotationAngleCenterpointIcon: " << this->GetSlicerAnnotationAngleCenterpointIcon() << "\n";
    os << indent << "SlicerRotateToPixelSpaceIcon: " << this->GetSlicerRotateToPixelSpaceIcon() << "\n";
}
