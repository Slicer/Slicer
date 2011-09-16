
proc PLASTIMATCHGetPixelTypeFromVolumeNode { volumeNode } {

    set referenceVolume [$volumeNode GetImageData]
    set scalarType [$referenceVolume GetScalarTypeAsString]
    switch -exact "$scalarType" {
        "char"           { set PIXELTYPE "char" }
        "unsigned char"  { set PIXELTYPE "uchar" }
        "short"          { set PIXELTYPE "short" }
        "unsigned short" { set PIXELTYPE "ushort" }
        "int"            { set PIXELTYPE "int" }
        "unsigned int"   { set PIXELTYPE "uint" }
        "float"          { set PIXELTYPE "float" }
        "double"         { set PIXELTYPE "double" }
        default {
            PrintError "PLASTIMATCHGetPixelTypeFromVolumeNode: Cannot handle a volume of type $scalarType"
            set PIXELTYPE "INVALID"
        }
    }
    return $PIXELTYPE
}


# ----------------------------------------------------------------------------
proc PLASTIMATCHResampleCLI { inputVolumeNode referenceVolumeNode outVolumeNode transformFileName interpolationType backgroundLevel } {
    variable SCENE
    variable LOGIC
    variable REGISTRATION_PACKAGE_FOLDER

    $LOGIC PrintText "TCL: =========================================="
    $LOGIC PrintText "TCL: == Resample Image CLI : PLASTIMATCHResampleCLI "
    $LOGIC PrintText "TCL: =========================================="

    set CMD "$REGISTRATION_PACKAGE_FOLDER/plastimatch_slicer_xformwarp"

    set outVolumeFileName [CreateTemporaryFileNameForNode $outVolumeNode]
    if { $outVolumeFileName == "" } { return 1 }

    set inputVolumeFileName [WriteDataToTemporaryDir $inputVolumeNode Volume]
    if { $inputVolumeFileName == "" } { return 1 }

    set referenceVolumeFileName [WriteDataToTemporaryDir $referenceVolumeNode Volume]
    if { $referenceVolumeFileName == "" } { return 1 }

    set CMD "$CMD --output_type [PLASTIMATCHGetPixelTypeFromVolumeNode $referenceVolumeNode]"
    set CMD "$CMD --plmslc_xformwarp_output_img \"$outVolumeFileName\""
    set CMD "$CMD --plmslc_xformwarp_input_xform_f \"$transformFileName\""
    set CMD "$CMD --plmslc_xformwarp_input_img \"$inputVolumeFileName\""
    set CMD "$CMD --reference_vol \"$referenceVolumeFileName\""

    $LOGIC PrintText "TCL: Executing $CMD"
    catch { eval exec $CMD } errmsg
    $LOGIC PrintText "TCL: $errmsg"

    # Write results back to scene
    ReadDataFromDisk $outVolumeNode $outVolumeFileName Volume

    # clean up
    file delete -force $outVolumeFileName
    file delete -force $inputVolumeFileName
    file delete -force $referenceVolumeFileName

    return 0
}


# ----------------------------------------------------------------------------
proc PLASTIMATCHRegistration { fixedVolumeNode movingVolumeNode outVolumeNode backgroundLevel deformableType affineType} {
    variable SCENE
    variable LOGIC
    variable REGISTRATION_PACKAGE_FOLDER
    variable mrmlManager

    # Do not get rid of debug mode variable - it is sometimes very helpful !
    set PLASTIMATCH_DEBUG_MODE 1

    if { $PLASTIMATCH_DEBUG_MODE } {
        $LOGIC PrintText ""
        $LOGIC PrintText "DEBUG: ==========PLASTIMATCHRegistration DEBUG MODE ============="
        $LOGIC PrintText ""
    }

    $LOGIC PrintText "TCL: =========================================="
    $LOGIC PrintText "TCL: == Image Alignment CommandLine: $deformableType "
    $LOGIC PrintText "TCL: =========================================="

    # check arguments

    if { $fixedVolumeNode == "" || [$fixedVolumeNode GetImageData] == "" } {
        PrintError "PLASTIMATCHRegistration: fixed volume node not correctly defined"
        return ""
    }

    if { $movingVolumeNode == "" || [$movingVolumeNode GetImageData] == "" } {
        PrintError "PLASTIMATCHRegistration: moving volume node not correctly defined"
        return ""
    }

    if { $outVolumeNode == "" } {
        PrintError "PLASTIMATCHRegistration: output volume node not correctly defined"
        return ""
    }

    set fixedVolumeFileName [WriteDataToTemporaryDir $fixedVolumeNode Volume]
    if { $fixedVolumeFileName == "" } {
        # remove files
        return ""
    }
    set RemoveFiles "$fixedVolumeFileName"


    set movingVolumeFileName [WriteDataToTemporaryDir $movingVolumeNode Volume]
    if { $movingVolumeFileName == "" } {
        #remove files
        return ""
    }
    set RemoveFiles "$RemoveFiles $movingVolumeFileName"


    set outVolumeFileName [CreateTemporaryFileNameForNode $outVolumeNode]
    if { $outVolumeFileName == "" } {
        #remove files
        return ""
    }
    set RemoveFiles "$RemoveFiles $outVolumeFileName"

    ## PLASTIMATCH specific arguments

    set CMD "$REGISTRATION_PACKAGE_FOLDER/plastimatch_slicer_bspline"

    if { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationTest] } {
        set CMD "$CMD --stage1its 3"
    } elseif { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationFast] } {
        set CMD "$CMD --stage1its 3"
    } elseif { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationSlow] } {
        set CMD "$CMD --stage1its 3"
    } else {
        PrintError "PLASTIMATCHRegistration: Unknown deformableType: $deformableType"
        return ""
    }


    # affine
    set outLinearTransformFileName [CreateFileName "Text"]

    set outTransformFileName $outLinearTransformFileName

    #        set CMD "$CMD --plmslc_output_bsp_f \"$outLinearTransformFileName\""
    #        set CMD "$CMD --plmslc_output_vf \"$outVolumeFileName\""
    set CMD "$CMD --output_type [PLASTIMATCHGetPixelTypeFromVolumeNode $fixedVolumeNode]"
    set CMD "$CMD --plmslc_output_warped  \"$outVolumeFileName\""
    set CMD "$CMD --plmslc_output_bsp  \"$outLinearTransformFileName\""

    set CMD "$CMD \"$fixedVolumeFileName\""
    set CMD "$CMD \"$movingVolumeFileName\""
#    set CMD "$CMD \"$outVolumeFileName\""


    $LOGIC PrintText "TCL: Executing $CMD"
    catch { eval exec $CMD } errmsg
    $LOGIC PrintText "TCL: $errmsg"

    ## Read results back to scene
    if { [ReadDataFromDisk $outVolumeNode $outVolumeFileName Volume] == 0 } {
        if { [file exists $outVolumeFileName] == 0 } {
            set outTransformDirName ""
            PrintError "PLASTIMATCHRegistration: out volume file doesn't exists"
        }
    }

    if { [file exists $outTransformFileName] == 0 } {
        set outTransformFileName ""
        PrintError "PLASTIMATCHRegistration: out transform file doesn't exists"
    }

    foreach NAME $RemoveFiles {
        #file delete -force $NAME
    }

    # Remove Transformation from image
    $movingVolumeNode SetAndObserveTransformNodeID ""
    $SCENE Edited

    # return transformation directory name or ""
    puts "outTransformFileName: $outTransformFileName"
    return $outTransformFileName
}
