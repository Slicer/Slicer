package require Itcl

#########################################################
#
if {0} { ;# comment

    This is function is executed by EMSegmenter

    # TODO :

}
#
#########################################################

#
# namespace procs
#

#
# Remember to source first GenericTask.tcl as it has all the variables/basic structure defined
#
namespace eval EMSegmenterPreProcessingTcl {

    #
    # Variables Specific to this Preprocessing
    #
    variable TextLabelSize 1
    variable CheckButtonSize 2
    variable VolumeMenuButtonSize 0
    variable TextEntrySize 0

    # Check Button
    variable atlasAlignedFlagID 0
    variable rightHandFlagID 1

    # Text Entry
    # not defined for this task

    #
    # OVERWRITE DEFAULT
    #

    # -------------------------------------
    # Define GUI
    # return 1 when error occurs
    # -------------------------------------
    proc ShowUserInterface { {LOGIC ""} } {
        variable preGUI
        variable atlasAlignedFlagID
        variable rightHandFlagID

        # Always has to be done initially so that variables are correctly defined
        if { [InitVariables $LOGIC] } {
            PrintError "ERROR: CT Hand Bone: ShowUserInterface: Not all variables are correctly defined!"
            return 1
        }
        $LOGIC PrintText "TCLCT: Preprocessing CT Hand Bone - ShowUserInterface"

        $preGUI DefineTextLabel "This task only applies to right handed scans scans! \n\nShould the EMSegmenter " 0
        $preGUI DefineCheckButton "- register the atlas to the input scan ?" 0 $atlasAlignedFlagID
        $preGUI DefineCheckButton "- right hand scan?" 0 $rightHandFlagID

        # Define this at the end of the function so that values are set by corresponding MRML node
        $preGUI SetButtonsFromMRML
    }

    # -------------------------------------
    # Define Preprocessing Pipeline
    # return 1 when error occurs
    # -------------------------------------
    proc Run { } {
        variable preGUI
        variable workingDN
        variable alignedTargetNode
        variable inputAtlasNode
        variable mrmlManager
        variable LOGIC

        variable atlasAlignedFlagID
        variable rightHandFlagID

        $LOGIC PrintText "TCLCT: =========================================="
        $LOGIC PrintText "TCLCT: == Preprocress Data"
        $LOGIC PrintText "TCLCT: =========================================="
        # ---------------------------------------
        # Step 1 : Initialize/Check Input
        if {[InitPreProcessing]} {
            return 1
        }

        set atlasAlignedFlag [ GetCheckButtonValueFromMRML $atlasAlignedFlagID ]
        set rightHandFlag [ GetCheckButtonValueFromMRML $rightHandFlagID ]

        # ----------------------------------------------------------------------------
        # Step 2 : Registration
        #
        CTHandRegistration $atlasAlignedFlag  $rightHandFlag

        # -------------------------------------
        # Step 3: Perform autosampling to define intensity distribution
        if { [ComputeIntensityDistributions] } {
            PrintError "Run: Could not automatically compute intensity distribution !"
            return 1
        }

        # -------------------------------------
        # Step 4: Now create background - do not do it before bc otherwise it compute intensity distribution
        CTHandGenerateBackground

        # -------------------------------------
        # Step 5: Check validity of Distributions
        set failedIDList [CheckAndCorrectTreeCovarianceMatrix]
        if { $failedIDList != "" } {
            set MSG "Log Covariance matrices for the following classes seemed incorrect:\n "
            foreach ID $failedIDList {
                set MSG "${MSG}[$mrmlManager GetTreeNodeName $ID]\n"
            }
            set MSG "${MSG}This can cause failure of the automatic segmentation. To address the issue, please visit the web site listed under Help"
            $preGUI PopUpWarningWindow "$MSG"
        }
        return 0
    }

    # ----------------------------------------------------------------------------
    proc CTHandRegistration { alignFlag rightHandFlag } {
        variable mrmlManager
        variable workingDN
        variable LOGIC

        $LOGIC PrintText "TCLCT: =========================================="
        $LOGIC PrintText "TCLCT: == Register CT Atlas"
        $LOGIC PrintText "TCLCT: =========================================="

        $LOGIC PrintText "TCLCT: ==> Preprocessing Setting: $alignFlag $rightHandFlag"

        set affineFlag [expr ([$mrmlManager GetRegistrationAffineType] != [$mrmlManager GetRegistrationTypeFromString RegistrationOff])]
        set bSplineFlag [expr ([$mrmlManager GetRegistrationDeformableType] != [$mrmlManager GetRegistrationTypeFromString RegistrationOff])]

        if {($alignFlag == 0) || ( ($affineFlag == 0) && ($bSplineFlag == 0) ) } {
            return [SkipAtlasRegistration]
        }

        # (flip), fiducial_threshold, blur, binarize, choose largest component of: atlas and target
        # RegisterHandAtlas


        # ----------------------------------------------------------------
        # Setup
        # ----------------------------------------------------------------

        set inputAtlasNode [$mrmlManager GetAtlasInputNode]
        set alignedAtlasNode [$mrmlManager GetAtlasAlignedNode]

        ##debug
        if { $inputAtlasNode != "" } {
            $LOGIC PrintText "Detected [$inputAtlasNode GetNumberOfVolumes] inputAtlasNodeVolumes"
        }
        if { $alignedAtlasNode != "" } {
            $LOGIC PrintText "Detected [$alignedAtlasNode GetNumberOfVolumes] alignedAtlasNodeVolumes"
        }
        ##debug

        if { $alignedAtlasNode == "" } {
            $LOGIC PrintText "TCL: Aligned Atlas was empty"
            #  $LOGIC PrintText "TCL: set outputAtlasNode \[ $mrmlManager CloneAtlasNode $inputAtlasNode \"AlignedAtlas\"\] "
            set alignedAtlasNode [ $mrmlManager CloneAtlasNode $inputAtlasNode "Aligned"]
            $workingDN SetReferenceAlignedAtlasNodeID [$alignedAtlasNode GetID]
            ###???      $workingDN SetReferenceAlignedAtlasNodeID [$alignedAtlasNode GetID]
        } else {
            $LOGIC PrintText "TCL: Atlas was just synchronized"
            $mrmlManager SynchronizeAtlasNode $inputAtlasNode $alignedAtlasNode "Aligned"
        }

        set inputTargetNode [$mrmlManager GetTargetInputNode]
        if { $inputTargetNode != "" } {
            $LOGIC PrintText "Detected [$inputTargetNode GetNumberOfVolumes] inputTargetNodeVolumes"
        }

        set alignedTargetNode [$workingDN GetAlignedTargetNode]
        if { $alignedTargetNode != "" } {
            $LOGIC PrintText "Detected [$alignedTargetNode GetNumberOfVolumes] alignedTargetNodeVolumes"
        }

        set inputTargetVolumeNode [$inputTargetNode GetNthVolumeNode 0]
        set inputTargetVolumeFileName [WriteDataToTemporaryDir $inputTargetVolumeNode Volume]

        if { $rightHandFlag } {
            set flip_is_necessary_for_target 0
            set flip_is_necessary_for_atlas 0
        } else {
            set flip_is_necessary_for_target 0
            set flip_is_necessary_for_atlas 1
        }

        set atlasRegistrationVolumeIndex -1;
        if {[[$mrmlManager GetGlobalParametersNode] GetRegistrationAtlasVolumeKey] != "" } {
            set atlasRegistrationVolumeKey [[$mrmlManager GetGlobalParametersNode] GetRegistrationAtlasVolumeKey]
            set atlasRegistrationVolumeIndex [$inputAtlasNode GetIndexByKey $atlasRegistrationVolumeKey]
        }

        if { $atlasRegistrationVolumeIndex < 0 } {
            PrintError "RegisterAtlas: Attempt to register atlas image but no atlas image selected!"
            return 1
        }

        set inputAtlasVolumeNode [$inputAtlasNode GetNthVolumeNode $atlasRegistrationVolumeIndex]
        set inputAtlasVolumeFileName [WriteDataToTemporaryDir $inputAtlasVolumeNode Volume]



        set blurredInputTargetVolumeFileName [CreateTemporaryFileNameForNode $inputTargetVolumeNode]
        CTHandExtractBlurrySegmentation $inputTargetVolumeFileName $blurredInputTargetVolumeFileName $flip_is_necessary_for_target

        $LOGIC PrintText "pre-process atlas template..."

        set blurredInputAtlasVolumeFileName [CreateTemporaryFileNameForNode $inputAtlasVolumeNode]
        CTHandExtractBlurrySegmentation $inputAtlasVolumeFileName $blurredInputAtlasVolumeFileName $flip_is_necessary_for_atlas


        ### Call BRAINSFit ###

        #  set transformfile RegisterAtlasToSubject { $outputAtlasFileName $outputFileName }

        set PLUGINS_DIR "[$LOGIC GetPluginsDirectory]"
        set CMD "${PLUGINS_DIR}/BRAINSFit"

        set fixedVolumeFileName $blurredInputTargetVolumeFileName
        set CMD "$CMD --fixedVolume $fixedVolumeFileName"

        set movingVolumeFileName $blurredInputAtlasVolumeFileName
        set CMD "$CMD --movingVolume $movingVolumeFileName"

        set outputVolumeFileName [CreateFileName "Volume"]
        if { $outputVolumeFileName == "" } {
            PrintError "Failed to create a temporary file"
        }
        set CMD "$CMD --outputVolume $outputVolumeFileName"

        set linearTransform [CreateFileName "LinearTransform"]
        if { $linearTransform == "" } {
            PrintError "Failed to create a temporary file"
        }
        set CMD "$CMD --outputTransform $linearTransform"

        set CMD "$CMD --initializeTransformMode useMomentsAlign --transformType Rigid,Affine"

        $LOGIC PrintText "TCL: Executing $CMD"
        catch { eval exec $CMD } errmsg
        $LOGIC PrintText "TCL: $errmsg"



        ### Call BRAINSDemonWarp ###

        set CMD "${PLUGINS_DIR}/BRAINSDemonWarp"
        set CMD "$CMD -m $movingVolumeFileName -f $fixedVolumeFileName"
        set CMD "$CMD --initializeWithTransform $linearTransform"
        set oArgument [CreateFileName "Volume"]
        if { $oArgument == "" } {
            PrintError "Failed to create a temporary file"
        }
        set deformationfield [CreateFileName "Volume"]
        if { $deformationfield == "" } {
            PrintError "Failed to create a temporary file"
        }
        set CMD "$CMD -o $oArgument -O $deformationfield"
        set CMD "$CMD -i 1000,500,250,125,60 -n 5 -e --numberOfMatchPoints 16"
        # fast - for debugging
        #set CMD "$CMD -i 5,5,2,2,1 -n 5 -e --numberOfMatchPoints 16"

        $LOGIC PrintText "TCL: Executing $CMD"
        catch { eval exec $CMD } errmsg
        $LOGIC PrintText "TCL: $errmsg"


        #TODO: check here for return code

        ### Call Resample ###

        set fixedTargetChannel 0
        set fixedTargetVolumeNode [$alignedTargetNode GetNthVolumeNode $fixedTargetChannel]
        set fixedTargetVolumeFileName [WriteImageDataToTemporaryDir $fixedTargetVolumeNode]

        for { set i 0 } { $i < [$alignedAtlasNode GetNumberOfVolumes] } { incr i } {
            #            if { $i == $atlasRegistrationVolumeIndex} { continue }
            $LOGIC PrintText "TCL: Resampling atlas image $i ..."
            set inputAtlasVolumeNode [$inputAtlasNode GetNthVolumeNode $i]
            set outputAtlasVolumeNode [$alignedAtlasNode GetNthVolumeNode $i]
            set backgroundLevel [$LOGIC GuessRegistrationBackgroundLevel $inputAtlasVolumeNode]
            $LOGIC PrintText "TCL: Guessed background level: $backgroundLevel"

            set inputAtlasVolumeFileName [WriteImageDataToTemporaryDir $inputAtlasVolumeNode]
            set outputAtlasVolumeFileName [WriteImageDataToTemporaryDir $outputAtlasVolumeNode]

            set CMD "${PLUGINS_DIR}/BRAINSResample"
            set CMD "$CMD --inputVolume $inputAtlasVolumeFileName  --referenceVolume $fixedTargetVolumeFileName"
            set CMD "$CMD --outputVolume $outputAtlasVolumeFileName --deformationVolume $deformationfield"

            set CMD "$CMD --pixelType [BRAINSGetPixelTypeFromVolumeNode $fixedTargetVolumeNode]"

            $LOGIC PrintText "TCL: Executing $CMD"
            catch { eval exec $CMD } errmsg
            $LOGIC PrintText "TCL: $errmsg"

            ReadDataFromDisk $outputAtlasVolumeNode $outputAtlasVolumeFileName Volume
            file delete -force $outputAtlasVolumeFileName
        }
        #end for loop

    }
    # end CTHandRegistration

    #
    # TASK SPECIFIC FUNCTIONS
    #
    proc CTHandExtractBlurrySegmentation { inputFileName outputFileName flip_is_necessary } {
        variable LOGIC

        set CTHandBoneHelper [vtkCTHandBoneClass New]

        if { $flip_is_necessary } {
            set TargetFlipFileName [CreateFileName "Volume"]
            if { $TargetFlipFileName == "" } {
                PrintError "Failed to create a temporary file"
            }
        }

        set TargetFlipThresholdFileName [CreateFileName "Volume"]
        if { $TargetFlipThresholdFileName == "" } {
            PrintError "Failed to create a temporary file"
        }
        set TargetFlipThresholdBlurFileName [CreateFileName "Volume"]
        if { $TargetFlipThresholdBlurFileName == "" } {
            PrintError "Failed to create a temporary file"
        }
        set TargetFlipThresholdBlurBinaryFileName [CreateFileName "Volume"]
        if { $TargetFlipThresholdBlurBinaryFileName == "" } {
            PrintError "Failed to create a temporary file"
        }

        if { $flip_is_necessary } {
            $LOGIC PrintText "flip..."
            set ret [$CTHandBoneHelper flip $inputFileName $TargetFlipFileName "1" "0" "0"]
        } else {
            # skip flipping
            $LOGIC PrintText "skip flipping..."
            set TargetFlipFileName $inputFileName
        }

        $LOGIC PrintText "threshold..."
        set ret [$CTHandBoneHelper fiducial_threshold $TargetFlipFileName $TargetFlipThresholdFileName]

        $LOGIC PrintText "blur..."
        set ret [$CTHandBoneHelper blur $TargetFlipThresholdFileName $TargetFlipThresholdBlurFileName "1.5" "5"]

        $LOGIC PrintText "binary..."
        set ret [$CTHandBoneHelper binary_threshold $TargetFlipThresholdBlurFileName $TargetFlipThresholdBlurBinaryFileName "0" "30"]

        $LOGIC PrintText "largest..."
        set ret [$CTHandBoneHelper largest_component $TargetFlipThresholdBlurBinaryFileName $outputFileName]

        $CTHandBoneHelper Delete

    }
    # end CTHandExtractBlurrySegmentation


    # Generates Background class by subtracting the atlases of the other classes and inverting the results
    proc CTHandGenerateBackground  { } {
        variable outputAtlasNode
        variable inputAtlasNode
        variable mrmlManager
        variable LOGIC
        variable ERROR_NODE_VTKID

        # All nodes that are not parent classes are assigned to background

        # add all tree leafes to $bgList
        set rootID [$mrmlManager GetTreeRootNodeID]
        set n [$mrmlManager GetTreeNodeNumberOfChildren $rootID]
        set bgList ""
        for { set i 0 } { $i < $n } { incr i } {
            set id [ $mrmlManager GetTreeNodeChildNodeID $rootID $i ]
            if { [ $mrmlManager GetTreeNodeIsLeaf $id ] } {
                set bgList "${bgList} $id"
            }
        }

        # Check if atlas bg node is defined 
        # Implicit assumption: is first leaf a background leaf ?
        set treeID    [lindex $bgList 0]
        set bgAtlasID [$mrmlManager GetTreeNodeSpatialPriorVolumeID $treeID]


        # Create dummy AtlasBgNode for inputnode
        if { $bgAtlasID == $ERROR_NODE_VTKID } {
            # I do this bc it is easier for debugging
            set inputAtlasNode [$mrmlManager GetAtlasInputNode]
            set inputAtlasVolumeNode [$inputAtlasNode GetNthVolumeNode 0]
            set  bgAtlasID [$mrmlManager CreateVolumeScalarNodeVolumeID $inputAtlasVolumeNode "BGAtlas(Place Holder)" ]
        }

        # Assign node to all background classes
        foreach bgID $bgList {
            $mrmlManager SetTreeNodeSpatialPriorVolumeID $bgID $bgAtlasID
        }

        # Check if outputAtlasNode is defined
        set bgAtlasNode [$mrmlManager GetAlignedSpatialPriorFromTreeNodeID $treeID]
        if { $bgAtlasNode == "" } {
            # We need to create the node
            set outputAtlasNode [$mrmlManager GetAtlasAlignedNode]
            set outputAtlasVolumeNode [$outputAtlasNode GetNthVolumeNode 0]
            set bgAtlasNode [$mrmlManager  CreateVolumeScalarNode $outputAtlasVolumeNode "BGAtlas(Aligned)"]
        }

        # Make sure all the output also point to the same volume
        foreach bgID $bgList {
            set treeNode [$mrmlManager GetTreeNode $bgID]
            $outputAtlasNode AddVolume [$treeNode GetID] [$bgAtlasNode GetID]
        }

        $LOGIC PrintText "TCLCT: Creating BG Atlas Volume Node [$bgAtlasNode GetName]"

        # Check if output Data is defined
        set bgAtlasData [$bgAtlasNode GetImageData]
        if { $bgAtlasData == "" } {
            $LOGIC PrintText "TCLCT: Created Image Data"
            set bgAtlasData [vtkImageData New]
            $bgAtlasNode SetAndObserveImageData $bgAtlasData
            $bgAtlasData Delete

            # important to do this in tcl
            set bgAtlasData [$bgAtlasNode GetImageData]
        }

        #
        # Set up operation
        #
        set addResults [vtkImageData New]
        set addFilter [vtkImageMathematics New]
        $addFilter SetOperationToAdd
        $addFilter SetInput1 $addResults

        set numInputs 0

        # carefull - this setup can fail if adding values up is beyond scalar range of scalar type
        foreach ID [GetAllLeafNodeIDsInTree] {
            # background class
            if { [lsearch "$bgList" $ID] > -1 } {
                continue
            }

            # no spatial prior defined
            set leafAtlasNode [$mrmlManager GetAlignedSpatialPriorFromTreeNodeID $ID]
            if { $leafAtlasNode == "" }   {
                continue
            }

            $LOGIC PrintText "TCLCT:  Adding [$leafAtlasNode GetName ] from class [[ $mrmlManager GetTreeNode $ID] GetName] "
            if { $numInputs > 0 } {
                $addFilter SetInput2 [$leafAtlasNode GetImageData]
                $addFilter Update
                $addResults DeepCopy [$addFilter GetOutput]
            } else {
                $addResults DeepCopy [$leafAtlasNode GetImageData]
            }
            incr numInputs
        }

        $addFilter Delete

        if { $numInputs  == 0 } {
            PrintError "CT-Hand-Bone: No enough spatial atlases defined "
            $addResults Delete
            return
        }

        set MAX [lindex [$addResults GetScalarRange] 1]
        # Need to do that for unsigned types
        set castInFilter [vtkImageCast New]
        $castInFilter SetOutputScalarTypeToFloat
        $castInFilter SetInput $addResults
        $castInFilter Update

        set subFilter [vtkImageMathematics New]
        $subFilter SetOperationToAddConstant
        $subFilter SetConstantC -$MAX
        $subFilter SetInput1 [$castInFilter GetOutput]
        $subFilter Update

        set mulFilter [vtkImageMathematics New]
        $mulFilter SetOperationToMultiplyByK
        $mulFilter SetConstantK -1.0
        $mulFilter SetInput1 [$subFilter GetOutput]
        $mulFilter Update

        set castOutFilter [ vtkImageCast New]
        $castOutFilter SetOutputScalarType [$addResults GetScalarType]
        $castOutFilter SetInput [$mulFilter  GetOutput]
        $castOutFilter Update

        $bgAtlasData DeepCopy [$castOutFilter GetOutput]

        $castOutFilter Delete
        $mulFilter Delete
        $subFilter Delete
        $castInFilter Delete
        $addResults Delete
    }
}
# end namespace eval EMSegmenterPreProcessingTcl

namespace eval EMSegmenterSimpleTcl {
    # 0 = Do not create a check list for the simple user interface
    # simply remove
    # 1 = Create one - then also define ShowCheckList and
    #     ValidateCheckList where results of checklist are transfered to Preprocessing

    proc CreateCheckList { } {
        return 1
    }

    proc ShowCheckList { {LOGIC ""} } {
        variable inputChannelGUI

        # Always has to be done initially so that variables are correctly defined
        if { [InitVariables $LOGIC] } {
            PrintError "ShowCheckList: Not all variables are correctly defined!"
            return 1
        }

        $inputChannelGUI DefineTextLabel "Is the subject right handed?" 0
        $inputChannelGUI DefineCheckButton "- Are you providing a right hand scan?" 0 $EMSegmenterPreProcessingTcl::rightHandFlagID


        # Define this at the end of the function so that values are set by corresponding MRML node
        $inputChannelGUI SetButtonsFromMRML
        return 0

    }

    proc ValidateCheckList { } {
        return 0
    }

    proc PrintError { TEXT } {
        puts stderr "TCLCT: ERROR:EMSegmenterSimpleTcl::${TEXT}"
    }
}
