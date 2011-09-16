package require Itcl

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
    variable FlagID1 0
    variable FlagID2 1

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

        variable FlagID1
        variable FlagID2

        # Always has to be done initially so that variables are correctly defined
        if { [InitVariables $LOGIC] } {
            $LOGIC PrintText "ShowUserInterface: Not all variables are correctly defined!"
            return 1
        }
        $LOGIC PrintText  "TCLTemplate: Preprocessing - ShowUserInterface"

        $preGUI DefineTextLabel "This is a template! \n\nShould the EMSegmenter " 0
        $preGUI DefineCheckButton "- This is a flag" 0 $FlagID1
        $preGUI DefineCheckButton "- This is a second flag" 0 $FlagID2

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

        $LOGIC PrintText "TCLTemplate: =========================================="
        $LOGIC PrintText "TCLTemplate: == Preprocress Data"
        $LOGIC PrintText "TCLTemplate: =========================================="
        # ---------------------------------------
        # Step 1 : Initialize/Check Input
        if {[InitPreProcessing]} {
            return 1
        }

    }

    #
    # TASK SPECIFIC FUNCTIONS
    #


    proc PrintError { TEXT } {
        variable LOGIC
        $LOGIC PrintText "TCLTemplate: ERROR: EMSegmenterPreProcessingTcl::${TEXT}"
    }

}


namespace eval EMSegmenterSimpleTcl {
    # 0 = Do not create a check list for the simple user interface
    # simply remove
    # 1 = Create one - then also define ShowCheckList and
    #     ValidateCheckList where results of checklist are transfered to Preprocessing

    proc CreateCheckList { } {
        PrintError "CreateCheckList"
        return 1
    }

    proc ShowCheckList { {LOGIC ""} } {
        PrintError "ShowCheckList"
        variable inputChannelGUI
        # Always has to be done initially so that variables are correctly defined
        if { [InitVariables $LOGIC] } {
            PrintError "ShowCheckList: Not all variables are correctly defined!"
            return 1
        }

        $inputChannelGUI DefineTextLabel "Please insure that input scans are not skull stripped" 0
        $inputChannelGUI DefineCheckButton "Perform ... ?" 0 $EMSegmenterPreProcessingTcl::FlagID2

        # Define this at the end of the function so that values are set by corresponding MRML node
        $inputChannelGUI SetButtonsFromMRML
        return 0

    }

    proc ValidateCheckList { } {
        PrintError "ValidateCheckList"
        return 0
    }

    proc PrintError { TEXT } {
        puts stderr "TCLTemplate: ERROR:EMSegmenterSimpleTcl::${TEXT}"
    }
}
