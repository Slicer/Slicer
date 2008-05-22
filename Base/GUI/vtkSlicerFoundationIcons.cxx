
#include "vtkObjectFactory.h"
#include "vtkSlicerFoundationIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerFoundationIcons );
vtkCxxRevisionMacro ( vtkSlicerFoundationIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerFoundationIcons::vtkSlicerFoundationIcons ( )
{
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
  this->SlicerVisibleIcon = vtkKWIcon::New();
  this->SlicerInvisibleIcon = vtkKWIcon::New();
  this->SlicerRefreshIcon = vtkKWIcon::New();
  this->SlicerVolumeIcon = vtkKWIcon::New();

  this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkSlicerFoundationIcons::~vtkSlicerFoundationIcons ( )
{

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

}

//---------------------------------------------------------------------------
void vtkSlicerFoundationIcons::AssignImageDataToIcons ( )
{
    // SliceVisibleIcon
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
  this->SlicerGlyphIcon->SetImage ( image_SlicerGlyph,
                                    image_SlicerGlyph_width,
                                    image_SlicerGlyph_height,
                                    image_SlicerGlyph_pixel_size,
                                    image_SlicerGlyph_length, 0);
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
  this->SlicerVisibleIcon->SetImage ( image_SlicerVisible,
                                      image_SlicerVisible_width,
                                      image_SlicerVisible_height,
                                      image_SlicerVisible_pixel_size,
                                      image_SlicerVisible_length, 0);
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

}



//---------------------------------------------------------------------------
void vtkSlicerFoundationIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerFoundationIcons: " << this->GetClassName ( ) << "\n";
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
    os << indent << "SlicerVisibleIcon: " << this->GetSlicerVisibleIcon() << "\n";
    os << indent << "SlicerInvisibleIcon: " << this->GetSlicerInvisibleIcon() << "\n";
    os << indent << "SlicerRefreshIcon: " << this->GetSlicerRefreshIcon() << "\n";
    os << indent << "SlicerVolumeIcon: " << this->GetSlicerVolumeIcon() << "\n";
}
