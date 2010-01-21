from ctypes import *
import ctypes.util

## This may not work, so you may need to put the shared library
## in the current working directory and use simply:
# libteem = CDLL('./libteem.so')
libteem = CDLL(ctypes.util.find_library('libteem'))

# see if this worked
if not libteem._name:
    print "**"
    print "**  teem.py couldn\'t find and load the \"libteem\" shared library."
    print "**"
    print "**  On Linux, try putting libteem.so in the current directory "
    print "**  and edit teem.py to replace: "
    print "**     libteem = CDLL(ctypes.util.find_library(\'libteem\')"
    print "**  with:"
    print "**     libteem = CDLL(\'./libteem.so\')"
    print "**"
    raise ImportError

STRING = c_char_p

class FILE(Structure):
    pass

# oddly, size_t is in ctypes, but not ptrdiff_t
# which is probably a bug
if sizeof(c_void_p) == 4:
    ptrdiff_t = c_int32
elif sizeof(c_void_p) == 8:
    ptrdiff_t = c_int64

# NOTE: see end of this file for definitions of stderr, stdin, stdout

nrrdKindQuaternion = 23
airInsane_not = 0
echoJitterGrid = 1
echoTypeSplit = 9
nrrdMeasureHistoProduct = 22
airInsane_FISize = 11
tenGageCa1Hessian = 181
nrrdField_centers = 15
tenAniso_Tr = 26
nrrdKindRGBAColor = 18
nrrdUnaryOpTan = 5
airInsane_nInfExists = 3
nrrdCenterCell = 2
tenGageNormGradVec = 36
gageSclHessEval1 = 11
seekTypeIsocontour = 1
echoJitterUnknown = -1
nrrdUnaryOpRand = 24
pullInfoSeedThresh = 11
limnSplineTypeHermite = 3
nrrdBasicInfoOldMax = 13
pullProcessModeAdding = 3
pullInterTypeUnivariate = 2
tenGageFA = 9
nrrdField_sizes = 9
miteValVdefT = 17
tenGageDelNormK3 = 24
nrrdBinaryOpSubtract = 2
tenGageDetNormal = 44
miteShadeMethodNone = 1
nrrdField_space_units = 29
nrrdTypeChar = 1
gageParmOrientationFromSpacing = 12
limnQN10checker = 12
gageVecMGEval = 28
nrrdSpaceLast = 13
tenDwiFiberType12BlendEvec0 = 3
tenDwiFiberTypeUnknown = 0
nrrdField_keyvalue = 27
tenGageCp1HessianEval2 = 176
limnQN12octa = 10
nrrdField_space_dimension = 8
nrrdZlibStrategyUnknown = 0
nrrdTernaryOpLast = 11
tenGageDetGradMag = 43
tenGageCp2 = 78
gagePvlFlagLast = 4
nrrdTernaryOpMax = 4
miteValNdotV = 13
tenGageCa1GradMag = 159
tenGageRNormal = 56
gageSclFlowlineCurv = 28
tenGageCp1GradVec = 155
nrrdMeasureMin = 1
limnQN14octa = 7
nrrdSpaceScannerXYZ = 7
nrrdSpacingStatusDirection = 4
tenGlyphTypeCylinder = 3
pullProcessModeNixing = 4
nrrdEncodingTypeGzip = 4
hooverErrRenderBegin = 2
nrrdMeasureUnknown = 0
limnCameraPathTrackAt = 2
nrrdKind3DMaskedMatrix = 31
nrrdAxisInfoLast = 11
tenDwiGageTensorLikelihood = 25
nrrdSpaceLeftPosteriorSuperiorTime = 6
nrrdTypeUnknown = 0
limnSpaceDevice = 4
dyeSpaceHSV = 1
tenGageOmegaHessianEval1 = 126
nrrdBoundaryWeight = 4
pullInfoTangent1 = 14
tenTripleTypeLast = 10
limnEdgeTypeBorder = 6
seekTypeMinimalSurface = 6
tenGageCp1HessianEvec0 = 178
ell_cubic_root_unknown = 0
nrrdKind2DSymMatrix = 24
nrrdUnaryOpNegative = 1
gagePvlFlagQuery = 2
tenGageConfidence = 2
tenInterpTypeLoxK = 7
airFP_POS_ZERO = 9
nrrdUnaryOpZero = 27
tenGageCl1GradMag = 153
gageCtxFlagUnknown = 0
tenGageTraceGradVec = 33
tenTripleTypeWheelParm = 9
nrrdBinaryOpLast = 23
tenAniso_Cs2 = 12
tenGageCp1GradMag = 156
gageSclHessFrob = 8
echoMatterGlassIndex = 0
nrrdBasicInfoLast = 16
tenEstimate2MethodPeled = 2
echoTypeTriMesh = 6
tenGageDelNormPhi1 = 27
tenGageSGradVec = 45
ell_cubic_root_triple = 2
gageSclGradVec = 2
tenGageTensorQuatGeoLoxR = 151
nrrdKindRGBColor = 14
gageVecCurlGradient = 17
miteValVrefN = 15
tenDwiGageTensorLLSError = 7
limnPolyDataInfoLast = 4
limnPolyDataInfoTex2 = 3
tenGageCa1HessianEvec2 = 189
nrrdField_max = 20
nrrdIoStateUnknown = 0
nrrdUnaryOpLog2 = 11
gageSclGaussCurv = 25
echoTypeSuperquad = 2
pullPropIdtag = 1
nrrdAxisInfoLabel = 9
nrrdKind2DMaskedMatrix = 27
tenDwiGageTensor = 22
airFP_QNAN = 2
limnSplineInfoScalar = 1
nrrdIoStateSkipData = 5
nrrdKind3DMatrix = 30
nrrdMeasureLineError = 16
nrrdEncodingTypeAscii = 2
nrrdFormatTypeUnknown = 0
seekTypeRidgeSurface = 2
tenGageCp1HessianEval1 = 175
tenGageFAHessianEvec1 = 94
tenGageCl1GradVec = 152
tenGageDetGradVec = 42
tenGageFAHessianEval0 = 89
nrrdUnaryOpRoundDown = 20
gageVecDivGradient = 16
nrrdUnaryOpExp = 9
tenGageTensorQuatGeoLoxK = 150
airNoDio_fpos = 9
tenAniso_Cl2 = 8
airTypeChar = 8
miteStageOpUnknown = 0
gageVecGradient0 = 23
nrrdBlind8BitRangeFalse = 2
tenGageSGradMag = 46
tenDwiGageUnknown = 0
miteValYw = 3
airEndianUnknown = 0
tenFiberStopUnknown = 0
miteShadeMethodLast = 4
tenDwiGageJustDWI = 3
airTypeDouble = 7
echoJittableNormalA = 3
nrrdField_sample_units = 28
tenGageCa1 = 75
nrrdField_units = 18
tenGageFANormal = 53
tenDwiGageTensorWLSError = 11
tenGageFAHessianEvec = 92
tenDwiFiberType2Evec0 = 2
nrrdTypeFloat = 9
tenFiberStopLength = 2
nrrdBinaryOpExists = 19
gageSclShapeTrace = 22
nrrdBinaryOpSgnPow = 6
limnQN16octa = 4
limnPrimitiveTriangleStrip = 3
limnPrimitiveLineStrip = 6
miteValNdotL = 14
gageVecMGFrob = 27
tenGlyphTypeSphere = 2
limnPrimitiveTriangles = 2
echoTypeUnknown = -1
nrrdSpacingStatusUnknown = 0
tenDwiGageTensorMLEError = 19
pullEnergyTypeButterworthParabola = 9
nrrdOriginStatusOkay = 4
nrrdAxisInfoCenter = 7
pullPropNeighDistMean = 9
nrrdIoStateDetachedHeader = 1
tenGageOmegaHessianEvec = 128
nrrdTernaryOpUnknown = 0
limnPrimitiveNoop = 1
nrrdMeasureSkew = 13
tenGageCp1Hessian = 172
nrrdField_thicknesses = 11
tenGageModeHessianEvec0 = 120
dyeSpaceHSL = 2
nrrdTernaryOpMin = 3
miteValUnknown = 0
miteStageOpAdd = 3
nrrdMeasureLast = 27
airMopNever = 0
nrrdKind2DMaskedSymMatrix = 25
tenGageCovarianceRGRT = 147
pullCondConstraintSatA = 2
tenDwiGageLast = 36
airNoDio_small = 6
tenAniso_Ca1 = 4
tenGageEvalGrads = 72
tenGageNormGradMag = 37
nrrdUnaryOpOne = 28
pullInfoLast = 22
tenTripleTypeJ = 6
miteRangeBlue = 3
tenGageSNormal = 47
tenGageNorm = 4
nrrdBinaryOpUnknown = 0
gageSclHessEval0 = 10
pullEnergyTypeQuartic = 6
miteRangeLast = 9
limnSplineTypeBC = 5
gageKernel21 = 5
limnDeviceLast = 3
nrrdField_byte_skip = 26
gageSclK1 = 19
airNoDio_okay = 0
nrrdBinaryOpPow = 5
tenAniso_Th = 23
tenEstimate2MethodQSegLLS = 1
gageErrBoundsStack = 3
nrrdTypeUShort = 4
gageParmKernelIntegralNearZero = 7
nrrdKindComplex = 11
echoTypeSphere = 0
nrrdTernaryOpIfElse = 6
tenGageQGradVec = 48
nrrdTypeLast = 12
tenGageThetaGradVec = 60
nrrdBlind8BitRangeTrue = 1
tenGageConfDiffusionAlign = 144
nrrdKindDomain = 1
tenDwiGageTensorNLSErrorLog = 16
limnPrimitiveUnknown = 0
nrrdUnaryOpCbrt = 15
airMopAlways = 3
nrrdHasNonExistTrue = 1
nrrdAxisInfoUnits = 10
tenGageDetHessian = 84
nrrdIoStateBzip2BlockSize = 9
tenInterpTypeLast = 11
gageCtxFlagLast = 7
echoJittableUnknown = -1
echoJitterNone = 0
tenDwiGage2TensorPeledError = 33
tenGageCa1Normal = 160
tenEstimate2MethodLast = 3
airInsane_pInfExists = 2
nrrdBoundaryBleed = 2
nrrdCenterNode = 1
tenGageB = 5
pullProcessModeNeighLearn = 2
nrrdSpaceScannerXYZTime = 8
pullEnergyTypeZero = 8
miteRangeEmissivity = 4
tenEstimate1MethodWLS = 2
tenGageEval0 = 16
limnSplineInfoLast = 7
nrrdHasNonExistLast = 4
echoMatterPhongKs = 2
pullInterTypeJustR = 1
nrrdUnaryOpRoundUp = 19
tenGageTensorGradRotE = 161
miteValView = 11
tenGageEval1 = 17
miteRangeKa = 5
nrrdBinaryOpAdd = 1
tenAniso_S = 20
tenAniso_Cl1 = 2
echoJittableNormalB = 4
nrrdBasicInfoSpaceOrigin = 10
nrrdTypeDefault = 0
airEndianBig = 4321
tenGageOmegaGradVecDotEvec0 = 140
nrrdBinaryOpMin = 10
gageSclGradMag = 3
nrrdBasicInfoContent = 5
limnPolyDataInfoNorm = 2
gageErrNone = 1
tenGageOmegaNormal = 65
tenGageBNormal = 41
pullInfoTangent2 = 15
tenFiberTypeLast = 7
tenGageOmegaHessianEval = 124
tenFiberStopMinNumSteps = 10
tenGageFADiffusionFraction = 139
echoMatterGlassKa = 1
tenAniso_eval1 = 28
tenDwiGageTensorWLSLikelihood = 13
tenGlyphTypeBox = 1
nrrdField_block_size = 5
gageSclLast = 33
tenGageOmegaLaplacian = 132
hooverErrInit = 1
tenGageCl1HessianEval1 = 166
tenFiberIntgEuler = 1
limnSpaceUnknown = 0
ell_cubic_root_single_double = 3
gageVecLength = 5
hooverErrThreadCreate = 3
gageErrUnknown = 0
tenGageModeGradMag = 58
limnSpaceScreen = 3
tenGageCl2 = 77
tenGageOmegaDiffusionAlign = 141
pullInfoIsovalue = 17
nrrdTernaryOpMultiply = 2
limnEdgeTypeFrontFacet = 5
tenAniso_FA = 15
seekTypeValleyLine = 5
tenGageHessian = 81
tenGageOmegaHessianEval2 = 127
nrrdBinaryOpIf = 20
pullStatusNixMe = 3
gagePvlFlagVolume = 1
nrrdBoundaryUnknown = 0
nrrdMeasureSum = 7
nrrdMeasureHistoL2 = 24
tenTripleTypeK = 7
tenGageCa1HessianEval0 = 183
limnSplineInfoUnknown = 0
nrrdUnaryOpUnknown = 0
tenGageFARidgeSurfaceAlignment = 101
airFP_NEG_INF = 4
nrrdKind4Vector = 22
nrrdUnaryOpAtan = 8
tenGageTraceGradVecDotEvec0 = 134
gageVecMultiGrad = 26
tenGageQ = 8
limnSplineInfo3Vector = 3
nrrdField_labels = 17
gageVecProjHelGradient = 22
nrrdUnaryOpSgn = 22
tenGageCa1GradVec = 158
pullInfoInside = 4
nrrdField_content = 2
tenAniso_B = 17
pullStatusEdge = 4
nrrdFormatTypePNM = 2
miteValXi = 2
tenGageEvec2 = 22
miteShadeMethodPhong = 2
nrrdBinaryOpAtan2 = 9
tenInterpTypeUnknown = 0
tenDwiGageTensorLLS = 6
gageSclHessEvec = 13
nrrdBasicInfoSpaceDimension = 8
dyeSpaceXYZ = 4
nrrdTypeULLong = 8
airMopOnOkay = 2
tenGageFAHessianEvec2 = 95
tenDwiFiberType1Evec0 = 1
tenEstimate1MethodLast = 5
nrrdFormatTypeVTK = 4
tenGageTraceGradMag = 34
tenFiberStopRadius = 5
tenGageFARidgeSurfaceStrength = 96
gageVecLambda2 = 13
airNoDio_format = 2
nrrdSpace3DLeftHandedTime = 12
nrrdMeasureMode = 5
tenFiberTypeUnknown = 0
pullInfoLiveThresh = 12
gageKernelUnknown = 0
gageSclHessValleyness = 30
nrrdSpacingStatusLast = 5
airNoDio_arch = 1
nrrdSpaceLeftPosteriorSuperior = 3
tenGageCl1Hessian = 163
tenGageRotTans = 70
nrrdField_endian = 23
hooverErrThreadJoin = 9
airTypeLast = 12
tenGageOmegaHessianEvec1 = 130
tenInterpTypeAffineInvariant = 3
dyeSpaceLUV = 6
tenGageCl1HessianEvec0 = 169
nrrdIoStateZlibLevel = 7
tenGageQGradMag = 49
tenDwiGageFA = 27
limnEdgeTypeBackFacet = 1
miteStageOpMultiply = 4
nrrdField_dimension = 6
echoTypeIsosurface = 7
nrrdMeasureHistoMode = 21
tenAniso_eval0 = 27
tenGageCp1HessianEvec2 = 180
miteStageOpMin = 1
tenEstimate1MethodUnknown = 0
nrrdUnaryOpCos = 4
tenAniso_Ct1 = 7
tenGageCp1Normal = 157
miteRangeSP = 8
airInsane_UCSize = 10
tenGageFATotalCurv = 106
tenGageBGradVec = 39
tenGageMode = 11
tenAniso_RA = 14
tenGageModeNormal = 59
echoMatterUnknown = 0
nrrdZlibStrategyHuffman = 2
tenGageDelNormR1 = 25
limnSplineTypeLinear = 1
hooverErrRenderEnd = 10
tenDwiGageB0 = 2
tenGageQHessian = 86
nrrdSpacingStatusScalarNoSpace = 2
nrrdMeasureLinf = 10
nrrdBinaryOpMax = 11
gageParmUnknown = 0
limnCameraPathTrackLast = 4
tenGageClpmin1 = 76
nrrdKindScalar = 10
tenFiberStopAniso = 1
nrrdSpaceRightAnteriorSuperior = 1
nrrdTernaryOpInClosed = 10
nrrdHasNonExistFalse = 0
tenGageOmegaHessianEval0 = 125
limnCameraPathTrackBoth = 3
tenAniso_Skew = 21
nrrdKindSpace = 2
limnPrimitiveQuads = 5
tenFiberParmVerbose = 4
gageSclCurvDir2 = 27
tenGageTraceDiffusionFraction = 136
tenGageR = 10
echoMatterPhongKd = 1
gageSclMeanCurv = 24
limnQNUnknown = 0
pullInterTypeUnknown = 0
hooverErrSample = 6
tenDwiGage2TensorQSegAndError = 31
ell_cubic_root_three = 4
nrrdOriginStatusNoMaxOrSpacing = 3
nrrdFormatTypePNG = 3
tenFiberParmUnknown = 0
tenInterpTypeGeoLoxR = 6
tenDwiGageTensorErrorLog = 24
echoMatterMetalKd = 2
hooverErrRayBegin = 5
tenFiberIntgMidpoint = 2
pullInfoTensorInverse = 2
nrrdSpaceLeftAnteriorSuperiorTime = 5
seekTypeRidgeSurfaceT = 9
tenGageCp1HessianEvec = 177
miteStageOpMax = 2
tenGageModeHessianEval2 = 118
limnQN9octa = 14
tenAniso_Cp1 = 3
gageCtxFlagRadius = 5
pullEnergyTypeCotan = 4
echoTypeCube = 3
nrrdUnaryOpLast = 29
airNoDio_setfl = 10
airInsane_FltDblFPClass = 5
tenTripleTypeRThetaPhi = 5
tenAniso_Clpmin2 = 11
tenGageOmegaHessian = 123
nrrdAxisInfoKind = 8
limnSpaceLast = 5
tenGageFAShapeIndex = 107
tenTripleTypeEigenvalue = 1
pullPropForce = 8
gageSclValue = 1
gageErrLast = 6
nrrdKind2Vector = 12
nrrdBinaryOpGTE = 15
tenGageTraceDiffusionAlign = 135
nrrdOriginStatusNoMin = 2
pullCondUnknown = 0
tenGageCa1HessianEval1 = 184
pullPropStepConstr = 5
gageParmVerbose = 1
tenGageFADiffusionAlign = 138
tenGageFAValleySurfaceStrength = 97
nrrdField_comment = 1
nrrdBasicInfoUnknown = 0
seekTypeValleySurfaceOP = 10
tenGageFAFlowlineCurv = 112
airTypeEnum = 10
nrrdUnaryOpNormalRand = 25
gageCtxFlagNeedK = 3
tenInterpTypeLogLinear = 2
tenDwiGageTensorMLELikelihood = 21
nrrdField_last = 33
gageVecDirHelDeriv = 21
tenTripleTypeR = 8
nrrdMeasureLineIntercept = 15
nrrdTernaryOpExists = 8
tenGageCp1HessianEval0 = 174
gageParmLast = 13
pullInfoHessian = 3
tenGageFAGradVecDotEvec0 = 137
limnQN15octa = 5
nrrdField_number = 3
tenGageCa2 = 79
gageVecHessian = 15
nrrdKind3Vector = 19
tenGageTensorLogEuclidean = 149
nrrdBoundaryWrap = 3
nrrdField_type = 4
airNoDio_ptr = 8
echoTypeLast = 12
airFP_POS_DENORM = 7
tenGageTensorGradMag = 31
pullValLast = 3
airFP_NEG_DENORM = 8
limnSplineInfoQuaternion = 6
miteShadeMethodUnknown = 0
limnQN14checker = 6
echoJittableMotionA = 5
nrrdAxisInfoMax = 5
seekTypeRidgeLine = 4
gageErrStackIntegral = 4
nrrdBinaryOpFmod = 8
tenDwiGageTensorWLS = 10
echoMatterLight = 4
nrrdBasicInfoSampleUnits = 6
nrrdTypeLLong = 7
gageParmStackNormalizeRecon = 11
miteValYi = 4
tenGageOmegaGradMag = 64
echoTypeRectangle = 5
nrrdMeasureHistoSum = 23
nrrdUnaryOpFloor = 18
gageVecVector1 = 3
gageSclTotalCurv = 21
gageParmStackUse = 9
pullCondOld = 1
gageVecUnknown = 0
tenFiberTypePureLine = 5
pullCondLast = 8
tenGageBGradMag = 40
nrrdMeasureMean = 3
gageSclUnknown = 0
gageVecVector = 1
tenGageFACurvDir1 = 110
nrrdUnaryOpCeil = 17
limnDeviceUnknown = 0
tenGageCp1 = 74
tenDwiGageTensorMLE = 18
nrrdEncodingTypeBzip2 = 5
tenEstimate1MethodMLE = 4
nrrdField_kinds = 16
nrrdMeasureSD = 12
hooverErrThreadEnd = 8
tenGageCl1HessianEvec2 = 171
tenGageModeHessianEval0 = 116
tenGageFAHessianEval1 = 90
nrrdField_space_directions = 14
limnSpaceView = 2
nrrdKindNormal = 8
nrrdUnaryOpIf = 26
nrrdBoundaryLast = 5
tenGageModeWarp = 13
pullInfoIsovalueGradient = 18
tenGageFAGradMag = 52
gageVecDivergence = 8
airFP_Unknown = 0
miteValRi = 8
tenGageCovariance = 146
pullCondConstraintFail = 5
tenGageFAMeanCurv = 108
tenDwiGage2TensorPeledLevmarInfo = 35
miteRangeRed = 1
nrrdSpace3DRightHandedTime = 11
tenGageQNormal = 50
seekTypeLast = 12
airTypeUInt = 3
tenGageFARidgeLineAlignment = 100
tenGageModeHessianEvec1 = 121
nrrdBasicInfoSpace = 7
nrrdUnaryOpAcos = 7
limnQN13octa = 8
airInsane_32Bit = 9
nrrdMeasureHistoMean = 19
tenGageCa1HessianEvec1 = 188
limnSplineInfo2Vector = 2
nrrdBasicInfoComments = 14
nrrdField_old_min = 21
pullCondEnergyBad = 6
echoMatterGlass = 2
miteRangeKs = 7
miteValVdefTdotV = 18
tenEstimate1MethodLLS = 1
gageKernel20 = 4
airNoDio_fd = 4
nrrdBasicInfoMeasurementFrame = 11
echoMatterMetal = 3
nrrdMeasureL2 = 9
nrrdBinaryOpDivide = 4
tenAniso_Mode = 22
tenDwiGageMeanDWIValue = 5
nrrdTypeShort = 3
nrrdKindList = 4
nrrdTernaryOpInOpen = 9
gageVecVector0 = 2
echoJittablePixel = 0
nrrdBasicInfoSpaceUnits = 9
echoMatterLightPower = 0
nrrdTypeBlock = 11
gageParmStackNormalizeDeriv = 10
nrrdBinaryOpNotEqual = 18
pullInfoHeightGradient = 7
nrrdIoStateValsPerLine = 4
tenAnisoLast = 30
nrrdUnaryOpLog10 = 12
tenDwiGageTensorNLSError = 15
nrrdSpaceLeftAnteriorSuperior = 2
tenFiberStopBounds = 6
gageVecCurl = 9
nrrdBinaryOpGT = 14
nrrdOriginStatusUnknown = 0
pullProcessModeLast = 5
nrrdSpacingStatusScalarWithSpace = 3
nrrdHasNonExistOnly = 2
tenGageFAKappa2 = 105
limnQN16checker = 3
gageKernelLast = 8
hooverErrThreadBegin = 4
tenGageCl1HessianEvec = 168
pullEnergyTypeButterworth = 3
tenFiberIntgLast = 4
dyeSpaceLast = 7
pullPropPosition = 7
tenDwiGage2TensorPeled = 32
limnDeviceGL = 2
tenInterpTypeQuatGeoLoxR = 10
tenGageThetaNormal = 62
tenGageSHessian = 85
nrrdHasNonExistUnknown = 3
nrrdField_axis_mins = 12
tenGageConfGradVecDotEvec0 = 143
gageSclHessEval = 9
limnEdgeTypeLast = 8
seekTypeRidgeSurfaceOP = 8
airTypeLongInt = 4
tenGageDelNormPhi2 = 28
nrrdMeasureHistoMax = 18
nrrdUnaryOpSin = 3
gageVecGradient2 = 25
nrrdMeasureHistoSD = 26
nrrdUnaryOpLog = 10
tenAniso_Cs1 = 6
tenGageCa1HessianEval2 = 185
tenAniso_Q = 18
airFP_SNAN = 1
gageVecGradient1 = 24
nrrdMeasureProduct = 6
limnCameraPathTrackFrom = 1
pullInfoUnknown = 0
miteValTw = 9
airNoDio_disable = 12
tenAniso_R = 19
tenDwiGageAll = 1
nrrdAxisInfoSpaceDirection = 6
nrrdBinaryOpLT = 12
gageSclNPerp = 5
pullInfoQuality = 21
pullInterTypeLast = 5
gageSclHessEvec1 = 15
nrrdFormatTypeLast = 7
tenTripleTypeXYZ = 3
nrrdOriginStatusLast = 5
nrrdKindStub = 9
tenFiberStopStub = 8
echoMatterGlassFuzzy = 3
tenTripleTypeRThetaZ = 4
gageParmK3Pack = 4
nrrdMeasureL1 = 8
tenGlyphTypeSuperquad = 4
tenFiberTypeEvec0 = 1
pullPropUnknown = 0
gageSclHessMode = 32
pullProcessModeUnknown = 0
tenGageLast = 191
tenGageCl1HessianEval0 = 165
pullInfoHeightHessian = 8
ell_cubic_root_last = 5
nrrdSpaceUnknown = 0
pullPropEnergy = 3
echoJittableLens = 2
nrrdKind2DMatrix = 26
tenInterpTypeGeoLoxK = 5
limnCameraPathTrackUnknown = 0
pullPropLast = 11
pullStatusUnknown = 0
airFP_Last = 11
tenFiberStopMinLength = 9
nrrdTernaryOpAdd = 1
tenDwiGage2TensorQSeg = 29
limnEdgeTypeFrontCrease = 4
tenGageFAGradVec = 51
tenGageThetaGradMag = 61
nrrdAxisInfoSpacing = 2
gageVecLast = 30
nrrdBinaryOpNormalRandScaleAdd = 21
tenGageCa1HessianEval = 182
tenGageModeHessianEvec2 = 122
pullEnergyTypeSpring = 1
nrrdBoundaryPad = 1
tenGageS = 7
tenGageModeHessianEval1 = 117
tenFiberTypeEvec2 = 3
nrrdUnaryOpExists = 23
miteValGTdotV = 16
miteRangeUnknown = -1
nrrdBlind8BitRangeLast = 4
tenTripleTypeMoment = 2
airNoDio_std = 3
limnSplineTypeCubicBezier = 4
tenAniso_VF = 16
gageVecNormalized = 6
tenGageFAHessian = 87
seekTypeValleySurface = 3
tenGageDelNormK2 = 23
nrrdBinaryOpCompare = 16
tenDwiGageTensorLLSLikelihood = 9
gageSclHessEval2 = 12
nrrdField_encoding = 24
tenFiberTypeTensorLine = 4
nrrdAxisInfoThickness = 3
echoJittableLight = 1
tenDwiGage2TensorQSegError = 30
limnQN10octa = 13
tenFiberStopConfidence = 4
pullCondConstraintSatB = 3
tenGageRGradMag = 55
tenInterpTypeWang = 4
tenGageInvarRGradMags = 69
limnQN11octa = 11
limnPrimitiveLast = 8
tenFiberParmUseIndexSpace = 2
gageSclMedian = 29
nrrdAxisInfoUnknown = 0
airMopOnError = 1
seekTypeValleySurfaceT = 11
nrrdKindLast = 32
gageCtxFlagK3Pack = 2
tenInterpTypeLinear = 1
echoMatterLightUnit = 1
tenAniso_Det = 25
nrrdTypeDouble = 10
gageVecHelGradient = 20
nrrdIoStateCharsPerLine = 3
nrrdKindCovariantVector = 7
tenGageTraceHessian = 82
nrrdBlind8BitRangeUnknown = 0
dyeSpaceUnknown = 0
echoMatterMetalR0 = 0
airEndianLittle = 1234
nrrdMeasureHistoMedian = 20
echoMatterMetalKa = 1
miteRangeAlpha = 0
nrrdSpace3DRightHanded = 9
tenGageCp1HessianEvec1 = 179
pullStatusLast = 5
tenGageTensor = 1
tenGageNormNormal = 38
airTypeBool = 1
tenDwiGageConfidence = 26
airFP_NEG_NORM = 6
tenGageCa1HessianEvec = 186
tenGageCl1Normal = 154
pullInfoLiveThresh2 = 13
pullValSliceGradVec = 2
pullValUnknown = 0
tenAniso_Ct2 = 13
limnSplineInfo4Vector = 5
echoTypeInstance = 11
pullInfoInsideGradient = 5
gageKernel22 = 6
airFP_NEG_ZERO = 10
gageCtxFlagNeedD = 1
nrrdField_min = 19
nrrdZlibStrategyDefault = 1
pullEnergyTypeCubic = 5
tenGageEval2 = 18
nrrdBlind8BitRangeState = 3
nrrdTypeUInt = 6
limnQN8checker = 15
nrrdField_unknown = 0
tenFiberStopNumSteps = 3
gageParmGradMagCurvMin = 5
nrrdKindVector = 6
limnPrimitiveTriangleFan = 4
gageVecNormHelicity = 12
tenFiberParmLast = 5
gageSclCurvDir1 = 26
tenGageInvarKGrads = 66
pullCondNew = 7
echoJitterRandom = 3
tenGageOmegaHessianEvec0 = 129
tenInterpTypeQuatGeoLoxK = 9
tenDwiGageTensorError = 23
hooverErrRayEnd = 7
tenGageClpmin2 = 80
limnSpaceWorld = 1
dyeSpaceLAB = 5
tenGageModeHessianEvec = 119
tenGageFAGeomTens = 103
nrrdMeasureHistoMin = 17
gageKernel00 = 1
nrrdEncodingTypeRaw = 1
gageCtxFlagShape = 6
limnPolyDataInfoUnknown = 0
pullStatusStuck = 1
nrrdBasicInfoBlocksize = 3
pullEnergyTypeLast = 10
nrrdCenterUnknown = 0
tenGageOmega = 14
tenGageOmegaGradVec = 63
pullInfoHeight = 6
airTypeInt = 2
pullEnergyTypeGauss = 2
miteValRw = 7
nrrdUnaryOpAsin = 6
airInsane_NaNExists = 4
nrrdCenterLast = 3
tenGageFA2ndDD = 102
tenGageTraceNormal = 35
tenTripleTypeUnknown = 0
tenAniso_Ca2 = 10
nrrdBasicInfoType = 2
tenGageFAHessianEval = 88
tenGageTheta = 12
pullInfoHeightLaplacian = 9
nrrdBasicInfoKeyValuePairs = 15
nrrdKindXYZColor = 16
miteRangeKd = 6
gageSclGeomTens = 18
tenGageDelNormPhi3 = 29
limnSplineTypeTimeWarp = 2
limnSplineInfoNormal = 4
gageVecHelicity = 11
nrrdField_space_origin = 30
nrrdBinaryOpMultiply = 3
nrrdAxisInfoMin = 4
nrrdTypeUChar = 2
limnQNLast = 17
nrrdZlibStrategyFiltered = 3
limnSplineTypeUnknown = 0
tenDwiGageTensorWLSErrorLog = 12
pullInterTypeAdditive = 4
nrrdFormatTypeText = 5
nrrdBasicInfoDimension = 4
nrrdAxisInfoSize = 1
miteValXw = 1
nrrdField_axis_maxs = 13
tenGageFAGaussCurv = 109
pullInfoIsovalueHessian = 19
gageSclShapeIndex = 23
nrrdBasicInfoData = 1
echoTypeAABBox = 8
gageSclHessEvec2 = 16
tenGageFALaplacian = 98
gageVecCurlNormGrad = 18
gageParmRenormalize = 2
hooverErrNone = 0
tenGageFACurvDir2 = 111
pullInfoStrength = 20
airTypeUnknown = 0
tenGageOmegaHessianEvec2 = 131
tenDwiGageTensorMLEErrorLog = 20
limnEdgeTypeBackCrease = 2
tenFiberIntgRK4 = 3
nrrdUnaryOpSqrt = 14
pullPropStuck = 6
nrrdFormatTypeEPS = 6
nrrdIoStateZlibStrategy = 8
nrrdMeasureLineSlope = 14
gageKernel11 = 3
nrrdEncodingTypeUnknown = 0
hooverErrLast = 11
tenGageCp1HessianEval = 173
tenGageModeHessian = 114
miteStageOpLast = 5
dyeSpaceRGB = 3
gageVecNCurlNormGrad = 19
echoJittableLast = 7
miteValZw = 5
gagePvlFlagUnknown = 0
tenGageUnknown = 0
airNoDio_dioinfo = 5
pullPropStepEnergy = 4
limnEdgeTypeLone = 7
seekTypeMaximalSurface = 7
nrrdKind3Gradient = 20
miteValZi = 6
nrrdUnaryOpReciprocal = 2
tenGageCovarianceKGRT = 148
gagePvlFlagNeedD = 3
tenGageTrace = 3
airNoDio_size = 7
nrrdSpace3DLeftHanded = 10
tenAniso_Clpmin1 = 5
miteRangeGreen = 2
tenGageDelNormR2 = 26
limnSplineTypeLast = 6
tenGageAniso = 190
airInsane_dio = 8
limnPolyDataInfoRGBA = 1
nrrdKind4Color = 17
nrrdUnaryOpErf = 16
tenEstimate1MethodNLS = 3
pullInfoTensor = 1
nrrdKind3Normal = 21
tenGageBHessian = 83
gageSclNormal = 4
gageErrStackSearch = 5
pullStatusNewbie = 2
nrrdSpaceRightAnteriorSuperiorTime = 4
nrrdBinaryOpLTE = 13
tenAniso_Omega = 24
tenGageModeGradVec = 57
tenDwiGageTensorLLSErrorLog = 8
echoMatterPhong = 1
miteValWdotD = 19
tenDwiFiberTypeLast = 4
gageErrBoundsSpace = 2
gageParmDefaultCenter = 8
limnQN8octa = 16
nrrdIoStateKeepNrrdDataFileOpen = 6
nrrdKindHSVColor = 15
gageSclK2 = 20
nrrdFormatTypeNRRD = 1
miteValNormal = 12
tenFiberStopFraction = 7
echoTypeTriangle = 4
tenFiberTypeZhukov = 6
gageParmCheckIntegrals = 3
tenGageRGradVec = 54
tenGageFAHessianEvalMode = 99
airInsane_AIR_NAN = 7
tenFiberTypeEvec1 = 2
tenGageOmega2ndDD = 133
nrrdField_space = 7
tenDwiGageTensorNLSLikelihood = 17
tenGageEvec0 = 20
nrrdField_old_max = 22
nrrdOriginStatusDirection = 1
pullPropIdCC = 2
airInsane_QNaNHiBit = 6
pullEnergyTypeUnknown = 0
tenGageTensorGrad = 30
nrrdMeasureVariance = 11
limnQN16simple = 1
limnEdgeTypeUnknown = 0
seekTypeUnknown = 0
tenGageCl1HessianEvec1 = 170
nrrdField_measurement_frame = 31
nrrdEncodingTypeLast = 6
tenGageRotTanMags = 71
gageKernel10 = 2
limnEdgeTypeContour = 3
tenGageConfDiffusionFraction = 145
limnQN12checker = 9
tenDwiGage2TensorPeledAndError = 34
tenAniso_Conf = 1
tenAnisoUnknown = 0
gageCtxFlagKernel = 4
airNoDio_test = 11
tenGageDet = 6
nrrdMeasureMax = 2
tenGageCa1HessianEvec0 = 187
tenInterpTypeLoxR = 8
pullEnergyTypeCubicWell = 7
nrrdUnaryOpLog1p = 13
pullInfoSeedPreThresh = 10
nrrdField_spacings = 10
nrrdKind3Color = 13
tenGageEvalHessian = 162
airTypeFloat = 6
miteShadeMethodLitTen = 3
nrrdBinaryOpEqual = 17
tenDwiGageADC = 4
gageVecJacobian = 7
echoJittableMotionB = 6
nrrdKindTime = 3
airEndianLast = 4322
gageVecMGEvec = 29
echoMatterLast = 5
gageScl2ndDD = 17
miteValLast = 20
airInsane_endian = 1
gageVecImaginaryPart = 14
echoMatterGlassKd = 2
nrrdField_data_file = 32
nrrdKindUnknown = 0
tenFiberStopLast = 11
echoMatterPhongKa = 0
tenGageInvarRGrads = 68
limnPrimitiveLines = 7
tenFiberParmWPunct = 3
tenAniso_eval2 = 29
tenDwiGageTensorNLS = 14
echoMatterPhongSp = 3
tenGageInvarKGradMags = 67
echoJitterJitter = 2
nrrdBinaryOpRicianRand = 22
tenGlyphTypeUnknown = 0
airTypeOther = 11
gageVecVector2 = 4
gageVecCurlNorm = 10
nrrdZlibStrategyLast = 4
nrrdIoStateLast = 10
airTypeString = 9
gageKernelStack = 7
tenGageCl1HessianEval2 = 167
pullCondEnergyTry = 4
airInsane_DLSize = 12
nrrdIoStateBareText = 2
tenGageCl1 = 73
tenGageFAKappa1 = 104
airFP_POS_INF = 3
airTypeSize_t = 5
tenGageFAHessianEvec0 = 93
nrrdBinaryOpMod = 7
tenGageOmegaDiffusionFraction = 142
nrrdTernaryOpClamp = 5
tenFiberIntgUnknown = 0
gageSclLaplacian = 7
ell_cubic_root_single = 1
tenEstimate2MethodUnknown = 0
nrrdMeasureHistoVariance = 25
gageSclHessian = 6
tenAniso_Cp2 = 9
nrrdKind3DSymMatrix = 28
airFP_POS_NORM = 5
tenGageTensorGradMagMag = 32
pullValSlice = 1
echoTypeList = 10
nrrdBasicInfoOldMin = 12
nrrdUnaryOpAbs = 21
tenGageEval = 15
tenGageEvec1 = 21
pullProcessModeDescent = 1
gageSclHessEvec0 = 14
nrrdTypeInt = 5
gageParmCurvNormalSide = 6
tenGageEvec = 19
pullInfoTangentMode = 16
nrrdTernaryOpLerp = 7
pullInterTypeSeparable = 3
nrrdKindPoint = 5
tenGageCl1HessianEval = 164
nrrdField_line_skip = 25
tenGageRHessian = 113
echoTypeCylinder = 1
tenGlyphTypeLast = 5
miteValTi = 10
tenFiberParmStepSize = 1
gageSclHessRidgeness = 31
tenGageFAHessianEval2 = 91
nrrdSpacingStatusNone = 1
limnQN16border1 = 2
echoJitterLast = 4
pullPropScale = 10
limnDevicePS = 1
echoMatterMetalFuzzy = 3
nrrdKind3DMaskedSymMatrix = 29
nrrdMeasureMedian = 4
tenDwiGageTensorAllDWIError = 28
nrrdEncodingTypeHex = 3
tenGageModeHessianEval = 115
airLLong = c_longlong
airULLong = c_ulonglong
class airEnum(Structure):
    pass
airEnum._fields_ = [
    ('name', STRING),
    ('M', c_uint),
    ('str', POINTER(STRING)),
    ('val', POINTER(c_int)),
    ('desc', POINTER(STRING)),
    ('strEqv', POINTER(STRING)),
    ('valEqv', POINTER(c_int)),
    ('sense', c_int),
]
airEnumUnknown = libteem.airEnumUnknown
airEnumUnknown.restype = c_int
airEnumUnknown.argtypes = [POINTER(airEnum)]
airEnumLast = libteem.airEnumLast
airEnumLast.restype = c_int
airEnumLast.argtypes = [POINTER(airEnum)]
airEnumValCheck = libteem.airEnumValCheck
airEnumValCheck.restype = c_int
airEnumValCheck.argtypes = [POINTER(airEnum), c_int]
airEnumStr = libteem.airEnumStr
airEnumStr.restype = STRING
airEnumStr.argtypes = [POINTER(airEnum), c_int]
airEnumDesc = libteem.airEnumDesc
airEnumDesc.restype = STRING
airEnumDesc.argtypes = [POINTER(airEnum), c_int]
airEnumVal = libteem.airEnumVal
airEnumVal.restype = c_int
airEnumVal.argtypes = [POINTER(airEnum), STRING]
airEnumFmtDesc = libteem.airEnumFmtDesc
airEnumFmtDesc.restype = STRING
airEnumFmtDesc.argtypes = [POINTER(airEnum), c_int, c_int, STRING]
airEnumPrint = libteem.airEnumPrint
airEnumPrint.restype = None
airEnumPrint.argtypes = [POINTER(FILE), POINTER(airEnum)]
airEndian = (POINTER(airEnum)).in_dll(libteem, 'airEndian')
airMyEndian = (c_int).in_dll(libteem, 'airMyEndian')
class airArray(Structure):
    pass
__darwin_size_t = c_ulong
size_t = __darwin_size_t
airArray._fields_ = [
    ('data', c_void_p),
    ('dataP', POINTER(c_void_p)),
    ('len', c_uint),
    ('lenP', POINTER(c_uint)),
    ('incr', c_uint),
    ('size', c_uint),
    ('unit', size_t),
    ('noReallocWhenSmaller', c_int),
    ('allocCB', CFUNCTYPE(c_void_p)),
    ('freeCB', CFUNCTYPE(c_void_p, c_void_p)),
    ('initCB', CFUNCTYPE(None, c_void_p)),
    ('doneCB', CFUNCTYPE(None, c_void_p)),
]
airArrayNew = libteem.airArrayNew
airArrayNew.restype = POINTER(airArray)
airArrayNew.argtypes = [POINTER(c_void_p), POINTER(c_uint), size_t, c_uint]
airArrayStructCB = libteem.airArrayStructCB
airArrayStructCB.restype = None
airArrayStructCB.argtypes = [POINTER(airArray), CFUNCTYPE(None, c_void_p), CFUNCTYPE(None, c_void_p)]
airArrayPointerCB = libteem.airArrayPointerCB
airArrayPointerCB.restype = None
airArrayPointerCB.argtypes = [POINTER(airArray), CFUNCTYPE(c_void_p), CFUNCTYPE(c_void_p, c_void_p)]
airArrayLenSet = libteem.airArrayLenSet
airArrayLenSet.restype = None
airArrayLenSet.argtypes = [POINTER(airArray), c_uint]
airArrayLenPreSet = libteem.airArrayLenPreSet
airArrayLenPreSet.restype = None
airArrayLenPreSet.argtypes = [POINTER(airArray), c_uint]
airArrayLenIncr = libteem.airArrayLenIncr
airArrayLenIncr.restype = c_uint
airArrayLenIncr.argtypes = [POINTER(airArray), c_int]
airArrayNix = libteem.airArrayNix
airArrayNix.restype = POINTER(airArray)
airArrayNix.argtypes = [POINTER(airArray)]
airArrayNuke = libteem.airArrayNuke
airArrayNuke.restype = POINTER(airArray)
airArrayNuke.argtypes = [POINTER(airArray)]
airThreadCapable = (c_int).in_dll(libteem, 'airThreadCapable')
airThreadNoopWarning = (c_int).in_dll(libteem, 'airThreadNoopWarning')
class _airThread(Structure):
    pass
airThread = _airThread
class _airThreadMutex(Structure):
    pass
airThreadMutex = _airThreadMutex
class _airThreadCond(Structure):
    pass
airThreadCond = _airThreadCond
class airThreadBarrier(Structure):
    pass
airThreadBarrier._fields_ = [
    ('numUsers', c_uint),
    ('numDone', c_uint),
    ('doneMutex', POINTER(airThreadMutex)),
    ('doneCond', POINTER(airThreadCond)),
]
airThreadNew = libteem.airThreadNew
airThreadNew.restype = POINTER(airThread)
airThreadNew.argtypes = []
airThreadStart = libteem.airThreadStart
airThreadStart.restype = c_int
airThreadStart.argtypes = [POINTER(airThread), CFUNCTYPE(c_void_p, c_void_p), c_void_p]
airThreadJoin = libteem.airThreadJoin
airThreadJoin.restype = c_int
airThreadJoin.argtypes = [POINTER(airThread), POINTER(c_void_p)]
airThreadNix = libteem.airThreadNix
airThreadNix.restype = POINTER(airThread)
airThreadNix.argtypes = [POINTER(airThread)]
airThreadMutexNew = libteem.airThreadMutexNew
airThreadMutexNew.restype = POINTER(airThreadMutex)
airThreadMutexNew.argtypes = []
airThreadMutexLock = libteem.airThreadMutexLock
airThreadMutexLock.restype = c_int
airThreadMutexLock.argtypes = [POINTER(airThreadMutex)]
airThreadMutexUnlock = libteem.airThreadMutexUnlock
airThreadMutexUnlock.restype = c_int
airThreadMutexUnlock.argtypes = [POINTER(airThreadMutex)]
airThreadMutexNix = libteem.airThreadMutexNix
airThreadMutexNix.restype = POINTER(airThreadMutex)
airThreadMutexNix.argtypes = [POINTER(airThreadMutex)]
airThreadCondNew = libteem.airThreadCondNew
airThreadCondNew.restype = POINTER(airThreadCond)
airThreadCondNew.argtypes = []
airThreadCondWait = libteem.airThreadCondWait
airThreadCondWait.restype = c_int
airThreadCondWait.argtypes = [POINTER(airThreadCond), POINTER(airThreadMutex)]
airThreadCondSignal = libteem.airThreadCondSignal
airThreadCondSignal.restype = c_int
airThreadCondSignal.argtypes = [POINTER(airThreadCond)]
airThreadCondBroadcast = libteem.airThreadCondBroadcast
airThreadCondBroadcast.restype = c_int
airThreadCondBroadcast.argtypes = [POINTER(airThreadCond)]
airThreadCondNix = libteem.airThreadCondNix
airThreadCondNix.restype = POINTER(airThreadCond)
airThreadCondNix.argtypes = [POINTER(airThreadCond)]
airThreadBarrierNew = libteem.airThreadBarrierNew
airThreadBarrierNew.restype = POINTER(airThreadBarrier)
airThreadBarrierNew.argtypes = [c_uint]
airThreadBarrierWait = libteem.airThreadBarrierWait
airThreadBarrierWait.restype = c_int
airThreadBarrierWait.argtypes = [POINTER(airThreadBarrier)]
airThreadBarrierNix = libteem.airThreadBarrierNix
airThreadBarrierNix.restype = POINTER(airThreadBarrier)
airThreadBarrierNix.argtypes = [POINTER(airThreadBarrier)]
class airFloat(Union):
    pass
airFloat._fields_ = [
    ('i', c_uint),
    ('f', c_float),
]
class airDouble(Union):
    pass
airDouble._pack_ = 4
airDouble._fields_ = [
    ('i', airULLong),
    ('d', c_double),
]
airMyQNaNHiBit = (c_int).in_dll(libteem, 'airMyQNaNHiBit')
airFPPartsToVal_f = libteem.airFPPartsToVal_f
airFPPartsToVal_f.restype = c_float
airFPPartsToVal_f.argtypes = [c_uint, c_uint, c_uint]
airFPValToParts_f = libteem.airFPValToParts_f
airFPValToParts_f.restype = None
airFPValToParts_f.argtypes = [POINTER(c_uint), POINTER(c_uint), POINTER(c_uint), c_float]
airFPPartsToVal_d = libteem.airFPPartsToVal_d
airFPPartsToVal_d.restype = c_double
airFPPartsToVal_d.argtypes = [c_uint, c_uint, c_uint, c_uint]
airFPValToParts_d = libteem.airFPValToParts_d
airFPValToParts_d.restype = None
airFPValToParts_d.argtypes = [POINTER(c_uint), POINTER(c_uint), POINTER(c_uint), POINTER(c_uint), c_double]
airFPGen_f = libteem.airFPGen_f
airFPGen_f.restype = c_float
airFPGen_f.argtypes = [c_int]
airFPGen_d = libteem.airFPGen_d
airFPGen_d.restype = c_double
airFPGen_d.argtypes = [c_int]
airFPClass_f = libteem.airFPClass_f
airFPClass_f.restype = c_int
airFPClass_f.argtypes = [c_float]
airFPClass_d = libteem.airFPClass_d
airFPClass_d.restype = c_int
airFPClass_d.argtypes = [c_double]
airFPFprintf_f = libteem.airFPFprintf_f
airFPFprintf_f.restype = None
airFPFprintf_f.argtypes = [POINTER(FILE), c_float]
airFPFprintf_d = libteem.airFPFprintf_d
airFPFprintf_d.restype = None
airFPFprintf_d.argtypes = [POINTER(FILE), c_double]
airFloatQNaN = (airFloat).in_dll(libteem, 'airFloatQNaN')
airFloatSNaN = (airFloat).in_dll(libteem, 'airFloatSNaN')
airFloatPosInf = (airFloat).in_dll(libteem, 'airFloatPosInf')
airFloatNegInf = (airFloat).in_dll(libteem, 'airFloatNegInf')
airNaN = libteem.airNaN
airNaN.restype = c_float
airNaN.argtypes = []
airIsNaN = libteem.airIsNaN
airIsNaN.restype = c_int
airIsNaN.argtypes = [c_double]
airIsInf_f = libteem.airIsInf_f
airIsInf_f.restype = c_int
airIsInf_f.argtypes = [c_float]
airIsInf_d = libteem.airIsInf_d
airIsInf_d.restype = c_int
airIsInf_d.argtypes = [c_double]
airExists = libteem.airExists
airExists.restype = c_int
airExists.argtypes = [c_double]
class airRandMTState(Structure):
    pass
airRandMTState._fields_ = [
    ('state', c_uint * 624),
    ('pNext', POINTER(c_uint)),
    ('left', c_uint),
]
airRandMTStateGlobal = (POINTER(airRandMTState)).in_dll(libteem, 'airRandMTStateGlobal')
airRandMTStateGlobalInit = libteem.airRandMTStateGlobalInit
airRandMTStateGlobalInit.restype = None
airRandMTStateGlobalInit.argtypes = []
airRandMTStateNew = libteem.airRandMTStateNew
airRandMTStateNew.restype = POINTER(airRandMTState)
airRandMTStateNew.argtypes = [c_uint]
airRandMTStateNix = libteem.airRandMTStateNix
airRandMTStateNix.restype = POINTER(airRandMTState)
airRandMTStateNix.argtypes = [POINTER(airRandMTState)]
airSrandMT_r = libteem.airSrandMT_r
airSrandMT_r.restype = None
airSrandMT_r.argtypes = [POINTER(airRandMTState), c_uint]
airDrandMT_r = libteem.airDrandMT_r
airDrandMT_r.restype = c_double
airDrandMT_r.argtypes = [POINTER(airRandMTState)]
airUIrandMT_r = libteem.airUIrandMT_r
airUIrandMT_r.restype = c_uint
airUIrandMT_r.argtypes = [POINTER(airRandMTState)]
airDrandMT53_r = libteem.airDrandMT53_r
airDrandMT53_r.restype = c_double
airDrandMT53_r.argtypes = [POINTER(airRandMTState)]
airRandInt = libteem.airRandInt
airRandInt.restype = c_uint
airRandInt.argtypes = [c_uint]
airRandInt_r = libteem.airRandInt_r
airRandInt_r.restype = c_uint
airRandInt_r.argtypes = [POINTER(airRandMTState), c_uint]
airSrandMT = libteem.airSrandMT
airSrandMT.restype = None
airSrandMT.argtypes = [c_uint]
airDrandMT = libteem.airDrandMT
airDrandMT.restype = c_double
airDrandMT.argtypes = []
airAtod = libteem.airAtod
airAtod.restype = c_double
airAtod.argtypes = [STRING]
airSingleSscanf = libteem.airSingleSscanf
airSingleSscanf.restype = c_int
airSingleSscanf.argtypes = [STRING, STRING, c_void_p]
airBool = (POINTER(airEnum)).in_dll(libteem, 'airBool')
airParseStrB = libteem.airParseStrB
airParseStrB.restype = c_uint
airParseStrB.argtypes = [POINTER(c_int), STRING, STRING, c_uint]
airParseStrI = libteem.airParseStrI
airParseStrI.restype = c_uint
airParseStrI.argtypes = [POINTER(c_int), STRING, STRING, c_uint]
airParseStrUI = libteem.airParseStrUI
airParseStrUI.restype = c_uint
airParseStrUI.argtypes = [POINTER(c_uint), STRING, STRING, c_uint]
airParseStrZ = libteem.airParseStrZ
airParseStrZ.restype = c_uint
airParseStrZ.argtypes = [POINTER(size_t), STRING, STRING, c_uint]
airParseStrF = libteem.airParseStrF
airParseStrF.restype = c_uint
airParseStrF.argtypes = [POINTER(c_float), STRING, STRING, c_uint]
airParseStrD = libteem.airParseStrD
airParseStrD.restype = c_uint
airParseStrD.argtypes = [POINTER(c_double), STRING, STRING, c_uint]
airParseStrC = libteem.airParseStrC
airParseStrC.restype = c_uint
airParseStrC.argtypes = [STRING, STRING, STRING, c_uint]
airParseStrS = libteem.airParseStrS
airParseStrS.restype = c_uint
airParseStrS.argtypes = [POINTER(STRING), STRING, STRING, c_uint]
airParseStrE = libteem.airParseStrE
airParseStrE.restype = c_uint
airParseStrE.argtypes = [POINTER(c_int), STRING, STRING, c_uint]
airParseStr = (CFUNCTYPE(c_uint, c_void_p, STRING, STRING, c_uint) * 12).in_dll(libteem, 'airParseStr')
airStrdup = libteem.airStrdup
airStrdup.restype = STRING
airStrdup.argtypes = [STRING]
airStrlen = libteem.airStrlen
airStrlen.restype = size_t
airStrlen.argtypes = [STRING]
airStrtokQuoting = (c_int).in_dll(libteem, 'airStrtokQuoting')
airStrtok = libteem.airStrtok
airStrtok.restype = STRING
airStrtok.argtypes = [STRING, STRING, POINTER(STRING)]
airStrntok = libteem.airStrntok
airStrntok.restype = c_uint
airStrntok.argtypes = [STRING, STRING]
airStrtrans = libteem.airStrtrans
airStrtrans.restype = STRING
airStrtrans.argtypes = [STRING, c_char, c_char]
airEndsWith = libteem.airEndsWith
airEndsWith.restype = c_int
airEndsWith.argtypes = [STRING, STRING]
airUnescape = libteem.airUnescape
airUnescape.restype = STRING
airUnescape.argtypes = [STRING]
airOneLinify = libteem.airOneLinify
airOneLinify.restype = STRING
airOneLinify.argtypes = [STRING]
airToLower = libteem.airToLower
airToLower.restype = STRING
airToLower.argtypes = [STRING]
airToUpper = libteem.airToUpper
airToUpper.restype = STRING
airToUpper.argtypes = [STRING]
airOneLine = libteem.airOneLine
airOneLine.restype = c_uint
airOneLine.argtypes = [POINTER(FILE), STRING, c_int]
airInsaneErr = libteem.airInsaneErr
airInsaneErr.restype = STRING
airInsaneErr.argtypes = [c_int]
airSanity = libteem.airSanity
airSanity.restype = c_int
airSanity.argtypes = []
airTeemVersion = (STRING).in_dll(libteem, 'airTeemVersion')
airTeemReleaseDate = (STRING).in_dll(libteem, 'airTeemReleaseDate')
airNull = libteem.airNull
airNull.restype = c_void_p
airNull.argtypes = []
airSetNull = libteem.airSetNull
airSetNull.restype = c_void_p
airSetNull.argtypes = [POINTER(c_void_p)]
airFree = libteem.airFree
airFree.restype = c_void_p
airFree.argtypes = [c_void_p]
airFopen = libteem.airFopen
airFopen.restype = POINTER(FILE)
airFopen.argtypes = [STRING, POINTER(FILE), STRING]
airFclose = libteem.airFclose
airFclose.restype = POINTER(FILE)
airFclose.argtypes = [POINTER(FILE)]
airSinglePrintf = libteem.airSinglePrintf
airSinglePrintf.restype = c_int
airSinglePrintf.argtypes = [POINTER(FILE), STRING, STRING]
airMy32Bit = (c_int).in_dll(libteem, 'airMy32Bit')
airStderr = libteem.airStderr
airStderr.restype = POINTER(FILE)
airStderr.argtypes = []
airStdout = libteem.airStdout
airStdout.restype = POINTER(FILE)
airStdout.argtypes = []
airStdin = libteem.airStdin
airStdin.restype = POINTER(FILE)
airStdin.argtypes = []
airIndex = libteem.airIndex
airIndex.restype = c_uint
airIndex.argtypes = [c_double, c_double, c_double, c_uint]
airIndexClamp = libteem.airIndexClamp
airIndexClamp.restype = c_uint
airIndexClamp.argtypes = [c_double, c_double, c_double, c_uint]
airIndexULL = libteem.airIndexULL
airIndexULL.restype = airULLong
airIndexULL.argtypes = [c_double, c_double, c_double, airULLong]
airIndexClampULL = libteem.airIndexClampULL
airIndexClampULL.restype = airULLong
airIndexClampULL.argtypes = [c_double, c_double, c_double, airULLong]
airMyFmt_size_t = (c_char * 0).in_dll(libteem, 'airMyFmt_size_t')
airDoneStr = libteem.airDoneStr
airDoneStr.restype = STRING
airDoneStr.argtypes = [c_double, c_double, c_double, STRING]
airTime = libteem.airTime
airTime.restype = c_double
airTime.argtypes = []
airBinaryPrintUInt = libteem.airBinaryPrintUInt
airBinaryPrintUInt.restype = None
airBinaryPrintUInt.argtypes = [POINTER(FILE), c_int, c_uint]
airTypeStr = (c_char * 129 * 12).in_dll(libteem, 'airTypeStr')
airTypeSize = (size_t * 12).in_dll(libteem, 'airTypeSize')
airILoad = libteem.airILoad
airILoad.restype = c_int
airILoad.argtypes = [c_void_p, c_int]
airFLoad = libteem.airFLoad
airFLoad.restype = c_float
airFLoad.argtypes = [c_void_p, c_int]
airDLoad = libteem.airDLoad
airDLoad.restype = c_double
airDLoad.argtypes = [c_void_p, c_int]
airIStore = libteem.airIStore
airIStore.restype = c_int
airIStore.argtypes = [c_void_p, c_int, c_int]
airFStore = libteem.airFStore
airFStore.restype = c_float
airFStore.argtypes = [c_void_p, c_int, c_float]
airDStore = libteem.airDStore
airDStore.restype = c_double
airDStore.argtypes = [c_void_p, c_int, c_double]
airEqvAdd = libteem.airEqvAdd
airEqvAdd.restype = None
airEqvAdd.argtypes = [POINTER(airArray), c_uint, c_uint]
airEqvMap = libteem.airEqvMap
airEqvMap.restype = c_uint
airEqvMap.argtypes = [POINTER(airArray), POINTER(c_uint), c_uint]
airEqvSettle = libteem.airEqvSettle
airEqvSettle.restype = c_uint
airEqvSettle.argtypes = [POINTER(c_uint), c_uint]
airFastExp = libteem.airFastExp
airFastExp.restype = c_double
airFastExp.argtypes = [c_double]
airNormalRand = libteem.airNormalRand
airNormalRand.restype = None
airNormalRand.argtypes = [POINTER(c_double), POINTER(c_double)]
airNormalRand_r = libteem.airNormalRand_r
airNormalRand_r.restype = None
airNormalRand_r.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(airRandMTState)]
airShuffle = libteem.airShuffle
airShuffle.restype = None
airShuffle.argtypes = [POINTER(c_uint), c_uint, c_int]
airShuffle_r = libteem.airShuffle_r
airShuffle_r.restype = None
airShuffle_r.argtypes = [POINTER(airRandMTState), POINTER(c_uint), c_uint, c_int]
airCbrt = libteem.airCbrt
airCbrt.restype = c_double
airCbrt.argtypes = [c_double]
airMode3 = libteem.airMode3
airMode3.restype = c_double
airMode3.argtypes = [c_double, c_double, c_double]
airMode3_d = libteem.airMode3_d
airMode3_d.restype = c_double
airMode3_d.argtypes = [POINTER(c_double)]
airSgnPow = libteem.airSgnPow
airSgnPow.restype = c_double
airSgnPow.argtypes = [c_double, c_double]
airIntPow = libteem.airIntPow
airIntPow.restype = c_double
airIntPow.argtypes = [c_double, c_int]
airSgn = libteem.airSgn
airSgn.restype = c_int
airSgn.argtypes = [c_double]
airLog2 = libteem.airLog2
airLog2.restype = c_int
airLog2.argtypes = [c_double]
airErfc = libteem.airErfc
airErfc.restype = c_double
airErfc.argtypes = [c_double]
airErf = libteem.airErf
airErf.restype = c_double
airErf.argtypes = [c_double]
airGaussian = libteem.airGaussian
airGaussian.restype = c_double
airGaussian.argtypes = [c_double, c_double, c_double]
airBesselI0 = libteem.airBesselI0
airBesselI0.restype = c_double
airBesselI0.argtypes = [c_double]
airBesselI1 = libteem.airBesselI1
airBesselI1.restype = c_double
airBesselI1.argtypes = [c_double]
airBesselI0ExpScaled = libteem.airBesselI0ExpScaled
airBesselI0ExpScaled.restype = c_double
airBesselI0ExpScaled.argtypes = [c_double]
airBesselI1ExpScaled = libteem.airBesselI1ExpScaled
airBesselI1ExpScaled.restype = c_double
airBesselI1ExpScaled.argtypes = [c_double]
airLogBesselI0 = libteem.airLogBesselI0
airLogBesselI0.restype = c_double
airLogBesselI0.argtypes = [c_double]
airLogRician = libteem.airLogRician
airLogRician.restype = c_double
airLogRician.argtypes = [c_double, c_double, c_double]
airRician = libteem.airRician
airRician.restype = c_double
airRician.argtypes = [c_double, c_double, c_double]
airBesselI1By0 = libteem.airBesselI1By0
airBesselI1By0.restype = c_double
airBesselI1By0.argtypes = [c_double]
airBesselIn = libteem.airBesselIn
airBesselIn.restype = c_double
airBesselIn.argtypes = [c_int, c_double]
airBesselInExpScaled = libteem.airBesselInExpScaled
airBesselInExpScaled.restype = c_double
airBesselInExpScaled.argtypes = [c_int, c_double]
airNoDioErr = libteem.airNoDioErr
airNoDioErr.restype = STRING
airNoDioErr.argtypes = [c_int]
airMyDio = (c_int).in_dll(libteem, 'airMyDio')
airDisableDio = (c_int).in_dll(libteem, 'airDisableDio')
airDioInfo = libteem.airDioInfo
airDioInfo.restype = None
airDioInfo.argtypes = [POINTER(c_int), POINTER(c_int), POINTER(c_int), c_int]
airDioTest = libteem.airDioTest
airDioTest.restype = c_int
airDioTest.argtypes = [c_int, c_void_p, size_t]
airDioMalloc = libteem.airDioMalloc
airDioMalloc.restype = c_void_p
airDioMalloc.argtypes = [size_t, c_int]
airDioRead = libteem.airDioRead
airDioRead.restype = size_t
airDioRead.argtypes = [c_int, c_void_p, size_t]
airDioWrite = libteem.airDioWrite
airDioWrite.restype = size_t
airDioWrite.argtypes = [c_int, c_void_p, size_t]
airMopper = CFUNCTYPE(c_void_p, c_void_p)
class airMop(Structure):
    pass
airMop._fields_ = [
    ('ptr', c_void_p),
    ('mop', airMopper),
    ('when', c_int),
]
airMopNew = libteem.airMopNew
airMopNew.restype = POINTER(airArray)
airMopNew.argtypes = []
airMopAdd = libteem.airMopAdd
airMopAdd.restype = c_int
airMopAdd.argtypes = [POINTER(airArray), c_void_p, airMopper, c_int]
airMopSub = libteem.airMopSub
airMopSub.restype = None
airMopSub.argtypes = [POINTER(airArray), c_void_p, airMopper]
airMopMem = libteem.airMopMem
airMopMem.restype = None
airMopMem.argtypes = [POINTER(airArray), c_void_p, c_int]
airMopUnMem = libteem.airMopUnMem
airMopUnMem.restype = None
airMopUnMem.argtypes = [POINTER(airArray), c_void_p]
airMopPrint = libteem.airMopPrint
airMopPrint.restype = None
airMopPrint.argtypes = [POINTER(airArray), c_void_p, c_int]
airMopDone = libteem.airMopDone
airMopDone.restype = None
airMopDone.argtypes = [POINTER(airArray), c_int]
airMopError = libteem.airMopError
airMopError.restype = None
airMopError.argtypes = [POINTER(airArray)]
airMopOkay = libteem.airMopOkay
airMopOkay.restype = None
airMopOkay.argtypes = [POINTER(airArray)]
airMopDebug = libteem.airMopDebug
airMopDebug.restype = None
airMopDebug.argtypes = [POINTER(airArray)]
class biffMsg(Structure):
    pass
biffMsg._fields_ = [
    ('key', STRING),
    ('err', POINTER(STRING)),
    ('errNum', c_uint),
    ('errArr', POINTER(airArray)),
]
biffMsgNew = libteem.biffMsgNew
biffMsgNew.restype = POINTER(biffMsg)
biffMsgNew.argtypes = [STRING]
biffMsgNix = libteem.biffMsgNix
biffMsgNix.restype = POINTER(biffMsg)
biffMsgNix.argtypes = [POINTER(biffMsg)]
biffMsgAdd = libteem.biffMsgAdd
biffMsgAdd.restype = None
biffMsgAdd.argtypes = [POINTER(biffMsg), STRING]
__darwin_va_list = STRING
va_list = __darwin_va_list
biffMsgAddVL = libteem.biffMsgAddVL
biffMsgAddVL.restype = None
biffMsgAddVL.argtypes = [POINTER(biffMsg), STRING, va_list]
biffMsgAddf = libteem.biffMsgAddf
biffMsgAddf.restype = None
biffMsgAddf.argtypes = [POINTER(biffMsg), STRING]
biffMsgClear = libteem.biffMsgClear
biffMsgClear.restype = None
biffMsgClear.argtypes = [POINTER(biffMsg)]
biffMsgLineLenMax = libteem.biffMsgLineLenMax
biffMsgLineLenMax.restype = c_uint
biffMsgLineLenMax.argtypes = [POINTER(biffMsg)]
biffMsgMove = libteem.biffMsgMove
biffMsgMove.restype = None
biffMsgMove.argtypes = [POINTER(biffMsg), POINTER(biffMsg), STRING]
biffMsgMoveVL = libteem.biffMsgMoveVL
biffMsgMoveVL.restype = None
biffMsgMoveVL.argtypes = [POINTER(biffMsg), POINTER(biffMsg), STRING, va_list]
biffMsgMovef = libteem.biffMsgMovef
biffMsgMovef.restype = None
biffMsgMovef.argtypes = [POINTER(biffMsg), POINTER(biffMsg), STRING]
biffMsgStrlen = libteem.biffMsgStrlen
biffMsgStrlen.restype = c_uint
biffMsgStrlen.argtypes = [POINTER(biffMsg)]
biffMsgStrAlloc = libteem.biffMsgStrAlloc
biffMsgStrAlloc.restype = STRING
biffMsgStrAlloc.argtypes = [POINTER(biffMsg)]
biffMsgStrSet = libteem.biffMsgStrSet
biffMsgStrSet.restype = None
biffMsgStrSet.argtypes = [STRING, POINTER(biffMsg)]
biffMsgStrGet = libteem.biffMsgStrGet
biffMsgStrGet.restype = STRING
biffMsgStrGet.argtypes = [POINTER(biffMsg)]
biffMsgNoop = (POINTER(biffMsg)).in_dll(libteem, 'biffMsgNoop')
biffAdd = libteem.biffAdd
biffAdd.restype = None
biffAdd.argtypes = [STRING, STRING]
biffAddVL = libteem.biffAddVL
biffAddVL.restype = None
biffAddVL.argtypes = [STRING, STRING, va_list]
biffAddf = libteem.biffAddf
biffAddf.restype = None
biffAddf.argtypes = [STRING, STRING]
biffMaybeAdd = libteem.biffMaybeAdd
biffMaybeAdd.restype = None
biffMaybeAdd.argtypes = [STRING, STRING, c_int]
biffMaybeAddf = libteem.biffMaybeAddf
biffMaybeAddf.restype = None
biffMaybeAddf.argtypes = [c_int, STRING, STRING]
biffGet = libteem.biffGet
biffGet.restype = STRING
biffGet.argtypes = [STRING]
biffGetStrlen = libteem.biffGetStrlen
biffGetStrlen.restype = c_int
biffGetStrlen.argtypes = [STRING]
biffSetStr = libteem.biffSetStr
biffSetStr.restype = None
biffSetStr.argtypes = [STRING, STRING]
biffCheck = libteem.biffCheck
biffCheck.restype = c_int
biffCheck.argtypes = [STRING]
biffDone = libteem.biffDone
biffDone.restype = None
biffDone.argtypes = [STRING]
biffMove = libteem.biffMove
biffMove.restype = None
biffMove.argtypes = [STRING, STRING, STRING]
biffMoveVL = libteem.biffMoveVL
biffMoveVL.restype = None
biffMoveVL.argtypes = [STRING, STRING, STRING, va_list]
biffMovef = libteem.biffMovef
biffMovef.restype = None
biffMovef.argtypes = [STRING, STRING, STRING]
biffGetDone = libteem.biffGetDone
biffGetDone.restype = STRING
biffGetDone.argtypes = [STRING]
biffSetStrDone = libteem.biffSetStrDone
biffSetStrDone.restype = None
biffSetStrDone.argtypes = [STRING, STRING]
class dyeColor(Structure):
    pass
dyeColor._fields_ = [
    ('val', c_float * 3 * 2),
    ('xWhite', c_float),
    ('yWhite', c_float),
    ('spc', c_byte * 2),
    ('ii', c_byte),
]
dyeBiffKey = (STRING).in_dll(libteem, 'dyeBiffKey')
dyeSpaceToStr = (c_char * 129 * 0).in_dll(libteem, 'dyeSpaceToStr')
dyeStrToSpace = libteem.dyeStrToSpace
dyeStrToSpace.restype = c_int
dyeStrToSpace.argtypes = [STRING]
dyeColorInit = libteem.dyeColorInit
dyeColorInit.restype = POINTER(dyeColor)
dyeColorInit.argtypes = [POINTER(dyeColor)]
dyeColorSet = libteem.dyeColorSet
dyeColorSet.restype = POINTER(dyeColor)
dyeColorSet.argtypes = [POINTER(dyeColor), c_int, c_float, c_float, c_float]
dyeColorGet = libteem.dyeColorGet
dyeColorGet.restype = c_int
dyeColorGet.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), POINTER(dyeColor)]
dyeColorGetAs = libteem.dyeColorGetAs
dyeColorGetAs.restype = c_int
dyeColorGetAs.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), POINTER(dyeColor), c_int]
dyeColorNew = libteem.dyeColorNew
dyeColorNew.restype = POINTER(dyeColor)
dyeColorNew.argtypes = []
dyeColorCopy = libteem.dyeColorCopy
dyeColorCopy.restype = POINTER(dyeColor)
dyeColorCopy.argtypes = [POINTER(dyeColor), POINTER(dyeColor)]
dyeColorNix = libteem.dyeColorNix
dyeColorNix.restype = POINTER(dyeColor)
dyeColorNix.argtypes = [POINTER(dyeColor)]
dyeColorParse = libteem.dyeColorParse
dyeColorParse.restype = c_int
dyeColorParse.argtypes = [POINTER(dyeColor), STRING]
dyeColorSprintf = libteem.dyeColorSprintf
dyeColorSprintf.restype = STRING
dyeColorSprintf.argtypes = [STRING, POINTER(dyeColor)]
dyeConverter = CFUNCTYPE(None, POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float)
dyeRGBtoHSV = libteem.dyeRGBtoHSV
dyeRGBtoHSV.restype = None
dyeRGBtoHSV.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeHSVtoRGB = libteem.dyeHSVtoRGB
dyeHSVtoRGB.restype = None
dyeHSVtoRGB.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeRGBtoHSL = libteem.dyeRGBtoHSL
dyeRGBtoHSL.restype = None
dyeRGBtoHSL.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeHSLtoRGB = libteem.dyeHSLtoRGB
dyeHSLtoRGB.restype = None
dyeHSLtoRGB.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeRGBtoXYZ = libteem.dyeRGBtoXYZ
dyeRGBtoXYZ.restype = None
dyeRGBtoXYZ.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeXYZtoRGB = libteem.dyeXYZtoRGB
dyeXYZtoRGB.restype = None
dyeXYZtoRGB.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeXYZtoLAB = libteem.dyeXYZtoLAB
dyeXYZtoLAB.restype = None
dyeXYZtoLAB.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeXYZtoLUV = libteem.dyeXYZtoLUV
dyeXYZtoLUV.restype = None
dyeXYZtoLUV.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeLABtoXYZ = libteem.dyeLABtoXYZ
dyeLABtoXYZ.restype = None
dyeLABtoXYZ.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeLUVtoXYZ = libteem.dyeLUVtoXYZ
dyeLUVtoXYZ.restype = None
dyeLUVtoXYZ.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeSimpleConvert = (dyeConverter * 7 * 7).in_dll(libteem, 'dyeSimpleConvert')
dyeConvert = libteem.dyeConvert
dyeConvert.restype = c_int
dyeConvert.argtypes = [POINTER(dyeColor), c_int]
echoPos_t = c_double
echoCol_t = c_float
class echoRTParm(Structure):
    pass
echoRTParm._pack_ = 4
echoRTParm._fields_ = [
    ('jitterType', c_int),
    ('reuseJitter', c_int),
    ('permuteJitter', c_int),
    ('textureNN', c_int),
    ('numSamples', c_int),
    ('imgResU', c_int),
    ('imgResV', c_int),
    ('maxRecDepth', c_int),
    ('renderLights', c_int),
    ('renderBoxes', c_int),
    ('seedRand', c_int),
    ('sqNRI', c_int),
    ('numThreads', c_int),
    ('sqTol', echoPos_t),
    ('shadow', echoCol_t),
    ('glassC', echoCol_t),
    ('aperture', c_float),
    ('timeGamma', c_float),
    ('boxOpac', c_float),
    ('maxRecCol', echoCol_t * 3),
]
class echoGlobalState(Structure):
    pass
class Nrrd(Structure):
    pass
class limnCamera_t(Structure):
    pass
limnCamera = limnCamera_t
class echoScene_t(Structure):
    pass
echoGlobalState._pack_ = 4
echoGlobalState._fields_ = [
    ('verbose', c_int),
    ('time', c_double),
    ('nraw', POINTER(Nrrd)),
    ('cam', POINTER(limnCamera)),
    ('scene', POINTER(echoScene_t)),
    ('parm', POINTER(echoRTParm)),
    ('workIdx', c_int),
    ('workMutex', POINTER(airThreadMutex)),
]
class echoThreadState(Structure):
    pass
echoThreadState._fields_ = [
    ('thread', POINTER(airThread)),
    ('gstate', POINTER(echoGlobalState)),
    ('verbose', c_int),
    ('threadIdx', c_int),
    ('depth', c_int),
    ('nperm', POINTER(Nrrd)),
    ('njitt', POINTER(Nrrd)),
    ('permBuff', POINTER(c_uint)),
    ('jitt', POINTER(echoPos_t)),
    ('chanBuff', POINTER(echoCol_t)),
    ('rst', POINTER(airRandMTState)),
    ('returnPtr', c_void_p),
]
class echoObject(Structure):
    pass
echoObject._fields_ = [
    ('type', c_byte),
    ('matter', c_ubyte),
    ('rgba', echoCol_t * 4),
    ('mat', echoCol_t * 4),
    ('ntext', POINTER(Nrrd)),
]
class echoSphere(Structure):
    pass
echoSphere._pack_ = 4
echoSphere._fields_ = [
    ('type', c_byte),
    ('matter', c_ubyte),
    ('rgba', echoCol_t * 4),
    ('mat', echoCol_t * 4),
    ('ntext', POINTER(Nrrd)),
    ('pos', echoPos_t * 3),
    ('rad', echoPos_t),
]
class echoCylinder(Structure):
    pass
echoCylinder._fields_ = [
    ('type', c_byte),
    ('matter', c_ubyte),
    ('rgba', echoCol_t * 4),
    ('mat', echoCol_t * 4),
    ('ntext', POINTER(Nrrd)),
    ('axis', c_int),
]
class echoSuperquad(Structure):
    pass
echoSuperquad._pack_ = 4
echoSuperquad._fields_ = [
    ('type', c_byte),
    ('matter', c_ubyte),
    ('rgba', echoCol_t * 4),
    ('mat', echoCol_t * 4),
    ('ntext', POINTER(Nrrd)),
    ('axis', c_int),
    ('A', echoPos_t),
    ('B', echoPos_t),
]
class echoCube(Structure):
    pass
echoCube._fields_ = [
    ('type', c_byte),
    ('matter', c_ubyte),
    ('rgba', echoCol_t * 4),
    ('mat', echoCol_t * 4),
    ('ntext', POINTER(Nrrd)),
]
class echoTriangle(Structure):
    pass
echoTriangle._pack_ = 4
echoTriangle._fields_ = [
    ('type', c_byte),
    ('matter', c_ubyte),
    ('rgba', echoCol_t * 4),
    ('mat', echoCol_t * 4),
    ('ntext', POINTER(Nrrd)),
    ('vert', echoPos_t * 3 * 3),
]
class echoRectangle(Structure):
    pass
echoRectangle._pack_ = 4
echoRectangle._fields_ = [
    ('type', c_byte),
    ('matter', c_ubyte),
    ('rgba', echoCol_t * 4),
    ('mat', echoCol_t * 4),
    ('ntext', POINTER(Nrrd)),
    ('origin', echoPos_t * 3),
    ('edge0', echoPos_t * 3),
    ('edge1', echoPos_t * 3),
]
class echoTriMesh(Structure):
    pass
echoTriMesh._pack_ = 4
echoTriMesh._fields_ = [
    ('type', c_byte),
    ('matter', c_ubyte),
    ('rgba', echoCol_t * 4),
    ('mat', echoCol_t * 4),
    ('ntext', POINTER(Nrrd)),
    ('meanvert', echoPos_t * 3),
    ('min', echoPos_t * 3),
    ('max', echoPos_t * 3),
    ('numV', c_int),
    ('numF', c_int),
    ('pos', POINTER(echoPos_t)),
    ('vert', POINTER(c_int)),
]
class echoIsosurface(Structure):
    pass
echoIsosurface._fields_ = [
    ('type', c_byte),
    ('matter', c_ubyte),
    ('rgba', echoCol_t * 4),
    ('mat', echoCol_t * 4),
    ('ntext', POINTER(Nrrd)),
    ('volume', POINTER(Nrrd)),
    ('value', c_float),
]
class echoAABBox(Structure):
    pass
echoAABBox._pack_ = 4
echoAABBox._fields_ = [
    ('type', c_byte),
    ('obj', POINTER(echoObject)),
    ('min', echoPos_t * 3),
    ('max', echoPos_t * 3),
]
class echoSplit(Structure):
    pass
echoSplit._pack_ = 4
echoSplit._fields_ = [
    ('type', c_byte),
    ('axis', c_int),
    ('min0', echoPos_t * 3),
    ('max0', echoPos_t * 3),
    ('min1', echoPos_t * 3),
    ('max1', echoPos_t * 3),
    ('obj0', POINTER(echoObject)),
    ('obj1', POINTER(echoObject)),
]
class echoList(Structure):
    pass
echoList._fields_ = [
    ('type', c_byte),
    ('obj', POINTER(POINTER(echoObject))),
    ('objArr', POINTER(airArray)),
]
class echoInstance(Structure):
    pass
echoInstance._pack_ = 4
echoInstance._fields_ = [
    ('type', c_byte),
    ('Mi', echoPos_t * 16),
    ('M', echoPos_t * 16),
    ('obj', POINTER(echoObject)),
]
echoScene_t._fields_ = [
    ('cat', POINTER(POINTER(echoObject))),
    ('catArr', POINTER(airArray)),
    ('rend', POINTER(POINTER(echoObject))),
    ('rendArr', POINTER(airArray)),
    ('light', POINTER(POINTER(echoObject))),
    ('lightArr', POINTER(airArray)),
    ('nrrd', POINTER(POINTER(Nrrd))),
    ('nrrdArr', POINTER(airArray)),
    ('envmap', POINTER(Nrrd)),
    ('ambi', echoCol_t * 3),
    ('bkgr', echoCol_t * 3),
]
echoScene = echoScene_t
class echoRay(Structure):
    pass
echoRay._pack_ = 4
echoRay._fields_ = [
    ('from', echoPos_t * 3),
    ('dir', echoPos_t * 3),
    ('neer', echoPos_t),
    ('faar', echoPos_t),
    ('shadow', c_int),
    ('transp', echoCol_t),
]
class echoIntx(Structure):
    pass
echoIntx._pack_ = 4
echoIntx._fields_ = [
    ('obj', POINTER(echoObject)),
    ('t', echoPos_t),
    ('u', echoPos_t),
    ('v', echoPos_t),
    ('norm', echoPos_t * 3),
    ('view', echoPos_t * 3),
    ('refl', echoPos_t * 3),
    ('pos', echoPos_t * 3),
    ('face', c_int),
    ('boxhits', c_int),
]
echoJitter = (POINTER(airEnum)).in_dll(libteem, 'echoJitter')
echoType = (POINTER(airEnum)).in_dll(libteem, 'echoType')
echoMatter = (POINTER(airEnum)).in_dll(libteem, 'echoMatter')
echoBiffKey = (STRING).in_dll(libteem, 'echoBiffKey')
echoRTParmNew = libteem.echoRTParmNew
echoRTParmNew.restype = POINTER(echoRTParm)
echoRTParmNew.argtypes = []
echoRTParmNix = libteem.echoRTParmNix
echoRTParmNix.restype = POINTER(echoRTParm)
echoRTParmNix.argtypes = [POINTER(echoRTParm)]
echoGlobalStateNew = libteem.echoGlobalStateNew
echoGlobalStateNew.restype = POINTER(echoGlobalState)
echoGlobalStateNew.argtypes = []
echoGlobalStateNix = libteem.echoGlobalStateNix
echoGlobalStateNix.restype = POINTER(echoGlobalState)
echoGlobalStateNix.argtypes = [POINTER(echoGlobalState)]
echoThreadStateNew = libteem.echoThreadStateNew
echoThreadStateNew.restype = POINTER(echoThreadState)
echoThreadStateNew.argtypes = []
echoThreadStateNix = libteem.echoThreadStateNix
echoThreadStateNix.restype = POINTER(echoThreadState)
echoThreadStateNix.argtypes = [POINTER(echoThreadState)]
echoSceneNew = libteem.echoSceneNew
echoSceneNew.restype = POINTER(echoScene)
echoSceneNew.argtypes = []
echoSceneNix = libteem.echoSceneNix
echoSceneNix.restype = POINTER(echoScene)
echoSceneNix.argtypes = [POINTER(echoScene)]
echoObjectNew = libteem.echoObjectNew
echoObjectNew.restype = POINTER(echoObject)
echoObjectNew.argtypes = [POINTER(echoScene), c_byte]
echoObjectAdd = libteem.echoObjectAdd
echoObjectAdd.restype = c_int
echoObjectAdd.argtypes = [POINTER(echoScene), POINTER(echoObject)]
echoObjectNix = libteem.echoObjectNix
echoObjectNix.restype = POINTER(echoObject)
echoObjectNix.argtypes = [POINTER(echoObject)]
echoRoughSphereNew = libteem.echoRoughSphereNew
echoRoughSphereNew.restype = POINTER(echoObject)
echoRoughSphereNew.argtypes = [POINTER(echoScene), c_int, c_int, POINTER(echoPos_t)]
echoBoundsGet = libteem.echoBoundsGet
echoBoundsGet.restype = None
echoBoundsGet.argtypes = [POINTER(echoPos_t), POINTER(echoPos_t), POINTER(echoObject)]
echoListAdd = libteem.echoListAdd
echoListAdd.restype = None
echoListAdd.argtypes = [POINTER(echoObject), POINTER(echoObject)]
echoListSplit = libteem.echoListSplit
echoListSplit.restype = POINTER(echoObject)
echoListSplit.argtypes = [POINTER(echoScene), POINTER(echoObject), c_int]
echoListSplit3 = libteem.echoListSplit3
echoListSplit3.restype = POINTER(echoObject)
echoListSplit3.argtypes = [POINTER(echoScene), POINTER(echoObject), c_int]
echoSphereSet = libteem.echoSphereSet
echoSphereSet.restype = None
echoSphereSet.argtypes = [POINTER(echoObject), echoPos_t, echoPos_t, echoPos_t, echoPos_t]
echoCylinderSet = libteem.echoCylinderSet
echoCylinderSet.restype = None
echoCylinderSet.argtypes = [POINTER(echoObject), c_int]
echoSuperquadSet = libteem.echoSuperquadSet
echoSuperquadSet.restype = None
echoSuperquadSet.argtypes = [POINTER(echoObject), c_int, echoPos_t, echoPos_t]
echoRectangleSet = libteem.echoRectangleSet
echoRectangleSet.restype = None
echoRectangleSet.argtypes = [POINTER(echoObject), echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t]
echoTriangleSet = libteem.echoTriangleSet
echoTriangleSet.restype = None
echoTriangleSet.argtypes = [POINTER(echoObject), echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t]
echoTriMeshSet = libteem.echoTriMeshSet
echoTriMeshSet.restype = None
echoTriMeshSet.argtypes = [POINTER(echoObject), c_int, POINTER(echoPos_t), c_int, POINTER(c_int)]
echoInstanceSet = libteem.echoInstanceSet
echoInstanceSet.restype = None
echoInstanceSet.argtypes = [POINTER(echoObject), POINTER(echoPos_t), POINTER(echoObject)]
echoObjectHasMatter = (c_int * 12).in_dll(libteem, 'echoObjectHasMatter')
echoColorSet = libteem.echoColorSet
echoColorSet.restype = None
echoColorSet.argtypes = [POINTER(echoObject), echoCol_t, echoCol_t, echoCol_t, echoCol_t]
echoMatterPhongSet = libteem.echoMatterPhongSet
echoMatterPhongSet.restype = None
echoMatterPhongSet.argtypes = [POINTER(echoScene), POINTER(echoObject), echoCol_t, echoCol_t, echoCol_t, echoCol_t]
echoMatterGlassSet = libteem.echoMatterGlassSet
echoMatterGlassSet.restype = None
echoMatterGlassSet.argtypes = [POINTER(echoScene), POINTER(echoObject), echoCol_t, echoCol_t, echoCol_t, echoCol_t]
echoMatterMetalSet = libteem.echoMatterMetalSet
echoMatterMetalSet.restype = None
echoMatterMetalSet.argtypes = [POINTER(echoScene), POINTER(echoObject), echoCol_t, echoCol_t, echoCol_t, echoCol_t]
echoMatterLightSet = libteem.echoMatterLightSet
echoMatterLightSet.restype = None
echoMatterLightSet.argtypes = [POINTER(echoScene), POINTER(echoObject), echoCol_t, echoCol_t]
echoMatterTextureSet = libteem.echoMatterTextureSet
echoMatterTextureSet.restype = None
echoMatterTextureSet.argtypes = [POINTER(echoScene), POINTER(echoObject), POINTER(Nrrd)]
echoLightPosition = libteem.echoLightPosition
echoLightPosition.restype = None
echoLightPosition.argtypes = [POINTER(echoPos_t), POINTER(echoObject), POINTER(echoThreadState)]
echoLightColor = libteem.echoLightColor
echoLightColor.restype = None
echoLightColor.argtypes = [POINTER(echoCol_t), echoPos_t, POINTER(echoObject), POINTER(echoRTParm), POINTER(echoThreadState)]
echoEnvmapLookup = libteem.echoEnvmapLookup
echoEnvmapLookup.restype = None
echoEnvmapLookup.argtypes = [POINTER(echoCol_t), POINTER(echoPos_t), POINTER(Nrrd)]
echoTextureLookup = libteem.echoTextureLookup
echoTextureLookup.restype = None
echoTextureLookup.argtypes = [POINTER(echoCol_t), POINTER(Nrrd), echoPos_t, echoPos_t, POINTER(echoRTParm)]
echoIntxMaterialColor = libteem.echoIntxMaterialColor
echoIntxMaterialColor.restype = None
echoIntxMaterialColor.argtypes = [POINTER(echoCol_t), POINTER(echoIntx), POINTER(echoRTParm)]
echoIntxLightColor = libteem.echoIntxLightColor
echoIntxLightColor.restype = None
echoIntxLightColor.argtypes = [POINTER(echoCol_t), POINTER(echoCol_t), POINTER(echoCol_t), echoCol_t, POINTER(echoIntx), POINTER(echoScene), POINTER(echoRTParm), POINTER(echoThreadState)]
echoIntxFuzzify = libteem.echoIntxFuzzify
echoIntxFuzzify.restype = None
echoIntxFuzzify.argtypes = [POINTER(echoIntx), echoCol_t, POINTER(echoThreadState)]
echoRayIntx = libteem.echoRayIntx
echoRayIntx.restype = c_int
echoRayIntx.argtypes = [POINTER(echoIntx), POINTER(echoRay), POINTER(echoScene), POINTER(echoRTParm), POINTER(echoThreadState)]
echoIntxColor = libteem.echoIntxColor
echoIntxColor.restype = None
echoIntxColor.argtypes = [POINTER(echoCol_t), POINTER(echoIntx), POINTER(echoScene), POINTER(echoRTParm), POINTER(echoThreadState)]
echoThreadStateInit = libteem.echoThreadStateInit
echoThreadStateInit.restype = c_int
echoThreadStateInit.argtypes = [c_int, POINTER(echoThreadState), POINTER(echoRTParm), POINTER(echoGlobalState)]
echoJitterCompute = libteem.echoJitterCompute
echoJitterCompute.restype = None
echoJitterCompute.argtypes = [POINTER(echoRTParm), POINTER(echoThreadState)]
echoRayColor = libteem.echoRayColor
echoRayColor.restype = None
echoRayColor.argtypes = [POINTER(echoCol_t), POINTER(echoRay), POINTER(echoScene), POINTER(echoRTParm), POINTER(echoThreadState)]
echoChannelAverage = libteem.echoChannelAverage
echoChannelAverage.restype = None
echoChannelAverage.argtypes = [POINTER(echoCol_t), POINTER(echoRTParm), POINTER(echoThreadState)]
echoRTRenderCheck = libteem.echoRTRenderCheck
echoRTRenderCheck.restype = c_int
echoRTRenderCheck.argtypes = [POINTER(Nrrd), POINTER(limnCamera), POINTER(echoScene), POINTER(echoRTParm), POINTER(echoGlobalState)]
echoRTRender = libteem.echoRTRender
echoRTRender.restype = c_int
echoRTRender.argtypes = [POINTER(Nrrd), POINTER(limnCamera), POINTER(echoScene), POINTER(echoRTParm), POINTER(echoGlobalState)]
ell_biff_key = (STRING).in_dll(libteem, 'ell_biff_key')
ell_cubic_root = (POINTER(airEnum)).in_dll(libteem, 'ell_cubic_root')
ell_debug = (c_int).in_dll(libteem, 'ell_debug')
ell_3m_print_f = libteem.ell_3m_print_f
ell_3m_print_f.restype = None
ell_3m_print_f.argtypes = [POINTER(FILE), POINTER(c_float)]
ell_3v_print_f = libteem.ell_3v_print_f
ell_3v_print_f.restype = None
ell_3v_print_f.argtypes = [POINTER(FILE), POINTER(c_float)]
ell_3m_print_d = libteem.ell_3m_print_d
ell_3m_print_d.restype = None
ell_3m_print_d.argtypes = [POINTER(FILE), POINTER(c_double)]
ell_3v_print_d = libteem.ell_3v_print_d
ell_3v_print_d.restype = None
ell_3v_print_d.argtypes = [POINTER(FILE), POINTER(c_double)]
ell_4m_print_f = libteem.ell_4m_print_f
ell_4m_print_f.restype = None
ell_4m_print_f.argtypes = [POINTER(FILE), POINTER(c_float)]
ell_4v_print_f = libteem.ell_4v_print_f
ell_4v_print_f.restype = None
ell_4v_print_f.argtypes = [POINTER(FILE), POINTER(c_float)]
ell_4m_print_d = libteem.ell_4m_print_d
ell_4m_print_d.restype = None
ell_4m_print_d.argtypes = [POINTER(FILE), POINTER(c_double)]
ell_4v_print_d = libteem.ell_4v_print_d
ell_4v_print_d.restype = None
ell_4v_print_d.argtypes = [POINTER(FILE), POINTER(c_double)]
ell_3v_perp_f = libteem.ell_3v_perp_f
ell_3v_perp_f.restype = None
ell_3v_perp_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3v_perp_d = libteem.ell_3v_perp_d
ell_3v_perp_d.restype = None
ell_3v_perp_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_3mv_mul_f = libteem.ell_3mv_mul_f
ell_3mv_mul_f.restype = None
ell_3mv_mul_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_3mv_mul_d = libteem.ell_3mv_mul_d
ell_3mv_mul_d.restype = None
ell_3mv_mul_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_4mv_mul_f = libteem.ell_4mv_mul_f
ell_4mv_mul_f.restype = None
ell_4mv_mul_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_4mv_mul_d = libteem.ell_4mv_mul_d
ell_4mv_mul_d.restype = None
ell_4mv_mul_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_3v_angle_f = libteem.ell_3v_angle_f
ell_3v_angle_f.restype = c_float
ell_3v_angle_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3v_angle_d = libteem.ell_3v_angle_d
ell_3v_angle_d.restype = c_double
ell_3v_angle_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_3v_area_spherical_d = libteem.ell_3v_area_spherical_d
ell_3v_area_spherical_d.restype = c_double
ell_3v_area_spherical_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_3v_barycentric_spherical_d = libteem.ell_3v_barycentric_spherical_d
ell_3v_barycentric_spherical_d.restype = None
ell_3v_barycentric_spherical_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_3m_mul_f = libteem.ell_3m_mul_f
ell_3m_mul_f.restype = None
ell_3m_mul_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_3m_mul_d = libteem.ell_3m_mul_d
ell_3m_mul_d.restype = None
ell_3m_mul_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_3m_pre_mul_f = libteem.ell_3m_pre_mul_f
ell_3m_pre_mul_f.restype = None
ell_3m_pre_mul_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3m_pre_mul_d = libteem.ell_3m_pre_mul_d
ell_3m_pre_mul_d.restype = None
ell_3m_pre_mul_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_3m_post_mul_f = libteem.ell_3m_post_mul_f
ell_3m_post_mul_f.restype = None
ell_3m_post_mul_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3m_post_mul_d = libteem.ell_3m_post_mul_d
ell_3m_post_mul_d.restype = None
ell_3m_post_mul_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_3m_det_f = libteem.ell_3m_det_f
ell_3m_det_f.restype = c_float
ell_3m_det_f.argtypes = [POINTER(c_float)]
ell_3m_det_d = libteem.ell_3m_det_d
ell_3m_det_d.restype = c_double
ell_3m_det_d.argtypes = [POINTER(c_double)]
ell_3m_inv_f = libteem.ell_3m_inv_f
ell_3m_inv_f.restype = None
ell_3m_inv_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3m_inv_d = libteem.ell_3m_inv_d
ell_3m_inv_d.restype = None
ell_3m_inv_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_4m_mul_f = libteem.ell_4m_mul_f
ell_4m_mul_f.restype = None
ell_4m_mul_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_4m_mul_d = libteem.ell_4m_mul_d
ell_4m_mul_d.restype = None
ell_4m_mul_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_4m_pre_mul_f = libteem.ell_4m_pre_mul_f
ell_4m_pre_mul_f.restype = None
ell_4m_pre_mul_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_4m_post_mul_f = libteem.ell_4m_post_mul_f
ell_4m_post_mul_f.restype = None
ell_4m_post_mul_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_4m_post_mul_d = libteem.ell_4m_post_mul_d
ell_4m_post_mul_d.restype = None
ell_4m_post_mul_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_4m_det_f = libteem.ell_4m_det_f
ell_4m_det_f.restype = c_float
ell_4m_det_f.argtypes = [POINTER(c_float)]
ell_4m_det_d = libteem.ell_4m_det_d
ell_4m_det_d.restype = c_double
ell_4m_det_d.argtypes = [POINTER(c_double)]
ell_4m_inv_f = libteem.ell_4m_inv_f
ell_4m_inv_f.restype = None
ell_4m_inv_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_4m_inv_d = libteem.ell_4m_inv_d
ell_4m_inv_d.restype = None
ell_4m_inv_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_6m_mul_d = libteem.ell_6m_mul_d
ell_6m_mul_d.restype = None
ell_6m_mul_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_3m_rotate_between_d = libteem.ell_3m_rotate_between_d
ell_3m_rotate_between_d.restype = None
ell_3m_rotate_between_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_3m_to_q_f = libteem.ell_3m_to_q_f
ell_3m_to_q_f.restype = None
ell_3m_to_q_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3m_to_q_d = libteem.ell_3m_to_q_d
ell_3m_to_q_d.restype = None
ell_3m_to_q_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_4m_to_q_f = libteem.ell_4m_to_q_f
ell_4m_to_q_f.restype = None
ell_4m_to_q_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_4m_to_q_d = libteem.ell_4m_to_q_d
ell_4m_to_q_d.restype = None
ell_4m_to_q_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_to_3m_f = libteem.ell_q_to_3m_f
ell_q_to_3m_f.restype = None
ell_q_to_3m_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_q_to_3m_d = libteem.ell_q_to_3m_d
ell_q_to_3m_d.restype = None
ell_q_to_3m_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_to_4m_f = libteem.ell_q_to_4m_f
ell_q_to_4m_f.restype = None
ell_q_to_4m_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_q_to_4m_d = libteem.ell_q_to_4m_d
ell_q_to_4m_d.restype = None
ell_q_to_4m_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_to_aa_f = libteem.ell_q_to_aa_f
ell_q_to_aa_f.restype = c_float
ell_q_to_aa_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_q_to_aa_d = libteem.ell_q_to_aa_d
ell_q_to_aa_d.restype = c_double
ell_q_to_aa_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_aa_to_q_f = libteem.ell_aa_to_q_f
ell_aa_to_q_f.restype = None
ell_aa_to_q_f.argtypes = [POINTER(c_float), c_float, POINTER(c_float)]
ell_aa_to_q_d = libteem.ell_aa_to_q_d
ell_aa_to_q_d.restype = None
ell_aa_to_q_d.argtypes = [POINTER(c_double), c_double, POINTER(c_double)]
ell_aa_to_3m_f = libteem.ell_aa_to_3m_f
ell_aa_to_3m_f.restype = None
ell_aa_to_3m_f.argtypes = [POINTER(c_float), c_float, POINTER(c_float)]
ell_aa_to_3m_d = libteem.ell_aa_to_3m_d
ell_aa_to_3m_d.restype = None
ell_aa_to_3m_d.argtypes = [POINTER(c_double), c_double, POINTER(c_double)]
ell_aa_to_4m_f = libteem.ell_aa_to_4m_f
ell_aa_to_4m_f.restype = None
ell_aa_to_4m_f.argtypes = [POINTER(c_float), c_float, POINTER(c_float)]
ell_aa_to_4m_d = libteem.ell_aa_to_4m_d
ell_aa_to_4m_d.restype = None
ell_aa_to_4m_d.argtypes = [POINTER(c_double), c_double, POINTER(c_double)]
ell_3m_to_aa_f = libteem.ell_3m_to_aa_f
ell_3m_to_aa_f.restype = c_float
ell_3m_to_aa_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3m_to_aa_d = libteem.ell_3m_to_aa_d
ell_3m_to_aa_d.restype = c_double
ell_3m_to_aa_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_4m_to_aa_f = libteem.ell_4m_to_aa_f
ell_4m_to_aa_f.restype = c_float
ell_4m_to_aa_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_4m_to_aa_d = libteem.ell_4m_to_aa_d
ell_4m_to_aa_d.restype = c_double
ell_4m_to_aa_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_mul_f = libteem.ell_q_mul_f
ell_q_mul_f.restype = None
ell_q_mul_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_q_mul_d = libteem.ell_q_mul_d
ell_q_mul_d.restype = None
ell_q_mul_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_q_inv_f = libteem.ell_q_inv_f
ell_q_inv_f.restype = None
ell_q_inv_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_q_inv_d = libteem.ell_q_inv_d
ell_q_inv_d.restype = None
ell_q_inv_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_pow_f = libteem.ell_q_pow_f
ell_q_pow_f.restype = None
ell_q_pow_f.argtypes = [POINTER(c_float), POINTER(c_float), c_float]
ell_q_pow_d = libteem.ell_q_pow_d
ell_q_pow_d.restype = None
ell_q_pow_d.argtypes = [POINTER(c_double), POINTER(c_double), c_double]
ell_q_div_f = libteem.ell_q_div_f
ell_q_div_f.restype = None
ell_q_div_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_q_div_d = libteem.ell_q_div_d
ell_q_div_d.restype = None
ell_q_div_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_q_exp_f = libteem.ell_q_exp_f
ell_q_exp_f.restype = None
ell_q_exp_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_q_exp_d = libteem.ell_q_exp_d
ell_q_exp_d.restype = None
ell_q_exp_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_log_f = libteem.ell_q_log_f
ell_q_log_f.restype = None
ell_q_log_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_q_log_d = libteem.ell_q_log_d
ell_q_log_d.restype = None
ell_q_log_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_3v_rotate_f = libteem.ell_q_3v_rotate_f
ell_q_3v_rotate_f.restype = None
ell_q_3v_rotate_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_q_3v_rotate_d = libteem.ell_q_3v_rotate_d
ell_q_3v_rotate_d.restype = None
ell_q_3v_rotate_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_q_4v_rotate_f = libteem.ell_q_4v_rotate_f
ell_q_4v_rotate_f.restype = None
ell_q_4v_rotate_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_q_4v_rotate_d = libteem.ell_q_4v_rotate_d
ell_q_4v_rotate_d.restype = None
ell_q_4v_rotate_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_q_avg4_d = libteem.ell_q_avg4_d
ell_q_avg4_d.restype = c_int
ell_q_avg4_d.argtypes = [POINTER(c_double), POINTER(c_uint), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_double, c_uint]
ell_q_avgN_d = libteem.ell_q_avgN_d
ell_q_avgN_d.restype = c_int
ell_q_avgN_d.argtypes = [POINTER(c_double), POINTER(c_uint), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_uint, c_double, c_uint]
ell_Nm_check = libteem.ell_Nm_check
ell_Nm_check.restype = c_int
ell_Nm_check.argtypes = [POINTER(Nrrd), c_int]
ell_Nm_tran = libteem.ell_Nm_tran
ell_Nm_tran.restype = c_int
ell_Nm_tran.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
ell_Nm_mul = libteem.ell_Nm_mul
ell_Nm_mul.restype = c_int
ell_Nm_mul.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
ell_Nm_inv = libteem.ell_Nm_inv
ell_Nm_inv.restype = c_int
ell_Nm_inv.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
ell_Nm_pseudo_inv = libteem.ell_Nm_pseudo_inv
ell_Nm_pseudo_inv.restype = c_int
ell_Nm_pseudo_inv.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
ell_Nm_wght_pseudo_inv = libteem.ell_Nm_wght_pseudo_inv
ell_Nm_wght_pseudo_inv.restype = c_int
ell_Nm_wght_pseudo_inv.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
ell_cubic = libteem.ell_cubic
ell_cubic.restype = c_int
ell_cubic.argtypes = [POINTER(c_double), c_double, c_double, c_double, c_int]
ell_3m_1d_nullspace_d = libteem.ell_3m_1d_nullspace_d
ell_3m_1d_nullspace_d.restype = None
ell_3m_1d_nullspace_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_3m_2d_nullspace_d = libteem.ell_3m_2d_nullspace_d
ell_3m_2d_nullspace_d.restype = None
ell_3m_2d_nullspace_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_3m_eigenvalues_d = libteem.ell_3m_eigenvalues_d
ell_3m_eigenvalues_d.restype = c_int
ell_3m_eigenvalues_d.argtypes = [POINTER(c_double), POINTER(c_double), c_int]
ell_3m_eigensolve_d = libteem.ell_3m_eigensolve_d
ell_3m_eigensolve_d.restype = c_int
ell_3m_eigensolve_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int]
ell_3m_svd_d = libteem.ell_3m_svd_d
ell_3m_svd_d.restype = c_int
ell_3m_svd_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int]
ell_6ms_eigensolve_d = libteem.ell_6ms_eigensolve_d
ell_6ms_eigensolve_d.restype = c_int
ell_6ms_eigensolve_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), c_double]
class gageItemEntry(Structure):
    pass
gageItemEntry._fields_ = [
    ('enumVal', c_int),
    ('answerLength', c_uint),
    ('needDeriv', c_int),
    ('prereq', c_int * 8),
    ('parentItem', c_int),
    ('parentIndex', c_int),
    ('needData', c_int),
]
class gageShape_t(Structure):
    pass
gageShape_t._pack_ = 4
gageShape_t._fields_ = [
    ('defaultCenter', c_int),
    ('orientationFromSpacing', c_int),
    ('center', c_int),
    ('fromOrientation', c_int),
    ('size', c_uint * 3),
    ('spacing', c_double * 3),
    ('ItoW', c_double * 16),
    ('WtoI', c_double * 16),
    ('ItoWSubInvTransp', c_double * 9),
    ('ItoWSubInv', c_double * 9),
]
gageShape = gageShape_t
class gageParm_t(Structure):
    pass
gageParm_t._pack_ = 4
gageParm_t._fields_ = [
    ('renormalize', c_int),
    ('checkIntegrals', c_int),
    ('k3pack', c_int),
    ('gradMagCurvMin', c_double),
    ('kernelIntegralNearZero', c_double),
    ('curvNormalSide', c_int),
    ('defaultCenter', c_int),
    ('stackUse', c_int),
    ('stackNormalizeRecon', c_int),
    ('stackNormalizeDeriv', c_int),
    ('orientationFromSpacing', c_int),
]
gageParm = gageParm_t
class gagePoint_t(Structure):
    pass
gagePoint_t._pack_ = 4
gagePoint_t._fields_ = [
    ('frac', c_double * 4),
    ('idx', c_uint * 4),
    ('stackFwNonZeroNum', c_uint),
]
gagePoint = gagePoint_t
gageQuery = c_ubyte * 32
class gageContext_t(Structure):
    pass
class NrrdKernelSpec(Structure):
    pass
class gagePerVolume_t(Structure):
    pass
gageContext_t._pack_ = 4
gageContext_t._fields_ = [
    ('verbose', c_int),
    ('parm', gageParm),
    ('ksp', POINTER(NrrdKernelSpec) * 8),
    ('pvl', POINTER(POINTER(gagePerVolume_t))),
    ('pvlNum', c_uint),
    ('pvlArr', POINTER(airArray)),
    ('shape', POINTER(gageShape)),
    ('stackPos', POINTER(c_double)),
    ('stackFsl', POINTER(c_double)),
    ('stackFw', POINTER(c_double)),
    ('flag', c_int * 7),
    ('needD', c_int * 3),
    ('needK', c_int * 8),
    ('radius', c_uint),
    ('fsl', POINTER(c_double)),
    ('fw', POINTER(c_double)),
    ('off', POINTER(c_uint)),
    ('point', gagePoint),
    ('errStr', c_char * 513),
    ('errNum', c_int),
    ('edgeFrac', c_double),
]
gageContext = gageContext_t
class gageKind_t(Structure):
    pass
gagePerVolume = gagePerVolume_t
gageKind_t._fields_ = [
    ('dynamicAlloc', c_int),
    ('name', c_char * 129),
    ('enm', POINTER(airEnum)),
    ('baseDim', c_uint),
    ('valLen', c_uint),
    ('itemMax', c_int),
    ('table', POINTER(gageItemEntry)),
    ('iv3Print', CFUNCTYPE(None, POINTER(FILE), POINTER(gageContext), POINTER(gagePerVolume))),
    ('filter', CFUNCTYPE(None, POINTER(gageContext), POINTER(gagePerVolume))),
    ('answer', CFUNCTYPE(None, POINTER(gageContext), POINTER(gagePerVolume))),
    ('pvlDataNew', CFUNCTYPE(c_void_p, POINTER(gageKind_t))),
    ('pvlDataCopy', CFUNCTYPE(c_void_p, POINTER(gageKind_t), c_void_p)),
    ('pvlDataNix', CFUNCTYPE(c_void_p, POINTER(gageKind_t), c_void_p)),
    ('pvlDataUpdate', CFUNCTYPE(c_int, POINTER(gageKind_t), POINTER(gageContext), POINTER(gagePerVolume), c_void_p)),
    ('data', c_void_p),
]
class NrrdAxisInfo(Structure):
    pass
NrrdAxisInfo._pack_ = 4
NrrdAxisInfo._fields_ = [
    ('size', size_t),
    ('spacing', c_double),
    ('thickness', c_double),
    ('min', c_double),
    ('max', c_double),
    ('spaceDirection', c_double * 8),
    ('center', c_int),
    ('kind', c_int),
    ('label', STRING),
    ('units', STRING),
]
Nrrd._pack_ = 4
Nrrd._fields_ = [
    ('data', c_void_p),
    ('type', c_int),
    ('dim', c_uint),
    ('axis', NrrdAxisInfo * 16),
    ('content', STRING),
    ('sampleUnits', STRING),
    ('space', c_int),
    ('spaceDim', c_uint),
    ('spaceUnits', STRING * 8),
    ('spaceOrigin', c_double * 8),
    ('measurementFrame', c_double * 8 * 8),
    ('blockSize', size_t),
    ('oldMin', c_double),
    ('oldMax', c_double),
    ('ptr', c_void_p),
    ('cmt', POINTER(STRING)),
    ('cmtArr', POINTER(airArray)),
    ('kvp', POINTER(STRING)),
    ('kvpArr', POINTER(airArray)),
]
gagePerVolume_t._fields_ = [
    ('verbose', c_int),
    ('kind', POINTER(gageKind_t)),
    ('query', gageQuery),
    ('needD', c_int * 3),
    ('nin', POINTER(Nrrd)),
    ('flag', c_int * 4),
    ('iv3', POINTER(c_double)),
    ('iv2', POINTER(c_double)),
    ('iv1', POINTER(c_double)),
    ('lup', CFUNCTYPE(c_double, c_void_p, size_t)),
    ('answer', POINTER(c_double)),
    ('directAnswer', POINTER(POINTER(c_double))),
    ('data', c_void_p),
]
gageKind = gageKind_t
class gageItemSpec(Structure):
    pass
gageItemSpec._fields_ = [
    ('kind', POINTER(gageKind)),
    ('item', c_int),
]
class gageStackBlurParm(Structure):
    pass
gageStackBlurParm._fields_ = [
    ('num', c_uint),
    ('scale', POINTER(c_double)),
    ('kspec', POINTER(NrrdKernelSpec)),
    ('boundary', c_int),
    ('renormalize', c_int),
    ('verbose', c_int),
]
class gageOptimSigParm(Structure):
    pass
gageOptimSigParm._pack_ = 4
gageOptimSigParm._fields_ = [
    ('dim', c_uint),
    ('sigmaMax', c_double),
    ('cutoff', c_double),
    ('measrSampleNum', c_uint),
    ('sampleNum', c_uint),
    ('volMeasr', c_int),
    ('lineMeasr', c_int),
    ('plotting', c_int),
    ('tentRecon', c_int),
    ('maxIter', c_uint),
    ('convEps', c_double),
    ('sx', c_uint),
    ('sy', c_uint),
    ('sz', c_uint),
    ('sigmatru', POINTER(c_double)),
    ('truth', POINTER(c_double)),
    ('ntruth', POINTER(Nrrd)),
    ('nerr', POINTER(Nrrd)),
    ('ntruline', POINTER(Nrrd)),
    ('ninterp', POINTER(Nrrd)),
    ('ndiff', POINTER(Nrrd)),
    ('sampleNumMax', c_uint),
    ('scalePos', POINTER(c_double)),
    ('step', POINTER(c_double)),
    ('nsampvol', POINTER(POINTER(Nrrd))),
    ('pvl', POINTER(gagePerVolume)),
    ('pvlSS', POINTER(POINTER(gagePerVolume))),
    ('gctx', POINTER(gageContext)),
    ('finalErr', c_double),
]
gageBiffKey = (STRING).in_dll(libteem, 'gageBiffKey')
gageDefVerbose = (c_int).in_dll(libteem, 'gageDefVerbose')
gageDefGradMagCurvMin = (c_double).in_dll(libteem, 'gageDefGradMagCurvMin')
gageDefRenormalize = (c_int).in_dll(libteem, 'gageDefRenormalize')
gageDefCheckIntegrals = (c_int).in_dll(libteem, 'gageDefCheckIntegrals')
gageDefK3Pack = (c_int).in_dll(libteem, 'gageDefK3Pack')
gageDefCurvNormalSide = (c_int).in_dll(libteem, 'gageDefCurvNormalSide')
gageDefKernelIntegralNearZero = (c_double).in_dll(libteem, 'gageDefKernelIntegralNearZero')
gageDefDefaultCenter = (c_int).in_dll(libteem, 'gageDefDefaultCenter')
gageDefStackUse = (c_int).in_dll(libteem, 'gageDefStackUse')
gageDefStackNormalizeRecon = (c_int).in_dll(libteem, 'gageDefStackNormalizeRecon')
gageDefStackNormalizeDeriv = (c_int).in_dll(libteem, 'gageDefStackNormalizeDeriv')
gageDefOrientationFromSpacing = (c_int).in_dll(libteem, 'gageDefOrientationFromSpacing')
gageZeroNormal = (c_double * 3).in_dll(libteem, 'gageZeroNormal')
gageErr = (POINTER(airEnum)).in_dll(libteem, 'gageErr')
gageKernel = (POINTER(airEnum)).in_dll(libteem, 'gageKernel')
gageParmReset = libteem.gageParmReset
gageParmReset.restype = None
gageParmReset.argtypes = [POINTER(gageParm)]
gagePointReset = libteem.gagePointReset
gagePointReset.restype = None
gagePointReset.argtypes = [POINTER(gagePoint)]
gageItemSpecNew = libteem.gageItemSpecNew
gageItemSpecNew.restype = POINTER(gageItemSpec)
gageItemSpecNew.argtypes = []
gageItemSpecInit = libteem.gageItemSpecInit
gageItemSpecInit.restype = None
gageItemSpecInit.argtypes = [POINTER(gageItemSpec)]
gageItemSpecNix = libteem.gageItemSpecNix
gageItemSpecNix.restype = POINTER(gageItemSpec)
gageItemSpecNix.argtypes = [POINTER(gageItemSpec)]
gageKindCheck = libteem.gageKindCheck
gageKindCheck.restype = c_int
gageKindCheck.argtypes = [POINTER(gageKind)]
gageKindTotalAnswerLength = libteem.gageKindTotalAnswerLength
gageKindTotalAnswerLength.restype = c_uint
gageKindTotalAnswerLength.argtypes = [POINTER(gageKind)]
gageKindAnswerLength = libteem.gageKindAnswerLength
gageKindAnswerLength.restype = c_uint
gageKindAnswerLength.argtypes = [POINTER(gageKind), c_int]
gageKindAnswerOffset = libteem.gageKindAnswerOffset
gageKindAnswerOffset.restype = c_int
gageKindAnswerOffset.argtypes = [POINTER(gageKind), c_int]
gageKindVolumeCheck = libteem.gageKindVolumeCheck
gageKindVolumeCheck.restype = c_int
gageKindVolumeCheck.argtypes = [POINTER(gageKind), POINTER(Nrrd)]
gageQueryPrint = libteem.gageQueryPrint
gageQueryPrint.restype = None
gageQueryPrint.argtypes = [POINTER(FILE), POINTER(gageKind), POINTER(c_ubyte)]
gageScl3PFilter_t = CFUNCTYPE(None, POINTER(gageShape), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int, c_int)
gageScl3PFilter2 = libteem.gageScl3PFilter2
gageScl3PFilter2.restype = None
gageScl3PFilter2.argtypes = [POINTER(gageShape), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int, c_int]
gageScl3PFilter4 = libteem.gageScl3PFilter4
gageScl3PFilter4.restype = None
gageScl3PFilter4.argtypes = [POINTER(gageShape), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int, c_int]
gageScl3PFilter6 = libteem.gageScl3PFilter6
gageScl3PFilter6.restype = None
gageScl3PFilter6.argtypes = [POINTER(gageShape), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int, c_int]
gageScl3PFilter8 = libteem.gageScl3PFilter8
gageScl3PFilter8.restype = None
gageScl3PFilter8.argtypes = [POINTER(gageShape), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int, c_int]
gageScl3PFilterN = libteem.gageScl3PFilterN
gageScl3PFilterN.restype = None
gageScl3PFilterN.argtypes = [POINTER(gageShape), c_int, POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int, c_int]
gageScl = (POINTER(airEnum)).in_dll(libteem, 'gageScl')
gageKindScl = (POINTER(gageKind)).in_dll(libteem, 'gageKindScl')
gageVec = (POINTER(airEnum)).in_dll(libteem, 'gageVec')
gageKindVec = (POINTER(gageKind)).in_dll(libteem, 'gageKindVec')
gageShapeReset = libteem.gageShapeReset
gageShapeReset.restype = None
gageShapeReset.argtypes = [POINTER(gageShape)]
gageShapeNew = libteem.gageShapeNew
gageShapeNew.restype = POINTER(gageShape)
gageShapeNew.argtypes = []
gageShapeCopy = libteem.gageShapeCopy
gageShapeCopy.restype = POINTER(gageShape)
gageShapeCopy.argtypes = [POINTER(gageShape)]
gageShapeNix = libteem.gageShapeNix
gageShapeNix.restype = POINTER(gageShape)
gageShapeNix.argtypes = [POINTER(gageShape)]
gageShapeSet = libteem.gageShapeSet
gageShapeSet.restype = c_int
gageShapeSet.argtypes = [POINTER(gageShape), POINTER(Nrrd), c_int]
gageShapeWtoI = libteem.gageShapeWtoI
gageShapeWtoI.restype = None
gageShapeWtoI.argtypes = [POINTER(gageShape), POINTER(c_double), POINTER(c_double)]
gageShapeItoW = libteem.gageShapeItoW
gageShapeItoW.restype = None
gageShapeItoW.argtypes = [POINTER(gageShape), POINTER(c_double), POINTER(c_double)]
gageShapeEqual = libteem.gageShapeEqual
gageShapeEqual.restype = c_int
gageShapeEqual.argtypes = [POINTER(gageShape), STRING, POINTER(gageShape), STRING]
gageShapeBoundingBox = libteem.gageShapeBoundingBox
gageShapeBoundingBox.restype = None
gageShapeBoundingBox.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(gageShape)]
gageVolumeCheck = libteem.gageVolumeCheck
gageVolumeCheck.restype = c_int
gageVolumeCheck.argtypes = [POINTER(gageContext), POINTER(Nrrd), POINTER(gageKind)]
gagePerVolumeNew = libteem.gagePerVolumeNew
gagePerVolumeNew.restype = POINTER(gagePerVolume)
gagePerVolumeNew.argtypes = [POINTER(gageContext), POINTER(Nrrd), POINTER(gageKind)]
gagePerVolumeNix = libteem.gagePerVolumeNix
gagePerVolumeNix.restype = POINTER(gagePerVolume)
gagePerVolumeNix.argtypes = [POINTER(gagePerVolume)]
gageAnswerPointer = libteem.gageAnswerPointer
gageAnswerPointer.restype = POINTER(c_double)
gageAnswerPointer.argtypes = [POINTER(gageContext), POINTER(gagePerVolume), c_int]
gageAnswerLength = libteem.gageAnswerLength
gageAnswerLength.restype = c_uint
gageAnswerLength.argtypes = [POINTER(gageContext), POINTER(gagePerVolume), c_int]
gageQueryReset = libteem.gageQueryReset
gageQueryReset.restype = c_int
gageQueryReset.argtypes = [POINTER(gageContext), POINTER(gagePerVolume)]
gageQuerySet = libteem.gageQuerySet
gageQuerySet.restype = c_int
gageQuerySet.argtypes = [POINTER(gageContext), POINTER(gagePerVolume), POINTER(c_ubyte)]
gageQueryAdd = libteem.gageQueryAdd
gageQueryAdd.restype = c_int
gageQueryAdd.argtypes = [POINTER(gageContext), POINTER(gagePerVolume), POINTER(c_ubyte)]
gageQueryItemOn = libteem.gageQueryItemOn
gageQueryItemOn.restype = c_int
gageQueryItemOn.argtypes = [POINTER(gageContext), POINTER(gagePerVolume), c_int]
gageOptimSigSet = libteem.gageOptimSigSet
gageOptimSigSet.restype = c_int
gageOptimSigSet.argtypes = [POINTER(c_double), c_uint, c_uint]
gageOptimSigParmNew = libteem.gageOptimSigParmNew
gageOptimSigParmNew.restype = POINTER(gageOptimSigParm)
gageOptimSigParmNew.argtypes = [c_uint]
gageOptimSigParmNix = libteem.gageOptimSigParmNix
gageOptimSigParmNix.restype = POINTER(gageOptimSigParm)
gageOptimSigParmNix.argtypes = [POINTER(gageOptimSigParm)]
gageOptimSigTruthSet = libteem.gageOptimSigTruthSet
gageOptimSigTruthSet.restype = c_int
gageOptimSigTruthSet.argtypes = [POINTER(gageOptimSigParm), c_uint, c_double, c_double, c_uint]
gageOptimSigCalculate = libteem.gageOptimSigCalculate
gageOptimSigCalculate.restype = c_int
gageOptimSigCalculate.argtypes = [POINTER(gageOptimSigParm), POINTER(c_double), c_uint, c_int, c_int, c_double, c_uint]
gageOptimSigPlot = libteem.gageOptimSigPlot
gageOptimSigPlot.restype = c_int
gageOptimSigPlot.argtypes = [POINTER(gageOptimSigParm), POINTER(Nrrd), POINTER(c_double), c_uint, c_int, c_int]
gageTauOfTee = libteem.gageTauOfTee
gageTauOfTee.restype = c_double
gageTauOfTee.argtypes = [c_double]
gageTeeOfTau = libteem.gageTeeOfTau
gageTeeOfTau.restype = c_double
gageTeeOfTau.argtypes = [c_double]
gageSigOfTau = libteem.gageSigOfTau
gageSigOfTau.restype = c_double
gageSigOfTau.argtypes = [c_double]
gageTauOfSig = libteem.gageTauOfSig
gageTauOfSig.restype = c_double
gageTauOfSig.argtypes = [c_double]
gageStackWtoI = libteem.gageStackWtoI
gageStackWtoI.restype = c_double
gageStackWtoI.argtypes = [POINTER(gageContext), c_double, POINTER(c_int)]
gageStackItoW = libteem.gageStackItoW
gageStackItoW.restype = c_double
gageStackItoW.argtypes = [POINTER(gageContext), c_double, POINTER(c_int)]
gageStackPerVolumeNew = libteem.gageStackPerVolumeNew
gageStackPerVolumeNew.restype = c_int
gageStackPerVolumeNew.argtypes = [POINTER(gageContext), POINTER(POINTER(gagePerVolume)), POINTER(POINTER(Nrrd)), c_uint, POINTER(gageKind)]
gageStackPerVolumeAttach = libteem.gageStackPerVolumeAttach
gageStackPerVolumeAttach.restype = c_int
gageStackPerVolumeAttach.argtypes = [POINTER(gageContext), POINTER(gagePerVolume), POINTER(POINTER(gagePerVolume)), POINTER(c_double), c_uint]
gageStackProbe = libteem.gageStackProbe
gageStackProbe.restype = c_int
gageStackProbe.argtypes = [POINTER(gageContext), c_double, c_double, c_double, c_double]
gageStackProbeSpace = libteem.gageStackProbeSpace
gageStackProbeSpace.restype = c_int
gageStackProbeSpace.argtypes = [POINTER(gageContext), c_double, c_double, c_double, c_double, c_int, c_int]
gageStackBlurParmNew = libteem.gageStackBlurParmNew
gageStackBlurParmNew.restype = POINTER(gageStackBlurParm)
gageStackBlurParmNew.argtypes = []
gageStackBlurParmNix = libteem.gageStackBlurParmNix
gageStackBlurParmNix.restype = POINTER(gageStackBlurParm)
gageStackBlurParmNix.argtypes = [POINTER(gageStackBlurParm)]
gageStackBlurParmScaleSet = libteem.gageStackBlurParmScaleSet
gageStackBlurParmScaleSet.restype = c_int
gageStackBlurParmScaleSet.argtypes = [POINTER(gageStackBlurParm), c_uint, c_double, c_double, c_int, c_int]
class NrrdKernel(Structure):
    pass
NrrdKernel._fields_ = [
    ('name', c_char * 129),
    ('numParm', c_uint),
    ('support', CFUNCTYPE(c_double, POINTER(c_double))),
    ('integral', CFUNCTYPE(c_double, POINTER(c_double))),
    ('eval1_f', CFUNCTYPE(c_float, c_float, POINTER(c_double))),
    ('evalN_f', CFUNCTYPE(None, POINTER(c_float), POINTER(c_float), size_t, POINTER(c_double))),
    ('eval1_d', CFUNCTYPE(c_double, c_double, POINTER(c_double))),
    ('evalN_d', CFUNCTYPE(None, POINTER(c_double), POINTER(c_double), size_t, POINTER(c_double))),
]
NrrdKernelSpec._pack_ = 4
NrrdKernelSpec._fields_ = [
    ('kernel', POINTER(NrrdKernel)),
    ('parm', c_double * 8),
]
gageStackBlurParmKernelSet = libteem.gageStackBlurParmKernelSet
gageStackBlurParmKernelSet.restype = c_int
gageStackBlurParmKernelSet.argtypes = [POINTER(gageStackBlurParm), POINTER(NrrdKernelSpec), c_int, c_int, c_int]
gageStackBlurParmCheck = libteem.gageStackBlurParmCheck
gageStackBlurParmCheck.restype = c_int
gageStackBlurParmCheck.argtypes = [POINTER(gageStackBlurParm)]
gageStackBlur = libteem.gageStackBlur
gageStackBlur.restype = c_int
gageStackBlur.argtypes = [POINTER(POINTER(Nrrd)), POINTER(gageStackBlurParm), POINTER(Nrrd), POINTER(gageKind)]
gageStackBlurCheck = libteem.gageStackBlurCheck
gageStackBlurCheck.restype = c_int
gageStackBlurCheck.argtypes = [POINTER(POINTER(Nrrd)), POINTER(gageStackBlurParm), POINTER(Nrrd), POINTER(gageKind)]
gageStackBlurGet = libteem.gageStackBlurGet
gageStackBlurGet.restype = c_int
gageStackBlurGet.argtypes = [POINTER(POINTER(Nrrd)), POINTER(c_int), POINTER(gageStackBlurParm), STRING, POINTER(Nrrd), POINTER(gageKind)]
class NrrdEncoding_t(Structure):
    pass
NrrdEncoding = NrrdEncoding_t
gageStackBlurManage = libteem.gageStackBlurManage
gageStackBlurManage.restype = c_int
gageStackBlurManage.argtypes = [POINTER(POINTER(POINTER(Nrrd))), POINTER(c_int), POINTER(gageStackBlurParm), STRING, c_int, POINTER(NrrdEncoding), POINTER(Nrrd), POINTER(gageKind)]
gageContextNew = libteem.gageContextNew
gageContextNew.restype = POINTER(gageContext)
gageContextNew.argtypes = []
gageContextCopy = libteem.gageContextCopy
gageContextCopy.restype = POINTER(gageContext)
gageContextCopy.argtypes = [POINTER(gageContext)]
gageContextNix = libteem.gageContextNix
gageContextNix.restype = POINTER(gageContext)
gageContextNix.argtypes = [POINTER(gageContext)]
gageParmSet = libteem.gageParmSet
gageParmSet.restype = None
gageParmSet.argtypes = [POINTER(gageContext), c_int, c_double]
gagePerVolumeIsAttached = libteem.gagePerVolumeIsAttached
gagePerVolumeIsAttached.restype = c_int
gagePerVolumeIsAttached.argtypes = [POINTER(gageContext), POINTER(gagePerVolume)]
gagePerVolumeAttach = libteem.gagePerVolumeAttach
gagePerVolumeAttach.restype = c_int
gagePerVolumeAttach.argtypes = [POINTER(gageContext), POINTER(gagePerVolume)]
gagePerVolumeDetach = libteem.gagePerVolumeDetach
gagePerVolumeDetach.restype = c_int
gagePerVolumeDetach.argtypes = [POINTER(gageContext), POINTER(gagePerVolume)]
gageKernelSet = libteem.gageKernelSet
gageKernelSet.restype = c_int
gageKernelSet.argtypes = [POINTER(gageContext), c_int, POINTER(NrrdKernel), POINTER(c_double)]
gageKernelReset = libteem.gageKernelReset
gageKernelReset.restype = None
gageKernelReset.argtypes = [POINTER(gageContext)]
gageProbe = libteem.gageProbe
gageProbe.restype = c_int
gageProbe.argtypes = [POINTER(gageContext), c_double, c_double, c_double]
gageProbeSpace = libteem.gageProbeSpace
gageProbeSpace.restype = c_int
gageProbeSpace.argtypes = [POINTER(gageContext), c_double, c_double, c_double, c_int, c_int]
gageUpdate = libteem.gageUpdate
gageUpdate.restype = c_int
gageUpdate.argtypes = [POINTER(gageContext)]
gageStructureTensor = libteem.gageStructureTensor
gageStructureTensor.restype = c_int
gageStructureTensor.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, c_int, c_int]
gageDeconvolve = libteem.gageDeconvolve
gageDeconvolve.restype = c_int
gageDeconvolve.argtypes = [POINTER(Nrrd), POINTER(c_double), POINTER(Nrrd), POINTER(gageKind), POINTER(NrrdKernelSpec), c_int, c_uint, c_int, c_double, c_double, c_int]
class hestCB(Structure):
    pass
hestCB._fields_ = [
    ('size', size_t),
    ('type', STRING),
    ('parse', CFUNCTYPE(c_int, c_void_p, STRING, STRING)),
    ('destroy', CFUNCTYPE(c_void_p, c_void_p)),
]
class hestOpt(Structure):
    pass
hestOpt._fields_ = [
    ('flag', STRING),
    ('name', STRING),
    ('type', c_int),
    ('min', c_uint),
    ('max', c_int),
    ('valueP', c_void_p),
    ('dflt', STRING),
    ('info', STRING),
    ('sawP', POINTER(c_uint)),
    ('enm', POINTER(airEnum)),
    ('CB', POINTER(hestCB)),
    ('kind', c_int),
    ('alloc', c_int),
]
class hestParm(Structure):
    pass
hestParm._fields_ = [
    ('verbosity', c_int),
    ('respFileEnable', c_int),
    ('elideSingleEnumType', c_int),
    ('elideSingleOtherType', c_int),
    ('elideSingleOtherDefault', c_int),
    ('elideSingleNonExistFloatDefault', c_int),
    ('elideMultipleNonExistFloatDefault', c_int),
    ('elideSingleEmptyStringDefault', c_int),
    ('elideMultipleEmptyStringDefault', c_int),
    ('greedySingleString', c_int),
    ('cleverPluralizeOtherY', c_int),
    ('columns', c_int),
    ('respFileFlag', c_char),
    ('respFileComment', c_char),
    ('varParamStopFlag', c_char),
    ('multiFlagSep', c_char),
]
hestVerbosity = (c_int).in_dll(libteem, 'hestVerbosity')
hestRespFileEnable = (c_int).in_dll(libteem, 'hestRespFileEnable')
hestElideSingleEnumType = (c_int).in_dll(libteem, 'hestElideSingleEnumType')
hestElideSingleOtherType = (c_int).in_dll(libteem, 'hestElideSingleOtherType')
hestElideSingleOtherDefault = (c_int).in_dll(libteem, 'hestElideSingleOtherDefault')
hestElideSingleNonExistFloatDefault = (c_int).in_dll(libteem, 'hestElideSingleNonExistFloatDefault')
hestElideMultipleNonExistFloatDefault = (c_int).in_dll(libteem, 'hestElideMultipleNonExistFloatDefault')
hestElideSingleEmptyStringDefault = (c_int).in_dll(libteem, 'hestElideSingleEmptyStringDefault')
hestElideMultipleEmptyStringDefault = (c_int).in_dll(libteem, 'hestElideMultipleEmptyStringDefault')
hestGreedySingleString = (c_int).in_dll(libteem, 'hestGreedySingleString')
hestCleverPluralizeOtherY = (c_int).in_dll(libteem, 'hestCleverPluralizeOtherY')
hestColumns = (c_int).in_dll(libteem, 'hestColumns')
hestRespFileFlag = (c_char).in_dll(libteem, 'hestRespFileFlag')
hestRespFileComment = (c_char).in_dll(libteem, 'hestRespFileComment')
hestVarParamStopFlag = (c_char).in_dll(libteem, 'hestVarParamStopFlag')
hestMultiFlagSep = (c_char).in_dll(libteem, 'hestMultiFlagSep')
hestParmNew = libteem.hestParmNew
hestParmNew.restype = POINTER(hestParm)
hestParmNew.argtypes = []
hestParmFree = libteem.hestParmFree
hestParmFree.restype = POINTER(hestParm)
hestParmFree.argtypes = [POINTER(hestParm)]
hestOptAdd = libteem.hestOptAdd
hestOptAdd.restype = None
hestOptAdd.argtypes = [POINTER(POINTER(hestOpt)), STRING, STRING, c_int, c_int, c_int, c_void_p, STRING, STRING]
hestOptFree = libteem.hestOptFree
hestOptFree.restype = POINTER(hestOpt)
hestOptFree.argtypes = [POINTER(hestOpt)]
hestOptCheck = libteem.hestOptCheck
hestOptCheck.restype = c_int
hestOptCheck.argtypes = [POINTER(hestOpt), POINTER(STRING)]
hestParse = libteem.hestParse
hestParse.restype = c_int
hestParse.argtypes = [POINTER(hestOpt), c_int, POINTER(STRING), POINTER(STRING), POINTER(hestParm)]
hestParseFree = libteem.hestParseFree
hestParseFree.restype = c_void_p
hestParseFree.argtypes = [POINTER(hestOpt)]
hestParseOrDie = libteem.hestParseOrDie
hestParseOrDie.restype = None
hestParseOrDie.argtypes = [POINTER(hestOpt), c_int, POINTER(STRING), POINTER(hestParm), STRING, STRING, c_int, c_int, c_int]
hestMinNumArgs = libteem.hestMinNumArgs
hestMinNumArgs.restype = c_int
hestMinNumArgs.argtypes = [POINTER(hestOpt)]
hestUsage = libteem.hestUsage
hestUsage.restype = None
hestUsage.argtypes = [POINTER(FILE), POINTER(hestOpt), STRING, POINTER(hestParm)]
hestGlossary = libteem.hestGlossary
hestGlossary.restype = None
hestGlossary.argtypes = [POINTER(FILE), POINTER(hestOpt), POINTER(hestParm)]
hestInfo = libteem.hestInfo
hestInfo.restype = None
hestInfo.argtypes = [POINTER(FILE), STRING, STRING, POINTER(hestParm)]
hooverRenderBegin_t = CFUNCTYPE(c_int, POINTER(c_void_p), c_void_p)
hooverThreadBegin_t = CFUNCTYPE(c_int, POINTER(c_void_p), c_void_p, c_void_p, c_int)
hooverRayBegin_t = CFUNCTYPE(c_int, c_void_p, c_void_p, c_void_p, c_int, c_int, c_double, POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double))
hooverSample_t = CFUNCTYPE(c_double, c_void_p, c_void_p, c_void_p, c_int, c_double, c_int, POINTER(c_double), POINTER(c_double))
hooverRayEnd_t = CFUNCTYPE(c_int, c_void_p, c_void_p, c_void_p)
hooverThreadEnd_t = CFUNCTYPE(c_int, c_void_p, c_void_p, c_void_p)
hooverRenderEnd_t = CFUNCTYPE(c_int, c_void_p, c_void_p)
class hooverContext(Structure):
    pass
hooverContext._pack_ = 4
hooverContext._fields_ = [
    ('cam', POINTER(limnCamera)),
    ('volSize', c_int * 3),
    ('volSpacing', c_double * 3),
    ('volCentering', c_int),
    ('imgSize', c_int * 2),
    ('imgCentering', c_int),
    ('user', c_void_p),
    ('numThreads', c_int),
    ('workIdx', c_int),
    ('workMutex', POINTER(airThreadMutex)),
    ('renderBegin', POINTER(hooverRenderBegin_t)),
    ('threadBegin', POINTER(hooverThreadBegin_t)),
    ('rayBegin', POINTER(hooverRayBegin_t)),
    ('sample', POINTER(hooverSample_t)),
    ('rayEnd', POINTER(hooverRayEnd_t)),
    ('threadEnd', POINTER(hooverThreadEnd_t)),
    ('renderEnd', POINTER(hooverRenderEnd_t)),
]
hooverBiffKey = (STRING).in_dll(libteem, 'hooverBiffKey')
hooverDefVolCentering = (c_int).in_dll(libteem, 'hooverDefVolCentering')
hooverDefImgCentering = (c_int).in_dll(libteem, 'hooverDefImgCentering')
hooverErr = (POINTER(airEnum)).in_dll(libteem, 'hooverErr')
hooverContextNew = libteem.hooverContextNew
hooverContextNew.restype = POINTER(hooverContext)
hooverContextNew.argtypes = []
hooverContextCheck = libteem.hooverContextCheck
hooverContextCheck.restype = c_int
hooverContextCheck.argtypes = [POINTER(hooverContext)]
hooverContextNix = libteem.hooverContextNix
hooverContextNix.restype = None
hooverContextNix.argtypes = [POINTER(hooverContext)]
hooverRender = libteem.hooverRender
hooverRender.restype = c_int
hooverRender.argtypes = [POINTER(hooverContext), POINTER(c_int), POINTER(c_int)]
hooverStubRenderBegin = libteem.hooverStubRenderBegin
hooverStubRenderBegin.restype = c_int
hooverStubRenderBegin.argtypes = [POINTER(c_void_p), c_void_p]
hooverStubThreadBegin = libteem.hooverStubThreadBegin
hooverStubThreadBegin.restype = c_int
hooverStubThreadBegin.argtypes = [POINTER(c_void_p), c_void_p, c_void_p, c_int]
hooverStubRayBegin = libteem.hooverStubRayBegin
hooverStubRayBegin.restype = c_int
hooverStubRayBegin.argtypes = [c_void_p, c_void_p, c_void_p, c_int, c_int, c_double, POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double)]
hooverStubSample = libteem.hooverStubSample
hooverStubSample.restype = c_double
hooverStubSample.argtypes = [c_void_p, c_void_p, c_void_p, c_int, c_double, c_int, POINTER(c_double), POINTER(c_double)]
hooverStubRayEnd = libteem.hooverStubRayEnd
hooverStubRayEnd.restype = c_int
hooverStubRayEnd.argtypes = [c_void_p, c_void_p, c_void_p]
hooverStubThreadEnd = libteem.hooverStubThreadEnd
hooverStubThreadEnd.restype = c_int
hooverStubThreadEnd.argtypes = [c_void_p, c_void_p, c_void_p]
hooverStubRenderEnd = libteem.hooverStubRenderEnd
hooverStubRenderEnd.restype = c_int
hooverStubRenderEnd.argtypes = [c_void_p, c_void_p]
limnCamera_t._pack_ = 4
limnCamera_t._fields_ = [
    ('from', c_double * 3),
    ('at', c_double * 3),
    ('up', c_double * 3),
    ('uRange', c_double * 2),
    ('vRange', c_double * 2),
    ('fov', c_double),
    ('aspect', c_double),
    ('neer', c_double),
    ('faar', c_double),
    ('dist', c_double),
    ('atRelative', c_int),
    ('orthographic', c_int),
    ('rightHanded', c_int),
    ('W2V', c_double * 16),
    ('V2W', c_double * 16),
    ('U', c_double * 4),
    ('V', c_double * 4),
    ('N', c_double * 4),
    ('vspNeer', c_double),
    ('vspFaar', c_double),
    ('vspDist', c_double),
]
class limnLight(Structure):
    pass
limnLight._fields_ = [
    ('amb', c_float * 4),
    ('_dir', c_float * 4 * 8),
    ('dir', c_float * 4 * 8),
    ('col', c_float * 4 * 8),
    ('on', c_int * 8),
    ('vsp', c_int * 8),
]
class limnOptsPS(Structure):
    pass
limnOptsPS._fields_ = [
    ('lineWidth', c_float * 8),
    ('creaseAngle', c_float),
    ('bg', c_float * 3),
    ('edgeColor', c_float * 3),
    ('showpage', c_int),
    ('wireFrame', c_int),
    ('noBackground', c_int),
]
class limnWindow(Structure):
    pass
limnWindow._fields_ = [
    ('ps', limnOptsPS),
    ('device', c_int),
    ('scale', c_float),
    ('bbox', c_float * 4),
    ('yFlip', c_int),
    ('file', POINTER(FILE)),
]
class limnLook(Structure):
    pass
limnLook._fields_ = [
    ('rgba', c_float * 4),
    ('kads', c_float * 3),
    ('spow', c_float),
]
class limnVertex(Structure):
    pass
limnVertex._fields_ = [
    ('world', c_float * 4),
    ('rgba', c_float * 4),
    ('coord', c_float * 4),
    ('worldNormal', c_float * 3),
]
class limnEdge_t(Structure):
    pass
limnEdge_t._fields_ = [
    ('vertIdx', c_uint * 2),
    ('lookIdx', c_uint),
    ('partIdx', c_uint),
    ('type', c_int),
    ('faceIdx', c_int * 2),
    ('once', c_int),
]
limnEdge = limnEdge_t
class limnFace_t(Structure):
    pass
limnFace_t._fields_ = [
    ('worldNormal', c_float * 3),
    ('screenNormal', c_float * 3),
    ('vertIdx', POINTER(c_uint)),
    ('edgeIdx', POINTER(c_uint)),
    ('sideNum', c_uint),
    ('lookIdx', c_uint),
    ('partIdx', c_uint),
    ('visible', c_int),
    ('depth', c_float),
]
limnFace = limnFace_t
class limnPart_t(Structure):
    pass
limnPart_t._fields_ = [
    ('vertIdx', POINTER(c_uint)),
    ('vertIdxNum', c_uint),
    ('vertIdxArr', POINTER(airArray)),
    ('edgeIdx', POINTER(c_uint)),
    ('edgeIdxNum', c_uint),
    ('edgeIdxArr', POINTER(airArray)),
    ('faceIdx', POINTER(c_uint)),
    ('faceIdxNum', c_uint),
    ('faceIdxArr', POINTER(airArray)),
    ('lookIdx', c_int),
    ('depth', c_float),
]
limnPart = limnPart_t
class limnObject(Structure):
    pass
limnObject._fields_ = [
    ('vert', POINTER(limnVertex)),
    ('vertNum', c_uint),
    ('vertArr', POINTER(airArray)),
    ('edge', POINTER(limnEdge)),
    ('edgeNum', c_uint),
    ('edgeArr', POINTER(airArray)),
    ('face', POINTER(limnFace)),
    ('faceNum', c_uint),
    ('faceArr', POINTER(airArray)),
    ('faceSort', POINTER(POINTER(limnFace))),
    ('part', POINTER(POINTER(limnPart))),
    ('partNum', c_uint),
    ('partArr', POINTER(airArray)),
    ('partPool', POINTER(POINTER(limnPart))),
    ('partPoolNum', c_uint),
    ('partPoolArr', POINTER(airArray)),
    ('look', POINTER(limnLook)),
    ('lookNum', c_uint),
    ('lookArr', POINTER(airArray)),
    ('vertSpace', c_int),
    ('setVertexRGBAFromLook', c_int),
    ('doEdges', c_int),
    ('incr', c_uint),
]
class limnPolyData(Structure):
    pass
limnPolyData._fields_ = [
    ('xyzw', POINTER(c_float)),
    ('xyzwNum', c_uint),
    ('rgba', POINTER(c_ubyte)),
    ('rgbaNum', c_uint),
    ('norm', POINTER(c_float)),
    ('normNum', c_uint),
    ('tex2', POINTER(c_float)),
    ('tex2Num', c_uint),
    ('indxNum', c_uint),
    ('indx', POINTER(c_uint)),
    ('primNum', c_uint),
    ('type', POINTER(c_ubyte)),
    ('icnt', POINTER(c_uint)),
]
class limnSpline_t(Structure):
    pass
limnSpline_t._pack_ = 4
limnSpline_t._fields_ = [
    ('type', c_int),
    ('info', c_int),
    ('loop', c_int),
    ('B', c_double),
    ('C', c_double),
    ('ncpt', POINTER(Nrrd)),
    ('time', POINTER(c_double)),
]
limnSpline = limnSpline_t
class limnSplineTypeSpec_t(Structure):
    pass
limnSplineTypeSpec_t._pack_ = 4
limnSplineTypeSpec_t._fields_ = [
    ('type', c_int),
    ('B', c_double),
    ('C', c_double),
]
limnSplineTypeSpec = limnSplineTypeSpec_t
limnBiffKey = (STRING).in_dll(libteem, 'limnBiffKey')
limnDefCameraAtRelative = (c_int).in_dll(libteem, 'limnDefCameraAtRelative')
limnDefCameraOrthographic = (c_int).in_dll(libteem, 'limnDefCameraOrthographic')
limnDefCameraRightHanded = (c_int).in_dll(libteem, 'limnDefCameraRightHanded')
limnSpace = (POINTER(airEnum)).in_dll(libteem, 'limnSpace')
limnPolyDataInfo = (POINTER(airEnum)).in_dll(libteem, 'limnPolyDataInfo')
limnCameraPathTrack = (POINTER(airEnum)).in_dll(libteem, 'limnCameraPathTrack')
limnPrimitive = (POINTER(airEnum)).in_dll(libteem, 'limnPrimitive')
limnQNBins = (c_int * 17).in_dll(libteem, 'limnQNBins')
limnQNtoV_f = (CFUNCTYPE(None, POINTER(c_float), c_int) * 17).in_dll(libteem, 'limnQNtoV_f')
limnQNtoV_d = (CFUNCTYPE(None, POINTER(c_double), c_int) * 17).in_dll(libteem, 'limnQNtoV_d')
limnVtoQN_f = (CFUNCTYPE(c_int, POINTER(c_float)) * 17).in_dll(libteem, 'limnVtoQN_f')
limnVtoQN_d = (CFUNCTYPE(c_int, POINTER(c_double)) * 17).in_dll(libteem, 'limnVtoQN_d')
limnLightSet = libteem.limnLightSet
limnLightSet.restype = None
limnLightSet.argtypes = [POINTER(limnLight), c_int, c_int, c_float, c_float, c_float, c_float, c_float, c_float]
limnLightAmbientSet = libteem.limnLightAmbientSet
limnLightAmbientSet.restype = None
limnLightAmbientSet.argtypes = [POINTER(limnLight), c_float, c_float, c_float]
limnLightSwitch = libteem.limnLightSwitch
limnLightSwitch.restype = None
limnLightSwitch.argtypes = [POINTER(limnLight), c_int, c_int]
limnLightReset = libteem.limnLightReset
limnLightReset.restype = None
limnLightReset.argtypes = [POINTER(limnLight)]
limnLightUpdate = libteem.limnLightUpdate
limnLightUpdate.restype = c_int
limnLightUpdate.argtypes = [POINTER(limnLight), POINTER(limnCamera)]
limnEnvMapCB = CFUNCTYPE(None, POINTER(c_float), POINTER(c_float), c_void_p)
limnEnvMapFill = libteem.limnEnvMapFill
limnEnvMapFill.restype = c_int
limnEnvMapFill.argtypes = [POINTER(Nrrd), limnEnvMapCB, c_int, c_void_p]
limnLightDiffuseCB = libteem.limnLightDiffuseCB
limnLightDiffuseCB.restype = None
limnLightDiffuseCB.argtypes = [POINTER(c_float), POINTER(c_float), c_void_p]
limnEnvMapCheck = libteem.limnEnvMapCheck
limnEnvMapCheck.restype = c_int
limnEnvMapCheck.argtypes = [POINTER(Nrrd)]
limnLightNew = libteem.limnLightNew
limnLightNew.restype = POINTER(limnLight)
limnLightNew.argtypes = []
limnCameraInit = libteem.limnCameraInit
limnCameraInit.restype = None
limnCameraInit.argtypes = [POINTER(limnCamera)]
limnLightNix = libteem.limnLightNix
limnLightNix.restype = POINTER(limnLight)
limnLightNix.argtypes = [POINTER(limnLight)]
limnCameraNew = libteem.limnCameraNew
limnCameraNew.restype = POINTER(limnCamera)
limnCameraNew.argtypes = []
limnCameraNix = libteem.limnCameraNix
limnCameraNix.restype = POINTER(limnCamera)
limnCameraNix.argtypes = [POINTER(limnCamera)]
limnWindowNew = libteem.limnWindowNew
limnWindowNew.restype = POINTER(limnWindow)
limnWindowNew.argtypes = [c_int]
limnWindowNix = libteem.limnWindowNix
limnWindowNix.restype = POINTER(limnWindow)
limnWindowNix.argtypes = [POINTER(limnWindow)]
limnHestCameraOptAdd = libteem.limnHestCameraOptAdd
limnHestCameraOptAdd.restype = None
limnHestCameraOptAdd.argtypes = [POINTER(POINTER(hestOpt)), POINTER(limnCamera), STRING, STRING, STRING, STRING, STRING, STRING, STRING, STRING, STRING]
limnCameraAspectSet = libteem.limnCameraAspectSet
limnCameraAspectSet.restype = c_int
limnCameraAspectSet.argtypes = [POINTER(limnCamera), c_int, c_int, c_int]
limnCameraUpdate = libteem.limnCameraUpdate
limnCameraUpdate.restype = c_int
limnCameraUpdate.argtypes = [POINTER(limnCamera)]
limnCameraPathMake = libteem.limnCameraPathMake
limnCameraPathMake.restype = c_int
limnCameraPathMake.argtypes = [POINTER(limnCamera), c_int, POINTER(limnCamera), POINTER(c_double), c_int, c_int, POINTER(limnSplineTypeSpec), POINTER(limnSplineTypeSpec), POINTER(limnSplineTypeSpec), POINTER(limnSplineTypeSpec)]
limnObjectLookAdd = libteem.limnObjectLookAdd
limnObjectLookAdd.restype = c_int
limnObjectLookAdd.argtypes = [POINTER(limnObject)]
limnObjectNew = libteem.limnObjectNew
limnObjectNew.restype = POINTER(limnObject)
limnObjectNew.argtypes = [c_int, c_int]
limnObjectNix = libteem.limnObjectNix
limnObjectNix.restype = POINTER(limnObject)
limnObjectNix.argtypes = [POINTER(limnObject)]
limnObjectEmpty = libteem.limnObjectEmpty
limnObjectEmpty.restype = None
limnObjectEmpty.argtypes = [POINTER(limnObject)]
limnObjectPreSet = libteem.limnObjectPreSet
limnObjectPreSet.restype = c_int
limnObjectPreSet.argtypes = [POINTER(limnObject), c_uint, c_uint, c_uint, c_uint, c_uint]
limnObjectPartAdd = libteem.limnObjectPartAdd
limnObjectPartAdd.restype = c_int
limnObjectPartAdd.argtypes = [POINTER(limnObject)]
limnObjectVertexNumPreSet = libteem.limnObjectVertexNumPreSet
limnObjectVertexNumPreSet.restype = c_int
limnObjectVertexNumPreSet.argtypes = [POINTER(limnObject), c_uint, c_uint]
limnObjectVertexAdd = libteem.limnObjectVertexAdd
limnObjectVertexAdd.restype = c_int
limnObjectVertexAdd.argtypes = [POINTER(limnObject), c_uint, c_float, c_float, c_float]
limnObjectEdgeAdd = libteem.limnObjectEdgeAdd
limnObjectEdgeAdd.restype = c_int
limnObjectEdgeAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_uint, c_uint, c_uint]
limnObjectFaceNumPreSet = libteem.limnObjectFaceNumPreSet
limnObjectFaceNumPreSet.restype = c_int
limnObjectFaceNumPreSet.argtypes = [POINTER(limnObject), c_uint, c_uint]
limnObjectFaceAdd = libteem.limnObjectFaceAdd
limnObjectFaceAdd.restype = c_int
limnObjectFaceAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_uint, POINTER(c_uint)]
limnPolyDataNew = libteem.limnPolyDataNew
limnPolyDataNew.restype = POINTER(limnPolyData)
limnPolyDataNew.argtypes = []
limnPolyDataNix = libteem.limnPolyDataNix
limnPolyDataNix.restype = POINTER(limnPolyData)
limnPolyDataNix.argtypes = [POINTER(limnPolyData)]
limnPolyDataInfoBitFlag = libteem.limnPolyDataInfoBitFlag
limnPolyDataInfoBitFlag.restype = c_uint
limnPolyDataInfoBitFlag.argtypes = [POINTER(limnPolyData)]
limnPolyDataAlloc = libteem.limnPolyDataAlloc
limnPolyDataAlloc.restype = c_int
limnPolyDataAlloc.argtypes = [POINTER(limnPolyData), c_uint, c_uint, c_uint, c_uint]
limnPolyDataSize = libteem.limnPolyDataSize
limnPolyDataSize.restype = size_t
limnPolyDataSize.argtypes = [POINTER(limnPolyData)]
limnPolyDataCopy = libteem.limnPolyDataCopy
limnPolyDataCopy.restype = c_int
limnPolyDataCopy.argtypes = [POINTER(limnPolyData), POINTER(limnPolyData)]
limnPolyDataCopyN = libteem.limnPolyDataCopyN
limnPolyDataCopyN.restype = c_int
limnPolyDataCopyN.argtypes = [POINTER(limnPolyData), POINTER(limnPolyData), c_uint]
limnPolyDataTransform_f = libteem.limnPolyDataTransform_f
limnPolyDataTransform_f.restype = None
limnPolyDataTransform_f.argtypes = [POINTER(limnPolyData), POINTER(c_float)]
limnPolyDataTransform_d = libteem.limnPolyDataTransform_d
limnPolyDataTransform_d.restype = None
limnPolyDataTransform_d.argtypes = [POINTER(limnPolyData), POINTER(c_double)]
limnPolyDataPolygonNumber = libteem.limnPolyDataPolygonNumber
limnPolyDataPolygonNumber.restype = c_uint
limnPolyDataPolygonNumber.argtypes = [POINTER(limnPolyData)]
limnPolyDataVertexNormals = libteem.limnPolyDataVertexNormals
limnPolyDataVertexNormals.restype = c_int
limnPolyDataVertexNormals.argtypes = [POINTER(limnPolyData)]
limnPolyDataPrimitiveTypes = libteem.limnPolyDataPrimitiveTypes
limnPolyDataPrimitiveTypes.restype = c_uint
limnPolyDataPrimitiveTypes.argtypes = [POINTER(limnPolyData)]
limnPolyDataPrimitiveVertexNumber = libteem.limnPolyDataPrimitiveVertexNumber
limnPolyDataPrimitiveVertexNumber.restype = c_int
limnPolyDataPrimitiveVertexNumber.argtypes = [POINTER(Nrrd), POINTER(limnPolyData)]
limnPolyDataPrimitiveArea = libteem.limnPolyDataPrimitiveArea
limnPolyDataPrimitiveArea.restype = c_int
limnPolyDataPrimitiveArea.argtypes = [POINTER(Nrrd), POINTER(limnPolyData)]
limnPolyDataRasterize = libteem.limnPolyDataRasterize
limnPolyDataRasterize.restype = c_int
limnPolyDataRasterize.argtypes = [POINTER(Nrrd), POINTER(limnPolyData), POINTER(c_double), POINTER(c_double), POINTER(size_t), c_int]
limnPolyDataColorSet = libteem.limnPolyDataColorSet
limnPolyDataColorSet.restype = None
limnPolyDataColorSet.argtypes = [POINTER(limnPolyData), c_ubyte, c_ubyte, c_ubyte, c_ubyte]
limnPolyDataCube = libteem.limnPolyDataCube
limnPolyDataCube.restype = c_int
limnPolyDataCube.argtypes = [POINTER(limnPolyData), c_uint, c_int]
limnPolyDataCone = libteem.limnPolyDataCone
limnPolyDataCone.restype = c_int
limnPolyDataCone.argtypes = [POINTER(limnPolyData), c_uint, c_uint, c_int]
limnPolyDataCylinder = libteem.limnPolyDataCylinder
limnPolyDataCylinder.restype = c_int
limnPolyDataCylinder.argtypes = [POINTER(limnPolyData), c_uint, c_uint, c_int]
limnPolyDataSuperquadric = libteem.limnPolyDataSuperquadric
limnPolyDataSuperquadric.restype = c_int
limnPolyDataSuperquadric.argtypes = [POINTER(limnPolyData), c_uint, c_float, c_float, c_uint, c_uint]
limnPolyDataSpiralSuperquadric = libteem.limnPolyDataSpiralSuperquadric
limnPolyDataSpiralSuperquadric.restype = c_int
limnPolyDataSpiralSuperquadric.argtypes = [POINTER(limnPolyData), c_uint, c_float, c_float, c_uint, c_uint]
limnPolyDataPolarSphere = libteem.limnPolyDataPolarSphere
limnPolyDataPolarSphere.restype = c_int
limnPolyDataPolarSphere.argtypes = [POINTER(limnPolyData), c_uint, c_uint, c_uint]
limnPolyDataSpiralSphere = libteem.limnPolyDataSpiralSphere
limnPolyDataSpiralSphere.restype = c_int
limnPolyDataSpiralSphere.argtypes = [POINTER(limnPolyData), c_uint, c_uint, c_uint]
limnPolyDataPlane = libteem.limnPolyDataPlane
limnPolyDataPlane.restype = c_int
limnPolyDataPlane.argtypes = [POINTER(limnPolyData), c_uint, c_uint, c_uint]
limnPolyDataVertexWindingFix = libteem.limnPolyDataVertexWindingFix
limnPolyDataVertexWindingFix.restype = c_int
limnPolyDataVertexWindingFix.argtypes = [POINTER(limnPolyData), c_int]
limnPolyDataClip = libteem.limnPolyDataClip
limnPolyDataClip.restype = c_int
limnPolyDataClip.argtypes = [POINTER(limnPolyData), POINTER(Nrrd), c_double]
limnPolyDataVertexWindingFlip = libteem.limnPolyDataVertexWindingFlip
limnPolyDataVertexWindingFlip.restype = c_int
limnPolyDataVertexWindingFlip.argtypes = [POINTER(limnPolyData)]
limnPolyDataCCFind = libteem.limnPolyDataCCFind
limnPolyDataCCFind.restype = c_int
limnPolyDataCCFind.argtypes = [POINTER(limnPolyData)]
limnPolyDataPrimitiveSort = libteem.limnPolyDataPrimitiveSort
limnPolyDataPrimitiveSort.restype = c_int
limnPolyDataPrimitiveSort.argtypes = [POINTER(limnPolyData), POINTER(Nrrd)]
limnPolyDataPrimitiveSelect = libteem.limnPolyDataPrimitiveSelect
limnPolyDataPrimitiveSelect.restype = c_int
limnPolyDataPrimitiveSelect.argtypes = [POINTER(limnPolyData), POINTER(limnPolyData), POINTER(Nrrd)]
limnPolyDataSpiralTubeWrap = libteem.limnPolyDataSpiralTubeWrap
limnPolyDataSpiralTubeWrap.restype = c_int
limnPolyDataSpiralTubeWrap.argtypes = [POINTER(limnPolyData), POINTER(limnPolyData), c_uint, POINTER(Nrrd), c_uint, c_uint, c_double]
limnObjectDescribe = libteem.limnObjectDescribe
limnObjectDescribe.restype = c_int
limnObjectDescribe.argtypes = [POINTER(FILE), POINTER(limnObject)]
limnObjectReadOFF = libteem.limnObjectReadOFF
limnObjectReadOFF.restype = c_int
limnObjectReadOFF.argtypes = [POINTER(limnObject), POINTER(FILE)]
limnObjectWriteOFF = libteem.limnObjectWriteOFF
limnObjectWriteOFF.restype = c_int
limnObjectWriteOFF.argtypes = [POINTER(FILE), POINTER(limnObject)]
limnPolyDataWriteIV = libteem.limnPolyDataWriteIV
limnPolyDataWriteIV.restype = c_int
limnPolyDataWriteIV.argtypes = [POINTER(FILE), POINTER(limnPolyData)]
limnPolyDataWriteLMPD = libteem.limnPolyDataWriteLMPD
limnPolyDataWriteLMPD.restype = c_int
limnPolyDataWriteLMPD.argtypes = [POINTER(FILE), POINTER(limnPolyData)]
limnPolyDataReadLMPD = libteem.limnPolyDataReadLMPD
limnPolyDataReadLMPD.restype = c_int
limnPolyDataReadLMPD.argtypes = [POINTER(limnPolyData), POINTER(FILE)]
limnPolyDataWriteVTK = libteem.limnPolyDataWriteVTK
limnPolyDataWriteVTK.restype = c_int
limnPolyDataWriteVTK.argtypes = [POINTER(FILE), POINTER(limnPolyData)]
limnPolyDataReadOFF = libteem.limnPolyDataReadOFF
limnPolyDataReadOFF.restype = c_int
limnPolyDataReadOFF.argtypes = [POINTER(limnPolyData), POINTER(FILE)]
limnPolyDataSave = libteem.limnPolyDataSave
limnPolyDataSave.restype = c_int
limnPolyDataSave.argtypes = [STRING, POINTER(limnPolyData)]
limnHestPolyDataLMPD = (POINTER(hestCB)).in_dll(libteem, 'limnHestPolyDataLMPD')
limnHestPolyDataOFF = (POINTER(hestCB)).in_dll(libteem, 'limnHestPolyDataOFF')
limnObjectCubeAdd = libteem.limnObjectCubeAdd
limnObjectCubeAdd.restype = c_int
limnObjectCubeAdd.argtypes = [POINTER(limnObject), c_uint]
limnObjectSquareAdd = libteem.limnObjectSquareAdd
limnObjectSquareAdd.restype = c_int
limnObjectSquareAdd.argtypes = [POINTER(limnObject), c_uint]
limnObjectCylinderAdd = libteem.limnObjectCylinderAdd
limnObjectCylinderAdd.restype = c_int
limnObjectCylinderAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_uint]
limnObjectPolarSphereAdd = libteem.limnObjectPolarSphereAdd
limnObjectPolarSphereAdd.restype = c_int
limnObjectPolarSphereAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_uint, c_uint]
limnObjectConeAdd = libteem.limnObjectConeAdd
limnObjectConeAdd.restype = c_int
limnObjectConeAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_uint]
limnObjectPolarSuperquadAdd = libteem.limnObjectPolarSuperquadAdd
limnObjectPolarSuperquadAdd.restype = c_int
limnObjectPolarSuperquadAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_float, c_float, c_uint, c_uint]
limnObjectPolarSuperquadFancyAdd = libteem.limnObjectPolarSuperquadFancyAdd
limnObjectPolarSuperquadFancyAdd.restype = c_int
limnObjectPolarSuperquadFancyAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_float, c_float, c_float, c_float, c_uint, c_uint]
limnObjectWorldHomog = libteem.limnObjectWorldHomog
limnObjectWorldHomog.restype = c_int
limnObjectWorldHomog.argtypes = [POINTER(limnObject)]
limnObjectFaceNormals = libteem.limnObjectFaceNormals
limnObjectFaceNormals.restype = c_int
limnObjectFaceNormals.argtypes = [POINTER(limnObject), c_int]
limnObjectVertexNormals = libteem.limnObjectVertexNormals
limnObjectVertexNormals.restype = c_int
limnObjectVertexNormals.argtypes = [POINTER(limnObject)]
limnObjectSpaceTransform = libteem.limnObjectSpaceTransform
limnObjectSpaceTransform.restype = c_int
limnObjectSpaceTransform.argtypes = [POINTER(limnObject), POINTER(limnCamera), POINTER(limnWindow), c_int]
limnObjectPartTransform = libteem.limnObjectPartTransform
limnObjectPartTransform.restype = c_int
limnObjectPartTransform.argtypes = [POINTER(limnObject), c_uint, POINTER(c_float)]
limnObjectDepthSortParts = libteem.limnObjectDepthSortParts
limnObjectDepthSortParts.restype = c_int
limnObjectDepthSortParts.argtypes = [POINTER(limnObject)]
limnObjectDepthSortFaces = libteem.limnObjectDepthSortFaces
limnObjectDepthSortFaces.restype = c_int
limnObjectDepthSortFaces.argtypes = [POINTER(limnObject)]
limnObjectFaceReverse = libteem.limnObjectFaceReverse
limnObjectFaceReverse.restype = c_int
limnObjectFaceReverse.argtypes = [POINTER(limnObject)]
limnObjectRender = libteem.limnObjectRender
limnObjectRender.restype = c_int
limnObjectRender.argtypes = [POINTER(limnObject), POINTER(limnCamera), POINTER(limnWindow)]
limnObjectPSDraw = libteem.limnObjectPSDraw
limnObjectPSDraw.restype = c_int
limnObjectPSDraw.argtypes = [POINTER(limnObject), POINTER(limnCamera), POINTER(Nrrd), POINTER(limnWindow)]
limnObjectPSDrawConcave = libteem.limnObjectPSDrawConcave
limnObjectPSDrawConcave.restype = c_int
limnObjectPSDrawConcave.argtypes = [POINTER(limnObject), POINTER(limnCamera), POINTER(Nrrd), POINTER(limnWindow)]
limnSplineTypeSpecNew = libteem.limnSplineTypeSpecNew
limnSplineTypeSpecNew.restype = POINTER(limnSplineTypeSpec)
limnSplineTypeSpecNew.argtypes = [c_int]
limnSplineTypeSpecNix = libteem.limnSplineTypeSpecNix
limnSplineTypeSpecNix.restype = POINTER(limnSplineTypeSpec)
limnSplineTypeSpecNix.argtypes = [POINTER(limnSplineTypeSpec)]
limnSplineNew = libteem.limnSplineNew
limnSplineNew.restype = POINTER(limnSpline)
limnSplineNew.argtypes = [POINTER(Nrrd), c_int, POINTER(limnSplineTypeSpec)]
limnSplineNix = libteem.limnSplineNix
limnSplineNix.restype = POINTER(limnSpline)
limnSplineNix.argtypes = [POINTER(limnSpline)]
limnSplineNrrdCleverFix = libteem.limnSplineNrrdCleverFix
limnSplineNrrdCleverFix.restype = c_int
limnSplineNrrdCleverFix.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, c_int]
limnSplineCleverNew = libteem.limnSplineCleverNew
limnSplineCleverNew.restype = POINTER(limnSpline)
limnSplineCleverNew.argtypes = [POINTER(Nrrd), c_int, POINTER(limnSplineTypeSpec)]
limnSplineUpdate = libteem.limnSplineUpdate
limnSplineUpdate.restype = c_int
limnSplineUpdate.argtypes = [POINTER(limnSpline), POINTER(Nrrd)]
limnSplineType = (POINTER(airEnum)).in_dll(libteem, 'limnSplineType')
limnSplineInfo = (POINTER(airEnum)).in_dll(libteem, 'limnSplineInfo')
limnSplineParse = libteem.limnSplineParse
limnSplineParse.restype = POINTER(limnSpline)
limnSplineParse.argtypes = [STRING]
limnSplineTypeSpecParse = libteem.limnSplineTypeSpecParse
limnSplineTypeSpecParse.restype = POINTER(limnSplineTypeSpec)
limnSplineTypeSpecParse.argtypes = [STRING]
limnHestSpline = (POINTER(hestCB)).in_dll(libteem, 'limnHestSpline')
limnHestSplineTypeSpec = (POINTER(hestCB)).in_dll(libteem, 'limnHestSplineTypeSpec')
limnSplineInfoSize = (c_uint * 7).in_dll(libteem, 'limnSplineInfoSize')
limnSplineTypeHasImplicitTangents = (c_int * 6).in_dll(libteem, 'limnSplineTypeHasImplicitTangents')
limnSplineNumPoints = libteem.limnSplineNumPoints
limnSplineNumPoints.restype = c_int
limnSplineNumPoints.argtypes = [POINTER(limnSpline)]
limnSplineMinT = libteem.limnSplineMinT
limnSplineMinT.restype = c_double
limnSplineMinT.argtypes = [POINTER(limnSpline)]
limnSplineMaxT = libteem.limnSplineMaxT
limnSplineMaxT.restype = c_double
limnSplineMaxT.argtypes = [POINTER(limnSpline)]
limnSplineBCSet = libteem.limnSplineBCSet
limnSplineBCSet.restype = None
limnSplineBCSet.argtypes = [POINTER(limnSpline), c_double, c_double]
limnSplineEvaluate = libteem.limnSplineEvaluate
limnSplineEvaluate.restype = None
limnSplineEvaluate.argtypes = [POINTER(c_double), POINTER(limnSpline), c_double]
limnSplineNrrdEvaluate = libteem.limnSplineNrrdEvaluate
limnSplineNrrdEvaluate.restype = c_int
limnSplineNrrdEvaluate.argtypes = [POINTER(Nrrd), POINTER(limnSpline), POINTER(Nrrd)]
limnSplineSample = libteem.limnSplineSample
limnSplineSample.restype = c_int
limnSplineSample.argtypes = [POINTER(Nrrd), POINTER(limnSpline), c_double, size_t, c_double]
meetBiffKey = (STRING).in_dll(libteem, 'meetBiffKey')
meetAirEnumAll = libteem.meetAirEnumAll
meetAirEnumAll.restype = POINTER(POINTER(airEnum))
meetAirEnumAll.argtypes = []
meetAirEnumAllPrint = libteem.meetAirEnumAllPrint
meetAirEnumAllPrint.restype = None
meetAirEnumAllPrint.argtypes = [POINTER(FILE)]
meetGageKindParse = libteem.meetGageKindParse
meetGageKindParse.restype = POINTER(gageKind)
meetGageKindParse.argtypes = [STRING]
meetConstGageKindParse = libteem.meetConstGageKindParse
meetConstGageKindParse.restype = POINTER(gageKind)
meetConstGageKindParse.argtypes = [STRING]
meetHestGageKind = (POINTER(hestCB)).in_dll(libteem, 'meetHestGageKind')
meetHestConstGageKind = (POINTER(hestCB)).in_dll(libteem, 'meetHestConstGageKind')
class meetPullVol(Structure):
    pass
meetPullVol._pack_ = 4
meetPullVol._fields_ = [
    ('kind', POINTER(gageKind)),
    ('fileName', STRING),
    ('volName', STRING),
    ('derivNormSS', c_int),
    ('uniformSS', c_int),
    ('optimSS', c_int),
    ('leeching', c_int),
    ('recomputedSS', c_int),
    ('numSS', c_uint),
    ('rangeSS', c_double * 2),
    ('posSS', POINTER(c_double)),
    ('nin', POINTER(Nrrd)),
    ('ninSS', POINTER(POINTER(Nrrd))),
]
class meetPullInfo(Structure):
    pass
meetPullInfo._pack_ = 4
meetPullInfo._fields_ = [
    ('info', c_int),
    ('constraint', c_int),
    ('volName', STRING),
    ('itemStr', STRING),
    ('zero', c_double),
    ('scale', c_double),
]
meetPullVolNew = libteem.meetPullVolNew
meetPullVolNew.restype = POINTER(meetPullVol)
meetPullVolNew.argtypes = []
meetPullVolParse = libteem.meetPullVolParse
meetPullVolParse.restype = c_int
meetPullVolParse.argtypes = [POINTER(meetPullVol), STRING]
meetPullVolLeechable = libteem.meetPullVolLeechable
meetPullVolLeechable.restype = c_int
meetPullVolLeechable.argtypes = [POINTER(meetPullVol), POINTER(meetPullVol)]
meetPullVolNuke = libteem.meetPullVolNuke
meetPullVolNuke.restype = POINTER(meetPullVol)
meetPullVolNuke.argtypes = [POINTER(meetPullVol)]
meetHestPullVol = (POINTER(hestCB)).in_dll(libteem, 'meetHestPullVol')
meetPullVolLoadMulti = libteem.meetPullVolLoadMulti
meetPullVolLoadMulti.restype = c_int
meetPullVolLoadMulti.argtypes = [POINTER(POINTER(meetPullVol)), c_uint, STRING, POINTER(NrrdKernelSpec), c_int]
class pullContext_t(Structure):
    pass
pullContext = pullContext_t
meetPullVolAddMulti = libteem.meetPullVolAddMulti
meetPullVolAddMulti.restype = c_int
meetPullVolAddMulti.argtypes = [POINTER(pullContext), POINTER(POINTER(meetPullVol)), c_uint, POINTER(NrrdKernelSpec), POINTER(NrrdKernelSpec), POINTER(NrrdKernelSpec), POINTER(NrrdKernelSpec)]
meetPullInfoNew = libteem.meetPullInfoNew
meetPullInfoNew.restype = POINTER(meetPullInfo)
meetPullInfoNew.argtypes = []
meetPullInfoNix = libteem.meetPullInfoNix
meetPullInfoNix.restype = POINTER(meetPullInfo)
meetPullInfoNix.argtypes = [POINTER(meetPullInfo)]
meetPullInfoParse = libteem.meetPullInfoParse
meetPullInfoParse.restype = c_int
meetPullInfoParse.argtypes = [POINTER(meetPullInfo), STRING]
meetHestPullInfo = (POINTER(hestCB)).in_dll(libteem, 'meetHestPullInfo')
meetPullInfoAddMulti = libteem.meetPullInfoAddMulti
meetPullInfoAddMulti.restype = c_int
meetPullInfoAddMulti.argtypes = [POINTER(pullContext), POINTER(POINTER(meetPullInfo)), c_uint]
mite_t = c_double
class miteUser(Structure):
    pass
miteUser._pack_ = 4
miteUser._fields_ = [
    ('nsin', POINTER(Nrrd)),
    ('nvin', POINTER(Nrrd)),
    ('ntin', POINTER(Nrrd)),
    ('ntxf', POINTER(POINTER(Nrrd))),
    ('nout', POINTER(Nrrd)),
    ('debug', POINTER(c_double)),
    ('debugArr', POINTER(airArray)),
    ('ndebug', POINTER(Nrrd)),
    ('debugIdx', c_int),
    ('ntxfNum', c_int),
    ('shadeStr', c_char * 257),
    ('normalStr', c_char * 257),
    ('rangeInit', mite_t * 9),
    ('refStep', c_double),
    ('rayStep', c_double),
    ('opacMatters', c_double),
    ('opacNear1', c_double),
    ('hctx', POINTER(hooverContext)),
    ('fakeFrom', c_double * 3),
    ('vectorD', c_double * 3),
    ('ksp', POINTER(NrrdKernelSpec) * 8),
    ('gctx0', POINTER(gageContext)),
    ('lit', POINTER(limnLight)),
    ('normalSide', c_int),
    ('verbUi', c_int),
    ('verbVi', c_int),
    ('umop', POINTER(airArray)),
    ('rendTime', c_double),
    ('sampRate', c_double),
]
class miteShadeSpec(Structure):
    pass
miteShadeSpec._fields_ = [
    ('method', c_int),
    ('vec0', POINTER(gageItemSpec)),
    ('vec1', POINTER(gageItemSpec)),
    ('scl0', POINTER(gageItemSpec)),
    ('scl1', POINTER(gageItemSpec)),
]
class miteRender(Structure):
    pass
class miteThread_t(Structure):
    pass
miteRender._pack_ = 4
miteRender._fields_ = [
    ('ntxf', POINTER(POINTER(Nrrd))),
    ('ntxfNum', c_int),
    ('sclPvlIdx', c_int),
    ('vecPvlIdx', c_int),
    ('tenPvlIdx', c_int),
    ('shadeSpec', POINTER(miteShadeSpec)),
    ('normalSpec', POINTER(gageItemSpec)),
    ('time0', c_double),
    ('queryMite', gageQuery),
    ('queryMiteNonzero', c_int),
    ('tt', POINTER(miteThread_t) * 512),
    ('rmop', POINTER(airArray)),
]
class miteStage(Structure):
    pass
miteStage._pack_ = 4
miteStage._fields_ = [
    ('val', POINTER(c_double)),
    ('size', c_int),
    ('op', c_int),
    ('qn', CFUNCTYPE(c_int, POINTER(c_double))),
    ('min', c_double),
    ('max', c_double),
    ('data', POINTER(mite_t)),
    ('rangeIdx', c_int * 9),
    ('rangeNum', c_int),
    ('label', STRING),
]
miteThread_t._pack_ = 4
miteThread_t._fields_ = [
    ('gctx', POINTER(gageContext)),
    ('ansScl', POINTER(c_double)),
    ('nPerp', POINTER(c_double)),
    ('geomTens', POINTER(c_double)),
    ('ansVec', POINTER(c_double)),
    ('ansTen', POINTER(c_double)),
    ('ansMiteVal', POINTER(c_double)),
    ('directAnsMiteVal', POINTER(POINTER(c_double))),
    ('_normal', POINTER(c_double)),
    ('shadeVec0', POINTER(c_double)),
    ('shadeVec1', POINTER(c_double)),
    ('shadeScl0', POINTER(c_double)),
    ('shadeScl1', POINTER(c_double)),
    ('verbose', c_int),
    ('skip', c_int),
    ('thrid', c_int),
    ('ui', c_int),
    ('vi', c_int),
    ('raySample', c_int),
    ('samples', c_int),
    ('stage', POINTER(miteStage)),
    ('stageNum', c_int),
    ('range', mite_t * 9),
    ('rayStep', mite_t),
    ('V', mite_t * 3),
    ('RR', mite_t),
    ('GG', mite_t),
    ('BB', mite_t),
    ('TT', mite_t),
    ('ZZ', mite_t),
    ('rmop', POINTER(airArray)),
]
miteThread = miteThread_t
miteBiffKey = (STRING).in_dll(libteem, 'miteBiffKey')
miteDefRefStep = (c_double).in_dll(libteem, 'miteDefRefStep')
miteDefRenorm = (c_int).in_dll(libteem, 'miteDefRenorm')
miteDefNormalSide = (c_int).in_dll(libteem, 'miteDefNormalSide')
miteDefOpacNear1 = (c_double).in_dll(libteem, 'miteDefOpacNear1')
miteDefOpacMatters = (c_double).in_dll(libteem, 'miteDefOpacMatters')
miteVal = (POINTER(airEnum)).in_dll(libteem, 'miteVal')
miteValGageKind = (POINTER(gageKind)).in_dll(libteem, 'miteValGageKind')
miteStageOp = (POINTER(airEnum)).in_dll(libteem, 'miteStageOp')
miteRangeChar = (c_char * 10).in_dll(libteem, 'miteRangeChar')
miteVariableParse = libteem.miteVariableParse
miteVariableParse.restype = c_int
miteVariableParse.argtypes = [POINTER(gageItemSpec), STRING]
miteVariablePrint = libteem.miteVariablePrint
miteVariablePrint.restype = None
miteVariablePrint.argtypes = [STRING, POINTER(gageItemSpec)]
miteNtxfCheck = libteem.miteNtxfCheck
miteNtxfCheck.restype = c_int
miteNtxfCheck.argtypes = [POINTER(Nrrd)]
miteQueryAdd = libteem.miteQueryAdd
miteQueryAdd.restype = None
miteQueryAdd.argtypes = [POINTER(c_ubyte), POINTER(c_ubyte), POINTER(c_ubyte), POINTER(c_ubyte), POINTER(gageItemSpec)]
miteUserNew = libteem.miteUserNew
miteUserNew.restype = POINTER(miteUser)
miteUserNew.argtypes = []
miteUserNix = libteem.miteUserNix
miteUserNix.restype = POINTER(miteUser)
miteUserNix.argtypes = [POINTER(miteUser)]
miteShadeSpecNew = libteem.miteShadeSpecNew
miteShadeSpecNew.restype = POINTER(miteShadeSpec)
miteShadeSpecNew.argtypes = []
miteShadeSpecNix = libteem.miteShadeSpecNix
miteShadeSpecNix.restype = POINTER(miteShadeSpec)
miteShadeSpecNix.argtypes = [POINTER(miteShadeSpec)]
miteShadeSpecParse = libteem.miteShadeSpecParse
miteShadeSpecParse.restype = c_int
miteShadeSpecParse.argtypes = [POINTER(miteShadeSpec), STRING]
miteShadeSpecPrint = libteem.miteShadeSpecPrint
miteShadeSpecPrint.restype = None
miteShadeSpecPrint.argtypes = [STRING, POINTER(miteShadeSpec)]
miteShadeSpecQueryAdd = libteem.miteShadeSpecQueryAdd
miteShadeSpecQueryAdd.restype = None
miteShadeSpecQueryAdd.argtypes = [POINTER(c_ubyte), POINTER(c_ubyte), POINTER(c_ubyte), POINTER(c_ubyte), POINTER(miteShadeSpec)]
miteRenderBegin = libteem.miteRenderBegin
miteRenderBegin.restype = c_int
miteRenderBegin.argtypes = [POINTER(POINTER(miteRender)), POINTER(miteUser)]
miteRenderEnd = libteem.miteRenderEnd
miteRenderEnd.restype = c_int
miteRenderEnd.argtypes = [POINTER(miteRender), POINTER(miteUser)]
miteThreadNew = libteem.miteThreadNew
miteThreadNew.restype = POINTER(miteThread)
miteThreadNew.argtypes = []
miteThreadNix = libteem.miteThreadNix
miteThreadNix.restype = POINTER(miteThread)
miteThreadNix.argtypes = [POINTER(miteThread)]
miteThreadBegin = libteem.miteThreadBegin
miteThreadBegin.restype = c_int
miteThreadBegin.argtypes = [POINTER(POINTER(miteThread)), POINTER(miteRender), POINTER(miteUser), c_int]
miteThreadEnd = libteem.miteThreadEnd
miteThreadEnd.restype = c_int
miteThreadEnd.argtypes = [POINTER(miteThread), POINTER(miteRender), POINTER(miteUser)]
miteRayBegin = libteem.miteRayBegin
miteRayBegin.restype = c_int
miteRayBegin.argtypes = [POINTER(miteThread), POINTER(miteRender), POINTER(miteUser), c_int, c_int, c_double, POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double)]
miteSample = libteem.miteSample
miteSample.restype = c_double
miteSample.argtypes = [POINTER(miteThread), POINTER(miteRender), POINTER(miteUser), c_int, c_double, c_int, POINTER(c_double), POINTER(c_double)]
miteRayEnd = libteem.miteRayEnd
miteRayEnd.restype = c_int
miteRayEnd.argtypes = [POINTER(miteThread), POINTER(miteRender), POINTER(miteUser)]
nrrdDefaultWriteEncodingType = (c_int).in_dll(libteem, 'nrrdDefaultWriteEncodingType')
nrrdDefaultWriteBareText = (c_int).in_dll(libteem, 'nrrdDefaultWriteBareText')
nrrdDefaultWriteCharsPerLine = (c_uint).in_dll(libteem, 'nrrdDefaultWriteCharsPerLine')
nrrdDefaultWriteValsPerLine = (c_uint).in_dll(libteem, 'nrrdDefaultWriteValsPerLine')
nrrdDefaultResampleBoundary = (c_int).in_dll(libteem, 'nrrdDefaultResampleBoundary')
nrrdDefaultResampleType = (c_int).in_dll(libteem, 'nrrdDefaultResampleType')
nrrdDefaultResampleRenormalize = (c_int).in_dll(libteem, 'nrrdDefaultResampleRenormalize')
nrrdDefaultResampleRound = (c_int).in_dll(libteem, 'nrrdDefaultResampleRound')
nrrdDefaultResampleClamp = (c_int).in_dll(libteem, 'nrrdDefaultResampleClamp')
nrrdDefaultResampleCheap = (c_int).in_dll(libteem, 'nrrdDefaultResampleCheap')
nrrdDefaultResamplePadValue = (c_double).in_dll(libteem, 'nrrdDefaultResamplePadValue')
nrrdDefaultKernelParm0 = (c_double).in_dll(libteem, 'nrrdDefaultKernelParm0')
nrrdDefaultCenter = (c_int).in_dll(libteem, 'nrrdDefaultCenter')
nrrdDefaultSpacing = (c_double).in_dll(libteem, 'nrrdDefaultSpacing')
nrrdStateVerboseIO = (c_int).in_dll(libteem, 'nrrdStateVerboseIO')
nrrdStateKeyValuePairsPropagate = (c_int).in_dll(libteem, 'nrrdStateKeyValuePairsPropagate')
nrrdStateBlind8BitRange = (c_int).in_dll(libteem, 'nrrdStateBlind8BitRange')
nrrdStateMeasureType = (c_int).in_dll(libteem, 'nrrdStateMeasureType')
nrrdStateMeasureModeBins = (c_int).in_dll(libteem, 'nrrdStateMeasureModeBins')
nrrdStateMeasureHistoType = (c_int).in_dll(libteem, 'nrrdStateMeasureHistoType')
nrrdStateDisallowIntegerNonExist = (c_int).in_dll(libteem, 'nrrdStateDisallowIntegerNonExist')
nrrdStateAlwaysSetContent = (c_int).in_dll(libteem, 'nrrdStateAlwaysSetContent')
nrrdStateDisableContent = (c_int).in_dll(libteem, 'nrrdStateDisableContent')
nrrdStateUnknownContent = (STRING).in_dll(libteem, 'nrrdStateUnknownContent')
nrrdStateGrayscaleImage3D = (c_int).in_dll(libteem, 'nrrdStateGrayscaleImage3D')
nrrdStateKeyValueReturnInternalPointers = (c_int).in_dll(libteem, 'nrrdStateKeyValueReturnInternalPointers')
nrrdStateKindNoop = (c_int).in_dll(libteem, 'nrrdStateKindNoop')
nrrdEnvVarDefaultWriteEncodingType = (STRING).in_dll(libteem, 'nrrdEnvVarDefaultWriteEncodingType')
nrrdEnvVarDefaultWriteBareText = (STRING).in_dll(libteem, 'nrrdEnvVarDefaultWriteBareText')
nrrdEnvVarDefaultWriteBareTextOld = (STRING).in_dll(libteem, 'nrrdEnvVarDefaultWriteBareTextOld')
nrrdEnvVarDefaultCenter = (STRING).in_dll(libteem, 'nrrdEnvVarDefaultCenter')
nrrdEnvVarDefaultCenterOld = (STRING).in_dll(libteem, 'nrrdEnvVarDefaultCenterOld')
nrrdEnvVarDefaultWriteCharsPerLine = (STRING).in_dll(libteem, 'nrrdEnvVarDefaultWriteCharsPerLine')
nrrdEnvVarDefaultWriteValsPerLine = (STRING).in_dll(libteem, 'nrrdEnvVarDefaultWriteValsPerLine')
nrrdEnvVarDefaultKernelParm0 = (STRING).in_dll(libteem, 'nrrdEnvVarDefaultKernelParm0')
nrrdEnvVarDefaultSpacing = (STRING).in_dll(libteem, 'nrrdEnvVarDefaultSpacing')
nrrdEnvVarStateKindNoop = (STRING).in_dll(libteem, 'nrrdEnvVarStateKindNoop')
nrrdEnvVarStateVerboseIO = (STRING).in_dll(libteem, 'nrrdEnvVarStateVerboseIO')
nrrdEnvVarStateKeyValuePairsPropagate = (STRING).in_dll(libteem, 'nrrdEnvVarStateKeyValuePairsPropagate')
nrrdEnvVarStateBlind8BitRange = (STRING).in_dll(libteem, 'nrrdEnvVarStateBlind8BitRange')
nrrdEnvVarStateAlwaysSetContent = (STRING).in_dll(libteem, 'nrrdEnvVarStateAlwaysSetContent')
nrrdEnvVarStateDisableContent = (STRING).in_dll(libteem, 'nrrdEnvVarStateDisableContent')
nrrdEnvVarStateMeasureType = (STRING).in_dll(libteem, 'nrrdEnvVarStateMeasureType')
nrrdEnvVarStateMeasureModeBins = (STRING).in_dll(libteem, 'nrrdEnvVarStateMeasureModeBins')
nrrdEnvVarStateMeasureHistoType = (STRING).in_dll(libteem, 'nrrdEnvVarStateMeasureHistoType')
nrrdEnvVarStateGrayscaleImage3D = (STRING).in_dll(libteem, 'nrrdEnvVarStateGrayscaleImage3D')
nrrdGetenvBool = libteem.nrrdGetenvBool
nrrdGetenvBool.restype = c_int
nrrdGetenvBool.argtypes = [POINTER(c_int), POINTER(STRING), STRING]
nrrdGetenvEnum = libteem.nrrdGetenvEnum
nrrdGetenvEnum.restype = c_int
nrrdGetenvEnum.argtypes = [POINTER(c_int), POINTER(STRING), POINTER(airEnum), STRING]
nrrdGetenvInt = libteem.nrrdGetenvInt
nrrdGetenvInt.restype = c_int
nrrdGetenvInt.argtypes = [POINTER(c_int), POINTER(STRING), STRING]
nrrdGetenvUInt = libteem.nrrdGetenvUInt
nrrdGetenvUInt.restype = c_int
nrrdGetenvUInt.argtypes = [POINTER(c_uint), POINTER(STRING), STRING]
nrrdGetenvDouble = libteem.nrrdGetenvDouble
nrrdGetenvDouble.restype = c_int
nrrdGetenvDouble.argtypes = [POINTER(c_double), POINTER(STRING), STRING]
nrrdDefaultGetenv = libteem.nrrdDefaultGetenv
nrrdDefaultGetenv.restype = None
nrrdDefaultGetenv.argtypes = []
nrrdStateGetenv = libteem.nrrdStateGetenv
nrrdStateGetenv.restype = None
nrrdStateGetenv.argtypes = []
nrrdFormatType = (POINTER(airEnum)).in_dll(libteem, 'nrrdFormatType')
nrrdType = (POINTER(airEnum)).in_dll(libteem, 'nrrdType')
nrrdEncodingType = (POINTER(airEnum)).in_dll(libteem, 'nrrdEncodingType')
nrrdCenter = (POINTER(airEnum)).in_dll(libteem, 'nrrdCenter')
nrrdKind = (POINTER(airEnum)).in_dll(libteem, 'nrrdKind')
nrrdField = (POINTER(airEnum)).in_dll(libteem, 'nrrdField')
nrrdSpace = (POINTER(airEnum)).in_dll(libteem, 'nrrdSpace')
nrrdSpacingStatus = (POINTER(airEnum)).in_dll(libteem, 'nrrdSpacingStatus')
nrrdBoundary = (POINTER(airEnum)).in_dll(libteem, 'nrrdBoundary')
nrrdMeasure = (POINTER(airEnum)).in_dll(libteem, 'nrrdMeasure')
nrrdUnaryOp = (POINTER(airEnum)).in_dll(libteem, 'nrrdUnaryOp')
nrrdBinaryOp = (POINTER(airEnum)).in_dll(libteem, 'nrrdBinaryOp')
nrrdTernaryOp = (POINTER(airEnum)).in_dll(libteem, 'nrrdTernaryOp')
nrrdTypePrintfStr = (c_char * 129 * 0).in_dll(libteem, 'nrrdTypePrintfStr')
nrrdTypeSize = (size_t * 0).in_dll(libteem, 'nrrdTypeSize')
nrrdTypeMin = (c_double * 0).in_dll(libteem, 'nrrdTypeMin')
nrrdTypeMax = (c_double * 0).in_dll(libteem, 'nrrdTypeMax')
nrrdTypeIsIntegral = (c_int * 0).in_dll(libteem, 'nrrdTypeIsIntegral')
nrrdTypeIsUnsigned = (c_int * 0).in_dll(libteem, 'nrrdTypeIsUnsigned')
nrrdTypeNumberOfValues = (c_double * 0).in_dll(libteem, 'nrrdTypeNumberOfValues')
class NrrdIoState_t(Structure):
    pass
NrrdIoState = NrrdIoState_t
nrrdIoStateNew = libteem.nrrdIoStateNew
nrrdIoStateNew.restype = POINTER(NrrdIoState)
nrrdIoStateNew.argtypes = []
nrrdIoStateInit = libteem.nrrdIoStateInit
nrrdIoStateInit.restype = None
nrrdIoStateInit.argtypes = [POINTER(NrrdIoState)]
nrrdIoStateNix = libteem.nrrdIoStateNix
nrrdIoStateNix.restype = POINTER(NrrdIoState)
nrrdIoStateNix.argtypes = [POINTER(NrrdIoState)]
class NrrdResampleInfo(Structure):
    pass
nrrdResampleInfoNew = libteem.nrrdResampleInfoNew
nrrdResampleInfoNew.restype = POINTER(NrrdResampleInfo)
nrrdResampleInfoNew.argtypes = []
nrrdResampleInfoNix = libteem.nrrdResampleInfoNix
nrrdResampleInfoNix.restype = POINTER(NrrdResampleInfo)
nrrdResampleInfoNix.argtypes = [POINTER(NrrdResampleInfo)]
nrrdKernelSpecNew = libteem.nrrdKernelSpecNew
nrrdKernelSpecNew.restype = POINTER(NrrdKernelSpec)
nrrdKernelSpecNew.argtypes = []
nrrdKernelSpecCopy = libteem.nrrdKernelSpecCopy
nrrdKernelSpecCopy.restype = POINTER(NrrdKernelSpec)
nrrdKernelSpecCopy.argtypes = [POINTER(NrrdKernelSpec)]
nrrdKernelSpecSet = libteem.nrrdKernelSpecSet
nrrdKernelSpecSet.restype = None
nrrdKernelSpecSet.argtypes = [POINTER(NrrdKernelSpec), POINTER(NrrdKernel), POINTER(c_double)]
nrrdKernelParmSet = libteem.nrrdKernelParmSet
nrrdKernelParmSet.restype = None
nrrdKernelParmSet.argtypes = [POINTER(POINTER(NrrdKernel)), POINTER(c_double), POINTER(NrrdKernelSpec)]
nrrdKernelSpecNix = libteem.nrrdKernelSpecNix
nrrdKernelSpecNix.restype = POINTER(NrrdKernelSpec)
nrrdKernelSpecNix.argtypes = [POINTER(NrrdKernelSpec)]
nrrdInit = libteem.nrrdInit
nrrdInit.restype = None
nrrdInit.argtypes = [POINTER(Nrrd)]
nrrdNew = libteem.nrrdNew
nrrdNew.restype = POINTER(Nrrd)
nrrdNew.argtypes = []
nrrdNix = libteem.nrrdNix
nrrdNix.restype = POINTER(Nrrd)
nrrdNix.argtypes = [POINTER(Nrrd)]
nrrdEmpty = libteem.nrrdEmpty
nrrdEmpty.restype = POINTER(Nrrd)
nrrdEmpty.argtypes = [POINTER(Nrrd)]
nrrdNuke = libteem.nrrdNuke
nrrdNuke.restype = POINTER(Nrrd)
nrrdNuke.argtypes = [POINTER(Nrrd)]
nrrdWrap_nva = libteem.nrrdWrap_nva
nrrdWrap_nva.restype = c_int
nrrdWrap_nva.argtypes = [POINTER(Nrrd), c_void_p, c_int, c_uint, POINTER(size_t)]
nrrdWrap_va = libteem.nrrdWrap_va
nrrdWrap_va.restype = c_int
nrrdWrap_va.argtypes = [POINTER(Nrrd), c_void_p, c_int, c_uint]
nrrdBasicInfoInit = libteem.nrrdBasicInfoInit
nrrdBasicInfoInit.restype = None
nrrdBasicInfoInit.argtypes = [POINTER(Nrrd), c_int]
nrrdBasicInfoCopy = libteem.nrrdBasicInfoCopy
nrrdBasicInfoCopy.restype = c_int
nrrdBasicInfoCopy.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdCopy = libteem.nrrdCopy
nrrdCopy.restype = c_int
nrrdCopy.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
nrrdAlloc_nva = libteem.nrrdAlloc_nva
nrrdAlloc_nva.restype = c_int
nrrdAlloc_nva.argtypes = [POINTER(Nrrd), c_int, c_uint, POINTER(size_t)]
nrrdAlloc_va = libteem.nrrdAlloc_va
nrrdAlloc_va.restype = c_int
nrrdAlloc_va.argtypes = [POINTER(Nrrd), c_int, c_uint]
nrrdMaybeAlloc_nva = libteem.nrrdMaybeAlloc_nva
nrrdMaybeAlloc_nva.restype = c_int
nrrdMaybeAlloc_nva.argtypes = [POINTER(Nrrd), c_int, c_uint, POINTER(size_t)]
nrrdMaybeAlloc_va = libteem.nrrdMaybeAlloc_va
nrrdMaybeAlloc_va.restype = c_int
nrrdMaybeAlloc_va.argtypes = [POINTER(Nrrd), c_int, c_uint]
nrrdPPM = libteem.nrrdPPM
nrrdPPM.restype = c_int
nrrdPPM.argtypes = [POINTER(Nrrd), size_t, size_t]
nrrdPGM = libteem.nrrdPGM
nrrdPGM.restype = c_int
nrrdPGM.argtypes = [POINTER(Nrrd), size_t, size_t]
nrrdKindIsDomain = libteem.nrrdKindIsDomain
nrrdKindIsDomain.restype = c_int
nrrdKindIsDomain.argtypes = [c_int]
nrrdKindSize = libteem.nrrdKindSize
nrrdKindSize.restype = c_uint
nrrdKindSize.argtypes = [c_int]
nrrdAxisInfoCopy = libteem.nrrdAxisInfoCopy
nrrdAxisInfoCopy.restype = c_int
nrrdAxisInfoCopy.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(c_int), c_int]
nrrdAxisInfoSet_nva = libteem.nrrdAxisInfoSet_nva
nrrdAxisInfoSet_nva.restype = None
nrrdAxisInfoSet_nva.argtypes = [POINTER(Nrrd), c_int, c_void_p]
nrrdAxisInfoSet_va = libteem.nrrdAxisInfoSet_va
nrrdAxisInfoSet_va.restype = None
nrrdAxisInfoSet_va.argtypes = [POINTER(Nrrd), c_int]
nrrdAxisInfoGet_nva = libteem.nrrdAxisInfoGet_nva
nrrdAxisInfoGet_nva.restype = None
nrrdAxisInfoGet_nva.argtypes = [POINTER(Nrrd), c_int, c_void_p]
nrrdAxisInfoGet_va = libteem.nrrdAxisInfoGet_va
nrrdAxisInfoGet_va.restype = None
nrrdAxisInfoGet_va.argtypes = [POINTER(Nrrd), c_int]
nrrdAxisInfoPos = libteem.nrrdAxisInfoPos
nrrdAxisInfoPos.restype = c_double
nrrdAxisInfoPos.argtypes = [POINTER(Nrrd), c_uint, c_double]
nrrdAxisInfoIdx = libteem.nrrdAxisInfoIdx
nrrdAxisInfoIdx.restype = c_double
nrrdAxisInfoIdx.argtypes = [POINTER(Nrrd), c_uint, c_double]
nrrdAxisInfoPosRange = libteem.nrrdAxisInfoPosRange
nrrdAxisInfoPosRange.restype = None
nrrdAxisInfoPosRange.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(Nrrd), c_uint, c_double, c_double]
nrrdAxisInfoIdxRange = libteem.nrrdAxisInfoIdxRange
nrrdAxisInfoIdxRange.restype = None
nrrdAxisInfoIdxRange.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(Nrrd), c_uint, c_double, c_double]
nrrdAxisInfoSpacingSet = libteem.nrrdAxisInfoSpacingSet
nrrdAxisInfoSpacingSet.restype = None
nrrdAxisInfoSpacingSet.argtypes = [POINTER(Nrrd), c_uint]
nrrdAxisInfoMinMaxSet = libteem.nrrdAxisInfoMinMaxSet
nrrdAxisInfoMinMaxSet.restype = None
nrrdAxisInfoMinMaxSet.argtypes = [POINTER(Nrrd), c_uint, c_int]
nrrdDomainAxesGet = libteem.nrrdDomainAxesGet
nrrdDomainAxesGet.restype = c_uint
nrrdDomainAxesGet.argtypes = [POINTER(Nrrd), POINTER(c_uint)]
nrrdRangeAxesGet = libteem.nrrdRangeAxesGet
nrrdRangeAxesGet.restype = c_uint
nrrdRangeAxesGet.argtypes = [POINTER(Nrrd), POINTER(c_uint)]
nrrdSpatialAxesGet = libteem.nrrdSpatialAxesGet
nrrdSpatialAxesGet.restype = c_uint
nrrdSpatialAxesGet.argtypes = [POINTER(Nrrd), POINTER(c_uint)]
nrrdNonSpatialAxesGet = libteem.nrrdNonSpatialAxesGet
nrrdNonSpatialAxesGet.restype = c_uint
nrrdNonSpatialAxesGet.argtypes = [POINTER(Nrrd), POINTER(c_uint)]
nrrdSpacingCalculate = libteem.nrrdSpacingCalculate
nrrdSpacingCalculate.restype = c_int
nrrdSpacingCalculate.argtypes = [POINTER(Nrrd), c_uint, POINTER(c_double), POINTER(c_double)]
nrrdOrientationReduce = libteem.nrrdOrientationReduce
nrrdOrientationReduce.restype = c_int
nrrdOrientationReduce.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdBiffKey = (STRING).in_dll(libteem, 'nrrdBiffKey')
nrrdSpaceDimension = libteem.nrrdSpaceDimension
nrrdSpaceDimension.restype = c_uint
nrrdSpaceDimension.argtypes = [c_int]
nrrdSpaceSet = libteem.nrrdSpaceSet
nrrdSpaceSet.restype = c_int
nrrdSpaceSet.argtypes = [POINTER(Nrrd), c_int]
nrrdSpaceDimensionSet = libteem.nrrdSpaceDimensionSet
nrrdSpaceDimensionSet.restype = c_int
nrrdSpaceDimensionSet.argtypes = [POINTER(Nrrd), c_uint]
nrrdSpaceOriginGet = libteem.nrrdSpaceOriginGet
nrrdSpaceOriginGet.restype = c_uint
nrrdSpaceOriginGet.argtypes = [POINTER(Nrrd), POINTER(c_double)]
nrrdSpaceOriginSet = libteem.nrrdSpaceOriginSet
nrrdSpaceOriginSet.restype = c_int
nrrdSpaceOriginSet.argtypes = [POINTER(Nrrd), POINTER(c_double)]
nrrdOriginCalculate = libteem.nrrdOriginCalculate
nrrdOriginCalculate.restype = c_int
nrrdOriginCalculate.argtypes = [POINTER(Nrrd), POINTER(c_uint), c_uint, c_int, POINTER(c_double)]
nrrdContentSet_va = libteem.nrrdContentSet_va
nrrdContentSet_va.restype = c_int
nrrdContentSet_va.argtypes = [POINTER(Nrrd), STRING, POINTER(Nrrd), STRING]
nrrdDescribe = libteem.nrrdDescribe
nrrdDescribe.restype = None
nrrdDescribe.argtypes = [POINTER(FILE), POINTER(Nrrd)]
nrrdCheck = libteem.nrrdCheck
nrrdCheck.restype = c_int
nrrdCheck.argtypes = [POINTER(Nrrd)]
nrrdElementSize = libteem.nrrdElementSize
nrrdElementSize.restype = size_t
nrrdElementSize.argtypes = [POINTER(Nrrd)]
nrrdElementNumber = libteem.nrrdElementNumber
nrrdElementNumber.restype = size_t
nrrdElementNumber.argtypes = [POINTER(Nrrd)]
nrrdSanity = libteem.nrrdSanity
nrrdSanity.restype = c_int
nrrdSanity.argtypes = []
nrrdSameSize = libteem.nrrdSameSize
nrrdSameSize.restype = c_int
nrrdSameSize.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdSpaceVecCopy = libteem.nrrdSpaceVecCopy
nrrdSpaceVecCopy.restype = None
nrrdSpaceVecCopy.argtypes = [POINTER(c_double), POINTER(c_double)]
nrrdSpaceVecScaleAdd2 = libteem.nrrdSpaceVecScaleAdd2
nrrdSpaceVecScaleAdd2.restype = None
nrrdSpaceVecScaleAdd2.argtypes = [POINTER(c_double), c_double, POINTER(c_double), c_double, POINTER(c_double)]
nrrdSpaceVecScale = libteem.nrrdSpaceVecScale
nrrdSpaceVecScale.restype = None
nrrdSpaceVecScale.argtypes = [POINTER(c_double), c_double, POINTER(c_double)]
nrrdSpaceVecNorm = libteem.nrrdSpaceVecNorm
nrrdSpaceVecNorm.restype = c_double
nrrdSpaceVecNorm.argtypes = [c_int, POINTER(c_double)]
nrrdSpaceVecSetNaN = libteem.nrrdSpaceVecSetNaN
nrrdSpaceVecSetNaN.restype = None
nrrdSpaceVecSetNaN.argtypes = [POINTER(c_double)]
nrrdCommentAdd = libteem.nrrdCommentAdd
nrrdCommentAdd.restype = c_int
nrrdCommentAdd.argtypes = [POINTER(Nrrd), STRING]
nrrdCommentClear = libteem.nrrdCommentClear
nrrdCommentClear.restype = None
nrrdCommentClear.argtypes = [POINTER(Nrrd)]
nrrdCommentCopy = libteem.nrrdCommentCopy
nrrdCommentCopy.restype = c_int
nrrdCommentCopy.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
nrrdKeyValueSize = libteem.nrrdKeyValueSize
nrrdKeyValueSize.restype = c_uint
nrrdKeyValueSize.argtypes = [POINTER(Nrrd)]
nrrdKeyValueAdd = libteem.nrrdKeyValueAdd
nrrdKeyValueAdd.restype = c_int
nrrdKeyValueAdd.argtypes = [POINTER(Nrrd), STRING, STRING]
nrrdKeyValueGet = libteem.nrrdKeyValueGet
nrrdKeyValueGet.restype = STRING
nrrdKeyValueGet.argtypes = [POINTER(Nrrd), STRING]
nrrdKeyValueIndex = libteem.nrrdKeyValueIndex
nrrdKeyValueIndex.restype = None
nrrdKeyValueIndex.argtypes = [POINTER(Nrrd), POINTER(STRING), POINTER(STRING), c_uint]
nrrdKeyValueErase = libteem.nrrdKeyValueErase
nrrdKeyValueErase.restype = c_int
nrrdKeyValueErase.argtypes = [POINTER(Nrrd), STRING]
nrrdKeyValueClear = libteem.nrrdKeyValueClear
nrrdKeyValueClear.restype = None
nrrdKeyValueClear.argtypes = [POINTER(Nrrd)]
nrrdKeyValueCopy = libteem.nrrdKeyValueCopy
nrrdKeyValueCopy.restype = c_int
nrrdKeyValueCopy.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
nrrdSwapEndian = libteem.nrrdSwapEndian
nrrdSwapEndian.restype = None
nrrdSwapEndian.argtypes = [POINTER(Nrrd)]
class NrrdFormat(Structure):
    pass
NrrdEncoding_t._fields_ = [
    ('name', c_char * 129),
    ('suffix', c_char * 129),
    ('endianMatters', c_int),
    ('isCompression', c_int),
    ('available', CFUNCTYPE(c_int)),
    ('read', CFUNCTYPE(c_int, POINTER(FILE), c_void_p, size_t, POINTER(Nrrd), POINTER(NrrdIoState_t))),
    ('write', CFUNCTYPE(c_int, POINTER(FILE), c_void_p, size_t, POINTER(Nrrd), POINTER(NrrdIoState_t))),
]
NrrdFormat._fields_ = [
    ('name', c_char * 129),
    ('isImage', c_int),
    ('readable', c_int),
    ('usesDIO', c_int),
    ('available', CFUNCTYPE(c_int)),
    ('nameLooksLike', CFUNCTYPE(c_int, STRING)),
    ('fitsInto', CFUNCTYPE(c_int, POINTER(Nrrd), POINTER(NrrdEncoding_t), c_int)),
    ('contentStartsLike', CFUNCTYPE(c_int, POINTER(NrrdIoState_t))),
    ('read', CFUNCTYPE(c_int, POINTER(FILE), POINTER(Nrrd), POINTER(NrrdIoState_t))),
    ('write', CFUNCTYPE(c_int, POINTER(FILE), POINTER(Nrrd), POINTER(NrrdIoState_t))),
]
nrrdFormatNRRD = (POINTER(NrrdFormat)).in_dll(libteem, 'nrrdFormatNRRD')
nrrdFormatPNM = (POINTER(NrrdFormat)).in_dll(libteem, 'nrrdFormatPNM')
nrrdFormatPNG = (POINTER(NrrdFormat)).in_dll(libteem, 'nrrdFormatPNG')
nrrdFormatVTK = (POINTER(NrrdFormat)).in_dll(libteem, 'nrrdFormatVTK')
nrrdFormatText = (POINTER(NrrdFormat)).in_dll(libteem, 'nrrdFormatText')
nrrdFormatEPS = (POINTER(NrrdFormat)).in_dll(libteem, 'nrrdFormatEPS')
nrrdFormatUnknown = (POINTER(NrrdFormat)).in_dll(libteem, 'nrrdFormatUnknown')
nrrdFormatArray = (POINTER(NrrdFormat) * 7).in_dll(libteem, 'nrrdFormatArray')
nrrdEncodingRaw = (POINTER(NrrdEncoding)).in_dll(libteem, 'nrrdEncodingRaw')
nrrdEncodingAscii = (POINTER(NrrdEncoding)).in_dll(libteem, 'nrrdEncodingAscii')
nrrdEncodingHex = (POINTER(NrrdEncoding)).in_dll(libteem, 'nrrdEncodingHex')
nrrdEncodingGzip = (POINTER(NrrdEncoding)).in_dll(libteem, 'nrrdEncodingGzip')
nrrdEncodingBzip2 = (POINTER(NrrdEncoding)).in_dll(libteem, 'nrrdEncodingBzip2')
nrrdEncodingUnknown = (POINTER(NrrdEncoding)).in_dll(libteem, 'nrrdEncodingUnknown')
nrrdEncodingArray = (POINTER(NrrdEncoding) * 6).in_dll(libteem, 'nrrdEncodingArray')
nrrdFieldInfoParse = (CFUNCTYPE(c_int, POINTER(FILE), POINTER(Nrrd), POINTER(NrrdIoState), c_int) * 33).in_dll(libteem, 'nrrdFieldInfoParse')
nrrdLineSkip = libteem.nrrdLineSkip
nrrdLineSkip.restype = c_int
nrrdLineSkip.argtypes = [POINTER(FILE), POINTER(NrrdIoState)]
nrrdByteSkip = libteem.nrrdByteSkip
nrrdByteSkip.restype = c_int
nrrdByteSkip.argtypes = [POINTER(FILE), POINTER(Nrrd), POINTER(NrrdIoState)]
nrrdLoad = libteem.nrrdLoad
nrrdLoad.restype = c_int
nrrdLoad.argtypes = [POINTER(Nrrd), STRING, POINTER(NrrdIoState)]
nrrdLoadMulti = libteem.nrrdLoadMulti
nrrdLoadMulti.restype = c_int
nrrdLoadMulti.argtypes = [POINTER(POINTER(Nrrd)), c_uint, STRING, c_uint, POINTER(NrrdIoState)]
nrrdRead = libteem.nrrdRead
nrrdRead.restype = c_int
nrrdRead.argtypes = [POINTER(Nrrd), POINTER(FILE), POINTER(NrrdIoState)]
nrrdStringRead = libteem.nrrdStringRead
nrrdStringRead.restype = c_int
nrrdStringRead.argtypes = [POINTER(Nrrd), STRING, POINTER(NrrdIoState)]
nrrdIoStateSet = libteem.nrrdIoStateSet
nrrdIoStateSet.restype = c_int
nrrdIoStateSet.argtypes = [POINTER(NrrdIoState), c_int, c_int]
nrrdIoStateEncodingSet = libteem.nrrdIoStateEncodingSet
nrrdIoStateEncodingSet.restype = c_int
nrrdIoStateEncodingSet.argtypes = [POINTER(NrrdIoState), POINTER(NrrdEncoding)]
nrrdIoStateFormatSet = libteem.nrrdIoStateFormatSet
nrrdIoStateFormatSet.restype = c_int
nrrdIoStateFormatSet.argtypes = [POINTER(NrrdIoState), POINTER(NrrdFormat)]
nrrdIoStateGet = libteem.nrrdIoStateGet
nrrdIoStateGet.restype = c_int
nrrdIoStateGet.argtypes = [POINTER(NrrdIoState), c_int]
nrrdIoStateEncodingGet = libteem.nrrdIoStateEncodingGet
nrrdIoStateEncodingGet.restype = POINTER(NrrdEncoding)
nrrdIoStateEncodingGet.argtypes = [POINTER(NrrdIoState)]
nrrdIoStateFormatGet = libteem.nrrdIoStateFormatGet
nrrdIoStateFormatGet.restype = POINTER(NrrdFormat)
nrrdIoStateFormatGet.argtypes = [POINTER(NrrdIoState)]
nrrdSave = libteem.nrrdSave
nrrdSave.restype = c_int
nrrdSave.argtypes = [STRING, POINTER(Nrrd), POINTER(NrrdIoState)]
nrrdSaveMulti = libteem.nrrdSaveMulti
nrrdSaveMulti.restype = c_int
nrrdSaveMulti.argtypes = [STRING, POINTER(POINTER(Nrrd)), c_uint, c_uint, POINTER(NrrdIoState)]
nrrdWrite = libteem.nrrdWrite
nrrdWrite.restype = c_int
nrrdWrite.argtypes = [POINTER(FILE), POINTER(Nrrd), POINTER(NrrdIoState)]
nrrdStringWrite = libteem.nrrdStringWrite
nrrdStringWrite.restype = c_int
nrrdStringWrite.argtypes = [POINTER(STRING), POINTER(Nrrd), POINTER(NrrdIoState)]
nrrdDLoad = (CFUNCTYPE(c_double, c_void_p) * 12).in_dll(libteem, 'nrrdDLoad')
nrrdFLoad = (CFUNCTYPE(c_float, c_void_p) * 12).in_dll(libteem, 'nrrdFLoad')
nrrdILoad = (CFUNCTYPE(c_int, c_void_p) * 12).in_dll(libteem, 'nrrdILoad')
nrrdUILoad = (CFUNCTYPE(c_uint, c_void_p) * 12).in_dll(libteem, 'nrrdUILoad')
nrrdDStore = (CFUNCTYPE(c_double, c_void_p, c_double) * 12).in_dll(libteem, 'nrrdDStore')
nrrdFStore = (CFUNCTYPE(c_float, c_void_p, c_float) * 12).in_dll(libteem, 'nrrdFStore')
nrrdIStore = (CFUNCTYPE(c_int, c_void_p, c_int) * 12).in_dll(libteem, 'nrrdIStore')
nrrdUIStore = (CFUNCTYPE(c_uint, c_void_p, c_uint) * 12).in_dll(libteem, 'nrrdUIStore')
nrrdDLookup = (CFUNCTYPE(c_double, c_void_p, size_t) * 12).in_dll(libteem, 'nrrdDLookup')
nrrdFLookup = (CFUNCTYPE(c_float, c_void_p, size_t) * 12).in_dll(libteem, 'nrrdFLookup')
nrrdILookup = (CFUNCTYPE(c_int, c_void_p, size_t) * 12).in_dll(libteem, 'nrrdILookup')
nrrdUILookup = (CFUNCTYPE(c_uint, c_void_p, size_t) * 12).in_dll(libteem, 'nrrdUILookup')
nrrdDInsert = (CFUNCTYPE(c_double, c_void_p, size_t, c_double) * 12).in_dll(libteem, 'nrrdDInsert')
nrrdFInsert = (CFUNCTYPE(c_float, c_void_p, size_t, c_float) * 12).in_dll(libteem, 'nrrdFInsert')
nrrdIInsert = (CFUNCTYPE(c_int, c_void_p, size_t, c_int) * 12).in_dll(libteem, 'nrrdIInsert')
nrrdUIInsert = (CFUNCTYPE(c_uint, c_void_p, size_t, c_uint) * 12).in_dll(libteem, 'nrrdUIInsert')
nrrdSprint = (CFUNCTYPE(c_int, STRING, c_void_p) * 12).in_dll(libteem, 'nrrdSprint')
nrrdFprint = (CFUNCTYPE(c_int, POINTER(FILE), c_void_p) * 12).in_dll(libteem, 'nrrdFprint')
nrrdMinMaxExactFind = (CFUNCTYPE(None, c_void_p, c_void_p, POINTER(c_int), POINTER(Nrrd)) * 12).in_dll(libteem, 'nrrdMinMaxExactFind')
nrrdValCompare = (CFUNCTYPE(c_int, c_void_p, c_void_p) * 12).in_dll(libteem, 'nrrdValCompare')
nrrdValCompareInv = (CFUNCTYPE(c_int, c_void_p, c_void_p) * 12).in_dll(libteem, 'nrrdValCompareInv')
nrrdAxesInsert = libteem.nrrdAxesInsert
nrrdAxesInsert.restype = c_int
nrrdAxesInsert.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdInvertPerm = libteem.nrrdInvertPerm
nrrdInvertPerm.restype = c_int
nrrdInvertPerm.argtypes = [POINTER(c_uint), POINTER(c_uint), c_uint]
nrrdAxesPermute = libteem.nrrdAxesPermute
nrrdAxesPermute.restype = c_int
nrrdAxesPermute.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(c_uint)]
nrrdShuffle = libteem.nrrdShuffle
nrrdShuffle.restype = c_int
nrrdShuffle.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, POINTER(size_t)]
nrrdAxesSwap = libteem.nrrdAxesSwap
nrrdAxesSwap.restype = c_int
nrrdAxesSwap.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, c_uint]
nrrdFlip = libteem.nrrdFlip
nrrdFlip.restype = c_int
nrrdFlip.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdJoin = libteem.nrrdJoin
nrrdJoin.restype = c_int
nrrdJoin.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), c_uint, c_uint, c_int]
nrrdReshape_va = libteem.nrrdReshape_va
nrrdReshape_va.restype = c_int
nrrdReshape_va.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdReshape_nva = libteem.nrrdReshape_nva
nrrdReshape_nva.restype = c_int
nrrdReshape_nva.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, POINTER(size_t)]
nrrdAxesSplit = libteem.nrrdAxesSplit
nrrdAxesSplit.restype = c_int
nrrdAxesSplit.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, size_t, size_t]
nrrdAxesDelete = libteem.nrrdAxesDelete
nrrdAxesDelete.restype = c_int
nrrdAxesDelete.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdAxesMerge = libteem.nrrdAxesMerge
nrrdAxesMerge.restype = c_int
nrrdAxesMerge.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdBlock = libteem.nrrdBlock
nrrdBlock.restype = c_int
nrrdBlock.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
nrrdUnblock = libteem.nrrdUnblock
nrrdUnblock.restype = c_int
nrrdUnblock.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdTile2D = libteem.nrrdTile2D
nrrdTile2D.restype = c_int
nrrdTile2D.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, c_uint, c_uint, size_t, size_t]
nrrdUntile2D = libteem.nrrdUntile2D
nrrdUntile2D.restype = c_int
nrrdUntile2D.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, c_uint, c_uint, size_t, size_t]
nrrdHestNrrd = (POINTER(hestCB)).in_dll(libteem, 'nrrdHestNrrd')
nrrdHestKernelSpec = (POINTER(hestCB)).in_dll(libteem, 'nrrdHestKernelSpec')
nrrdHestIter = (POINTER(hestCB)).in_dll(libteem, 'nrrdHestIter')
class NrrdIter(Structure):
    pass
nrrdIterNew = libteem.nrrdIterNew
nrrdIterNew.restype = POINTER(NrrdIter)
nrrdIterNew.argtypes = []
nrrdIterSetValue = libteem.nrrdIterSetValue
nrrdIterSetValue.restype = None
nrrdIterSetValue.argtypes = [POINTER(NrrdIter), c_double]
nrrdIterSetNrrd = libteem.nrrdIterSetNrrd
nrrdIterSetNrrd.restype = None
nrrdIterSetNrrd.argtypes = [POINTER(NrrdIter), POINTER(Nrrd)]
nrrdIterSetOwnNrrd = libteem.nrrdIterSetOwnNrrd
nrrdIterSetOwnNrrd.restype = None
nrrdIterSetOwnNrrd.argtypes = [POINTER(NrrdIter), POINTER(Nrrd)]
nrrdIterValue = libteem.nrrdIterValue
nrrdIterValue.restype = c_double
nrrdIterValue.argtypes = [POINTER(NrrdIter)]
nrrdIterContent = libteem.nrrdIterContent
nrrdIterContent.restype = STRING
nrrdIterContent.argtypes = [POINTER(NrrdIter)]
nrrdIterNix = libteem.nrrdIterNix
nrrdIterNix.restype = POINTER(NrrdIter)
nrrdIterNix.argtypes = [POINTER(NrrdIter)]
class NrrdRange(Structure):
    pass
nrrdRangeNew = libteem.nrrdRangeNew
nrrdRangeNew.restype = POINTER(NrrdRange)
nrrdRangeNew.argtypes = [c_double, c_double]
NrrdRange._pack_ = 4
NrrdRange._fields_ = [
    ('min', c_double),
    ('max', c_double),
    ('hasNonExist', c_int),
]
nrrdRangeCopy = libteem.nrrdRangeCopy
nrrdRangeCopy.restype = POINTER(NrrdRange)
nrrdRangeCopy.argtypes = [POINTER(NrrdRange)]
nrrdRangeNix = libteem.nrrdRangeNix
nrrdRangeNix.restype = POINTER(NrrdRange)
nrrdRangeNix.argtypes = [POINTER(NrrdRange)]
nrrdRangeReset = libteem.nrrdRangeReset
nrrdRangeReset.restype = None
nrrdRangeReset.argtypes = [POINTER(NrrdRange)]
nrrdRangeSet = libteem.nrrdRangeSet
nrrdRangeSet.restype = None
nrrdRangeSet.argtypes = [POINTER(NrrdRange), POINTER(Nrrd), c_int]
nrrdRangeSafeSet = libteem.nrrdRangeSafeSet
nrrdRangeSafeSet.restype = None
nrrdRangeSafeSet.argtypes = [POINTER(NrrdRange), POINTER(Nrrd), c_int]
nrrdRangeNewSet = libteem.nrrdRangeNewSet
nrrdRangeNewSet.restype = POINTER(NrrdRange)
nrrdRangeNewSet.argtypes = [POINTER(Nrrd), c_int]
nrrdHasNonExist = libteem.nrrdHasNonExist
nrrdHasNonExist.restype = c_int
nrrdHasNonExist.argtypes = [POINTER(Nrrd)]
nrrdFClamp = (CFUNCTYPE(c_float, c_float) * 12).in_dll(libteem, 'nrrdFClamp')
nrrdDClamp = (CFUNCTYPE(c_double, c_double) * 12).in_dll(libteem, 'nrrdDClamp')
nrrdConvert = libteem.nrrdConvert
nrrdConvert.restype = c_int
nrrdConvert.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdClampConvert = libteem.nrrdClampConvert
nrrdClampConvert.restype = c_int
nrrdClampConvert.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdQuantize = libteem.nrrdQuantize
nrrdQuantize.restype = c_int
nrrdQuantize.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), c_uint]
nrrdUnquantize = libteem.nrrdUnquantize
nrrdUnquantize.restype = c_int
nrrdUnquantize.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdHistoEq = libteem.nrrdHistoEq
nrrdHistoEq.restype = c_int
nrrdHistoEq.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(POINTER(Nrrd)), c_uint, c_uint, c_float]
nrrdApply1DLut = libteem.nrrdApply1DLut
nrrdApply1DLut.restype = c_int
nrrdApply1DLut.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), POINTER(Nrrd), c_int, c_int]
nrrdApplyMulti1DLut = libteem.nrrdApplyMulti1DLut
nrrdApplyMulti1DLut.restype = c_int
nrrdApplyMulti1DLut.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), POINTER(Nrrd), c_int, c_int]
nrrdApply1DRegMap = libteem.nrrdApply1DRegMap
nrrdApply1DRegMap.restype = c_int
nrrdApply1DRegMap.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), POINTER(Nrrd), c_int, c_int]
nrrdApplyMulti1DRegMap = libteem.nrrdApplyMulti1DRegMap
nrrdApplyMulti1DRegMap.restype = c_int
nrrdApplyMulti1DRegMap.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), POINTER(Nrrd), c_int, c_int]
nrrd1DIrregMapCheck = libteem.nrrd1DIrregMapCheck
nrrd1DIrregMapCheck.restype = c_int
nrrd1DIrregMapCheck.argtypes = [POINTER(Nrrd)]
nrrd1DIrregAclGenerate = libteem.nrrd1DIrregAclGenerate
nrrd1DIrregAclGenerate.restype = c_int
nrrd1DIrregAclGenerate.argtypes = [POINTER(Nrrd), POINTER(Nrrd), size_t]
nrrd1DIrregAclCheck = libteem.nrrd1DIrregAclCheck
nrrd1DIrregAclCheck.restype = c_int
nrrd1DIrregAclCheck.argtypes = [POINTER(Nrrd)]
nrrdApply1DIrregMap = libteem.nrrdApply1DIrregMap
nrrdApply1DIrregMap.restype = c_int
nrrdApply1DIrregMap.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), POINTER(Nrrd), POINTER(Nrrd), c_int, c_int]
nrrdApply1DSubstitution = libteem.nrrdApply1DSubstitution
nrrdApply1DSubstitution.restype = c_int
nrrdApply1DSubstitution.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
nrrdApply2DLut = libteem.nrrdApply2DLut
nrrdApply2DLut.restype = c_int
nrrdApply2DLut.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, POINTER(NrrdRange), POINTER(NrrdRange), POINTER(Nrrd), c_int, c_int, c_int]
nrrdSlice = libteem.nrrdSlice
nrrdSlice.restype = c_int
nrrdSlice.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, size_t]
nrrdCrop = libteem.nrrdCrop
nrrdCrop.restype = c_int
nrrdCrop.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(size_t), POINTER(size_t)]
nrrdSample_nva = libteem.nrrdSample_nva
nrrdSample_nva.restype = c_int
nrrdSample_nva.argtypes = [c_void_p, POINTER(Nrrd), POINTER(size_t)]
nrrdSample_va = libteem.nrrdSample_va
nrrdSample_va.restype = c_int
nrrdSample_va.argtypes = [c_void_p, POINTER(Nrrd)]
nrrdSimpleCrop = libteem.nrrdSimpleCrop
nrrdSimpleCrop.restype = c_int
nrrdSimpleCrop.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdSplice = libteem.nrrdSplice
nrrdSplice.restype = c_int
nrrdSplice.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), c_uint, size_t]
nrrdPad_nva = libteem.nrrdPad_nva
nrrdPad_nva.restype = c_int
nrrdPad_nva.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(ptrdiff_t), POINTER(ptrdiff_t), c_int, c_double]
nrrdPad_va = libteem.nrrdPad_va
nrrdPad_va.restype = c_int
nrrdPad_va.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(ptrdiff_t), POINTER(ptrdiff_t), c_int]
nrrdSimplePad_nva = libteem.nrrdSimplePad_nva
nrrdSimplePad_nva.restype = c_int
nrrdSimplePad_nva.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, c_int, c_double]
nrrdSimplePad_va = libteem.nrrdSimplePad_va
nrrdSimplePad_va.restype = c_int
nrrdSimplePad_va.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, c_int]
nrrdInset = libteem.nrrdInset
nrrdInset.restype = c_int
nrrdInset.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), POINTER(size_t)]
nrrdMeasureLine = (CFUNCTYPE(None, c_void_p, c_int, c_void_p, c_int, size_t, c_double, c_double) * 27).in_dll(libteem, 'nrrdMeasureLine')
nrrdProject = libteem.nrrdProject
nrrdProject.restype = c_int
nrrdProject.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, c_int, c_int]
nrrdHisto = libteem.nrrdHisto
nrrdHisto.restype = c_int
nrrdHisto.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), POINTER(Nrrd), size_t, c_int]
nrrdHistoCheck = libteem.nrrdHistoCheck
nrrdHistoCheck.restype = c_int
nrrdHistoCheck.argtypes = [POINTER(Nrrd)]
nrrdHistoDraw = libteem.nrrdHistoDraw
nrrdHistoDraw.restype = c_int
nrrdHistoDraw.argtypes = [POINTER(Nrrd), POINTER(Nrrd), size_t, c_int, c_double]
nrrdHistoAxis = libteem.nrrdHistoAxis
nrrdHistoAxis.restype = c_int
nrrdHistoAxis.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), c_uint, size_t, c_int]
nrrdHistoJoint = libteem.nrrdHistoJoint
nrrdHistoJoint.restype = c_int
nrrdHistoJoint.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(POINTER(NrrdRange)), c_uint, POINTER(Nrrd), POINTER(size_t), c_int, POINTER(c_int)]
nrrdHistoThresholdOtsu = libteem.nrrdHistoThresholdOtsu
nrrdHistoThresholdOtsu.restype = c_int
nrrdHistoThresholdOtsu.argtypes = [POINTER(c_double), POINTER(Nrrd), c_double]
nrrdArithGamma = libteem.nrrdArithGamma
nrrdArithGamma.restype = c_int
nrrdArithGamma.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), c_double]
nrrdArithUnaryOp = libteem.nrrdArithUnaryOp
nrrdArithUnaryOp.restype = c_int
nrrdArithUnaryOp.argtypes = [POINTER(Nrrd), c_int, POINTER(Nrrd)]
nrrdArithBinaryOp = libteem.nrrdArithBinaryOp
nrrdArithBinaryOp.restype = c_int
nrrdArithBinaryOp.argtypes = [POINTER(Nrrd), c_int, POINTER(Nrrd), POINTER(Nrrd)]
nrrdArithTernaryOp = libteem.nrrdArithTernaryOp
nrrdArithTernaryOp.restype = c_int
nrrdArithTernaryOp.argtypes = [POINTER(Nrrd), c_int, POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
nrrdArithIterBinaryOp = libteem.nrrdArithIterBinaryOp
nrrdArithIterBinaryOp.restype = c_int
nrrdArithIterBinaryOp.argtypes = [POINTER(Nrrd), c_int, POINTER(NrrdIter), POINTER(NrrdIter)]
nrrdArithIterTernaryOp = libteem.nrrdArithIterTernaryOp
nrrdArithIterTernaryOp.restype = c_int
nrrdArithIterTernaryOp.argtypes = [POINTER(Nrrd), c_int, POINTER(NrrdIter), POINTER(NrrdIter), POINTER(NrrdIter)]
nrrdCheapMedian = libteem.nrrdCheapMedian
nrrdCheapMedian.restype = c_int
nrrdCheapMedian.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, c_int, c_uint, c_float, c_uint]
nrrdDistanceL2 = libteem.nrrdDistanceL2
nrrdDistanceL2.restype = c_int
nrrdDistanceL2.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, POINTER(c_int), c_double, c_int]
nrrdDistanceL2Biased = libteem.nrrdDistanceL2Biased
nrrdDistanceL2Biased.restype = c_int
nrrdDistanceL2Biased.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, POINTER(c_int), c_double, c_double, c_int]
nrrdDistanceL2Signed = libteem.nrrdDistanceL2Signed
nrrdDistanceL2Signed.restype = c_int
nrrdDistanceL2Signed.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, POINTER(c_int), c_double, c_int]
nrrdResample_t = c_double
class NrrdResampleContext(Structure):
    pass
nrrdResampleContextNew = libteem.nrrdResampleContextNew
nrrdResampleContextNew.restype = POINTER(NrrdResampleContext)
nrrdResampleContextNew.argtypes = []
nrrdResampleContextNix = libteem.nrrdResampleContextNix
nrrdResampleContextNix.restype = POINTER(NrrdResampleContext)
nrrdResampleContextNix.argtypes = [POINTER(NrrdResampleContext)]
nrrdResampleDefaultCenterSet = libteem.nrrdResampleDefaultCenterSet
nrrdResampleDefaultCenterSet.restype = c_int
nrrdResampleDefaultCenterSet.argtypes = [POINTER(NrrdResampleContext), c_int]
nrrdResampleNrrdSet = libteem.nrrdResampleNrrdSet
nrrdResampleNrrdSet.restype = c_int
nrrdResampleNrrdSet.argtypes = [POINTER(NrrdResampleContext), POINTER(Nrrd)]
nrrdResampleKernelSet = libteem.nrrdResampleKernelSet
nrrdResampleKernelSet.restype = c_int
nrrdResampleKernelSet.argtypes = [POINTER(NrrdResampleContext), c_uint, POINTER(NrrdKernel), POINTER(c_double)]
nrrdResampleSamplesSet = libteem.nrrdResampleSamplesSet
nrrdResampleSamplesSet.restype = c_int
nrrdResampleSamplesSet.argtypes = [POINTER(NrrdResampleContext), c_uint, size_t]
nrrdResampleRangeSet = libteem.nrrdResampleRangeSet
nrrdResampleRangeSet.restype = c_int
nrrdResampleRangeSet.argtypes = [POINTER(NrrdResampleContext), c_uint, c_double, c_double]
nrrdResampleRangeFullSet = libteem.nrrdResampleRangeFullSet
nrrdResampleRangeFullSet.restype = c_int
nrrdResampleRangeFullSet.argtypes = [POINTER(NrrdResampleContext), c_uint]
nrrdResampleBoundarySet = libteem.nrrdResampleBoundarySet
nrrdResampleBoundarySet.restype = c_int
nrrdResampleBoundarySet.argtypes = [POINTER(NrrdResampleContext), c_int]
nrrdResamplePadValueSet = libteem.nrrdResamplePadValueSet
nrrdResamplePadValueSet.restype = c_int
nrrdResamplePadValueSet.argtypes = [POINTER(NrrdResampleContext), c_double]
nrrdResampleTypeOutSet = libteem.nrrdResampleTypeOutSet
nrrdResampleTypeOutSet.restype = c_int
nrrdResampleTypeOutSet.argtypes = [POINTER(NrrdResampleContext), c_int]
nrrdResampleRenormalizeSet = libteem.nrrdResampleRenormalizeSet
nrrdResampleRenormalizeSet.restype = c_int
nrrdResampleRenormalizeSet.argtypes = [POINTER(NrrdResampleContext), c_int]
nrrdResampleRoundSet = libteem.nrrdResampleRoundSet
nrrdResampleRoundSet.restype = c_int
nrrdResampleRoundSet.argtypes = [POINTER(NrrdResampleContext), c_int]
nrrdResampleClampSet = libteem.nrrdResampleClampSet
nrrdResampleClampSet.restype = c_int
nrrdResampleClampSet.argtypes = [POINTER(NrrdResampleContext), c_int]
nrrdResampleExecute = libteem.nrrdResampleExecute
nrrdResampleExecute.restype = c_int
nrrdResampleExecute.argtypes = [POINTER(NrrdResampleContext), POINTER(Nrrd)]
NrrdResampleInfo._pack_ = 4
NrrdResampleInfo._fields_ = [
    ('kernel', POINTER(NrrdKernel) * 16),
    ('samples', size_t * 16),
    ('parm', c_double * 8 * 16),
    ('min', c_double * 16),
    ('max', c_double * 16),
    ('boundary', c_int),
    ('type', c_int),
    ('renormalize', c_int),
    ('round', c_int),
    ('clamp', c_int),
    ('cheap', c_int),
    ('padValue', c_double),
]
nrrdSpatialResample = libteem.nrrdSpatialResample
nrrdSpatialResample.restype = c_int
nrrdSpatialResample.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdResampleInfo)]
nrrdSimpleResample = libteem.nrrdSimpleResample
nrrdSimpleResample.restype = c_int
nrrdSimpleResample.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdKernel), POINTER(c_double), POINTER(size_t), POINTER(c_double)]
nrrdCCValid = libteem.nrrdCCValid
nrrdCCValid.restype = c_int
nrrdCCValid.argtypes = [POINTER(Nrrd)]
nrrdCCSize = libteem.nrrdCCSize
nrrdCCSize.restype = c_uint
nrrdCCSize.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
nrrdCCMax = libteem.nrrdCCMax
nrrdCCMax.restype = c_uint
nrrdCCMax.argtypes = [POINTER(Nrrd)]
nrrdCCNum = libteem.nrrdCCNum
nrrdCCNum.restype = c_uint
nrrdCCNum.argtypes = [POINTER(Nrrd)]
nrrdCCFind = libteem.nrrdCCFind
nrrdCCFind.restype = c_int
nrrdCCFind.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(Nrrd), c_int, c_uint]
nrrdCCAdjacency = libteem.nrrdCCAdjacency
nrrdCCAdjacency.restype = c_int
nrrdCCAdjacency.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdCCMerge = libteem.nrrdCCMerge
nrrdCCMerge.restype = c_int
nrrdCCMerge.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), c_int, c_uint, c_uint, c_uint]
nrrdCCRevalue = libteem.nrrdCCRevalue
nrrdCCRevalue.restype = c_int
nrrdCCRevalue.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
nrrdCCSettle = libteem.nrrdCCSettle
nrrdCCSettle.restype = c_int
nrrdCCSettle.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(Nrrd)]
nrrdKernelTMF = (POINTER(NrrdKernel) * 5 * 5 * 4).in_dll(libteem, 'nrrdKernelTMF')
nrrdKernelTMF_maxD = (c_uint).in_dll(libteem, 'nrrdKernelTMF_maxD')
nrrdKernelTMF_maxC = (c_uint).in_dll(libteem, 'nrrdKernelTMF_maxC')
nrrdKernelTMF_maxA = (c_uint).in_dll(libteem, 'nrrdKernelTMF_maxA')
nrrdKernelHann = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelHann')
nrrdKernelHannD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelHannD')
nrrdKernelHannDD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelHannDD')
nrrdKernelBlackman = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelBlackman')
nrrdKernelBlackmanD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelBlackmanD')
nrrdKernelBlackmanDD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelBlackmanDD')
nrrdKernelZero = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelZero')
nrrdKernelBox = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelBox')
nrrdKernelCheap = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelCheap')
nrrdKernelHermiteFlag = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelHermiteFlag')
nrrdKernelTent = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelTent')
nrrdKernelForwDiff = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelForwDiff')
nrrdKernelCentDiff = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelCentDiff')
nrrdKernelBCCubic = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelBCCubic')
nrrdKernelBCCubicD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelBCCubicD')
nrrdKernelBCCubicDD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelBCCubicDD')
nrrdKernelAQuartic = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelAQuartic')
nrrdKernelAQuarticD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelAQuarticD')
nrrdKernelAQuarticDD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelAQuarticDD')
nrrdKernelC3Quintic = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelC3Quintic')
nrrdKernelC3QuinticD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelC3QuinticD')
nrrdKernelC3QuinticDD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelC3QuinticDD')
nrrdKernelC4Hexic = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelC4Hexic')
nrrdKernelC4HexicD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelC4HexicD')
nrrdKernelC4HexicDD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelC4HexicDD')
nrrdKernelGaussian = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelGaussian')
nrrdKernelGaussianD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelGaussianD')
nrrdKernelGaussianDD = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelGaussianDD')
nrrdKernelDiscreteGaussian = (POINTER(NrrdKernel)).in_dll(libteem, 'nrrdKernelDiscreteGaussian')
nrrdKernelParse = libteem.nrrdKernelParse
nrrdKernelParse.restype = c_int
nrrdKernelParse.argtypes = [POINTER(POINTER(NrrdKernel)), POINTER(c_double), STRING]
nrrdKernelSpecParse = libteem.nrrdKernelSpecParse
nrrdKernelSpecParse.restype = c_int
nrrdKernelSpecParse.argtypes = [POINTER(NrrdKernelSpec), STRING]
nrrdKernelSpecSprint = libteem.nrrdKernelSpecSprint
nrrdKernelSpecSprint.restype = c_int
nrrdKernelSpecSprint.argtypes = [STRING, POINTER(NrrdKernelSpec)]
class pullInfoSpec_t(Structure):
    pass
pullInfoSpec_t._pack_ = 4
pullInfoSpec_t._fields_ = [
    ('info', c_int),
    ('volName', STRING),
    ('item', c_int),
    ('scale', c_double),
    ('zero', c_double),
    ('constraint', c_int),
    ('volIdx', c_uint),
]
pullInfoSpec = pullInfoSpec_t
class pullPoint_t(Structure):
    pass
pullPoint_t._pack_ = 4
pullPoint_t._fields_ = [
    ('idtag', c_uint),
    ('idCC', c_uint),
    ('neighPoint', POINTER(POINTER(pullPoint_t))),
    ('neighPointNum', c_uint),
    ('neighPointArr', POINTER(airArray)),
    ('neighDistMean', c_double),
    ('neighMode', c_double),
    ('neighInterNum', c_uint),
    ('stuckIterNum', c_uint),
    ('status', c_uint),
    ('pos', c_double * 4),
    ('energy', c_double),
    ('force', c_double * 4),
    ('stepEnergy', c_double),
    ('stepConstr', c_double),
    ('info', c_double * 1),
]
pullPoint = pullPoint_t
class pullBin_t(Structure):
    pass
pullBin_t._fields_ = [
    ('point', POINTER(POINTER(pullPoint))),
    ('pointNum', c_uint),
    ('pointArr', POINTER(airArray)),
    ('neighBin', POINTER(POINTER(pullBin_t))),
]
pullBin = pullBin_t
class pullEnergy(Structure):
    pass
pullEnergy._fields_ = [
    ('name', c_char * 129),
    ('parmNum', c_uint),
    ('eval', CFUNCTYPE(c_double, POINTER(c_double), c_double, POINTER(c_double))),
]
class pullEnergySpec(Structure):
    pass
pullEnergySpec._pack_ = 4
pullEnergySpec._fields_ = [
    ('energy', POINTER(pullEnergy)),
    ('parm', c_double * 3),
]
class pullVolume(Structure):
    pass
pullVolume._fields_ = [
    ('verbose', c_int),
    ('name', STRING),
    ('kind', POINTER(gageKind)),
    ('ninSingle', POINTER(Nrrd)),
    ('ninScale', POINTER(POINTER(Nrrd))),
    ('scaleNum', c_uint),
    ('scalePos', POINTER(c_double)),
    ('scaleDerivNorm', c_int),
    ('ksp00', POINTER(NrrdKernelSpec)),
    ('ksp11', POINTER(NrrdKernelSpec)),
    ('ksp22', POINTER(NrrdKernelSpec)),
    ('kspSS', POINTER(NrrdKernelSpec)),
    ('pullValQuery', gageQuery),
    ('gctx', POINTER(gageContext)),
    ('gpvl', POINTER(gagePerVolume)),
    ('gpvlSS', POINTER(POINTER(gagePerVolume))),
    ('seedOnly', c_int),
]
class pullTask_t(Structure):
    pass
pullTask_t._fields_ = [
    ('pctx', POINTER(pullContext_t)),
    ('vol', POINTER(pullVolume) * 4),
    ('pullValAnswer', POINTER(c_double)),
    ('pullValDirectAnswer', POINTER(POINTER(c_double))),
    ('ans', POINTER(c_double) * 22),
    ('processMode', c_int),
    ('thread', POINTER(airThread)),
    ('threadIdx', c_uint),
    ('rng', POINTER(airRandMTState)),
    ('pointBuffer', POINTER(pullPoint)),
    ('neighPoint', POINTER(POINTER(pullPoint))),
    ('addPoint', POINTER(POINTER(pullPoint))),
    ('addPointNum', c_uint),
    ('addPointArr', POINTER(airArray)),
    ('nixPoint', POINTER(POINTER(pullPoint))),
    ('nixPointNum', c_uint),
    ('nixPointArr', POINTER(airArray)),
    ('returnPtr', c_void_p),
    ('stuckNum', c_uint),
]
pullTask = pullTask_t
pullContext_t._pack_ = 4
pullContext_t._fields_ = [
    ('verbose', c_int),
    ('liveThresholdInit', c_int),
    ('permuteOnRebin', c_int),
    ('noPopCntlWithZeroAlpha', c_int),
    ('restrictiveAddToBins', c_int),
    ('allowUnequalShapes', c_int),
    ('pointNumInitial', c_uint),
    ('npos', POINTER(Nrrd)),
    ('vol', POINTER(pullVolume) * 4),
    ('volNum', c_uint),
    ('ispec', POINTER(pullInfoSpec) * 22),
    ('stepInitial', c_double),
    ('radiusSpace', c_double),
    ('radiusScale', c_double),
    ('neighborTrueProb', c_double),
    ('probeProb', c_double),
    ('opporStepScale', c_double),
    ('stepScale', c_double),
    ('energyDecreaseMin', c_double),
    ('energyDecreasePopCntlMin', c_double),
    ('constraintStepMin', c_double),
    ('wall', c_double),
    ('energyIncreasePermit', c_double),
    ('energyFromStrength', c_int),
    ('nixAtVolumeEdgeSpace', c_int),
    ('constraintBeforeSeedThresh', c_int),
    ('pointPerVoxel', c_int),
    ('numSamplesScale', c_uint),
    ('rngSeed', c_uint),
    ('threadNum', c_uint),
    ('iterMax', c_uint),
    ('popCntlPeriod', c_uint),
    ('constraintIterMax', c_uint),
    ('stuckIterMax', c_uint),
    ('snap', c_uint),
    ('ppvZRange', c_uint * 2),
    ('progressBinMod', c_uint),
    ('interType', c_int),
    ('energySpecR', POINTER(pullEnergySpec)),
    ('energySpecS', POINTER(pullEnergySpec)),
    ('energySpecWin', POINTER(pullEnergySpec)),
    ('alpha', c_double),
    ('beta', c_double),
    ('gamma', c_double),
    ('jitter', c_double),
    ('sliceNormal', c_double * 3),
    ('binSingle', c_int),
    ('binIncr', c_uint),
    ('iter_cb', CFUNCTYPE(None, c_void_p)),
    ('data_cb', c_void_p),
    ('bboxMin', c_double * 4),
    ('bboxMax', c_double * 4),
    ('infoTotalLen', c_uint),
    ('infoIdx', c_uint * 22),
    ('idtagNext', c_uint),
    ('haveScale', c_int),
    ('constraint', c_int),
    ('finished', c_int),
    ('maxDistSpace', c_double),
    ('maxDistScale', c_double),
    ('constraintDim', c_double),
    ('targetDim', c_double),
    ('voxelSizeSpace', c_double),
    ('voxelSizeScale', c_double),
    ('bin', POINTER(pullBin)),
    ('binsEdge', c_uint * 4),
    ('binNum', c_uint),
    ('binNextIdx', c_uint),
    ('_sliceNormal', c_double * 3),
    ('tmpPointPerm', POINTER(c_uint)),
    ('tmpPointPtr', POINTER(POINTER(pullPoint))),
    ('tmpPointNum', c_uint),
    ('binMutex', POINTER(airThreadMutex)),
    ('task', POINTER(POINTER(pullTask))),
    ('iterBarrierA', POINTER(airThreadBarrier)),
    ('iterBarrierB', POINTER(airThreadBarrier)),
    ('timeIteration', c_double),
    ('timeRun', c_double),
    ('energy', c_double),
    ('addNum', c_uint),
    ('nixNum', c_uint),
    ('stuckNum', c_uint),
    ('pointNum', c_uint),
    ('CCNum', c_uint),
    ('iter', c_uint),
    ('noutPos', POINTER(Nrrd)),
]
pullPhistEnabled = (c_int).in_dll(libteem, 'pullPhistEnabled')
pullBiffKey = (STRING).in_dll(libteem, 'pullBiffKey')
pullInterType = (POINTER(airEnum)).in_dll(libteem, 'pullInterType')
pullEnergyType = (POINTER(airEnum)).in_dll(libteem, 'pullEnergyType')
pullEnergyUnknown = (POINTER(pullEnergy)).in_dll(libteem, 'pullEnergyUnknown')
pullEnergySpring = (POINTER(pullEnergy)).in_dll(libteem, 'pullEnergySpring')
pullEnergyGauss = (POINTER(pullEnergy)).in_dll(libteem, 'pullEnergyGauss')
pullEnergyButterworth = (POINTER(pullEnergy)).in_dll(libteem, 'pullEnergyButterworth')
pullEnergyCotan = (POINTER(pullEnergy)).in_dll(libteem, 'pullEnergyCotan')
pullEnergyCubic = (POINTER(pullEnergy)).in_dll(libteem, 'pullEnergyCubic')
pullEnergyQuartic = (POINTER(pullEnergy)).in_dll(libteem, 'pullEnergyQuartic')
pullEnergyCubicWell = (POINTER(pullEnergy)).in_dll(libteem, 'pullEnergyCubicWell')
pullEnergyZero = (POINTER(pullEnergy)).in_dll(libteem, 'pullEnergyZero')
pullEnergyButterworthParabola = (POINTER(pullEnergy)).in_dll(libteem, 'pullEnergyButterworthParabola')
pullEnergyAll = (POINTER(pullEnergy) * 10).in_dll(libteem, 'pullEnergyAll')
pullEnergySpecNew = libteem.pullEnergySpecNew
pullEnergySpecNew.restype = POINTER(pullEnergySpec)
pullEnergySpecNew.argtypes = []
pullEnergySpecSet = libteem.pullEnergySpecSet
pullEnergySpecSet.restype = c_int
pullEnergySpecSet.argtypes = [POINTER(pullEnergySpec), POINTER(pullEnergy), POINTER(c_double)]
pullEnergySpecNix = libteem.pullEnergySpecNix
pullEnergySpecNix.restype = POINTER(pullEnergySpec)
pullEnergySpecNix.argtypes = [POINTER(pullEnergySpec)]
pullEnergySpecParse = libteem.pullEnergySpecParse
pullEnergySpecParse.restype = c_int
pullEnergySpecParse.argtypes = [POINTER(pullEnergySpec), STRING]
pullHestEnergySpec = (POINTER(hestCB)).in_dll(libteem, 'pullHestEnergySpec')
pullVolumeNew = libteem.pullVolumeNew
pullVolumeNew.restype = POINTER(pullVolume)
pullVolumeNew.argtypes = []
pullVolumeNix = libteem.pullVolumeNix
pullVolumeNix.restype = POINTER(pullVolume)
pullVolumeNix.argtypes = [POINTER(pullVolume)]
pullVolumeSingleAdd = libteem.pullVolumeSingleAdd
pullVolumeSingleAdd.restype = c_int
pullVolumeSingleAdd.argtypes = [POINTER(pullContext), POINTER(gageKind), STRING, POINTER(Nrrd), POINTER(NrrdKernelSpec), POINTER(NrrdKernelSpec), POINTER(NrrdKernelSpec)]
pullVolumeStackAdd = libteem.pullVolumeStackAdd
pullVolumeStackAdd.restype = c_int
pullVolumeStackAdd.argtypes = [POINTER(pullContext), POINTER(gageKind), STRING, POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(c_double), c_uint, c_int, POINTER(NrrdKernelSpec), POINTER(NrrdKernelSpec), POINTER(NrrdKernelSpec), POINTER(NrrdKernelSpec)]
pullVolumeLookup = libteem.pullVolumeLookup
pullVolumeLookup.restype = POINTER(pullVolume)
pullVolumeLookup.argtypes = [POINTER(pullContext), STRING]
pullVal = (POINTER(airEnum)).in_dll(libteem, 'pullVal')
pullValGageKind = (POINTER(gageKind)).in_dll(libteem, 'pullValGageKind')
pullInfo = (POINTER(airEnum)).in_dll(libteem, 'pullInfo')
pullInfoAnswerLen = libteem.pullInfoAnswerLen
pullInfoAnswerLen.restype = c_uint
pullInfoAnswerLen.argtypes = [c_int]
pullInfoSpecNew = libteem.pullInfoSpecNew
pullInfoSpecNew.restype = POINTER(pullInfoSpec)
pullInfoSpecNew.argtypes = []
pullInfoSpecNix = libteem.pullInfoSpecNix
pullInfoSpecNix.restype = POINTER(pullInfoSpec)
pullInfoSpecNix.argtypes = [POINTER(pullInfoSpec)]
pullInfoSpecAdd = libteem.pullInfoSpecAdd
pullInfoSpecAdd.restype = c_int
pullInfoSpecAdd.argtypes = [POINTER(pullContext), POINTER(pullInfoSpec)]
pullContextNew = libteem.pullContextNew
pullContextNew.restype = POINTER(pullContext)
pullContextNew.argtypes = []
pullContextNix = libteem.pullContextNix
pullContextNix.restype = POINTER(pullContext)
pullContextNix.argtypes = [POINTER(pullContext)]
pullOutputGet = libteem.pullOutputGet
pullOutputGet.restype = c_int
pullOutputGet.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), POINTER(c_double), c_double, POINTER(pullContext)]
pullPositionHistoryGet = libteem.pullPositionHistoryGet
pullPositionHistoryGet.restype = c_int
pullPositionHistoryGet.argtypes = [POINTER(limnPolyData), POINTER(pullContext)]
pullPropGet = libteem.pullPropGet
pullPropGet.restype = c_int
pullPropGet.argtypes = [POINTER(Nrrd), c_int, POINTER(pullContext)]
pullVerboseSet = libteem.pullVerboseSet
pullVerboseSet.restype = None
pullVerboseSet.argtypes = [POINTER(pullContext), c_int]
pullPointNumber = libteem.pullPointNumber
pullPointNumber.restype = c_uint
pullPointNumber.argtypes = [POINTER(pullContext)]
pullPointNew = libteem.pullPointNew
pullPointNew.restype = POINTER(pullPoint)
pullPointNew.argtypes = [POINTER(pullContext)]
pullPointNix = libteem.pullPointNix
pullPointNix.restype = POINTER(pullPoint)
pullPointNix.argtypes = [POINTER(pullPoint)]
pullBinsPointAdd = libteem.pullBinsPointAdd
pullBinsPointAdd.restype = c_int
pullBinsPointAdd.argtypes = [POINTER(pullContext), POINTER(pullPoint), POINTER(POINTER(pullBin))]
pullBinsPointMaybeAdd = libteem.pullBinsPointMaybeAdd
pullBinsPointMaybeAdd.restype = c_int
pullBinsPointMaybeAdd.argtypes = [POINTER(pullContext), POINTER(pullPoint), POINTER(POINTER(pullBin)), POINTER(c_int)]
pullProcessMode = (POINTER(airEnum)).in_dll(libteem, 'pullProcessMode')
pullBinProcess = libteem.pullBinProcess
pullBinProcess.restype = c_int
pullBinProcess.argtypes = [POINTER(pullTask), c_uint]
pullGammaLearn = libteem.pullGammaLearn
pullGammaLearn.restype = c_int
pullGammaLearn.argtypes = [POINTER(pullContext)]
pullStart = libteem.pullStart
pullStart.restype = c_int
pullStart.argtypes = [POINTER(pullContext)]
pullRun = libteem.pullRun
pullRun.restype = c_int
pullRun.argtypes = [POINTER(pullContext)]
pullFinish = libteem.pullFinish
pullFinish.restype = c_int
pullFinish.argtypes = [POINTER(pullContext)]
pullCCFind = libteem.pullCCFind
pullCCFind.restype = c_int
pullCCFind.argtypes = [POINTER(pullContext)]
pullCCMeasure = libteem.pullCCMeasure
pullCCMeasure.restype = c_int
pullCCMeasure.argtypes = [POINTER(pullContext), POINTER(Nrrd), c_int, c_double]
pullCCSort = libteem.pullCCSort
pullCCSort.restype = c_int
pullCCSort.argtypes = [POINTER(pullContext), c_int, c_double]
class seekContext(Structure):
    pass
seekContext._pack_ = 4
seekContext._fields_ = [
    ('verbose', c_int),
    ('ninscl', POINTER(Nrrd)),
    ('gctx', POINTER(gageContext)),
    ('pvl', POINTER(gagePerVolume)),
    ('type', c_int),
    ('sclvItem', c_int),
    ('gradItem', c_int),
    ('normItem', c_int),
    ('evalItem', c_int),
    ('evecItem', c_int),
    ('stngItem', c_int),
    ('hessItem', c_int),
    ('lowerInside', c_int),
    ('normalsFind', c_int),
    ('strengthUse', c_int),
    ('strengthSign', c_int),
    ('isovalue', c_double),
    ('strength', c_double),
    ('evalDiffThresh', c_double),
    ('samples', size_t * 3),
    ('facesPerVoxel', c_double),
    ('vertsPerVoxel', c_double),
    ('pldArrIncr', c_uint),
    ('flag', POINTER(c_int)),
    ('nin', POINTER(Nrrd)),
    ('baseDim', c_uint),
    ('_shape', POINTER(gageShape)),
    ('shape', POINTER(gageShape)),
    ('nsclDerived', POINTER(Nrrd)),
    ('sclvAns', POINTER(c_double)),
    ('gradAns', POINTER(c_double)),
    ('normAns', POINTER(c_double)),
    ('evalAns', POINTER(c_double)),
    ('evecAns', POINTER(c_double)),
    ('stngAns', POINTER(c_double)),
    ('hessAns', POINTER(c_double)),
    ('reverse', c_int),
    ('txfNormal', c_double * 9),
    ('spanSize', size_t),
    ('nspanHist', POINTER(Nrrd)),
    ('range', POINTER(NrrdRange)),
    ('sx', size_t),
    ('sy', size_t),
    ('sz', size_t),
    ('txfIdx', c_double * 16),
    ('vidx', POINTER(c_int)),
    ('facevidx', POINTER(c_int)),
    ('sclv', POINTER(c_double)),
    ('grad', POINTER(c_double)),
    ('eval', POINTER(c_double)),
    ('evec', POINTER(c_double)),
    ('hess', POINTER(c_double)),
    ('t', POINTER(c_double)),
    ('edgealpha', POINTER(c_double)),
    ('edgenorm', POINTER(c_double)),
    ('edgeicoord', POINTER(c_double)),
    ('facecoord', POINTER(c_double)),
    ('facenorm', POINTER(c_double)),
    ('faceicoord', POINTER(c_double)),
    ('gradcontext', POINTER(c_double)),
    ('hesscontext', POINTER(c_double)),
    ('tcontext', POINTER(c_double)),
    ('stngcontext', POINTER(c_double)),
    ('flip', POINTER(c_byte)),
    ('pairs', POINTER(c_byte)),
    ('treated', POINTER(c_byte)),
    ('stng', POINTER(c_double)),
    ('nvidx', POINTER(Nrrd)),
    ('nsclv', POINTER(Nrrd)),
    ('ngrad', POINTER(Nrrd)),
    ('neval', POINTER(Nrrd)),
    ('nevec', POINTER(Nrrd)),
    ('nflip', POINTER(Nrrd)),
    ('nstng', POINTER(Nrrd)),
    ('nhess', POINTER(Nrrd)),
    ('nt', POINTER(Nrrd)),
    ('nfacevidx', POINTER(Nrrd)),
    ('nedgealpha', POINTER(Nrrd)),
    ('nedgenorm', POINTER(Nrrd)),
    ('nfacecoord', POINTER(Nrrd)),
    ('nfacenorm', POINTER(Nrrd)),
    ('npairs', POINTER(Nrrd)),
    ('nedgeicoord', POINTER(Nrrd)),
    ('nfaceicoord', POINTER(Nrrd)),
    ('ngradcontext', POINTER(Nrrd)),
    ('nhesscontext', POINTER(Nrrd)),
    ('ntcontext', POINTER(Nrrd)),
    ('nstngcontext', POINTER(Nrrd)),
    ('ntreated', POINTER(Nrrd)),
    ('voxNum', c_uint),
    ('vertNum', c_uint),
    ('faceNum', c_uint),
    ('strengthSeenMax', c_double),
    ('time', c_double),
]
seekBiffKey = (STRING).in_dll(libteem, 'seekBiffKey')
seekType = (POINTER(airEnum)).in_dll(libteem, 'seekType')
seekContour3DTopoHackEdge = (c_int * 256).in_dll(libteem, 'seekContour3DTopoHackEdge')
seekContour3DTopoHackTriangle = (c_int * 19 * 256).in_dll(libteem, 'seekContour3DTopoHackTriangle')
seekContextNew = libteem.seekContextNew
seekContextNew.restype = POINTER(seekContext)
seekContextNew.argtypes = []
seekContextNix = libteem.seekContextNix
seekContextNix.restype = POINTER(seekContext)
seekContextNix.argtypes = [POINTER(seekContext)]
seekVerboseSet = libteem.seekVerboseSet
seekVerboseSet.restype = None
seekVerboseSet.argtypes = [POINTER(seekContext), c_int]
seekDataSet = libteem.seekDataSet
seekDataSet.restype = c_int
seekDataSet.argtypes = [POINTER(seekContext), POINTER(Nrrd), POINTER(gageContext), c_uint]
seekNormalsFindSet = libteem.seekNormalsFindSet
seekNormalsFindSet.restype = c_int
seekNormalsFindSet.argtypes = [POINTER(seekContext), c_int]
seekStrengthUseSet = libteem.seekStrengthUseSet
seekStrengthUseSet.restype = c_int
seekStrengthUseSet.argtypes = [POINTER(seekContext), c_int]
seekStrengthSet = libteem.seekStrengthSet
seekStrengthSet.restype = c_int
seekStrengthSet.argtypes = [POINTER(seekContext), c_int, c_double]
seekSamplesSet = libteem.seekSamplesSet
seekSamplesSet.restype = c_int
seekSamplesSet.argtypes = [POINTER(seekContext), POINTER(size_t)]
seekTypeSet = libteem.seekTypeSet
seekTypeSet.restype = c_int
seekTypeSet.argtypes = [POINTER(seekContext), c_int]
seekLowerInsideSet = libteem.seekLowerInsideSet
seekLowerInsideSet.restype = c_int
seekLowerInsideSet.argtypes = [POINTER(seekContext), c_int]
seekItemScalarSet = libteem.seekItemScalarSet
seekItemScalarSet.restype = c_int
seekItemScalarSet.argtypes = [POINTER(seekContext), c_int]
seekItemStrengthSet = libteem.seekItemStrengthSet
seekItemStrengthSet.restype = c_int
seekItemStrengthSet.argtypes = [POINTER(seekContext), c_int]
seekItemNormalSet = libteem.seekItemNormalSet
seekItemNormalSet.restype = c_int
seekItemNormalSet.argtypes = [POINTER(seekContext), c_int]
seekItemGradientSet = libteem.seekItemGradientSet
seekItemGradientSet.restype = c_int
seekItemGradientSet.argtypes = [POINTER(seekContext), c_int]
seekItemEigensystemSet = libteem.seekItemEigensystemSet
seekItemEigensystemSet.restype = c_int
seekItemEigensystemSet.argtypes = [POINTER(seekContext), c_int, c_int]
seekItemHessSet = libteem.seekItemHessSet
seekItemHessSet.restype = c_int
seekItemHessSet.argtypes = [POINTER(seekContext), c_int]
seekIsovalueSet = libteem.seekIsovalueSet
seekIsovalueSet.restype = c_int
seekIsovalueSet.argtypes = [POINTER(seekContext), c_double]
seekEvalDiffThreshSet = libteem.seekEvalDiffThreshSet
seekEvalDiffThreshSet.restype = c_int
seekEvalDiffThreshSet.argtypes = [POINTER(seekContext), c_double]
seekUpdate = libteem.seekUpdate
seekUpdate.restype = c_int
seekUpdate.argtypes = [POINTER(seekContext)]
seekExtract = libteem.seekExtract
seekExtract.restype = c_int
seekExtract.argtypes = [POINTER(seekContext), POINTER(limnPolyData)]
seekDescendToDeg = libteem.seekDescendToDeg
seekDescendToDeg.restype = c_int
seekDescendToDeg.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_double, c_char]
seekDescendToDegCell = libteem.seekDescendToDegCell
seekDescendToDegCell.restype = c_int
seekDescendToDegCell.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_double, c_char]
seekDescendToRidge = libteem.seekDescendToRidge
seekDescendToRidge.restype = c_int
seekDescendToRidge.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_double, c_char, c_double]
class tenGlyphParm(Structure):
    pass
tenGlyphParm._fields_ = [
    ('verbose', c_int),
    ('nmask', POINTER(Nrrd)),
    ('anisoType', c_int),
    ('onlyPositive', c_int),
    ('confThresh', c_float),
    ('anisoThresh', c_float),
    ('maskThresh', c_float),
    ('glyphType', c_int),
    ('facetRes', c_int),
    ('glyphScale', c_float),
    ('sqdSharp', c_float),
    ('edgeWidth', c_float * 5),
    ('colEvec', c_int),
    ('colAnisoType', c_int),
    ('colMaxSat', c_float),
    ('colIsoGray', c_float),
    ('colGamma', c_float),
    ('colAnisoModulate', c_float),
    ('ADSP', c_float * 4),
    ('sliceAxis', c_uint),
    ('slicePos', size_t),
    ('doSlice', c_int),
    ('sliceAnisoType', c_int),
    ('sliceOffset', c_float),
    ('sliceBias', c_float),
    ('sliceGamma', c_float),
]
class tenEvecRGBParm(Structure):
    pass
tenEvecRGBParm._pack_ = 4
tenEvecRGBParm._fields_ = [
    ('which', c_uint),
    ('aniso', c_int),
    ('confThresh', c_double),
    ('anisoGamma', c_double),
    ('gamma', c_double),
    ('bgGray', c_double),
    ('isoGray', c_double),
    ('maxSat', c_double),
    ('typeOut', c_int),
    ('genAlpha', c_int),
]
class tenFiberContext(Structure):
    pass
tenFiberContext._pack_ = 4
tenFiberContext._fields_ = [
    ('nin', POINTER(Nrrd)),
    ('ksp', POINTER(NrrdKernelSpec)),
    ('useDwi', c_int),
    ('fiberType', c_int),
    ('intg', c_int),
    ('anisoStopType', c_int),
    ('anisoSpeedType', c_int),
    ('stop', c_int),
    ('useIndexSpace', c_int),
    ('verbose', c_int),
    ('anisoThresh', c_double),
    ('anisoSpeedFunc', c_double * 3),
    ('maxNumSteps', c_uint),
    ('minNumSteps', c_uint),
    ('stepSize', c_double),
    ('maxHalfLen', c_double),
    ('minWholeLen', c_double),
    ('confThresh', c_double),
    ('minRadius', c_double),
    ('minFraction', c_double),
    ('wPunct', c_double),
    ('ten2Which', c_uint),
    ('query', gageQuery),
    ('halfIdx', c_int),
    ('mframeUse', c_int),
    ('mframe', c_double * 9),
    ('mframeT', c_double * 9),
    ('wPos', c_double * 3),
    ('wDir', c_double * 3),
    ('lastDir', c_double * 3),
    ('seedEvec', c_double * 3),
    ('lastDirSet', c_int),
    ('lastTenSet', c_int),
    ('ten2Use', c_uint),
    ('gtx', POINTER(gageContext)),
    ('pvl', POINTER(gagePerVolume)),
    ('gageTen', POINTER(c_double)),
    ('gageEval', POINTER(c_double)),
    ('gageEvec', POINTER(c_double)),
    ('gageAnisoStop', POINTER(c_double)),
    ('gageAnisoSpeed', POINTER(c_double)),
    ('gageTen2', POINTER(c_double)),
    ('ten2AnisoStop', c_double),
    ('fiberTen', c_double * 7),
    ('fiberEval', c_double * 3),
    ('fiberEvec', c_double * 9),
    ('fiberAnisoStop', c_double),
    ('fiberAnisoSpeed', c_double),
    ('radius', c_double),
    ('halfLen', c_double * 2),
    ('numSteps', c_uint * 2),
    ('whyStop', c_int * 2),
    ('whyNowhere', c_int),
]
class tenFiberSingle(Structure):
    pass
tenFiberSingle._pack_ = 4
tenFiberSingle._fields_ = [
    ('seedPos', c_double * 3),
    ('dirIdx', c_uint),
    ('dirNum', c_uint),
    ('nvert', POINTER(Nrrd)),
    ('halfLen', c_double * 2),
    ('seedIdx', c_uint),
    ('stepNum', c_uint * 2),
    ('whyStop', c_int * 2),
    ('whyNowhere', c_int),
    ('nval', POINTER(Nrrd)),
    ('measr', c_double * 27),
]
class tenFiberMulti(Structure):
    pass
tenFiberMulti._fields_ = [
    ('fiber', POINTER(tenFiberSingle)),
    ('fiberNum', c_uint),
    ('fiberArr', POINTER(airArray)),
]
class tenEMBimodalParm(Structure):
    pass
tenEMBimodalParm._pack_ = 4
tenEMBimodalParm._fields_ = [
    ('minProb', c_double),
    ('minProb2', c_double),
    ('minDelta', c_double),
    ('minFraction', c_double),
    ('minConfidence', c_double),
    ('twoStage', c_double),
    ('verbose', c_double),
    ('maxIteration', c_uint),
    ('histo', POINTER(c_double)),
    ('pp1', POINTER(c_double)),
    ('pp2', POINTER(c_double)),
    ('vmin', c_double),
    ('vmax', c_double),
    ('delta', c_double),
    ('N', c_int),
    ('stage', c_int),
    ('iteration', c_uint),
    ('mean1', c_double),
    ('stdv1', c_double),
    ('mean2', c_double),
    ('stdv2', c_double),
    ('fraction1', c_double),
    ('confidence', c_double),
    ('threshold', c_double),
]
class tenGradientParm(Structure):
    pass
tenGradientParm._pack_ = 4
tenGradientParm._fields_ = [
    ('initStep', c_double),
    ('jitter', c_double),
    ('minVelocity', c_double),
    ('minPotentialChange', c_double),
    ('minMean', c_double),
    ('minMeanImprovement', c_double),
    ('single', c_int),
    ('snap', c_uint),
    ('report', c_uint),
    ('expo', c_uint),
    ('seed', c_uint),
    ('maxEdgeShrink', c_uint),
    ('minIteration', c_uint),
    ('maxIteration', c_uint),
    ('expo_d', c_double),
    ('step', c_double),
    ('nudge', c_double),
    ('itersUsed', c_uint),
    ('potential', c_double),
    ('potentialNorm', c_double),
    ('angle', c_double),
    ('edge', c_double),
]
class tenEstimateContext(Structure):
    pass
tenEstimateContext._pack_ = 4
tenEstimateContext._fields_ = [
    ('bValue', c_double),
    ('valueMin', c_double),
    ('sigma', c_double),
    ('dwiConfThresh', c_double),
    ('dwiConfSoft', c_double),
    ('_ngrad', POINTER(Nrrd)),
    ('_nbmat', POINTER(Nrrd)),
    ('skipList', POINTER(c_uint)),
    ('skipListArr', POINTER(airArray)),
    ('all_f', POINTER(c_float)),
    ('all_d', POINTER(c_double)),
    ('simulate', c_int),
    ('estimate1Method', c_int),
    ('estimateB0', c_int),
    ('recordTime', c_int),
    ('recordErrorDwi', c_int),
    ('recordErrorLogDwi', c_int),
    ('recordLikelihoodDwi', c_int),
    ('verbose', c_int),
    ('negEvalShift', c_int),
    ('progress', c_int),
    ('WLSIterNum', c_uint),
    ('flag', c_int * 128),
    ('allNum', c_uint),
    ('dwiNum', c_uint),
    ('nbmat', POINTER(Nrrd)),
    ('nwght', POINTER(Nrrd)),
    ('nemat', POINTER(Nrrd)),
    ('knownB0', c_double),
    ('all', POINTER(c_double)),
    ('bnorm', POINTER(c_double)),
    ('allTmp', POINTER(c_double)),
    ('dwiTmp', POINTER(c_double)),
    ('dwi', POINTER(c_double)),
    ('skipLut', POINTER(c_ubyte)),
    ('estimatedB0', c_double),
    ('ten', c_double * 7),
    ('conf', c_double),
    ('mdwi', c_double),
    ('time', c_double),
    ('errorDwi', c_double),
    ('errorLogDwi', c_double),
    ('likelihoodDwi', c_double),
]
class tenDwiGageKindData(Structure):
    pass
tenDwiGageKindData._pack_ = 4
tenDwiGageKindData._fields_ = [
    ('ngrad', POINTER(Nrrd)),
    ('nbmat', POINTER(Nrrd)),
    ('thresh', c_double),
    ('soft', c_double),
    ('bval', c_double),
    ('valueMin', c_double),
    ('est1Method', c_int),
    ('est2Method', c_int),
    ('randSeed', c_uint),
]
class tenDwiGagePvlData(Structure):
    pass
tenDwiGagePvlData._pack_ = 4
tenDwiGagePvlData._fields_ = [
    ('tec1', POINTER(tenEstimateContext)),
    ('tec2', POINTER(tenEstimateContext)),
    ('vbuf', POINTER(c_double)),
    ('wght', POINTER(c_uint)),
    ('qvals', POINTER(c_double)),
    ('qpoints', POINTER(c_double)),
    ('dists', POINTER(c_double)),
    ('weights', POINTER(c_double)),
    ('nten1EigenGrads', POINTER(Nrrd)),
    ('randState', POINTER(airRandMTState)),
    ('randSeed', c_uint),
    ('ten1', c_double * 7),
    ('ten1Evec', c_double * 9),
    ('ten1Eval', c_double * 3),
    ('levmarUseFastExp', c_int),
    ('levmarMaxIter', c_uint),
    ('levmarTau', c_double),
    ('levmarEps1', c_double),
    ('levmarEps2', c_double),
    ('levmarEps3', c_double),
    ('levmarDelta', c_double),
    ('levmarMinCp', c_double),
    ('levmarInfo', c_double * 9),
]
class tenInterpParm(Structure):
    pass
tenInterpParm._pack_ = 4
tenInterpParm._fields_ = [
    ('verbose', c_int),
    ('convStep', c_double),
    ('minNorm', c_double),
    ('convEps', c_double),
    ('wghtSumEps', c_double),
    ('enableRecurse', c_int),
    ('maxIter', c_uint),
    ('numSteps', c_uint),
    ('lengthFancy', c_int),
    ('allocLen', c_uint),
    ('eval', POINTER(c_double)),
    ('evec', POINTER(c_double)),
    ('rtIn', POINTER(c_double)),
    ('rtLog', POINTER(c_double)),
    ('qIn', POINTER(c_double)),
    ('qBuff', POINTER(c_double)),
    ('qInter', POINTER(c_double)),
    ('numIter', c_uint),
    ('convFinal', c_double),
    ('lengthShape', c_double),
    ('lengthOrient', c_double),
]
class tenExperSpec(Structure):
    pass
tenExperSpec._fields_ = [
    ('set', c_int),
    ('imgNum', c_uint),
    ('bval', POINTER(c_double)),
    ('grad', POINTER(c_double)),
]
class tenModelParmDesc(Structure):
    pass
tenModelParmDesc._pack_ = 4
tenModelParmDesc._fields_ = [
    ('name', c_char * 129),
    ('min', c_double),
    ('max', c_double),
    ('vec3', c_int),
    ('vecIdx', c_uint),
]
class tenModel_t(Structure):
    pass
tenModel_t._fields_ = [
    ('name', c_char * 129),
    ('parmNum', c_uint),
    ('parmDesc', POINTER(tenModelParmDesc)),
    ('simulate', CFUNCTYPE(None, POINTER(c_double), POINTER(c_double), POINTER(tenExperSpec))),
    ('sprint', CFUNCTYPE(STRING, STRING, POINTER(c_double))),
    ('alloc', CFUNCTYPE(POINTER(c_double))),
    ('rand', CFUNCTYPE(None, POINTER(c_double), POINTER(airRandMTState), c_int)),
    ('step', CFUNCTYPE(None, POINTER(c_double), c_double, POINTER(c_double), POINTER(c_double))),
    ('dist', CFUNCTYPE(c_double, POINTER(c_double), POINTER(c_double))),
    ('copy', CFUNCTYPE(None, POINTER(c_double), POINTER(c_double))),
    ('convert', CFUNCTYPE(c_int, POINTER(c_double), POINTER(c_double), POINTER(tenModel_t))),
    ('sqe', CFUNCTYPE(c_double, POINTER(c_double), POINTER(tenExperSpec), POINTER(c_double), POINTER(c_double), c_int)),
    ('sqeGrad', CFUNCTYPE(None, POINTER(c_double), POINTER(c_double), POINTER(tenExperSpec), POINTER(c_double), POINTER(c_double), c_int)),
    ('sqeFit', CFUNCTYPE(c_double, POINTER(c_double), POINTER(c_double), POINTER(tenExperSpec), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_uint, c_uint, c_double)),
    ('nll', CFUNCTYPE(c_double, POINTER(c_double), POINTER(tenExperSpec), POINTER(c_double), POINTER(c_double), c_int, c_double, c_int)),
    ('nllGrad', CFUNCTYPE(None, POINTER(c_double), POINTER(c_double), POINTER(tenExperSpec), POINTER(c_double), POINTER(c_double), c_int, c_double)),
    ('nllFit', CFUNCTYPE(c_double, POINTER(c_double), POINTER(tenExperSpec), POINTER(c_double), POINTER(c_double), c_int, c_double, c_int)),
]
tenModel = tenModel_t
tenBiffKey = (STRING).in_dll(libteem, 'tenBiffKey')
tenDefFiberKernel = (c_char * 0).in_dll(libteem, 'tenDefFiberKernel')
tenDefFiberStepSize = (c_double).in_dll(libteem, 'tenDefFiberStepSize')
tenDefFiberUseIndexSpace = (c_int).in_dll(libteem, 'tenDefFiberUseIndexSpace')
tenDefFiberMaxNumSteps = (c_int).in_dll(libteem, 'tenDefFiberMaxNumSteps')
tenDefFiberMaxHalfLen = (c_double).in_dll(libteem, 'tenDefFiberMaxHalfLen')
tenDefFiberAnisoStopType = (c_int).in_dll(libteem, 'tenDefFiberAnisoStopType')
tenDefFiberAnisoThresh = (c_double).in_dll(libteem, 'tenDefFiberAnisoThresh')
tenDefFiberIntg = (c_int).in_dll(libteem, 'tenDefFiberIntg')
tenDefFiberWPunct = (c_double).in_dll(libteem, 'tenDefFiberWPunct')
tenTripleConvertSingle_d = libteem.tenTripleConvertSingle_d
tenTripleConvertSingle_d.restype = None
tenTripleConvertSingle_d.argtypes = [POINTER(c_double), c_int, POINTER(c_double), c_int]
tenTripleConvertSingle_f = libteem.tenTripleConvertSingle_f
tenTripleConvertSingle_f.restype = None
tenTripleConvertSingle_f.argtypes = [POINTER(c_float), c_int, POINTER(c_float), c_int]
tenTripleCalcSingle_d = libteem.tenTripleCalcSingle_d
tenTripleCalcSingle_d.restype = None
tenTripleCalcSingle_d.argtypes = [POINTER(c_double), c_int, POINTER(c_double)]
tenTripleCalcSingle_f = libteem.tenTripleCalcSingle_f
tenTripleCalcSingle_f.restype = None
tenTripleCalcSingle_f.argtypes = [POINTER(c_float), c_int, POINTER(c_float)]
tenTripleCalc = libteem.tenTripleCalc
tenTripleCalc.restype = c_int
tenTripleCalc.argtypes = [POINTER(Nrrd), c_int, POINTER(Nrrd)]
tenTripleConvert = libteem.tenTripleConvert
tenTripleConvert.restype = c_int
tenTripleConvert.argtypes = [POINTER(Nrrd), c_int, POINTER(Nrrd), c_int]
tenGradientParmNew = libteem.tenGradientParmNew
tenGradientParmNew.restype = POINTER(tenGradientParm)
tenGradientParmNew.argtypes = []
tenGradientParmNix = libteem.tenGradientParmNix
tenGradientParmNix.restype = POINTER(tenGradientParm)
tenGradientParmNix.argtypes = [POINTER(tenGradientParm)]
tenGradientCheck = libteem.tenGradientCheck
tenGradientCheck.restype = c_int
tenGradientCheck.argtypes = [POINTER(Nrrd), c_int, c_uint]
tenGradientRandom = libteem.tenGradientRandom
tenGradientRandom.restype = c_int
tenGradientRandom.argtypes = [POINTER(Nrrd), c_uint, c_uint]
tenGradientIdealEdge = libteem.tenGradientIdealEdge
tenGradientIdealEdge.restype = c_double
tenGradientIdealEdge.argtypes = [c_uint, c_int]
tenGradientJitter = libteem.tenGradientJitter
tenGradientJitter.restype = c_int
tenGradientJitter.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double]
tenGradientBalance = libteem.tenGradientBalance
tenGradientBalance.restype = c_int
tenGradientBalance.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(tenGradientParm)]
tenGradientMeasure = libteem.tenGradientMeasure
tenGradientMeasure.restype = None
tenGradientMeasure.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(Nrrd), POINTER(tenGradientParm), c_int]
tenGradientDistribute = libteem.tenGradientDistribute
tenGradientDistribute.restype = c_int
tenGradientDistribute.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(tenGradientParm)]
tenGradientGenerate = libteem.tenGradientGenerate
tenGradientGenerate.restype = c_int
tenGradientGenerate.argtypes = [POINTER(Nrrd), c_uint, POINTER(tenGradientParm)]
tenAniso = (POINTER(airEnum)).in_dll(libteem, 'tenAniso')
tenInterpType = (POINTER(airEnum)).in_dll(libteem, 'tenInterpType')
tenGage = (POINTER(airEnum)).in_dll(libteem, 'tenGage')
tenFiberType = (POINTER(airEnum)).in_dll(libteem, 'tenFiberType')
tenDwiFiberType = (POINTER(airEnum)).in_dll(libteem, 'tenDwiFiberType')
tenFiberStop = (POINTER(airEnum)).in_dll(libteem, 'tenFiberStop')
tenFiberIntg = (POINTER(airEnum)).in_dll(libteem, 'tenFiberIntg')
tenGlyphType = (POINTER(airEnum)).in_dll(libteem, 'tenGlyphType')
tenEstimate1Method = (POINTER(airEnum)).in_dll(libteem, 'tenEstimate1Method')
tenEstimate2Method = (POINTER(airEnum)).in_dll(libteem, 'tenEstimate2Method')
tenTripleType = (POINTER(airEnum)).in_dll(libteem, 'tenTripleType')
tenInterpParmNew = libteem.tenInterpParmNew
tenInterpParmNew.restype = POINTER(tenInterpParm)
tenInterpParmNew.argtypes = []
tenInterpParmCopy = libteem.tenInterpParmCopy
tenInterpParmCopy.restype = POINTER(tenInterpParm)
tenInterpParmCopy.argtypes = [POINTER(tenInterpParm)]
tenInterpParmBufferAlloc = libteem.tenInterpParmBufferAlloc
tenInterpParmBufferAlloc.restype = c_int
tenInterpParmBufferAlloc.argtypes = [POINTER(tenInterpParm), c_uint]
tenInterpParmNix = libteem.tenInterpParmNix
tenInterpParmNix.restype = POINTER(tenInterpParm)
tenInterpParmNix.argtypes = [POINTER(tenInterpParm)]
tenInterpTwo_d = libteem.tenInterpTwo_d
tenInterpTwo_d.restype = None
tenInterpTwo_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_double, POINTER(tenInterpParm)]
tenInterpN_d = libteem.tenInterpN_d
tenInterpN_d.restype = c_int
tenInterpN_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), c_uint, c_int, POINTER(tenInterpParm)]
tenInterpPathLength = libteem.tenInterpPathLength
tenInterpPathLength.restype = c_double
tenInterpPathLength.argtypes = [POINTER(Nrrd), c_int, c_int, c_int]
tenInterpTwoDiscrete_d = libteem.tenInterpTwoDiscrete_d
tenInterpTwoDiscrete_d.restype = c_int
tenInterpTwoDiscrete_d.argtypes = [POINTER(Nrrd), POINTER(c_double), POINTER(c_double), c_int, c_uint, POINTER(tenInterpParm)]
tenInterpDistanceTwo_d = libteem.tenInterpDistanceTwo_d
tenInterpDistanceTwo_d.restype = c_double
tenInterpDistanceTwo_d.argtypes = [POINTER(c_double), POINTER(c_double), c_int, POINTER(tenInterpParm)]
tenInterpMulti3D = libteem.tenInterpMulti3D
tenInterpMulti3D.restype = c_int
tenInterpMulti3D.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(c_double), c_uint, c_int, POINTER(tenInterpParm)]
tenGlyphParmNew = libteem.tenGlyphParmNew
tenGlyphParmNew.restype = POINTER(tenGlyphParm)
tenGlyphParmNew.argtypes = []
tenGlyphParmNix = libteem.tenGlyphParmNix
tenGlyphParmNix.restype = POINTER(tenGlyphParm)
tenGlyphParmNix.argtypes = [POINTER(tenGlyphParm)]
tenGlyphParmCheck = libteem.tenGlyphParmCheck
tenGlyphParmCheck.restype = c_int
tenGlyphParmCheck.argtypes = [POINTER(tenGlyphParm), POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
tenGlyphGen = libteem.tenGlyphGen
tenGlyphGen.restype = c_int
tenGlyphGen.argtypes = [POINTER(limnObject), POINTER(echoScene), POINTER(tenGlyphParm), POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
tenVerbose = (c_int).in_dll(libteem, 'tenVerbose')
tenTensorCheck = libteem.tenTensorCheck
tenTensorCheck.restype = c_int
tenTensorCheck.argtypes = [POINTER(Nrrd), c_int, c_int, c_int]
tenMeasurementFrameReduce = libteem.tenMeasurementFrameReduce
tenMeasurementFrameReduce.restype = c_int
tenMeasurementFrameReduce.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
tenExpand = libteem.tenExpand
tenExpand.restype = c_int
tenExpand.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double, c_double]
tenShrink = libteem.tenShrink
tenShrink.restype = c_int
tenShrink.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
tenEigensolve_f = libteem.tenEigensolve_f
tenEigensolve_f.restype = c_int
tenEigensolve_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
tenEigensolve_d = libteem.tenEigensolve_d
tenEigensolve_d.restype = c_int
tenEigensolve_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
tenMakeSingle_f = libteem.tenMakeSingle_f
tenMakeSingle_f.restype = None
tenMakeSingle_f.argtypes = [POINTER(c_float), c_float, POINTER(c_float), POINTER(c_float)]
tenMakeSingle_d = libteem.tenMakeSingle_d
tenMakeSingle_d.restype = None
tenMakeSingle_d.argtypes = [POINTER(c_double), c_double, POINTER(c_double), POINTER(c_double)]
tenMake = libteem.tenMake
tenMake.restype = c_int
tenMake.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
tenSlice = libteem.tenSlice
tenSlice.restype = c_int
tenSlice.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, size_t, c_uint]
tenInvariantGradientsK_d = libteem.tenInvariantGradientsK_d
tenInvariantGradientsK_d.restype = None
tenInvariantGradientsK_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_double]
tenInvariantGradientsR_d = libteem.tenInvariantGradientsR_d
tenInvariantGradientsR_d.restype = None
tenInvariantGradientsR_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_double]
tenRotationTangents_d = libteem.tenRotationTangents_d
tenRotationTangents_d.restype = None
tenRotationTangents_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double)]
tenLogSingle_d = libteem.tenLogSingle_d
tenLogSingle_d.restype = None
tenLogSingle_d.argtypes = [POINTER(c_double), POINTER(c_double)]
tenLogSingle_f = libteem.tenLogSingle_f
tenLogSingle_f.restype = None
tenLogSingle_f.argtypes = [POINTER(c_float), POINTER(c_float)]
tenExpSingle_d = libteem.tenExpSingle_d
tenExpSingle_d.restype = None
tenExpSingle_d.argtypes = [POINTER(c_double), POINTER(c_double)]
tenExpSingle_f = libteem.tenExpSingle_f
tenExpSingle_f.restype = None
tenExpSingle_f.argtypes = [POINTER(c_float), POINTER(c_float)]
tenSqrtSingle_d = libteem.tenSqrtSingle_d
tenSqrtSingle_d.restype = None
tenSqrtSingle_d.argtypes = [POINTER(c_double), POINTER(c_double)]
tenSqrtSingle_f = libteem.tenSqrtSingle_f
tenSqrtSingle_f.restype = None
tenSqrtSingle_f.argtypes = [POINTER(c_float), POINTER(c_float)]
tenPowSingle_d = libteem.tenPowSingle_d
tenPowSingle_d.restype = None
tenPowSingle_d.argtypes = [POINTER(c_double), POINTER(c_double), c_double]
tenPowSingle_f = libteem.tenPowSingle_f
tenPowSingle_f.restype = None
tenPowSingle_f.argtypes = [POINTER(c_float), POINTER(c_float), c_float]
tenInv_f = libteem.tenInv_f
tenInv_f.restype = None
tenInv_f.argtypes = [POINTER(c_float), POINTER(c_float)]
tenInv_d = libteem.tenInv_d
tenInv_d.restype = None
tenInv_d.argtypes = [POINTER(c_double), POINTER(c_double)]
tenDoubleContract_d = libteem.tenDoubleContract_d
tenDoubleContract_d.restype = c_double
tenDoubleContract_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
tenDWMRIModalityKey = (STRING).in_dll(libteem, 'tenDWMRIModalityKey')
tenDWMRIModalityVal = (STRING).in_dll(libteem, 'tenDWMRIModalityVal')
tenDWMRINAVal = (STRING).in_dll(libteem, 'tenDWMRINAVal')
tenDWMRIBValueKey = (STRING).in_dll(libteem, 'tenDWMRIBValueKey')
tenDWMRIGradKeyFmt = (STRING).in_dll(libteem, 'tenDWMRIGradKeyFmt')
tenDWMRIBmatKeyFmt = (STRING).in_dll(libteem, 'tenDWMRIBmatKeyFmt')
tenDWMRINexKeyFmt = (STRING).in_dll(libteem, 'tenDWMRINexKeyFmt')
tenDWMRISkipKeyFmt = (STRING).in_dll(libteem, 'tenDWMRISkipKeyFmt')
tenDWMRIKeyValueParse = libteem.tenDWMRIKeyValueParse
tenDWMRIKeyValueParse.restype = c_int
tenDWMRIKeyValueParse.argtypes = [POINTER(POINTER(Nrrd)), POINTER(POINTER(Nrrd)), POINTER(c_double), POINTER(POINTER(c_uint)), POINTER(c_uint), POINTER(Nrrd)]
tenBMatrixCalc = libteem.tenBMatrixCalc
tenBMatrixCalc.restype = c_int
tenBMatrixCalc.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
tenEMatrixCalc = libteem.tenEMatrixCalc
tenEMatrixCalc.restype = c_int
tenEMatrixCalc.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
tenEstimateLinearSingle_f = libteem.tenEstimateLinearSingle_f
tenEstimateLinearSingle_f.restype = None
tenEstimateLinearSingle_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), POINTER(c_double), POINTER(c_double), c_uint, c_int, c_float, c_float, c_float]
tenEstimateLinearSingle_d = libteem.tenEstimateLinearSingle_d
tenEstimateLinearSingle_d.restype = None
tenEstimateLinearSingle_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_uint, c_int, c_double, c_double, c_double]
tenEstimateLinear3D = libteem.tenEstimateLinear3D
tenEstimateLinear3D.restype = c_int
tenEstimateLinear3D.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(POINTER(Nrrd)), POINTER(POINTER(Nrrd)), c_uint, POINTER(Nrrd), c_int, c_double, c_double, c_double]
tenEstimateLinear4D = libteem.tenEstimateLinear4D
tenEstimateLinear4D.restype = c_int
tenEstimateLinear4D.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(POINTER(Nrrd)), POINTER(Nrrd), POINTER(Nrrd), c_int, c_double, c_double, c_double]
tenSimulateSingle_f = libteem.tenSimulateSingle_f
tenSimulateSingle_f.restype = None
tenSimulateSingle_f.argtypes = [POINTER(c_float), c_float, POINTER(c_float), POINTER(c_double), c_uint, c_float]
tenSimulate = libteem.tenSimulate
tenSimulate.restype = c_int
tenSimulate.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), c_double]
tenEstimateContextNew = libteem.tenEstimateContextNew
tenEstimateContextNew.restype = POINTER(tenEstimateContext)
tenEstimateContextNew.argtypes = []
tenEstimateVerboseSet = libteem.tenEstimateVerboseSet
tenEstimateVerboseSet.restype = None
tenEstimateVerboseSet.argtypes = [POINTER(tenEstimateContext), c_int]
tenEstimateNegEvalShiftSet = libteem.tenEstimateNegEvalShiftSet
tenEstimateNegEvalShiftSet.restype = None
tenEstimateNegEvalShiftSet.argtypes = [POINTER(tenEstimateContext), c_int]
tenEstimateMethodSet = libteem.tenEstimateMethodSet
tenEstimateMethodSet.restype = c_int
tenEstimateMethodSet.argtypes = [POINTER(tenEstimateContext), c_int]
tenEstimateSigmaSet = libteem.tenEstimateSigmaSet
tenEstimateSigmaSet.restype = c_int
tenEstimateSigmaSet.argtypes = [POINTER(tenEstimateContext), c_double]
tenEstimateValueMinSet = libteem.tenEstimateValueMinSet
tenEstimateValueMinSet.restype = c_int
tenEstimateValueMinSet.argtypes = [POINTER(tenEstimateContext), c_double]
tenEstimateGradientsSet = libteem.tenEstimateGradientsSet
tenEstimateGradientsSet.restype = c_int
tenEstimateGradientsSet.argtypes = [POINTER(tenEstimateContext), POINTER(Nrrd), c_double, c_int]
tenEstimateBMatricesSet = libteem.tenEstimateBMatricesSet
tenEstimateBMatricesSet.restype = c_int
tenEstimateBMatricesSet.argtypes = [POINTER(tenEstimateContext), POINTER(Nrrd), c_double, c_int]
tenEstimateSkipSet = libteem.tenEstimateSkipSet
tenEstimateSkipSet.restype = c_int
tenEstimateSkipSet.argtypes = [POINTER(tenEstimateContext), c_uint, c_int]
tenEstimateSkipReset = libteem.tenEstimateSkipReset
tenEstimateSkipReset.restype = c_int
tenEstimateSkipReset.argtypes = [POINTER(tenEstimateContext)]
tenEstimateThresholdSet = libteem.tenEstimateThresholdSet
tenEstimateThresholdSet.restype = c_int
tenEstimateThresholdSet.argtypes = [POINTER(tenEstimateContext), c_double, c_double]
tenEstimateUpdate = libteem.tenEstimateUpdate
tenEstimateUpdate.restype = c_int
tenEstimateUpdate.argtypes = [POINTER(tenEstimateContext)]
tenEstimate1TensorSimulateSingle_f = libteem.tenEstimate1TensorSimulateSingle_f
tenEstimate1TensorSimulateSingle_f.restype = c_int
tenEstimate1TensorSimulateSingle_f.argtypes = [POINTER(tenEstimateContext), POINTER(c_float), c_float, c_float, c_float, POINTER(c_float)]
tenEstimate1TensorSimulateSingle_d = libteem.tenEstimate1TensorSimulateSingle_d
tenEstimate1TensorSimulateSingle_d.restype = c_int
tenEstimate1TensorSimulateSingle_d.argtypes = [POINTER(tenEstimateContext), POINTER(c_double), c_double, c_double, c_double, POINTER(c_double)]
tenEstimate1TensorSimulateVolume = libteem.tenEstimate1TensorSimulateVolume
tenEstimate1TensorSimulateVolume.restype = c_int
tenEstimate1TensorSimulateVolume.argtypes = [POINTER(tenEstimateContext), POINTER(Nrrd), c_double, c_double, POINTER(Nrrd), POINTER(Nrrd), c_int, c_int]
tenEstimate1TensorSingle_f = libteem.tenEstimate1TensorSingle_f
tenEstimate1TensorSingle_f.restype = c_int
tenEstimate1TensorSingle_f.argtypes = [POINTER(tenEstimateContext), POINTER(c_float), POINTER(c_float)]
tenEstimate1TensorSingle_d = libteem.tenEstimate1TensorSingle_d
tenEstimate1TensorSingle_d.restype = c_int
tenEstimate1TensorSingle_d.argtypes = [POINTER(tenEstimateContext), POINTER(c_double), POINTER(c_double)]
tenEstimate1TensorVolume4D = libteem.tenEstimate1TensorVolume4D
tenEstimate1TensorVolume4D.restype = c_int
tenEstimate1TensorVolume4D.argtypes = [POINTER(tenEstimateContext), POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(POINTER(Nrrd)), POINTER(Nrrd), c_int]
tenEstimateContextNix = libteem.tenEstimateContextNix
tenEstimateContextNix.restype = POINTER(tenEstimateContext)
tenEstimateContextNix.argtypes = [POINTER(tenEstimateContext)]
tenAnisoEval_f = libteem.tenAnisoEval_f
tenAnisoEval_f.restype = c_float
tenAnisoEval_f.argtypes = [POINTER(c_float), c_int]
tenAnisoEval_d = libteem.tenAnisoEval_d
tenAnisoEval_d.restype = c_double
tenAnisoEval_d.argtypes = [POINTER(c_double), c_int]
tenAnisoTen_f = libteem.tenAnisoTen_f
tenAnisoTen_f.restype = c_float
tenAnisoTen_f.argtypes = [POINTER(c_float), c_int]
tenAnisoTen_d = libteem.tenAnisoTen_d
tenAnisoTen_d.restype = c_double
tenAnisoTen_d.argtypes = [POINTER(c_double), c_int]
tenAnisoPlot = libteem.tenAnisoPlot
tenAnisoPlot.restype = c_int
tenAnisoPlot.argtypes = [POINTER(Nrrd), c_int, c_uint, c_int, c_int, c_int]
tenAnisoVolume = libteem.tenAnisoVolume
tenAnisoVolume.restype = c_int
tenAnisoVolume.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, c_double]
tenAnisoHistogram = libteem.tenAnisoHistogram
tenAnisoHistogram.restype = c_int
tenAnisoHistogram.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), c_int, c_int, c_uint]
tenEvecRGBParmNew = libteem.tenEvecRGBParmNew
tenEvecRGBParmNew.restype = POINTER(tenEvecRGBParm)
tenEvecRGBParmNew.argtypes = []
tenEvecRGBParmNix = libteem.tenEvecRGBParmNix
tenEvecRGBParmNix.restype = POINTER(tenEvecRGBParm)
tenEvecRGBParmNix.argtypes = [POINTER(tenEvecRGBParm)]
tenEvecRGBParmCheck = libteem.tenEvecRGBParmCheck
tenEvecRGBParmCheck.restype = c_int
tenEvecRGBParmCheck.argtypes = [POINTER(tenEvecRGBParm)]
tenEvecRGBSingle_f = libteem.tenEvecRGBSingle_f
tenEvecRGBSingle_f.restype = None
tenEvecRGBSingle_f.argtypes = [POINTER(c_float), c_float, POINTER(c_float), POINTER(c_float), POINTER(tenEvecRGBParm)]
tenEvecRGBSingle_d = libteem.tenEvecRGBSingle_d
tenEvecRGBSingle_d.restype = None
tenEvecRGBSingle_d.argtypes = [POINTER(c_double), c_double, POINTER(c_double), POINTER(c_double), POINTER(tenEvecRGBParm)]
tenEvecRGB = libteem.tenEvecRGB
tenEvecRGB.restype = c_int
tenEvecRGB.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(tenEvecRGBParm)]
tenEvqVolume = libteem.tenEvqVolume
tenEvqVolume.restype = c_int
tenEvqVolume.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, c_int, c_int]
tenBMatrixCheck = libteem.tenBMatrixCheck
tenBMatrixCheck.restype = c_int
tenBMatrixCheck.argtypes = [POINTER(Nrrd), c_int, c_uint]
tenFiberSingleInit = libteem.tenFiberSingleInit
tenFiberSingleInit.restype = None
tenFiberSingleInit.argtypes = [POINTER(tenFiberSingle)]
tenFiberSingleDone = libteem.tenFiberSingleDone
tenFiberSingleDone.restype = None
tenFiberSingleDone.argtypes = [POINTER(tenFiberSingle)]
tenFiberSingleNew = libteem.tenFiberSingleNew
tenFiberSingleNew.restype = POINTER(tenFiberSingle)
tenFiberSingleNew.argtypes = []
tenFiberSingleNix = libteem.tenFiberSingleNix
tenFiberSingleNix.restype = POINTER(tenFiberSingle)
tenFiberSingleNix.argtypes = [POINTER(tenFiberSingle)]
tenFiberContextNew = libteem.tenFiberContextNew
tenFiberContextNew.restype = POINTER(tenFiberContext)
tenFiberContextNew.argtypes = [POINTER(Nrrd)]
tenFiberContextDwiNew = libteem.tenFiberContextDwiNew
tenFiberContextDwiNew.restype = POINTER(tenFiberContext)
tenFiberContextDwiNew.argtypes = [POINTER(Nrrd), c_double, c_double, c_double, c_int, c_int]
tenFiberVerboseSet = libteem.tenFiberVerboseSet
tenFiberVerboseSet.restype = None
tenFiberVerboseSet.argtypes = [POINTER(tenFiberContext), c_int]
tenFiberTypeSet = libteem.tenFiberTypeSet
tenFiberTypeSet.restype = c_int
tenFiberTypeSet.argtypes = [POINTER(tenFiberContext), c_int]
tenFiberKernelSet = libteem.tenFiberKernelSet
tenFiberKernelSet.restype = c_int
tenFiberKernelSet.argtypes = [POINTER(tenFiberContext), POINTER(NrrdKernel), POINTER(c_double)]
tenFiberIntgSet = libteem.tenFiberIntgSet
tenFiberIntgSet.restype = c_int
tenFiberIntgSet.argtypes = [POINTER(tenFiberContext), c_int]
tenFiberStopSet = libteem.tenFiberStopSet
tenFiberStopSet.restype = c_int
tenFiberStopSet.argtypes = [POINTER(tenFiberContext), c_int]
tenFiberStopAnisoSet = libteem.tenFiberStopAnisoSet
tenFiberStopAnisoSet.restype = c_int
tenFiberStopAnisoSet.argtypes = [POINTER(tenFiberContext), c_int, c_double]
tenFiberStopDoubleSet = libteem.tenFiberStopDoubleSet
tenFiberStopDoubleSet.restype = c_int
tenFiberStopDoubleSet.argtypes = [POINTER(tenFiberContext), c_int, c_double]
tenFiberStopUIntSet = libteem.tenFiberStopUIntSet
tenFiberStopUIntSet.restype = c_int
tenFiberStopUIntSet.argtypes = [POINTER(tenFiberContext), c_int, c_uint]
tenFiberStopOn = libteem.tenFiberStopOn
tenFiberStopOn.restype = None
tenFiberStopOn.argtypes = [POINTER(tenFiberContext), c_int]
tenFiberStopOff = libteem.tenFiberStopOff
tenFiberStopOff.restype = None
tenFiberStopOff.argtypes = [POINTER(tenFiberContext), c_int]
tenFiberStopReset = libteem.tenFiberStopReset
tenFiberStopReset.restype = None
tenFiberStopReset.argtypes = [POINTER(tenFiberContext)]
tenFiberAnisoSpeedSet = libteem.tenFiberAnisoSpeedSet
tenFiberAnisoSpeedSet.restype = c_int
tenFiberAnisoSpeedSet.argtypes = [POINTER(tenFiberContext), c_int, c_double, c_double, c_double]
tenFiberAnisoSpeedReset = libteem.tenFiberAnisoSpeedReset
tenFiberAnisoSpeedReset.restype = c_int
tenFiberAnisoSpeedReset.argtypes = [POINTER(tenFiberContext)]
tenFiberParmSet = libteem.tenFiberParmSet
tenFiberParmSet.restype = c_int
tenFiberParmSet.argtypes = [POINTER(tenFiberContext), c_int, c_double]
tenFiberUpdate = libteem.tenFiberUpdate
tenFiberUpdate.restype = c_int
tenFiberUpdate.argtypes = [POINTER(tenFiberContext)]
tenFiberContextCopy = libteem.tenFiberContextCopy
tenFiberContextCopy.restype = POINTER(tenFiberContext)
tenFiberContextCopy.argtypes = [POINTER(tenFiberContext)]
tenFiberContextNix = libteem.tenFiberContextNix
tenFiberContextNix.restype = POINTER(tenFiberContext)
tenFiberContextNix.argtypes = [POINTER(tenFiberContext)]
tenFiberTraceSet = libteem.tenFiberTraceSet
tenFiberTraceSet.restype = c_int
tenFiberTraceSet.argtypes = [POINTER(tenFiberContext), POINTER(Nrrd), POINTER(c_double), c_uint, POINTER(c_uint), POINTER(c_uint), POINTER(c_double)]
tenFiberTrace = libteem.tenFiberTrace
tenFiberTrace.restype = c_int
tenFiberTrace.argtypes = [POINTER(tenFiberContext), POINTER(Nrrd), POINTER(c_double)]
tenFiberDirectionNumber = libteem.tenFiberDirectionNumber
tenFiberDirectionNumber.restype = c_uint
tenFiberDirectionNumber.argtypes = [POINTER(tenFiberContext), POINTER(c_double)]
tenFiberSingleTrace = libteem.tenFiberSingleTrace
tenFiberSingleTrace.restype = c_int
tenFiberSingleTrace.argtypes = [POINTER(tenFiberContext), POINTER(tenFiberSingle), POINTER(c_double), c_uint]
tenFiberMultiNew = libteem.tenFiberMultiNew
tenFiberMultiNew.restype = POINTER(tenFiberMulti)
tenFiberMultiNew.argtypes = []
tenFiberMultiNix = libteem.tenFiberMultiNix
tenFiberMultiNix.restype = POINTER(tenFiberMulti)
tenFiberMultiNix.argtypes = [POINTER(tenFiberMulti)]
tenFiberMultiTrace = libteem.tenFiberMultiTrace
tenFiberMultiTrace.restype = c_int
tenFiberMultiTrace.argtypes = [POINTER(tenFiberContext), POINTER(tenFiberMulti), POINTER(Nrrd)]
tenFiberMultiPolyData = libteem.tenFiberMultiPolyData
tenFiberMultiPolyData.restype = c_int
tenFiberMultiPolyData.argtypes = [POINTER(tenFiberContext), POINTER(limnPolyData), POINTER(tenFiberMulti)]
tenEpiRegister3D = libteem.tenEpiRegister3D
tenEpiRegister3D.restype = c_int
tenEpiRegister3D.argtypes = [POINTER(POINTER(Nrrd)), POINTER(POINTER(Nrrd)), c_uint, POINTER(Nrrd), c_int, c_double, c_double, c_double, c_double, c_int, POINTER(NrrdKernel), POINTER(c_double), c_int, c_int]
tenEpiRegister4D = libteem.tenEpiRegister4D
tenEpiRegister4D.restype = c_int
tenEpiRegister4D.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), c_int, c_double, c_double, c_double, c_double, c_int, POINTER(NrrdKernel), POINTER(c_double), c_int, c_int]
tenExperSpecNew = libteem.tenExperSpecNew
tenExperSpecNew.restype = POINTER(tenExperSpec)
tenExperSpecNew.argtypes = []
tenExperSpecGradSingleBValSet = libteem.tenExperSpecGradSingleBValSet
tenExperSpecGradSingleBValSet.restype = c_int
tenExperSpecGradSingleBValSet.argtypes = [POINTER(tenExperSpec), c_uint, c_double, POINTER(c_double)]
tenExperSpecGradBValSet = libteem.tenExperSpecGradBValSet
tenExperSpecGradBValSet.restype = c_int
tenExperSpecGradBValSet.argtypes = [POINTER(tenExperSpec), c_uint, POINTER(c_double), POINTER(c_double)]
tenExperSpecFromKeyValueSet = libteem.tenExperSpecFromKeyValueSet
tenExperSpecFromKeyValueSet.restype = c_int
tenExperSpecFromKeyValueSet.argtypes = [POINTER(tenExperSpec), POINTER(Nrrd)]
tenExperSpecNix = libteem.tenExperSpecNix
tenExperSpecNix.restype = POINTER(tenExperSpec)
tenExperSpecNix.argtypes = [POINTER(tenExperSpec)]
tenExperSpecKnownB0Get = libteem.tenExperSpecKnownB0Get
tenExperSpecKnownB0Get.restype = c_double
tenExperSpecKnownB0Get.argtypes = [POINTER(tenExperSpec), POINTER(c_double)]
tenExperSpecMaxBGet = libteem.tenExperSpecMaxBGet
tenExperSpecMaxBGet.restype = c_double
tenExperSpecMaxBGet.argtypes = [POINTER(tenExperSpec)]
tenDWMRIKeyValueFromExperSpecSet = libteem.tenDWMRIKeyValueFromExperSpecSet
tenDWMRIKeyValueFromExperSpecSet.restype = c_int
tenDWMRIKeyValueFromExperSpecSet.argtypes = [POINTER(Nrrd), POINTER(tenExperSpec)]
tenModelPrefixStr = (STRING).in_dll(libteem, 'tenModelPrefixStr')
tenModelParse = libteem.tenModelParse
tenModelParse.restype = c_int
tenModelParse.argtypes = [POINTER(POINTER(tenModel)), POINTER(c_int), c_int, STRING]
tenModelFromAxisLearn = libteem.tenModelFromAxisLearn
tenModelFromAxisLearn.restype = c_int
tenModelFromAxisLearn.argtypes = [POINTER(POINTER(tenModel)), POINTER(c_int), POINTER(NrrdAxisInfo)]
tenModelSimulate = libteem.tenModelSimulate
tenModelSimulate.restype = c_int
tenModelSimulate.argtypes = [POINTER(Nrrd), c_int, POINTER(tenExperSpec), POINTER(tenModel), POINTER(Nrrd), POINTER(Nrrd), c_int]
tenModelSqeFit = libteem.tenModelSqeFit
tenModelSqeFit.restype = c_int
tenModelSqeFit.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(tenModel), POINTER(tenExperSpec), POINTER(Nrrd), c_int, c_int, c_int, c_uint, c_uint, c_uint, c_double, POINTER(airRandMTState)]
tenModelNllFit = libteem.tenModelNllFit
tenModelNllFit.restype = c_int
tenModelNllFit.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(tenModel), POINTER(tenExperSpec), POINTER(Nrrd), c_int, c_double, c_int]
tenModelConvert = libteem.tenModelConvert
tenModelConvert.restype = c_int
tenModelConvert.argtypes = [POINTER(Nrrd), POINTER(c_int), POINTER(tenModel), POINTER(Nrrd), POINTER(tenModel)]
tenModelBall = (POINTER(tenModel)).in_dll(libteem, 'tenModelBall')
tenModel1Stick = (POINTER(tenModel)).in_dll(libteem, 'tenModel1Stick')
tenModelBall1Stick = (POINTER(tenModel)).in_dll(libteem, 'tenModelBall1Stick')
tenModelCylinder = (POINTER(tenModel)).in_dll(libteem, 'tenModelCylinder')
tenModelTensor2 = (POINTER(tenModel)).in_dll(libteem, 'tenModelTensor2')
tenSizeNormalize = libteem.tenSizeNormalize
tenSizeNormalize.restype = c_int
tenSizeNormalize.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(c_double), c_double, c_double]
tenSizeScale = libteem.tenSizeScale
tenSizeScale.restype = c_int
tenSizeScale.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double]
tenAnisoScale = libteem.tenAnisoScale
tenAnisoScale.restype = c_int
tenAnisoScale.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double, c_int, c_int]
tenEigenvaluePower = libteem.tenEigenvaluePower
tenEigenvaluePower.restype = c_int
tenEigenvaluePower.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double]
tenEigenvalueClamp = libteem.tenEigenvalueClamp
tenEigenvalueClamp.restype = c_int
tenEigenvalueClamp.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double, c_double]
tenEigenvalueAdd = libteem.tenEigenvalueAdd
tenEigenvalueAdd.restype = c_int
tenEigenvalueAdd.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double]
tenEigenvalueMultiply = libteem.tenEigenvalueMultiply
tenEigenvalueMultiply.restype = c_int
tenEigenvalueMultiply.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double]
tenLog = libteem.tenLog
tenLog.restype = c_int
tenLog.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
tenExp = libteem.tenExp
tenExp.restype = c_int
tenExp.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
tenBVecNonLinearFit = libteem.tenBVecNonLinearFit
tenBVecNonLinearFit.restype = c_int
tenBVecNonLinearFit.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(c_double), POINTER(c_double), c_int, c_double]
tenGageKind = (POINTER(gageKind)).in_dll(libteem, 'tenGageKind')
tenDwiGage = (POINTER(airEnum)).in_dll(libteem, 'tenDwiGage')
tenDwiGageKindNew = libteem.tenDwiGageKindNew
tenDwiGageKindNew.restype = POINTER(gageKind)
tenDwiGageKindNew.argtypes = []
tenDwiGageKindNix = libteem.tenDwiGageKindNix
tenDwiGageKindNix.restype = POINTER(gageKind)
tenDwiGageKindNix.argtypes = [POINTER(gageKind)]
tenDwiGageKindSet = libteem.tenDwiGageKindSet
tenDwiGageKindSet.restype = c_int
tenDwiGageKindSet.argtypes = [POINTER(gageKind), c_double, c_double, c_double, c_double, POINTER(Nrrd), POINTER(Nrrd), c_int, c_int, c_uint]
tenDwiGageKindCheck = libteem.tenDwiGageKindCheck
tenDwiGageKindCheck.restype = c_int
tenDwiGageKindCheck.argtypes = [POINTER(gageKind)]
tenEMBimodalParmNew = libteem.tenEMBimodalParmNew
tenEMBimodalParmNew.restype = POINTER(tenEMBimodalParm)
tenEMBimodalParmNew.argtypes = []
tenEMBimodalParmNix = libteem.tenEMBimodalParmNix
tenEMBimodalParmNix.restype = POINTER(tenEMBimodalParm)
tenEMBimodalParmNix.argtypes = [POINTER(tenEMBimodalParm)]
tenEMBimodal = libteem.tenEMBimodal
tenEMBimodal.restype = c_int
tenEMBimodal.argtypes = [POINTER(tenEMBimodalParm), POINTER(Nrrd)]
class unrrduCmd(Structure):
    pass
unrrduCmd._fields_ = [
    ('name', STRING),
    ('info', STRING),
    ('main', CFUNCTYPE(c_int, c_int, POINTER(STRING), STRING, POINTER(hestParm))),
]
tend_mfitCmd = (unrrduCmd).in_dll(libteem, 'tend_mfitCmd')
tend_mconvCmd = (unrrduCmd).in_dll(libteem, 'tend_mconvCmd')
tend_msimCmd = (unrrduCmd).in_dll(libteem, 'tend_msimCmd')
tend_gradsCmd = (unrrduCmd).in_dll(libteem, 'tend_gradsCmd')
tend_tconvCmd = (unrrduCmd).in_dll(libteem, 'tend_tconvCmd')
tend_fiberCmd = (unrrduCmd).in_dll(libteem, 'tend_fiberCmd')
tend_anplotCmd = (unrrduCmd).in_dll(libteem, 'tend_anplotCmd')
tend_anhistCmd = (unrrduCmd).in_dll(libteem, 'tend_anhistCmd')
tend_sliceCmd = (unrrduCmd).in_dll(libteem, 'tend_sliceCmd')
tend_tripleCmd = (unrrduCmd).in_dll(libteem, 'tend_tripleCmd')
tend_avgCmd = (unrrduCmd).in_dll(libteem, 'tend_avgCmd')
tend_anscaleCmd = (unrrduCmd).in_dll(libteem, 'tend_anscaleCmd')
tend_glyphCmd = (unrrduCmd).in_dll(libteem, 'tend_glyphCmd')
tend_epiregCmd = (unrrduCmd).in_dll(libteem, 'tend_epiregCmd')
tend_evaladdCmd = (unrrduCmd).in_dll(libteem, 'tend_evaladdCmd')
tend_evqCmd = (unrrduCmd).in_dll(libteem, 'tend_evqCmd')
tend_evalmultCmd = (unrrduCmd).in_dll(libteem, 'tend_evalmultCmd')
tend_normCmd = (unrrduCmd).in_dll(libteem, 'tend_normCmd')
tend_evalclampCmd = (unrrduCmd).in_dll(libteem, 'tend_evalclampCmd')
tend_makeCmd = (unrrduCmd).in_dll(libteem, 'tend_makeCmd')
tend_estimCmd = (unrrduCmd).in_dll(libteem, 'tend_estimCmd')
tend_aboutCmd = (unrrduCmd).in_dll(libteem, 'tend_aboutCmd')
tend_bfitCmd = (unrrduCmd).in_dll(libteem, 'tend_bfitCmd')
tend_evecCmd = (unrrduCmd).in_dll(libteem, 'tend_evecCmd')
tend_evalpowCmd = (unrrduCmd).in_dll(libteem, 'tend_evalpowCmd')
tend_expCmd = (unrrduCmd).in_dll(libteem, 'tend_expCmd')
tend_ellipseCmd = (unrrduCmd).in_dll(libteem, 'tend_ellipseCmd')
tend_shrinkCmd = (unrrduCmd).in_dll(libteem, 'tend_shrinkCmd')
tend_logCmd = (unrrduCmd).in_dll(libteem, 'tend_logCmd')
tend_satinCmd = (unrrduCmd).in_dll(libteem, 'tend_satinCmd')
tend_helixCmd = (unrrduCmd).in_dll(libteem, 'tend_helixCmd')
tend_unmfCmd = (unrrduCmd).in_dll(libteem, 'tend_unmfCmd')
tend_anvolCmd = (unrrduCmd).in_dll(libteem, 'tend_anvolCmd')
tend_simCmd = (unrrduCmd).in_dll(libteem, 'tend_simCmd')
tend_bmatCmd = (unrrduCmd).in_dll(libteem, 'tend_bmatCmd')
tend_stenCmd = (unrrduCmd).in_dll(libteem, 'tend_stenCmd')
tend_evalCmd = (unrrduCmd).in_dll(libteem, 'tend_evalCmd')
tend_pointCmd = (unrrduCmd).in_dll(libteem, 'tend_pointCmd')
tend_evecrgbCmd = (unrrduCmd).in_dll(libteem, 'tend_evecrgbCmd')
tend_expandCmd = (unrrduCmd).in_dll(libteem, 'tend_expandCmd')
tendCmdList = (POINTER(unrrduCmd) * 0).in_dll(libteem, 'tendCmdList')
tendUsage = libteem.tendUsage
tendUsage.restype = None
tendUsage.argtypes = [STRING, POINTER(hestParm)]
tendFiberStopCB = (POINTER(hestCB)).in_dll(libteem, 'tendFiberStopCB')
unrrdu_swapCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_swapCmd')
unrrdu_spliceCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_spliceCmd')
unrrdu_mlutCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_mlutCmd')
unrrdu_joinCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_joinCmd')
unrrdu_rmapCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_rmapCmd')
unrrdu_dhistoCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_dhistoCmd')
unrrdu_mrmapCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_mrmapCmd')
unrrdu_1opCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_1opCmd')
unrrdu_gammaCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_gammaCmd')
unrrdu_cropCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_cropCmd')
unrrdu_quantizeCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_quantizeCmd')
unrrdu_reshapeCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_reshapeCmd')
unrrdu_axinfoCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_axinfoCmd')
unrrdu_unorientCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_unorientCmd')
unrrdu_axsplitCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_axsplitCmd')
unrrdu_2opCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_2opCmd')
unrrdu_axmergeCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_axmergeCmd')
unrrdu_lutCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_lutCmd')
unrrdu_imapCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_imapCmd')
unrrdu_makeCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_makeCmd')
unrrdu_histaxCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_histaxCmd')
unrrdu_lut2Cmd = (unrrduCmd).in_dll(libteem, 'unrrdu_lut2Cmd')
unrrdu_3opCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_3opCmd')
unrrdu_ccmergeCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_ccmergeCmd')
unrrdu_heqCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_heqCmd')
unrrdu_histoCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_histoCmd')
unrrdu_axdeleteCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_axdeleteCmd')
unrrdu_untileCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_untileCmd')
unrrdu_substCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_substCmd')
unrrdu_resampleCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_resampleCmd')
unrrdu_minmaxCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_minmaxCmd')
unrrdu_jhistoCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_jhistoCmd')
unrrdu_saveCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_saveCmd')
unrrdu_distCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_distCmd')
unrrdu_permuteCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_permuteCmd')
unrrdu_ccsettleCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_ccsettleCmd')
unrrdu_cmedianCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_cmedianCmd')
unrrdu_convertCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_convertCmd')
unrrdu_tileCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_tileCmd')
unrrdu_axinsertCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_axinsertCmd')
unrrdu_shuffleCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_shuffleCmd')
unrrdu_headCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_headCmd')
unrrdu_ccadjCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_ccadjCmd')
unrrdu_insetCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_insetCmd')
unrrdu_unquantizeCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_unquantizeCmd')
unrrdu_aboutCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_aboutCmd')
unrrdu_padCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_padCmd')
unrrdu_dataCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_dataCmd')
unrrdu_diceCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_diceCmd')
unrrdu_envCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_envCmd')
unrrdu_projectCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_projectCmd')
unrrdu_sliceCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_sliceCmd')
unrrdu_ccfindCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_ccfindCmd')
unrrdu_flipCmd = (unrrduCmd).in_dll(libteem, 'unrrdu_flipCmd')
unrrduBiffKey = (STRING).in_dll(libteem, 'unrrduBiffKey')
unrrduDefNumColumns = (c_int).in_dll(libteem, 'unrrduDefNumColumns')
unrrduCmdList = (POINTER(unrrduCmd) * 0).in_dll(libteem, 'unrrduCmdList')
unrrduUsage = libteem.unrrduUsage
unrrduUsage.restype = None
unrrduUsage.argtypes = [STRING, POINTER(hestParm)]
unrrduHestPosCB = (hestCB).in_dll(libteem, 'unrrduHestPosCB')
unrrduHestMaybeTypeCB = (hestCB).in_dll(libteem, 'unrrduHestMaybeTypeCB')
unrrduHestScaleCB = (hestCB).in_dll(libteem, 'unrrduHestScaleCB')
unrrduHestBitsCB = (hestCB).in_dll(libteem, 'unrrduHestBitsCB')
unrrduHestFileCB = (hestCB).in_dll(libteem, 'unrrduHestFileCB')
unrrduHestEncodingCB = (hestCB).in_dll(libteem, 'unrrduHestEncodingCB')
_airThread._fields_ = [
]
_airThreadMutex._fields_ = [
]
_airThreadCond._fields_ = [
]
NrrdIoState_t._fields_ = [
    ('path', STRING),
    ('base', STRING),
    ('line', STRING),
    ('dataFNFormat', STRING),
    ('dataFN', POINTER(STRING)),
    ('headerStringWrite', STRING),
    ('headerStringRead', STRING),
    ('dataFNArr', POINTER(airArray)),
    ('headerFile', POINTER(FILE)),
    ('dataFile', POINTER(FILE)),
    ('dataFileDim', c_uint),
    ('lineLen', c_uint),
    ('charsPerLine', c_uint),
    ('valsPerLine', c_uint),
    ('lineSkip', c_uint),
    ('headerStrlen', c_uint),
    ('headerStrpos', c_uint),
    ('byteSkip', c_long),
    ('dataFNMin', c_int),
    ('dataFNMax', c_int),
    ('dataFNStep', c_int),
    ('dataFNIndex', c_int),
    ('pos', c_int),
    ('endian', c_int),
    ('seen', c_int * 33),
    ('detachedHeader', c_int),
    ('bareText', c_int),
    ('skipData', c_int),
    ('keepNrrdDataFileOpen', c_int),
    ('zlibLevel', c_int),
    ('zlibStrategy', c_int),
    ('bzip2BlockSize', c_int),
    ('learningHeaderStrlen', c_int),
    ('oldData', c_void_p),
    ('oldDataSize', size_t),
    ('format', POINTER(NrrdFormat)),
    ('encoding', POINTER(NrrdEncoding)),
]
class NrrdResampleAxis(Structure):
    pass
NrrdResampleAxis._pack_ = 4
NrrdResampleAxis._fields_ = [
    ('kernel', POINTER(NrrdKernel)),
    ('kparm', c_double * 8),
    ('min', c_double),
    ('max', c_double),
    ('samples', size_t),
    ('center', c_int),
    ('sizeIn', size_t),
    ('sizePerm', size_t * 16),
    ('axIdx', c_uint),
    ('passIdx', c_uint),
    ('axisPerm', c_uint * 16),
    ('ratio', c_double),
    ('nrsmp', POINTER(Nrrd)),
    ('nline', POINTER(Nrrd)),
    ('nindex', POINTER(Nrrd)),
    ('nweight', POINTER(Nrrd)),
]
NrrdResampleContext._pack_ = 4
NrrdResampleContext._fields_ = [
    ('nin', POINTER(Nrrd)),
    ('verbose', c_int),
    ('boundary', c_int),
    ('typeOut', c_int),
    ('renormalize', c_int),
    ('round', c_int),
    ('clamp', c_int),
    ('defaultCenter', c_int),
    ('padValue', c_double),
    ('dim', c_uint),
    ('passNum', c_uint),
    ('topRax', c_uint),
    ('botRax', c_uint),
    ('permute', c_uint * 17),
    ('passAxis', c_uint * 16),
    ('axis', NrrdResampleAxis * 17),
    ('flag', POINTER(c_int)),
    ('time', c_double),
]
NrrdIter._pack_ = 4
NrrdIter._fields_ = [
    ('nrrd', POINTER(Nrrd)),
    ('ownNrrd', POINTER(Nrrd)),
    ('val', c_double),
    ('size', size_t),
    ('data', STRING),
    ('left', size_t),
    ('load', CFUNCTYPE(c_double, c_void_p)),
]
__all__ = ['tenFiberStopUIntSet', 'biffMsgAddf',
           'limnCameraPathTrack', 'gageVecLambda2', 'miteUserNix',
           'tenFiberIntgLast', 'ell_4m_post_mul_f', 'nrrdArithGamma',
           'ell_Nm_check', 'tenEstimate1MethodMLE',
           'nrrdHasNonExistOnly', 'nrrdFormatPNG',
           'pullInterTypeLast', 'miteBiffKey', 'limnEdgeTypeLast',
           'tenGageFAShapeIndex', 'seekTypeMinimalSurface',
           'airRandMTStateNix', 'pullInfoTangent1',
           'pullInfoTangent2', 'gageContextCopy', 'miteValVdefTdotV',
           'nrrdKernelC4Hexic', 'tenEstimateContextNix',
           'tenGageFAGeomTens', 'airEndsWith', 'miteUser',
           'ell_4m_to_q_d', 'nrrdField_block_size',
           'tenEstimateSkipReset', 'pullInfoQuality', 'limnSpaceView',
           'limnSplineMinT', 'nrrdFieldInfoParse', 'nrrdField_space',
           'tenGageConfDiffusionAlign', 'gageSclK2',
           'hestRespFileComment', 'nrrdKind3DMaskedMatrix',
           'tenGageNormGradMag', 'nrrdBoundaryWrap',
           'seekLowerInsideSet', 'limnEdgeTypeBackFacet',
           'ell_q_exp_d', 'tenInterpTypeLoxR', 'airFP_SNAN',
           'tenDwiGageTensorMLEError', 'nrrdKernelBlackmanD',
           'seekContextNew', 'nrrdField_comment',
           'limnSplineInfoSize', 'nrrdSimpleResample',
           'tenInterpTypeLoxK', 'nrrdTernaryOpIfElse',
           'limnPolyDataCylinder', 'tendFiberStopCB',
           'tenGageTraceDiffusionAlign', 'unrrdu_flipCmd',
           'gageErrUnknown', 'nrrdBinaryOpSgnPow', 'nrrdKeyValueGet',
           'limnQN16octa', 'nrrdKernelC3QuinticD', 'airTypeFloat',
           'limnObjectVertexNumPreSet', 'tenGlyphTypeBox',
           'limnLightReset', 'nrrdField_space_units',
           'gageSclShapeTrace', 'pullCondEnergyTry',
           'tenDwiGage2TensorPeled', 'nrrdMeasureMax',
           'miteShadeMethodLast', 'unrrdu_minmaxCmd',
           'unrrdu_substCmd', 'limnHestPolyDataLMPD',
           'nrrdUnaryOpExists', 'nrrdSpace3DRightHandedTime',
           'airBesselI1ExpScaled', 'nrrdArithIterBinaryOp',
           'airNoDio_format', 'gageCtxFlagLast', 'dyeConvert',
           'nrrdFormatPNM', 'gageParmOrientationFromSpacing',
           'nrrdBasicInfoMeasurementFrame', 'limnQN8octa',
           'limnPolyDataNix', 'tenInterpTypeWang', 'unrrdu_envCmd',
           'nrrdSpace3DLeftHanded', 'nrrdUnaryOpCbrt',
           'nrrdAxisInfoLast', 'airFStore', 'hooverThreadBegin_t',
           'airFastExp', 'tenExperSpecNix', 'pullStatusNixMe',
           'tenGageQGradVec', 'tenGageOmegaLaplacian',
           'nrrdOriginStatusNoMin', 'tenGlyphType',
           'tenFiberSingleTrace', 'tenGageOmegaDiffusionFraction',
           'gageVecProjHelGradient', 'nrrdStateMeasureType',
           'pullInfoLast', 'tenTripleConvert',
           'hestElideSingleOtherDefault', 'echoTypeRectangle',
           'nrrdUntile2D', 'nrrdSpaceLeftAnteriorSuperior',
           'nrrdTypeULLong', 'airInsane_UCSize',
           'limnSplineTypeSpecParse', 'limnSplineInfo4Vector',
           'tenInterpParmBufferAlloc', 'nrrdResampleRangeSet',
           'unrrdu_dhistoCmd', 'tenFiberIntgRK4', 'tenGageModeWarp',
           'nrrdFormatUnknown', 'tenInterpTwoDiscrete_d',
           'tenFiberStopMinNumSteps', 'gageKindVolumeCheck',
           'tenGageNormGradVec', 'tenGageQHessian',
           'limnPolyDataCone', 'hestParmNew', 'tenEstimate2Method',
           'nrrdUnaryOpAcos', 'tenModel1Stick', 'tendCmdList',
           'airMopAlways', 'gageKindAnswerLength', 'nrrdMeasureSD',
           'tenBMatrixCalc', 'tenDwiGageKindSet', 'miteVariableParse',
           'pullCCSort', 'echoThreadStateNew', 'nrrdBinaryOpGT',
           'tenDefFiberMaxHalfLen', 'unrrdu_3opCmd', 'airFloatQNaN',
           'nrrdSplice', 'tenEigensolve_f', 'pullEnergyTypeZero',
           'tenEigensolve_d', 'tenGageQGradMag',
           'gageDefStackNormalizeDeriv', 'nrrdArithUnaryOp',
           'tenGageCp1HessianEvec2', 'tenGageCp1HessianEvec1',
           'tenGageCp1HessianEvec0', 'pullTask_t', 'echoColorSet',
           'nrrdEnvVarStateMeasureModeBins', 'miteRayBegin',
           'gageParmDefaultCenter', 'unrrdu_ccmergeCmd',
           'miteThreadNew', 'miteStage', 'airDrandMT53_r',
           'echoChannelAverage', 'tenFiberTypeEvec1',
           'tenFiberTypeEvec0', 'tenFiberTypeEvec2',
           'gageVecCurlNorm', 'ell_4m_to_q_f', 'ell_3v_print_d',
           'nrrdMeasureHistoMedian', 'ell_3v_print_f',
           'unrrdu_ccsettleCmd', 'tenGageCa1HessianEvec',
           'tenGageFADiffusionFraction', 'gageStackBlurParm',
           'limnPolyDataSpiralSuperquadric', 'seekTypeRidgeSurfaceT',
           'nrrdBasicInfoSpaceOrigin', 'airUnescape', 'airEnumPrint',
           'tenGageCa1HessianEval', 'biffGetDone',
           'pullProcessModeNixing', 'airFPGen_f', 'airFPGen_d',
           'airInsane_FltDblFPClass', 'pullInfoSpecNew',
           'nrrdBinaryOpIf', 'nrrdAxisInfoSpacing', 'airExists',
           'pullInfoLiveThresh', 'tenDWMRINexKeyFmt',
           'pullInfoInside', 'nrrdBinaryOpLTE', 'pullValLast',
           'nrrdSpaceLeftAnteriorSuperiorTime', 'limnPolyDataCopy',
           'nrrdAxisInfoUnknown', 'miteThreadBegin', 'gageShapeItoW',
           'miteThreadNix', 'limnPrimitiveLines', 'unrrduCmdList',
           'airSinglePrintf', 'gageShapeCopy', 'tenFiberVerboseSet',
           'nrrdSanity', 'biffAddf', 'tenAniso_RA',
           'tenTripleTypeRThetaZ', 'tenGageRHessian', 'nrrdFlip',
           'airNoDio_size', 'nrrdBoundaryWeight', 'pullEnergyZero',
           'gagePerVolumeNew', 'pullInfoLiveThresh2', 'nrrdAxesSwap',
           'gageOptimSigTruthSet', 'gageItemSpec',
           'nrrdDefaultWriteBareText', 'airShuffle_r',
           'limnObjectRender', 'miteValXw',
           'nrrdSpacingStatusScalarWithSpace', 'tenModelCylinder',
           'tenDWMRIModalityKey', 'gageSclK1', 'miteValXi',
           'nrrdOriginStatusDirection', 'tend_normCmd', 'nrrdWrite',
           'pullInfoIsovalue', 'nrrdAlloc_va', 'limnSplineUpdate',
           'seekDescendToRidge', 'tenFiberParmStepSize',
           'limnObjectSpaceTransform', 'nrrdKeyValueErase',
           'limnPolyDataPrimitiveArea', 'nrrdResampleTypeOutSet',
           'unrrduHestScaleCB', 'nrrdArithBinaryOp',
           'meetHestPullVol', 'nrrdZlibStrategyDefault',
           'tenEMBimodalParmNix', 'tenAniso_Cp1', 'nrrdIterNix',
           'tenAniso_Cp2', 'nrrdUIInsert', 'gageErrNone',
           'tend_shrinkCmd', 'tend_expCmd', 'ell_3m_svd_d',
           'limnQN9octa', 'unrrdu_axinsertCmd', 'airBesselIn',
           'pullCCMeasure', 'nrrdEnvVarDefaultSpacing',
           'nrrdTypeUChar', 'airNoDio_ptr', 'pullPropStepConstr',
           'tenGageFALaplacian', 'tend_ellipseCmd', 'airTypeLast',
           'nrrdBasicInfoSpace', 'miteStageOpLast',
           'tenDwiGageConfidence', 'ell_cubic_root_last', 'gageProbe',
           'nrrdMeasureL2', 'nrrdMeasureL1', 'miteShadeMethodLitTen',
           'nrrdCenterUnknown', 'limnHestCameraOptAdd', 'airBesselI0',
           'airBesselI1', 'tenGradientParmNew', 'nrrdIterValue',
           'nrrdKind2DMaskedMatrix', 'pullPhistEnabled', 'airSgnPow',
           'nrrdBasicInfoOldMin', 'limnSpaceDevice',
           'gageVecHelicity', 'gageSclHessFrob', 'airThreadNew',
           'tenGageOmegaHessianEval2', 'echoListSplit3',
           'tenGageOmegaHessianEval0', 'tenDwiGageTensorAllDWIError',
           'nrrdSpacingStatusScalarNoSpace', 'tend_glyphCmd',
           'nrrdHistoEq', 'limnSpline_t', 'biffDone', 'tenGage',
           'nrrdBinaryOpExists', 'tenEigenvalueMultiply',
           'nrrdArithTernaryOp', 'tenFiberStop', 'airDStore',
           'seekDescendToDegCell', 'echoGlobalStateNix', 'limnLook',
           'limnLightSet', 'pullInfoSpecNix', 'tenModelParmDesc',
           'nrrdUnaryOpExp', 'limnObjectReadOFF',
           'pullInfoIsovalueGradient', 'tenGradientParm',
           'hooverContextNew', 'tenRotationTangents_d',
           'limnPolyDataVertexWindingFlip', 'nrrdTypeNumberOfValues',
           'nrrdBoundaryBleed', 'pullPointNew', 'pullValGageKind',
           'meetHestPullInfo', 'tenBMatrixCheck', 'echoJitterUnknown',
           'nrrdResampleRenormalizeSet', 'ell_aa_to_q_f',
           'ell_aa_to_q_d', 'miteDefRefStep',
           'limnSplineNrrdCleverFix', 'miteValGageKind',
           'nrrdEnvVarDefaultWriteEncodingType',
           'pullInfoHeightLaplacian', 'echoSuperquadSet',
           'nrrdSpaceOriginSet', 'nrrdResampleInfoNew', 'dyeLUVtoXYZ',
           'tenAnisoUnknown', 'nrrdKernelCentDiff', 'miteThread',
           'tenGageFAHessianEvec', 'limnSplineInfoLast',
           'nrrdMeasureLineSlope', 'tenDwiGageKindData',
           'tenFiberKernelSet', 'nrrdTypeIsUnsigned',
           'airNoDio_setfl', 'tend_anhistCmd', 'ell_3m_print_f',
           'ell_3m_print_d', 'seekTypeMaximalSurface',
           'gagePvlFlagNeedD', 'nrrdBasicInfoComments',
           'echoSuperquad', 'nrrdKind3DMaskedSymMatrix',
           'limnPolyDataTransform_f', 'tenGlyphTypeCylinder',
           'tenEstimate1TensorSimulateSingle_f',
           'tenEstimate1TensorSimulateSingle_d', 'nrrdHistoAxis',
           'tenDWMRINAVal', 'tenDwiGageTensorWLS',
           'nrrdAxisInfoCenter', 'limnPrimitiveNoop',
           'tenGageTensorGradMag', 'unrrdu_sliceCmd',
           'nrrdIoStateSkipData', 'echoJittableLast',
           'nrrdCenterNode', 'echoCylinder', 'airDLoad',
           'airUIrandMT_r', 'nrrdDStore', 'nrrdRangeNix',
           'tenGageFAKappa2', 'nrrdUnaryOpLast', 'tenGageFAKappa1',
           'unrrdu_imapCmd', 'tenDwiGageTensorLLSErrorLog',
           'limnPrimitive', 'nrrdKind2DSymMatrix', 'tenAniso_Ca2',
           'pullFinish', 'tenAniso_Ca1', 'tenGageModeHessian',
           'tenInvariantGradientsK_d', 'NrrdResampleInfo',
           'gageDefDefaultCenter', 'airTypeUInt',
           'tenInterpTypeLinear', 'unrrduCmd', 'limnVertex',
           'nrrdCheck', 'pullCCFind', 'limnPolyDataTransform_d',
           'gageParmRenormalize', 'nrrdKernelGaussianDD',
           'nrrdKindUnknown', 'ell_cubic_root',
           'tenEstimateMethodSet', 'nrrdUnaryOpErf', 'airNoDio_arch',
           'tenInterpPathLength', 'echoMatterGlassFuzzy',
           'meetPullVolNew', 'pullValUnknown', 'tenInterpParmNew',
           'airCbrt', 'airTypeEnum', 'nrrdApply1DRegMap',
           'tend_mconvCmd', 'tenGageCl2', 'tenGageCl1',
           'limnObjectPartTransform', 'tenFiberStopMinLength',
           'nrrdKernelBlackman', 'echoInstance',
           'nrrdMeasureHistoMax', 'limnPolyDataPlane',
           'limnSplineSample', 'pullPropIdCC', 'nrrdFormatText',
           'tend_evalmultCmd', 'nrrdResampleNrrdSet',
           'nrrdEnvVarDefaultWriteValsPerLine',
           'pullCondConstraintSatB', 'miteStageOpMax',
           'pullCondConstraintSatA', 'airFP_POS_NORM',
           'nrrdKernelHann', 'ell_q_pow_d', 'nrrdBinaryOpGTE',
           'ell_q_pow_f', 'unrrdu_mrmapCmd',
           'tenGageFAGradVecDotEvec0', 'nrrdKindPoint',
           'limnPolyDataPolygonNumber', 'tenBiffKey',
           'nrrdFormatType', 'seekSamplesSet', 'nrrdUnaryOpLog',
           'limnPolyDataWriteLMPD', 'nrrdUnaryOpNegative',
           'pullCondNew', 'nrrdFInsert', 'limnCameraPathTrackFrom',
           'meetGageKindParse', 'hestRespFileFlag', 'nrrdMeasureMode',
           'nrrdDefaultWriteValsPerLine', 'gageSclLaplacian',
           'tenGlyphParm', 'nrrdConvert', 'biffMaybeAddf',
           'tenGlyphTypeSuperquad', 'nrrdFormatTypeText', 'miteValYw',
           'airDioRead', 'pullEnergyTypeButterworth', 'hooverContext',
           'echoMatterLast', 'airNoDio_dioinfo', 'miteValYi',
           'tenGageOmegaDiffusionAlign', 'nrrdMeasureHistoSD',
           'tenEstimateContextNew', 'tenAnisoPlot', 'nrrdIoStateInit',
           'dyeHSVtoRGB', 'nrrdEnvVarStateMeasureHistoType',
           'pullVerboseSet', 'nrrdAxisInfoKind', 'airTime',
           'limnFace', 'limnPolyDataInfoNorm', 'tenGageQNormal',
           'limnPolyDataInfoTex2', 'nrrdHisto',
           'nrrdHasNonExistFalse', 'miteVariablePrint',
           'tenFiberParmUseIndexSpace', 'nrrdKind3Gradient',
           'gageAnswerPointer', 'nrrdCCFind', 'nrrdKeyValueClear',
           'nrrdCenter', 'airArrayNix', 'nrrdKernelC3Quintic',
           'pullProcessModeLast', 'gageShapeNix',
           'echoJittableNormalA', 'echoJittableNormalB',
           'nrrdResampleDefaultCenterSet', 'pullPropForce',
           'tenGageSNormal', 'miteShadeSpecNix', 'nrrdTypeShort',
           'ell_cubic_root_single', 'gageShapeReset',
           'tenGageBGradMag', 'limnCameraPathTrackUnknown',
           'ell_3m_1d_nullspace_d', 'nrrdGetenvInt',
           'nrrdKindIsDomain', 'ell_3mv_mul_d', 'dyeXYZtoLUV',
           'ell_3mv_mul_f', 'nrrdType', 'echoThreadStateInit',
           'gageStackBlurParmCheck', 'NrrdResampleAxis',
           'tenInterpTypeQuatGeoLoxR', 'hestElideSingleOtherType',
           'tenEMBimodalParm', 'tenInterpTypeQuatGeoLoxK',
           'echoAABBox', 'NrrdIter', 'gageParmVerbose',
           'airThreadMutexNew', 'tenGageTensorGrad',
           'limnHestSplineTypeSpec', 'pullPropGet',
           'limnSplineTypeUnknown', 'airMopDone', 'tend_unmfCmd',
           'tenMeasurementFrameReduce', 'nrrdTernaryOpLerp',
           'NrrdIoState', 'airErfc', 'airSrandMT_r', 'biffSetStr',
           'nrrdBinaryOpDivide', 'nrrdFprint', 'biffMsg',
           'NrrdResampleContext', 'nrrdMeasureVariance',
           'nrrdHistoThresholdOtsu', 'airEnumVal', 'tenExperSpecNew',
           'nrrdSpaceLeftPosteriorSuperiorTime', 'airTypeStr',
           'tenFiberStopOn', 'hooverErrSample',
           'nrrdZlibStrategyHuffman', 'gageStackBlurCheck',
           'limnEdge', 'nrrdSprint', 'hooverBiffKey',
           'gageOptimSigPlot', 'nrrdTypeMax',
           'pullInterTypeUnivariate', 'gageParmLast',
           'pullPropUnknown', 'airParseStrF', 'airParseStrD',
           'airParseStrE', 'airParseStrB', 'airParseStrC',
           'nrrdDistanceL2', 'nrrdEnvVarStateKeyValuePairsPropagate',
           'gageSclMeanCurv', 'airParseStrI', 'airParseStrS',
           'nrrdTernaryOpMin', 'unrrdu_convertCmd', 'tenGageEval1',
           'tenGageEval0', 'tenAniso_Cs1', 'tenGageAniso',
           'tenDwiGageTensor', 'echoRTRender', 'dyeXYZtoRGB',
           'gageKindTotalAnswerLength', 'airBinaryPrintUInt',
           'dyeSpaceLUV', 'nrrdKernelGaussianD', 'pullInfoTensor',
           'airAtod', 'nrrdField_number', 'tenGageModeGradMag',
           'tenAniso_Conf', 'gageAnswerLength', 'nrrdByteSkip',
           'nrrdBasicInfoData', 'hestColumns', 'gageKernelReset',
           'airThreadCapable', 'nrrdFormatTypeLast', 'dyeColorCopy',
           'nrrdEnvVarStateVerboseIO', 'gageDefVerbose',
           'echoThreadStateNix', 'tenGageDelNormR2', 'miteNtxfCheck',
           'tenAniso_Omega', 'gageStackBlurParmNew',
           'hooverStubThreadBegin', 'unrrdu_1opCmd',
           'nrrdField_data_file', 'tenGageFACurvDir1',
           'tenGageFACurvDir2', 'hooverErrThreadCreate',
           'tenGageCl1HessianEvec2', 'tenGageCl1HessianEvec1',
           'tenGageCl1HessianEvec0', 'pullEnergyUnknown',
           'limnQN12checker', 'nrrdDLookup', 'ell_q_3v_rotate_f',
           'nrrdDInsert', 'echoRTParmNix', 'tenModelFromAxisLearn',
           'nrrdIoStateCharsPerLine', 'ell_3m_eigenvalues_d',
           'hooverDefVolCentering', 'gageVecHessian',
           'gageParmStackNormalizeDeriv', 'limnObject',
           'tenEpiRegister4D', 'tenFiberStopFraction', 'airToLower',
           'nrrd1DIrregAclCheck', 'miteValZw', 'airThreadCond',
           'tenMake', 'unrrdu_makeCmd', 'miteValZi', 'ell_cubic',
           'tenGageClpmin2', 'airMyDio', 'nrrdUnaryOpCeil',
           'nrrdUnaryOpReciprocal', 'limnObjectPreSet', 'gageShape',
           'tenDefFiberIntg', 'limnCameraNix',
           'tenDwiFiberType2Evec0', 'pullStatusNewbie',
           'tenMakeSingle_f', 'tenGageCa1HessianEval1',
           'tenGageCa1HessianEval0', 'nrrdUILookup',
           'tenGageCa1HessianEval2', 'tenEstimate1MethodWLS',
           'nrrdAxisInfoPos', 'hooverErrNone', 'tend_fiberCmd',
           'ell_Nm_pseudo_inv', 'nrrdIoStateNix', 'gageVecVector1',
           'pullEnergyQuartic', 'gageVecVector0',
           'tenBVecNonLinearFit', 'hooverSample_t',
           'tenEstimateNegEvalShiftSet', 'nrrdCCAdjacency',
           'pullTask', 'airEnumDesc', 'unrrdu_gammaCmd',
           'nrrdWrap_va', 'nrrdStateKeyValueReturnInternalPointers',
           'unrrduHestBitsCB', 'nrrdEncodingBzip2', 'miteValGTdotV',
           'tenSqrtSingle_d', 'nrrdKindTime', 'tenDwiFiberType',
           'airArrayLenPreSet', 'nrrdMeasure', 'tenModelSqeFit',
           'airParseStrZ', 'airMopAdd', 'limnObjectEmpty',
           'airNormalRand_r', 'ell_3m_inv_d', 'tenGageEval2',
           'miteValNormal', 'tenEstimate2MethodUnknown',
           'unrrdu_lutCmd', 'tenDwiGageUnknown', 'miteValTw',
           'tenFiberIntg', 'miteValTi', 'nrrdFormatTypeEPS',
           'tend_tripleCmd', 'dyeColorParse', 'airIndexClamp',
           'nrrdEncodingTypeLast', 'nrrdTypeDefault', 'pullBin_t',
           'nrrdEncodingTypeAscii', 'tenAniso_FA', 'tenGageFAHessian',
           'gageStackBlurGet', 'limnPolyDataReadOFF', 'gageSclValue',
           'airEqvSettle', 'echoIsosurface', 'echoMatterPhongSp',
           'nrrdField_max', 'seekTypeRidgeSurfaceOP', 'nrrdFLookup',
           'meetPullInfo', 'unrrdu_diceCmd', 'seekStrengthSet',
           'limnPolyDataInfoLast', 'nrrdResampleSamplesSet',
           'gageVecHelGradient', 'miteShadeMethodNone',
           'gagePvlFlagUnknown', 'nrrdIterContent',
           'tenEstimateLinear3D', 'gageContextNix', 'nrrdRangeCopy',
           'pullPositionHistoryGet', 'miteStageOpAdd',
           'tenFiberStopLast', 'tenModelConvert', 'hooverStubSample',
           'gagePerVolumeDetach', 'airBool', 'nrrdBinaryOpSubtract',
           'limnSplineNix', 'tenEvecRGB', 'hestGlossary',
           'nrrdUILoad', 'pullHestEnergySpec', 'airNull',
           'nrrdField_keyvalue', 'unrrdu_2opCmd', 'airTypeOther',
           'nrrdIoStateFormatSet', 'pullPropIdtag', 'ell_3v_angle_d',
           'ell_3v_angle_f', 'airEnumUnknown', 'nrrdFormatVTK',
           'echoRayIntx', 'nrrdField_space_dimension',
           'limnObjectPartAdd', 'tenEvecRGBParmCheck',
           'limnSplineNrrdEvaluate', 'limnQN12octa', 'seekBiffKey',
           'nrrdDefaultResampleCheap', 'airOneLine',
           'nrrdKernelForwDiff', 'miteShadeSpecNew', 'hestCB',
           'tenEstimate2MethodLast', 'tenAniso_eval2',
           'tenAniso_eval0', 'tenAniso_eval1', 'ell_4m_det_f',
           'ell_4m_det_d', 'tenEstimate2MethodPeled',
           'gageVecNCurlNormGrad', 'nrrdDefaultSpacing', 'va_list',
           'pullVal', 'tenFiberTraceSet', 'nrrdApplyMulti1DLut',
           'nrrdMeasureHistoMode', 'pullCondLast',
           'pullVolumeSingleAdd', 'nrrdBinaryOpMultiply',
           'tenInterpTypeUnknown', 'pullStart', 'biffMaybeAdd',
           'hooverContextNix', 'nrrdSpatialResample', 'pullPointNix',
           'echoSplit', 'dyeColorGetAs', 'tenDefFiberStepSize',
           'echoObjectNix', 'pullEnergySpecNew', 'nrrdBiffKey',
           'limnPrimitiveQuads', 'limnPolyDataVertexNormals',
           'limnLightNix', 'unrrdu_untileCmd', 'gageVecCurl',
           'nrrdInset', 'dyeLABtoXYZ', 'nrrdBinaryOpMod',
           'gageOptimSigCalculate', 'limnSplineNumPoints',
           'hestOptAdd', 'tenGageDelNormR1', 'nrrdTypeMin',
           'airStrdup', 'echoRoughSphereNew', 'airThreadMutexLock',
           'nrrdInit', 'gageKernelStack', 'tenGradientIdealEdge',
           'nrrdIoStateEncodingGet', 'tenFiberMultiTrace',
           'limnCameraPathTrackLast', 'tenTripleCalcSingle_f',
           'miteThread_t', 'limnBiffKey', 'tenAniso_Mode',
           'echoJitterJitter', 'limnObjectVertexAdd',
           'nrrdBinaryOpLT', 'airStrtokQuoting',
           'pullInfoHeightGradient', 'ell_q_to_3m_f', 'gageSclMedian',
           'miteQueryAdd', 'airTypeSize', 'nrrdBinaryOpMax',
           'airStrlen', 'tenGageCp1HessianEval', 'echoMatterUnknown',
           'unrrdu_saveCmd', 'gageSclGradVec', 'tenLog',
           'tenGageCp1HessianEval2', 'tenGageCp1HessianEval0',
           'tenGageCp1HessianEval1', 'nrrdSpaceDimension',
           'miteDefNormalSide', 'unrrdu_axmergeCmd',
           'nrrdField_labels', 'pullInfoSpec', 'nrrdFStore',
           'biffAdd', 'tenGradientDistribute', 'nrrdKind',
           'nrrdValCompare', 'gagePvlFlagQuery', 'tenDWMRISkipKeyFmt',
           'nrrdIterSetValue', 'limnCameraPathMake',
           'nrrdHasNonExistUnknown', 'miteRenderEnd', 'miteRender',
           'tenGageEvec2', 'limnObjectWorldHomog',
           'tenGradientParmNix', 'tenGageEvalHessian',
           'nrrdBinaryOpEqual', 'limnQN10checker', 'miteUserNew',
           'tenFiberStopAnisoSet', 'meetHestGageKind',
           'tenAnisoTen_d', 'tenAnisoTen_f', 'echoMatterPhongSet',
           'echoMatterMetal', 'nrrdTernaryOpMultiply',
           'tenFiberMultiNix', 'nrrdBinaryOpAdd', 'size_t',
           'nrrdTernaryOpExists', 'limnObjectCubeAdd',
           'nrrdKindVector', 'echoRTParm', 'limnLightAmbientSet',
           'gageShapeSet', 'tenDefFiberUseIndexSpace',
           'nrrdBlind8BitRangeFalse', 'tenGageDetGradVec',
           'airFP_NEG_INF', 'hestOptFree', 'tenGageModeHessianEval',
           'echoJitterGrid', 'nrrdIoStateBzip2BlockSize',
           'hooverErrRenderBegin', 'airFPPartsToVal_d',
           'airFPPartsToVal_f', 'hestVerbosity',
           'nrrdDefaultWriteEncodingType', 'limnLight',
           'tenAnisoVolume', 'echoMatterGlassKa', 'ell_q_3v_rotate_d',
           'echoMatterGlassKd', 'NrrdRange', 'limnObjectDescribe',
           'nrrdBinaryOpRicianRand', 'nrrdIoStateSet',
           'tenFiberContextCopy', 'tend_helixCmd',
           'echoTypeIsosurface', 'tenEstimate1MethodUnknown',
           'echoMatterLightPower', 'nrrdSpaceRightAnteriorSuperior',
           'gageVecCurlNormGrad', 'seekEvalDiffThreshSet',
           'gageVecNormHelicity', 'nrrdContentSet_va',
           'limnLightSwitch', 'nrrdEncodingAscii',
           'hooverErrRenderEnd', 'echoRectangleSet',
           'pullEnergyCotan', 'tenDoubleContract_d', 'ell_q_avgN_d',
           'airStdout', 'airThreadCondNix', 'unrrdu_quantizeCmd',
           'airInsane_not', 'tenExp', 'miteValVdefT', 'tenModel_t',
           'pullInfoTangentMode', 'nrrdField_line_skip',
           'tenFiberParmSet', 'airTypeChar', 'airRandInt_r',
           'nrrdResampleKernelSet', 'tenGageRNormal',
           'limnPolyDataReadLMPD', 'nrrdDefaultWriteCharsPerLine',
           'nrrdMaybeAlloc_va', 'nrrdStateDisableContent',
           'nrrdKindQuaternion', 'nrrdNonSpatialAxesGet',
           'tenExperSpecGradSingleBValSet',
           'tenDwiGageTensorErrorLog', 'limnPolyDataInfo',
           'airFP_QNAN', 'nrrdProject', 'nrrdField_endian',
           'nrrdKindCovariantVector', 'gageStackBlurParmNix',
           'ell_4v_print_d', 'ell_4v_print_f', 'nrrdEncodingTypeGzip',
           'gageParm', 'tenGageOmegaHessian', 'unrrdu_lut2Cmd',
           'gageKindCheck', 'limnWindowNix',
           'nrrdEnvVarDefaultCenter', 'tenEvqVolume',
           'nrrdEncodingTypeRaw', 'nrrdSpaceDimensionSet',
           'nrrdFormatEPS', 'gageCtxFlagKernel', 'tend_evaladdCmd',
           'limnObjectDepthSortParts', 'nrrdKindSize',
           'limnPrimitiveTriangleStrip', 'airThreadMutexNix',
           'tenGradientCheck', 'tenSimulateSingle_f',
           'tenGageFATotalCurv', 'airRandInt', 'nrrdAlloc_nva',
           'miteRayEnd', 'airFloatNegInf', 'nrrdKernelSpecSprint',
           'limnVtoQN_f', 'limnVtoQN_d', 'miteRangeSP',
           'pullStatusLast', 'airThreadCondSignal', 'airEndianBig',
           'tenGageClpmin1', 'biffMsgStrSet', 'hestOptCheck',
           'hooverErr', 'hooverRayEnd_t', 'echoTriMeshSet',
           'nrrdIterSetOwnNrrd', 'limnEnvMapFill', 'dyeSpaceLAB',
           'nrrdIoStateNew', 'tenTripleType', 'unrrdu_insetCmd',
           'nrrdSample_nva', 'airTypeSize_t', 'tenGradientRandom',
           'tenAniso_VF', 'seekDataSet', 'unrrdu_histaxCmd',
           'tenInterpParm', 'tenInterpParmCopy', 'gageBiffKey',
           'gageSclCurvDir2', 'tenGageRotTans', 'gageSclCurvDir1',
           'nrrdSpaceLeftPosteriorSuperior', 'nrrdHasNonExistTrue',
           'gageProbeSpace', 'tenDwiGageKindNix', 'limnSplineInfo',
           'pullEnergyTypeLast', 'tenGageCa1', 'NrrdEncoding_t',
           'tenGageCa2', 'nrrdCCValid', 'ell_4m_pre_mul_f',
           'tenMakeSingle_d', 'dyeBiffKey', 'miteVal',
           'nrrdAxisInfoSpacingSet', 'tenGageDet',
           'unrrdu_unorientCmd', 'tenAniso_Ct1', 'gageVecNormalized',
           'tenAniso_Ct2', 'limnEdgeTypeBackCrease',
           'hooverDefImgCentering', 'hooverErrThreadJoin', 'airFree',
           'unrrdu_jhistoCmd', 'hestRespFileEnable', 'nrrdSpaceSet',
           'limnSplineEvaluate', 'hooverStubRenderBegin',
           'biffSetStrDone', 'pullInfoStrength', 'gageKernel10',
           'gageKernel11', 'tenFiberTypeTensorLine', 'airFPFprintf_f',
           'airFPFprintf_d', 'limnSpaceUnknown',
           'pullEnergyTypeCubicWell', 'gageQueryItemOn', 'nrrdILoad',
           'tenTensorCheck', 'pullPropPosition', 'gageVecVector',
           'tenExpSingle_d', 'airMopPrint', 'tenExpSingle_f', 'Nrrd',
           'tenInterpDistanceTwo_d', 'ell_4m_to_aa_f',
           'gageOptimSigParm', 'ell_4m_to_aa_d', 'tenPowSingle_f',
           'tenPowSingle_d', 'airFP_Last', 'limnPolyDataColorSet',
           'tenGageFAFlowlineCurv', 'echoRTRenderCheck',
           'nrrdAxisInfoMin', 'hestParseOrDie', 'echoJittableMotionA',
           'echoJittableMotionB', 'echoInstanceSet',
           'gageStructureTensor', 'nrrdOrientationReduce',
           'tenGageRGradMag', 'ell_aa_to_4m_f',
           'tenDwiGage2TensorQSegAndError', 'ell_aa_to_4m_d',
           'tenGageB', 'nrrdPad_nva', 'nrrdKeyValueCopy',
           'pullStatusStuck', 'tenGageS', 'tenGageR', 'tenGageQ',
           'tenGageTheta', 'nrrdSave', 'gageSclGaussCurv',
           'tenEMBimodal', 'limnObjectFaceAdd', 'nrrdUnaryOpFloor',
           'gageErrLast', 'tenAniso', 'nrrdBasicInfoUnknown',
           'tenEigenvaluePower', 'tenSizeNormalize',
           'gageParmStackNormalizeRecon', 'tenFiberStopDoubleSet',
           'nrrdIInsert', 'airInsane_pInfExists',
           'limnObjectWriteOFF', 'tenDwiFiberType1Evec0',
           'airLogBesselI0', 'gageDefGradMagCurvMin', 'airMop',
           'limnPolyDataSuperquadric', 'nrrdTernaryOpUnknown',
           'nrrdHistoJoint', 'airInsane_QNaNHiBit',
           'nrrdEnvVarStateMeasureType', 'nrrdKind4Color',
           'nrrdField_dimension', 'pullInfoHeight',
           'nrrdBoundaryLast', 'miteStageOpMultiply', 'tenGageKind',
           'gageVec', 'nrrdEnvVarDefaultWriteBareTextOld',
           'nrrdTypeLast', 'nrrdStateUnknownContent',
           'meetHestConstGageKind', 'gageQuerySet',
           'tenDwiFiberType12BlendEvec0', 'echoMatterGlassIndex',
           'gageStackBlurManage', 'tenSimulate', 'pullPropLast',
           'nrrdEnvVarStateDisableContent', 'gageVecDivGradient',
           'nrrdKind3DSymMatrix', 'nrrdBasicInfoSpaceDimension',
           'nrrdAxisInfoIdx', 'pullValSlice', 'gageStackBlur',
           'pullEnergyTypeButterworthParabola', 'nrrd1DIrregMapCheck',
           'echoRay', 'echoMatterGlassSet', 'unrrdu_cropCmd',
           'meetPullVolAddMulti', 'gageDefOrientationFromSpacing',
           'limnCameraAspectSet', 'tenInvariantGradientsR_d',
           'meetPullVolLoadMulti', 'nrrdRangeNew',
           'nrrdDefaultResampleType', 'ell_q_mul_d', 'ell_q_mul_f',
           'nrrdEncodingRaw', 'airInsane_NaNExists', 'pullEnergy',
           'limnHestPolyDataOFF', 'nrrdQuantize',
           'nrrdSpacingStatusDirection', 'tenEstimateSigmaSet',
           'nrrdKernelHermiteFlag', 'pullBinProcess',
           'pullProcessModeAdding', 'limnEdgeTypeLone',
           'unrrdu_heqCmd', 'airStrntok', 'nrrdTernaryOpInOpen',
           'nrrdPGM', 'gageDefK3Pack', 'echoTypeAABBox',
           'nrrdZlibStrategyUnknown', 'tenGageFADiffusionAlign',
           'airRician', 'echoMatterLightSet',
           'limnPrimitiveTriangles', 'nrrdTernaryOpClamp',
           'echoObjectHasMatter', 'ell_3m_to_aa_f', 'ell_3m_to_aa_d',
           'tend_epiregCmd', 'gageVecCurlGradient', 'airMopMem',
           'biffMsgStrGet', 'gageDefRenormalize',
           'limnObjectPolarSuperquadAdd', 'tenGageCp2', 'tenGageCp1',
           'limnObjectNew', 'nrrdBasicInfoLast', 'tenInterpParmNix',
           'tenAniso_Clpmin1', 'tenAniso_Clpmin2', 'echoCube',
           'echoMatterMetalR0', 'limnEdge_t',
           'limnEdgeTypeFrontFacet', 'tenFiberIntgUnknown',
           'limnObjectDepthSortFaces', 'tenGageOmegaNormal',
           'ell_6ms_eigensolve_d', 'gageTeeOfTau',
           'tenGageTraceDiffusionFraction', 'airThreadCondBroadcast',
           'nrrdBinaryOp', 'tenDwiGage2TensorPeledAndError',
           'pullInfo', 'nrrdMeasureMin', 'nrrdNuke',
           'nrrdTypeIsIntegral', 'airNoDio_disable',
           'limnSplineTypeHermite', 'tenEvecRGBSingle_f',
           'tenDwiGagePvlData', 'limnSpaceScreen',
           'airThreadBarrierWait', 'seekDescendToDeg',
           'nrrdClampConvert', 'tenExpand', 'biffMsgAddVL',
           'ell_3m_mul_f', 'tenTripleTypeWheelParm',
           'hooverRayBegin_t', 'tenSlice', 'airIndexULL', 'airEqvAdd',
           'miteValView', 'nrrdReshape_va', 'nrrdKindComplex',
           'limnQN13octa', 'ell_3m_inv_f', 'limnQN14octa',
           'tenGageCp1GradVec', 'echoTriangleSet', 'gageKernel00',
           'nrrdEmpty', 'limnSplineTypeTimeWarp', 'hestMinNumArgs',
           'nrrdMeasureLine', 'limnObjectVertexNormals',
           'tenTripleTypeEigenvalue', 'nrrdKind4Vector',
           'tenGageFARidgeSurfaceAlignment', 'tenAnisoEval_f',
           'tenAnisoEval_d', 'tenDWMRIBValueKey',
           'tenGageCa1HessianEvec0', 'tenGageCa1HessianEvec1',
           'tenGageCa1HessianEvec2', 'hooverErrInit', 'tend_avgCmd',
           'nrrdUnquantize', 'gageErr', 'pullEnergyType',
           'tenGageOmegaHessianEval', 'echoTypeTriangle', 'airFclose',
           'pullPropEnergy', 'nrrdEnvVarDefaultWriteBareText',
           'limnPolyDataSpiralSphere', 'tenGageModeHessianEvec2',
           'tenGageModeHessianEvec1', 'tenGageModeHessianEvec0',
           'airFP_POS_INF', 'echoBoundsGet',
           'limnObjectPolarSuperquadFancyAdd', 'pullBinsPointAdd',
           'gageShapeNew', 'pullInterTypeJustR', '_airThread',
           'nrrdKind3DMatrix', 'hestGreedySingleString',
           'pullProcessMode', 'meetAirEnumAll', 'tenFiberStopAniso',
           'tendUsage', 'gageShapeWtoI', 'pullBinsPointMaybeAdd',
           'nrrdHestKernelSpec', 'nrrdMeasureLinf', 'nrrdUnaryOpTan',
           'gagePerVolume', 'tenGageCl1GradVec', 'seekUpdate',
           'tenGradientJitter', 'nrrdAxisInfoCopy',
           'gageParmGradMagCurvMin', 'miteDefOpacNear1',
           'nrrdEncodingTypeUnknown', 'tenTripleCalc', 'ell_debug',
           'tenDwiGageTensorNLSLikelihood',
           'limnSplineTypeCubicBezier', 'nrrdUnaryOpRand',
           'airArrayLenIncr', 'tenEstimate1TensorSingle_d',
           'tenEstimate1TensorSingle_f', 'tenDwiGageAll',
           'ell_4mv_mul_f', 'ell_4mv_mul_d',
           'tenDefFiberAnisoStopType', 'nrrdSlice',
           'tenFiberMultiNew', 'nrrdKernelBlackmanDD',
           'airNoDio_test', 'tenGageOmegaGradVecDotEvec0',
           'gageStackBlurParmKernelSet', 'limnDefCameraRightHanded',
           'nrrdKernelAQuarticD', 'seekTypeRidgeSurface',
           'tenGlyphParmCheck', 'ell_4m_inv_d',
           'nrrdSpaceVecScaleAdd2', 'nrrdFClamp',
           'nrrdBlind8BitRangeLast', 'nrrdField_centers', 'airLog2',
           'airThreadBarrier', 'ell_q_inv_d', 'miteRangeGreen',
           'airFP_NEG_DENORM', 'nrrdAxisInfoGet_va',
           'pullInterTypeUnknown', 'limnEnvMapCB',
           'tenGageOmegaGradVec', 'airFP_NEG_NORM', 'airToUpper',
           'tenEigenvalueClamp', 'nrrdApply1DIrregMap',
           'airFloatSNaN', 'nrrdKernelC3QuinticDD', 'airIntPow',
           'nrrdStateGrayscaleImage3D', 'hooverRenderEnd_t',
           'tenFiberMulti', 'ell_Nm_tran', 'nrrdUnaryOpLog10',
           'tenFiberMultiPolyData', 'nrrdFormatTypePNG',
           'gageParmKernelIntegralNearZero', 'meetPullVolLeechable',
           'nrrdFormatTypePNM', 'airErf', 'limnPolyDataWriteIV',
           'miteValNdotV', 'ell_3m_det_f', 'ell_3m_det_d',
           'ell_q_4v_rotate_d', 'ell_q_4v_rotate_f',
           'gagePerVolumeIsAttached', 'pullEnergySpecParse',
           'airMopOkay', 'limnCamera', 'nrrdPPM', 'echoTypeList',
           'nrrdMeasureLast', 'nrrdUnaryOpLog1p', 'airInsane_endian',
           'tenEMBimodalParmNew', 'pullInfoInsideGradient',
           'pullStatusUnknown', 'tenFiberParmLast',
           'nrrdBasicInfoInit', 'nrrdIterSetNrrd', 'tenModelSimulate',
           'tenGageFAHessianEvec2', 'tenGageFAHessianEvec1',
           'tenGageFAHessianEvec0', 'ell_4m_post_mul_d',
           'echoIntxMaterialColor', 'pullEnergySpring',
           'gageSclHessEval', 'airThreadNoopWarning', 'limnDevicePS',
           'nrrdBasicInfoSpaceUnits', 'tend_mfitCmd',
           'gageStackProbe', 'nrrdBinaryOpAtan2', 'unrrdu_cmedianCmd',
           'tenAniso_Tr', 'biffMove', 'hooverRenderBegin_t',
           'nrrdHestNrrd', 'echoSceneNew',
           'nrrdEnvVarStateAlwaysSetContent', 'limnWindowNew',
           'miteRangeChar', 'tenAniso_Th', 'ell_3m_rotate_between_d',
           'gageTauOfTee', 'nrrdTypeBlock', 'nrrdSpatialAxesGet',
           'seekItemGradientSet', 'tenGageTensor',
           'nrrdSpaceScannerXYZ', 'airBesselInExpScaled',
           'nrrdEncodingGzip', 'airTeemVersion', 'airThreadMutex',
           'unrrduUsage', 'nrrdDistanceL2Signed',
           'tenGageTensorQuatGeoLoxR', 'tenGageInvarKGrads',
           'tenSizeScale', 'tenGageTensorQuatGeoLoxK',
           'gageStackPerVolumeAttach', 'biffGetStrlen',
           'gageDefCheckIntegrals', 'nrrdBinaryOpCompare',
           'airNoDio_okay', 'airTypeUnknown', 'tenAnisoHistogram',
           'tenGageFA', 'nrrdSpaceVecNorm', 'biffGet',
           'tenDwiGageJustDWI', 'tenDwiGageTensorLLSError',
           'nrrdAxisInfoLabel', 'tenDwiGage', 'nrrdBinaryOpFmod',
           'nrrdKind3Vector', 'airStrtrans', '__darwin_va_list',
           'meetPullVolParse', 'nrrdTypeChar', 'nrrdCCRevalue',
           'ell_3v_barycentric_spherical_d', 'echoObject',
           'nrrdSpace3DLeftHandedTime', 'ell_biff_key',
           'gageSclNormal', 'unrrdu_axdeleteCmd', 'nrrdBasicInfoCopy',
           'tenModelTensor2', 'dyeConverter', 'nrrdCrop',
           'nrrdUnaryOpAbs', 'tenGageCovariance', 'echoCylinderSet',
           'nrrdElementNumber', 'tenEvecRGBParm', 'hestMultiFlagSep',
           'limnObjectPSDraw', 'nrrdJoin', 'airInsane_FISize',
           'hestParm', 'nrrdKeyValueIndex', 'tenDefFiberKernel',
           'seekType', 'nrrdSpacingStatusUnknown',
           'limnSplineTypeSpec', 'airEnumStr', 'airArrayNuke',
           'airDioTest', 'pullEnergyTypeQuartic', 'miteValVrefN',
           'tenFiberTrace', 'limnEdgeTypeFrontCrease', 'miteSample',
           'nrrdBinaryOpNormalRandScaleAdd', 'tenFiberIntgMidpoint',
           'meetPullInfoAddMulti', 'limnWindow', 'tend_bfitCmd',
           'nrrdField_old_min', 'ell_q_avg4_d', 'nrrdTypeFloat',
           'limnSplineNew', 'tenGageFARidgeSurfaceStrength',
           'airEqvMap', 'seekTypeLast', 'gageKind_t',
           'gageDefKernelIntegralNearZero', 'nrrdEncodingTypeHex',
           'tenGageEvec0', 'tenGageEvec1', 'nrrdIoStateValsPerLine',
           'unrrdu_tileCmd', 'pullInterTypeSeparable',
           'nrrdKind2DMaskedSymMatrix', 'airMopOnOkay',
           'limnPolyDataPrimitiveSort', 'tenGageTraceHessian',
           'nrrdField_space_directions', 'gageContext',
           'nrrdHasNonExist', 'tenTripleConvertSingle_f',
           'tenGageOmegaGradMag', 'airThreadMutexUnlock',
           'tenFiberTypeUnknown', 'nrrdMeasureHistoProduct',
           'tenTripleConvertSingle_d', 'nrrdUnaryOpIf',
           'tenGageUnknown', 'nrrdBasicInfoBlocksize',
           'unrrdu_distCmd', 'miteValUnknown', 'nrrdFormatNRRD',
           'pullEnergyTypeUnknown', 'tenGageNormNormal',
           'nrrdKindRGBColor', 'gageDefStackNormalizeRecon',
           'airSingleSscanf', 'airThreadCondWait',
           'pullProcessModeDescent', 'limnSplineTypeSpecNix',
           'gageOptimSigParmNew', 'gageVecMGEval', 'tenFiberIntgSet',
           'pullBin', 'nrrdField_unknown', 'nrrdCCNum',
           'tenGageOmega', 'tend_logCmd', 'limnObjectNix',
           'tenGageDetHessian', 'tenGageBGradVec', 'limnQN15octa',
           'pullEnergyCubic', 'limnSplineMaxT', 'seekStrengthUseSet',
           'gageErrStackIntegral', 'nrrdRangeNewSet',
           'miteShadeSpecQueryAdd', 'nrrdEnvVarStateKindNoop',
           'tenGageInvarRGradMags', 'nrrdSpaceVecSetNaN',
           'limnCameraInit', 'meetPullInfoParse', 'tend_sliceCmd',
           'tenFiberTypePureLine', 'nrrdKindScalar',
           'tenFiberStopNumSteps', 'airTypeInt',
           'tenFiberAnisoSpeedSet', 'pullValSliceGradVec',
           'tenGlyphTypeSphere', 'gagePointReset', 'hestOpt',
           'nrrdMeasureLineError', 'nrrdKernelBCCubicDD',
           'limnOptsPS', 'nrrdSpacingStatusLast',
           'tenFiberStopBounds', 'limnSplineInfo2Vector',
           'pullProcessModeNeighLearn', 'nrrdKernelSpecNew',
           'nrrdRangeSet', 'limnEdgeTypeContour', 'ell_4m_inv_f',
           'nrrdField_space_origin', 'dyeXYZtoLAB',
           'nrrdFormatTypeNRRD', 'echoLightPosition', 'tenAnisoScale',
           'echoScene_t', 'nrrdResampleClampSet', 'gageVecGradient2',
           'gageVecGradient0', 'gageVecGradient1', 'limnDeviceGL',
           'tenInterpTypeLast', 'tenGageFAHessianEval2',
           'tenGageFAHessianEval0', 'tenGageFAHessianEval1',
           'nrrdSpaceLast', 'tend_bmatCmd', 'nrrdAxisInfoThickness',
           'airEndian', 'nrrdIoStateBareText', 'ell_Nm_inv',
           'hestElideSingleEnumType', 'ell_q_log_d', 'ell_q_log_f',
           'airInsaneErr', 'tenGageCovarianceRGRT', 'airFLoad',
           'gageKernel21', 'gageKernel20', 'gageKernel22',
           'echoMatterPhong', 'tenGageFARidgeLineAlignment',
           'tenGageSGradMag', 'tenGageEvalGrads', 'limnLightNew',
           'miteRangeEmissivity', 'pullPropStepEnergy',
           'nrrdAxisInfoPosRange', 'gageItemEntry', 'gageSclNPerp',
           'echoJitter', 'nrrdReshape_nva', 'pullBiffKey',
           'tenGlyphTypeUnknown', 'airMyFmt_size_t',
           'ell_cubic_root_unknown', 'tenGageModeHessianEvec',
           'nrrdMeasureHistoMin', 'nrrdUnaryOpRoundUp',
           'tenFiberTypeLast', 'nrrdILookup',
           'gageParmCheckIntegrals', 'tend_evqCmd', 'nrrdUnaryOpCos',
           'tenDWMRIKeyValueFromExperSpecSet',
           'tenDwiGageTensorNLSError', 'ell_6m_mul_d', 'dyeRGBtoHSL',
           'limnPolyDataPrimitiveVertexNumber', 'nrrdField_content',
           'gageKernelUnknown', 'nrrdKindSpace', 'limnSplineTypeBC',
           'gageDefStackUse', 'nrrdEncodingArray', 'hestUsage',
           'nrrdHasNonExistLast', 'dyeRGBtoHSV', 'limnEnvMapCheck',
           'airEnumValCheck', 'echoEnvmapLookup', 'airThread',
           'tenGageMode', 'tenGageCp1HessianEvec', 'gageUpdate',
           'nrrdSpacingCalculate', 'miteDefRenorm',
           'pullEnergyTypeGauss', 'seekTypeSet', 'limnPolyData',
           'airThreadCondNew', 'nrrdStateGetenv', 'tenInterpType',
           'tenGageCp1Normal', 'airEndianUnknown', 'pullVolume',
           'echoIntxColor', 'nrrdMeasureMean', 'miteValWdotD',
           'tenGlyphGen', 'miteShadeSpecParse', 'airTypeBool',
           'tenDwiGage2TensorQSeg', 'biffMoveVL',
           'nrrdStateKeyValuePairsPropagate', 'gageShapeEqual',
           'echoJittableLens', 'pullGammaLearn',
           'limnObjectPolarSphereAdd', 'nrrdStateMeasureHistoType',
           'echoJitterNone', 'limnSplineInfoQuaternion',
           'tenGageTraceGradMag', 'NrrdKernel', 'nrrdField_encoding',
           'nrrdKernelGaussian', 'biffMovef', 'echoScene',
           'nrrdKindRGBAColor', 'nrrdIoStateZlibStrategy',
           'nrrdField_thicknesses', 'echoTypeLast',
           'gageStackPerVolumeNew', 'gageKernelSet', 'nrrdKindList',
           'limnCamera_t', 'echoSphereSet', 'echoGlobalState',
           'pullEnergyGauss', 'tenModel', 'gageItemSpecNix',
           'echoTypeSplit', 'nrrdArithIterTernaryOp', 'tenDwiGageFA',
           'tenEstimateGradientsSet', 'nrrdUnaryOpZero',
           'airFloatPosInf', 'nrrdCommentAdd', 'limnSplineType',
           'nrrdCCSize', 'echoBiffKey', 'tenGageOmegaHessianEvec2',
           'tenGageOmegaHessianEvec0', 'tenGageOmegaHessianEvec1',
           'nrrdBoundary', 'nrrdAxesPermute', 'tenFiberParmWPunct',
           'limnCameraPathTrackBoth', 'echoMatterMetalSet',
           'limnSplineTypeLinear', 'gageParmCurvNormalSide',
           'gageSclHessMode', 'tenGageFAGradMag', 'pullPointNumber',
           'pullContextNix', 'limnSplineInfoNormal',
           'tenGageFAValleySurfaceStrength', 'airFopen',
           'airEnumLast', 'tenFiberAnisoSpeedReset',
           'nrrdKernelParse', 'tenAniso_Skew', 'nrrdMeasureHistoMean',
           'limnQN16border1', 'nrrdCenterLast', 'nrrdAxisInfoUnits',
           'miteRangeKa', 'tenFiberSingleDone', 'miteRangeKd',
           'nrrdMeasureMedian', 'nrrdMinMaxExactFind',
           'pullEnergyTypeCotan', 'ell_q_div_f', 'ell_q_div_d',
           'nrrdPad_va', 'nrrdAxisInfoSize', 'miteRangeKs',
           'nrrdBinaryOpNotEqual', 'nrrdStateAlwaysSetContent',
           'tenGageFAGaussCurv', 'NrrdFormat', 'nrrdKernelCheap',
           'limnSplineTypeLast', 'airInsane_dio', 'pullPoint_t',
           'nrrdTernaryOpMax', 'tenInv_d', 'tenGageLast',
           'gageScl3PFilter4', 'tenGlyphParmNew', 'dyeColorNix',
           'tenFiberTypeZhukov', 'tenGageOmegaHessianEval1',
           'tenDwiGageKindNew', 'gageQueryPrint',
           'tenEstimateVerboseSet', 'airSgn', 'meetAirEnumAllPrint',
           'gageCtxFlagUnknown', 'echoJittableUnknown',
           'pullPropStuck', 'nrrdKernelBCCubicD',
           'tenGageCl1HessianEvec', 'nrrdBasicInfoDimension',
           'nrrdKernelHannD', 'limnPolyDataCCFind',
           'airTeemReleaseDate', 'limnObjectFaceNormals',
           'nrrdKernelTMF_maxC', 'nrrdIoStateDetachedHeader',
           'tend_expandCmd', 'tenEstimate2MethodQSegLLS',
           'unrrduHestMaybeTypeCB', 'airOneLinify',
           'tenEvecRGBSingle_d', 'dyeColorInit',
           'pullProcessModeUnknown', 'tenDwiGageKindCheck',
           'airMopError', 'echoTriangle', 'limnPolyDataRasterize',
           'hestElideMultipleEmptyStringDefault', 'miteValRi',
           'echoPos_t', 'hestVarParamStopFlag', 'seekTypeValleyLine',
           'nrrdUIStore', 'miteRangeBlue', 'limnPolyDataInfoRGBA',
           'miteValRw', 'tenGradientGenerate', 'tend_stenCmd',
           'limnEdgeTypeUnknown', 'nrrdBasicInfoOldMax',
           'unrrdu_shuffleCmd', 'echoJittableLight',
           'tenInterpMulti3D', 'seekIsovalueSet', 'airMopDebug',
           'nrrdEncodingType', 'tend_aboutCmd', 'pullCondUnknown',
           'limnQN11octa', 'ell_q_exp_f', 'tenDwiGageTensorMLE',
           'airILoad', 'ell_q_to_aa_d', 'gagePoint_t',
           'ell_q_to_aa_f', 'unrrdu_padCmd', 'airMopOnError',
           'nrrdDefaultKernelParm0', 'tend_pointCmd',
           'echoTypeUnknown', 'airFPClass_f', 'airFPClass_d',
           'tenDwiGageMeanDWIValue', 'nrrdGetenvEnum',
           'nrrdUnaryOpSqrt', 'airFP_POS_ZERO', 'nrrdIoStateGet',
           'tenGageHessian', 'nrrdUnaryOpOne', 'gageCtxFlagRadius',
           'tenGageSGradVec', 'tenInterpTypeLogLinear',
           'ell_3m_mul_d', 'airMopNever', 'miteShadeSpec',
           'gageKindAnswerOffset', 'nrrdNew', 'nrrdEncodingTypeBzip2',
           'nrrdField_sample_units', 'hooverErrRayEnd',
           'tend_anplotCmd', 'pullInfoTensorInverse', 'nrrdAxesSplit',
           'nrrdSameSize', 'tenEstimateContext', 'unrrdu_swapCmd',
           'seekItemEigensystemSet', 'airDioMalloc',
           'limnPrimitiveLineStrip', 'airThreadJoin',
           'pullEnergyTypeCubic', 'tenDefFiberWPunct',
           'nrrdTypeUShort', 'hooverContextCheck', 'tenFiberStopSet',
           'echoMatterPhongKs', 'airMopUnMem', 'nrrdUnaryOpAsin',
           'biffMsgAdd', 'tenGageInvarRGrads', 'echoMatterPhongKd',
           'nrrdUnblock', 'echoMatterPhongKa', 'tenFiberStopRadius',
           'limnPrimitiveUnknown', 'pullPropScale', 'nrrdIterNew',
           'airNoDio_std', 'airFloat', 'tenFiberStopConfidence',
           'nrrdKind3Normal', 'tenGageModeHessianEval2',
           'tenGageModeHessianEval0', 'tenGageModeHessianEval1',
           'gageKernelLast', 'gageSclTotalCurv', 'nrrdMeasureSum',
           'airGaussian', 'nrrdResampleBoundarySet',
           'nrrdBasicInfoType', 'tenEigenvalueAdd',
           'tenModelBall1Stick', 'hooverErrLast',
           'nrrdKernelC4HexicDD', 'nrrdTile2D', 'airIndex',
           'tenFiberStopStub', 'limnCameraUpdate', 'dyeSpaceLast',
           'airDrandMT_r', 'airMode3_d', 'miteRangeAlpha',
           'limnPolyDataSave', 'nrrdStateVerboseIO',
           'nrrdDefaultResamplePadValue', 'nrrd1DIrregAclGenerate',
           'tenGageModeNormal', 'tenTripleCalcSingle_d',
           'ell_3v_perp_d', 'biffMsgMoveVL', 'ell_3v_perp_f',
           'nrrdKind2DMatrix', 'airRandMTStateGlobal', 'airLLong',
           'tenGageConfidence', 'seekItemScalarSet',
           'hooverStubThreadEnd', 'tenGageEvec',
           'tenDwiGageTensorLLS', 'limnSplineParse',
           'seekItemNormalSet', 'limnSpaceLast', 'miteRenderBegin',
           'airMyEndian', 'miteRangeRed', 'gageVecVector2',
           'echoObjectAdd', 'nrrdBinaryOpMin', 'tenGageThetaNormal',
           'seekItemStrengthSet', 'nrrdSimplePad_nva',
           'pullEnergySpecNix', 'tenDwiGageTensorWLSErrorLog',
           'airThreadStart', 'tenFiberSingle', 'ell_3m_to_q_d',
           'ell_3m_to_q_f', 'airFP_POS_DENORM', 'nrrdAxesInsert',
           'airThreadBarrierNew', 'gageSclHessian', 'tenFiberUpdate',
           'limnSpline', 'meetPullInfoNix', 'limnDeviceLast',
           'nrrdSample_va', 'nrrdCCMerge', 'seekContext',
           'nrrdAxisInfoSpaceDirection', 'tenAniso_Det',
           'tenGageFA2ndDD', 'tenDWMRIBmatKeyFmt',
           'tenGageCl1GradMag', 'tenFiberContextNew',
           '__darwin_size_t', 'gageParmK3Pack', 'tenGageBNormal',
           'tenEpiRegister3D', 'echoSphere', 'gageOptimSigParmNix',
           'airEnumFmtDesc', 'tenExperSpecFromKeyValueSet',
           'echoJittablePixel', 'ell_3m_eigensolve_d',
           'echoIntxLightColor', 'nrrdIoStateKeepNrrdDataFileOpen',
           'airIStore', 'seekExtract', 'nrrdField_last',
           'nrrdHestIter', 'gageVecImaginaryPart',
           'nrrdBasicInfoContent', 'tenEstimateValueMinSet',
           'dyeSpaceRGB', 'tenGlyphTypeLast', 'airRandMTStateNew',
           'nrrdEnvVarDefaultCenterOld', 'tenExperSpecMaxBGet',
           'tenEstimateUpdate', 'pullInfoIsovalueHessian',
           'nrrdLineSkip', 'nrrdField_kinds', 'gageVecJacobian',
           'nrrdBlind8BitRangeState', 'tenEstimate1Method',
           'airFP_NEG_ZERO', 'echoMatter', 'tenFiberParmUnknown',
           'airStderr', 'unrrdu_aboutCmd', 'unrrduDefNumColumns',
           'airEndianLittle', 'airArrayStructCB', 'limnSpaceWorld',
           'seekTypeUnknown', 'nrrdCommentClear',
           'limnPolyDataSpiralTubeWrap', 'tenGageRGradVec',
           'nrrdHistoCheck', 'tenDwiFiberTypeUnknown',
           'tenGageOmegaHessianEvec', 'seekContextNix',
           'nrrdTernaryOpAdd', 'ell_cubic_root_triple',
           'nrrdKernelSpecNix', 'nrrdDefaultResampleBoundary',
           'nrrdAxesDelete', 'airIsNaN', 'nrrdKernelSpecSet',
           'limnPolyDataInfoUnknown', 'tenGageThetaGradVec',
           'gageItemSpecInit', 'tend_makeCmd', 'gagePoint',
           'seekVerboseSet', 'airMopper', 'nrrdApply1DLut',
           'tenGradientMeasure', 'nrrdKindStub', 'nrrdTypeSize',
           'tenDwiGageTensorLLSLikelihood', 'nrrdBlock',
           'airTypeString', 'nrrdKernelC4HexicD', 'miteStageOpMin',
           'nrrdFormatTypeUnknown', 'dyeColorSprintf',
           'meetConstGageKindParse', 'nrrdSpacingStatusNone',
           'gageShape_t', 'limnPolyDataCube',
           'limnDefCameraAtRelative', 'ell_cubic_root_three',
           'biffMsgNix', 'tenGageCa1GradMag', 'limnPolyDataNew',
           'nrrdDefaultResampleRound', 'limnSplineTypeSpec_t',
           'gageSclHessEvec0', 'gageSclHessEvec1', 'gageSclHessEvec2',
           'hestElideSingleNonExistFloatDefault',
           'tenFiberStopLength', 'ell_4m_print_f',
           'limnSplineInfo3Vector', 'ell_4m_print_d',
           'tenDefFiberMaxNumSteps', 'limnLightDiffuseCB',
           'nrrdMeasureHistoSum', 'echoRTParmNew',
           'nrrdField_axis_mins', 'pullEnergyCubicWell',
           'biffMsgStrAlloc', 'tenDwiGageADC', 'nrrdOriginCalculate',
           'tenInterpN_d', 'tenFiberContextNix', 'seekTypeRidgeLine',
           'nrrdGetenvBool', 'gageVecLength', 'limnSpace',
           'nrrdIoStateFormatGet', 'gageStackWtoI',
           'tenFiberDirectionNumber', 'pullContext_t',
           'nrrdTypePrintfStr', 'nrrdField_sizes',
           'nrrdAxisInfoGet_nva', 'nrrdSimplePad_va',
           'tenDwiGageTensorLikelihood', 'tenGageThetaGradMag',
           'meetPullVolNuke', 'nrrdKindLast', 'gageParm_t',
           'nrrdIoStateLast', 'tenEvecRGBParmNew', 'limnFace_t',
           'nrrdField_axis_maxs', 'tenGageRotTanMags',
           'tenGageBHessian', 'limnObjectPSDrawConcave',
           'limnObjectFaceNumPreSet', 'tenGageCovarianceKGRT',
           'biffMsgStrlen', 'tenGageOmega2ndDD',
           'nrrdDefaultResampleClamp',
           'pullEnergyButterworthParabola',
           'hestElideSingleEmptyStringDefault', 'tenExperSpec',
           'tenDwiGage2TensorQSegError',
           'hestElideMultipleNonExistFloatDefault', 'gageScl2ndDD',
           'unrrdu_unquantizeCmd', 'airTypeDouble', 'tend_evalpowCmd',
           'pullInfoUnknown', 'pullEnergyAll', 'unrrdu_permuteCmd',
           'limnDeviceUnknown', 'nrrdUnaryOpAtan',
           'tenGageDelNormPhi3', 'tenGageDelNormPhi2',
           'tenGageDelNormPhi1', 'airRandMTState', 'NrrdKernelSpec',
           'pullCondEnergyBad', 'tenGageTensorGradRotE',
           'nrrdInvertPerm', 'nrrdTernaryOp', 'nrrdUnaryOpSgn',
           'ell_3v_area_spherical_d', 'gageCtxFlagK3Pack',
           'dyeColorSet', 'nrrdMeasureHistoL2', 'limnPolyDataAlloc',
           'tenGradientBalance', 'gageSclHessEvec', 'meetBiffKey',
           'nrrdHistoDraw', 'nrrdGetenvDouble', 'nrrdLoadMulti',
           'echoMatterTextureSet', 'limnQNtoV_f', 'limnQNtoV_d',
           'nrrdApply1DSubstitution', 'echoSceneNix',
           'nrrdZlibStrategyFiltered', 'airMy32Bit',
           'nrrdSpaceVecCopy', 'unrrdu_ccfindCmd', 'biffAddVL',
           'nrrdKindHSVColor', 'nrrdFormatTypeVTK', 'pullCondOld',
           'nrrdStateBlind8BitRange', 'pullInfoSpecAdd',
           'tend_evecrgbCmd', 'echoJitterRandom', 'tenGageDetGradMag',
           'echoIntx', 'gageCtxFlagShape', 'gageVecDivergence',
           'limnCameraPathTrackAt', 'limnPolyDataCopyN',
           'nrrdKernelAQuartic', 'nrrdSpace3DRightHanded',
           'nrrdUnaryOpSin', 'echoType', 'ell_Nm_mul',
           'nrrdKindXYZColor', 'echoTypeCylinder',
           'nrrdAxisInfoMinMaxSet', 'gageVolumeCheck',
           'gageVecDirHelDeriv', 'nrrdStringWrite', 'tenAniso_B',
           'limnLightUpdate', 'nrrdOriginStatusOkay', 'tenAniso_Q',
           'tenAniso_R', 'tenAniso_S', 'limnQN16simple',
           'dyeColorGet', 'limnPart', 'tenGageFAMeanCurv',
           'ell_4m_mul_f', 'ell_4m_mul_d', 'tenDwiGageTensorWLSError',
           'miteRangeLast', 'tenTripleTypeLast', 'nrrdSwapEndian',
           'gageErrBoundsStack', 'tenEstimate1TensorSimulateVolume',
           'unrrdu_resampleCmd', 'limnPolyDataInfoBitFlag',
           'hooverStubRayBegin', 'nrrdRangeAxesGet',
           'tenGageConfDiffusionFraction', 'gageVecMGFrob',
           'pullVolumeStackAdd', 'tenGageCa1Normal', 'airDoneStr',
           'nrrdResamplePadValueSet', 'nrrdResampleExecute',
           'airParseStrUI', 'echoMatterMetalFuzzy',
           'tenGageCp1Hessian', 'gageKindVec', 'echoMatterMetalKd',
           'tenGageCp1GradMag', 'echoMatterMetalKa', 'gageKindScl',
           'hestCleverPluralizeOtherY', 'nrrdCheapMedian',
           'nrrdKernelDiscreteGaussian', 'limnSplineInfoUnknown',
           'tenEstimateLinearSingle_d', 'tenEstimateLinearSingle_f',
           'nrrdSaveMulti', 'tenLogSingle_d', 'tend_satinCmd',
           'nrrdIoStateZlibLevel', 'gageQueryAdd', 'nrrdKeyValueSize',
           'tenFiberSingleNix', 'gageVecLast', 'limnQNUnknown',
           'airIsInf_f', 'tend_anscaleCmd', 'airIsInf_d',
           'tenGageTensorGradMagMag', 'tenDWMRIGradKeyFmt',
           'tenGageCl1Hessian', 'seekTypeIsocontour', 'airFP_Unknown',
           'nrrdBasicInfoKeyValuePairs', 'tenTripleTypeRThetaPhi',
           'gageParmUnknown', 'miteShadeMethodUnknown',
           'nrrdBinaryOpPow', 'biffMsgNoop', 'nrrdTernaryOpLast',
           'tenGageTensorLogEuclidean', 'pullVolumeLookup',
           'nrrdKeyValueAdd', 'airParseStr', 'tenInterpTypeGeoLoxR',
           'nrrdKernelZero', 'pullRun', 'tenGageFAGradVec',
           'tenInterpTypeGeoLoxK', 'nrrdDefaultCenter',
           'limnPolyDataWriteVTK', 'ell_3m_post_mul_f',
           'ell_3m_post_mul_d', 'nrrdKernelBCCubic', 'limnPart_t',
           'gageScl', 'tenDWMRIKeyValueParse', 'airIndexClampULL',
           'tenExperSpecKnownB0Get', 'nrrdFLoad',
           'echoGlobalStateNew', 'tenEstimateThresholdSet',
           'unrrdu_joinCmd', 'biffMsgLineLenMax',
           'limnSplineTypeSpecNew', 'gageVecMGEvec', 'echoRectangle',
           'unrrdu_ccadjCmd', 'ell_q_to_4m_f',
           'nrrdStateDisallowIntegerNonExist', 'ell_q_to_4m_d',
           'biffMsgClear', 'ell_3m_2d_nullspace_d',
           'nrrdBinaryOpLast', 'tend_evalCmd', 'seekItemHessSet',
           'dyeSpaceUnknown', 'airRandMTStateGlobalInit',
           'nrrdDomainAxesGet', 'echoMatterLightUnit',
           'pullInterTypeAdditive', 'tenGageDetNormal', 'airDrandMT',
           'nrrdUnaryOpLog2', 'pullEnergyTypeSpring', 'airMode3',
           'limnObjectConeAdd', 'NrrdAxisInfo', 'tenSqrtSingle_f',
           'tenGageSHessian', 'nrrdField_min', 'dyeStrToSpace',
           'nrrdShuffle', 'nrrdLoad', 'mite_t', 'airSetNull',
           'airDioInfo', 'airArray', 'nrrdResampleContextNew',
           'echoListAdd', 'tenDefFiberAnisoThresh',
           'tenGageFAHessianEval', 'tenGlyphParmNix',
           'nrrdField_spacings', 'nrrdKernelSpecCopy',
           'gageParmStackUse', 'airNoDio_fpos',
           'nrrdOriginStatusNoMaxOrSpacing', 'gageSclHessRidgeness',
           'nrrdDClamp', 'unrrdu_reshapeCmd', 'pullInfoHessian',
           'limnPolyDataPrimitiveTypes', 'gageOptimSigSet',
           'gageStackItoW', 'tenTripleTypeMoment', 'nrrdIStore',
           'tenModelParse', 'miteRangeUnknown',
           'tenGageInvarKGradMags', 'nrrdTypeInt',
           'seekTypeValleySurfaceT', 'nrrdFormatArray', 'nrrdCCMax',
           'airBesselI0ExpScaled', 'tenTripleTypeK', 'tenTripleTypeJ',
           'tenTripleTypeR', 'tenVerbose', 'nrrdIoStateUnknown',
           'nrrdKernelSpecParse', 'gageDefCurvNormalSide',
           'tenGageCa1GradVec', 'tenGageDelNormK3',
           'tenGageDelNormK2', 'biffMsgMovef', 'limnHestSpline',
           'nrrdEnvVarStateBlind8BitRange', 'echoLightColor',
           'tenFiberSingleNew', 'dyeRGBtoXYZ', 'unrrdu_mlutCmd',
           'hestParmFree', 'tenFiberIntgEuler', 'echoTextureLookup',
           'pullOutputGet', 'nrrdGetenvUInt', 'pullVolumeNix',
           'pullInfoSpec_t', 'nrrdTernaryOpInClosed',
           'tenGageFAHessianEvalMode', 'echoTypeSphere',
           'gagePerVolumeAttach', 'nrrdField_units', 'ell_aa_to_3m_f',
           'ell_aa_to_3m_d', 'seekContour3DTopoHackTriangle',
           'pullCondConstraintFail', 'tenGageTraceGradVec',
           'tenAniso_Cs2', 'nrrdSimpleCrop', 'tenEstimate1MethodNLS',
           'gageKind', 'pullContextNew',
           'tenDwiGageTensorNLSErrorLog', 'echoList',
           'nrrdOriginStatusLast', 'gageQueryReset',
           'unrrdu_axsplitCmd', 'pullInfoSeedPreThresh',
           'seekNormalsFindSet', 'pullEnergySpec',
           'tenFiberSingleInit', 'unrrdu_projectCmd', 'biffCheck',
           'tend_simCmd', 'pullInfoSeedThresh',
           'tenDwiGageTensorMLEErrorLog', 'hooverRender',
           'biffMsgMove', 'tenDwiGage2TensorPeledError',
           'tenEstimate1MethodLLS',
           'nrrdEnvVarDefaultWriteCharsPerLine', 'tenInterpTwo_d',
           'hestParseFree', 'hooverStubRenderEnd',
           'tend_evalclampCmd', 'tenGageTrace', 'echoRayColor',
           'gageSclGradMag', 'nrrdCopy', 'airInsane_32Bit',
           'nrrdUnaryOp', 'nrrdKernelTMF', 'pullEnergyButterworth',
           'gagePerVolumeNix', 'tenEstimateSkipSet',
           'airInsane_nInfExists', 'tenDwiFiberTypeLast',
           'gageScl3PFilter_t', 'echoTriMesh',
           'tenDwiGageTensorError', 'airTypeLongInt',
           'nrrdKernelParmSet', 'limnPrimitiveLast',
           'gageVecMultiGrad', 'tenGageNorm',
           'tenDwiGage2TensorPeledLevmarInfo', 'airLogRician',
           'tenGageTraceGradVecDotEvec0',
           'tenDwiGageTensorWLSLikelihood', 'tenEMatrixCalc',
           'gageSigOfTau', 'nrrdRangeReset', 'nrrdKind3Color',
           'airSrandMT', 'tenGageConfGradVecDotEvec0',
           'airNoDio_small', 'gageSclUnknown',
           'gageStackBlurParmScaleSet', 'airNoDio_fd',
           'tend_tconvCmd', 'nrrdAxisInfoSet_va',
           'tenGageModeGradVec', 'nrrdValCompareInv',
           'nrrdStringRead', 'unrrdu_rmapCmd',
           'limnPolyDataPrimitiveSelect', 'ell_q_inv_f',
           'pullPropNeighDistMean', 'nrrdSpace', 'pullContext',
           'tend_anvolCmd', 'hestParse', 'limnObjectFaceReverse',
           'miteShadeSpecPrint', 'NrrdIoState_t',
           'nrrdEncodingUnknown', 'airFPValToParts_f',
           'airFPValToParts_d', 'dyeColor', 'tenFiberStopReset',
           'pullInterType', 'pullEnergySpecSet', 'gageCtxFlagNeedD',
           'limnPolyDataClip', 'gageCtxFlagNeedK', 'limnQN16checker',
           'tenExperSpecGradBValSet', 'gageZeroNormal', 'airStdin',
           'gageSclHessValleyness', 'nrrdSpacingStatus',
           '_airThreadCond', 'meetPullVol', 'gageItemSpecNew',
           'airBesselI1By0', 'nrrdBasicInfoSampleUnits',
           'nrrdIoStateEncodingSet', 'nrrdMeasureUnknown',
           'ell_3m_pre_mul_d', 'ell_3m_pre_mul_f', 'unrrduBiffKey',
           'nrrdResampleRoundSet', 'miteShadeMethodPhong',
           'biffMsgNew', 'airEndianLast', 'nrrdSpaceVecScale',
           'ell_q_to_3m_d', 'nrrdDLoad', 'hestInfo',
           'miteStageOpUnknown', 'nrrdField', 'nrrdEncodingHex',
           'nrrdStateMeasureModeBins', 'gagePerVolume_t',
           'limnPolyDataPolarSphere', 'tenDwiGageB0',
           'nrrdApplyMulti1DRegMap', 'limnQNLast',
           'echoTypeSuperquad', 'nrrdKindNormal', 'miteValLast',
           'gageErrStackSearch', 'tenGageTraceNormal',
           'nrrdRangeSafeSet', 'hooverErrRayBegin', 'nrrdAxesMerge',
           'echoJitterLast', 'echoTypeTriMesh', 'tenModelBall',
           'hooverThreadEnd_t', 'limnQN8checker',
           'nrrdAxisInfoSet_nva', 'nrrdEnvVarStateGrayscaleImage3D',
           'pullVolumeNew', 'tenEstimateBMatricesSet',
           'unrrdu_histoCmd', 'dyeColorNew', 'gageParmSet',
           'nrrdRead', 'echoJitterCompute', 'airInsane_AIR_NAN',
           'nrrdBinaryOpUnknown', 'nrrdDefaultResampleRenormalize',
           'tenEvecRGBParmNix', 'nrrdUnaryOpNormalRand',
           'tend_msimCmd', 'seekTypeValleySurface', 'airShuffle',
           'dyeSpaceXYZ', 'tenGageCl1HessianEval', 'nrrdApply2DLut',
           'tenAniso_Cl1', 'tenAniso_Cl2', 'miteValNdotL',
           'airULLong', 'gageKernel', 'gageSclGeomTens',
           'ell_cubic_root_single_double', 'airEnum',
           'nrrdCenterCell', 'hooverErrThreadBegin',
           'nrrdUnaryOpUnknown', 'airNaN',
           'limnPolyDataVertexWindingFix', 'miteStageOp',
           'dyeSpaceHSV', 'tenEstimate1MethodLast',
           'gageShapeBoundingBox', 'miteDefOpacMatters',
           'gageTauOfSig', 'nrrdField_byte_skip', 'airDisableDio',
           'airArrayLenSet', 'gageQuery', 'gageScl3PFilter8',
           'tenGageCl1HessianEval0', 'tenFiberContext',
           'gageScl3PFilter2', 'nrrdBlind8BitRangeTrue',
           'gageScl3PFilter6', 'dyeSpaceHSL',
           'nrrdSpaceScannerXYZTime', 'echoTypeCube',
           'nrrdCommentCopy', 'pullPoint', 'nrrdKernelTMF_maxA',
           'dyeSpaceToStr', 'nrrdKindDomain', 'nrrdKernelTMF_maxD',
           'unrrduHestPosCB', 'nrrdKernelTent', 'unrrduHestFileCB',
           'gageParmReset', 'airDouble', 'tenFiberType',
           'airInsane_DLSize', 'tenDwiGageLast', 'nrrdElementSize',
           'limnCameraNew', 'gageVecUnknown', 'nrrdMeasureSkew',
           'nrrdDistanceL2Biased', 'echoCol_t', 'echoListSplit',
           'airNormalRand', 'airThreadNix', 'airStrtok',
           'seekContour3DTopoHackEdge', 'nrrdBlind8BitRangeUnknown',
           'gageScl3PFilterN', 'unrrdu_spliceCmd', 'nrrdAxisInfoMax',
           'nrrdTypeLLong', 'echoMatterGlass', 'tenGageFANormal',
           'pullStatusEdge', 'dyeSimpleConvert', 'nrrdMaybeAlloc_nva',
           'nrrdSpaceUnknown', 'tenModelPrefixStr', '_airThreadMutex',
           'nrrdWrap_nva', 'nrrdNix', 'nrrdResampleInfoNix',
           'airSanity', 'nrrdCCSettle', 'gagePvlFlagLast',
           'airArrayNew', 'limnPrimitiveTriangleFan',
           'nrrdField_type', 'miteThreadEnd', 'pullInfoHeightHessian',
           'gageSclLast', 'tenInterpTypeAffineInvariant', 'airMopSub',
           'echoTypeInstance', 'dyeHSLtoRGB', 'unrrdu_axinfoCmd',
           'tenGageCl1Normal', 'tenGageCa1Hessian',
           'limnDefCameraOrthographic', 'nrrdStateKindNoop',
           'nrrdTypeDouble', 'tenDwiGageTensorMLELikelihood',
           'nrrdField_measurement_frame', 'gagePvlFlagVolume',
           'NrrdEncoding', 'tenFiberStopOff', 'nrrdBoundaryPad',
           'tenEstimate1TensorVolume4D', 'nrrdMeasureHistoVariance',
           'tenFiberTypeSet', 'tenDWMRIModalityVal',
           'limnSplineTypeHasImplicitTangents', 'tenTripleTypeXYZ',
           'tenAnisoLast', 'nrrdDescribe', 'limnObjectEdgeAdd',
           'gageStackProbeSpace', 'nrrdKernelBox', 'gageContext_t',
           'nrrdTypeUInt', 'tenShrink', 'tenLogSingle_f',
           'limnObjectCylinderAdd', 'gageErrBoundsSpace',
           'gageSclShapeIndex', 'nrrdKernelAQuarticDD',
           'nrrdKernelHannDD', 'limnQN10octa', 'echoIntxFuzzify',
           'nrrdTypeUnknown', 'nrrdResampleContextNix',
           'hooverStubRayEnd', 'limnSplineBCSet',
           'unrrduHestEncodingCB', 'limnQNBins',
           'ell_Nm_wght_pseudo_inv', 'limnObjectSquareAdd',
           'limnSplineCleverNew', 'limnQN14checker', 'tend_estimCmd',
           'gageContextNew', 'tend_evecCmd', 'unrrdu_dataCmd',
           'tenGageEval', 'airMopNew', 'nrrdZlibStrategyLast',
           'unrrdu_headCmd', 'limnSplineInfoScalar',
           'nrrdField_old_max', 'hooverErrThreadEnd',
           'seekTypeValleySurfaceOP', 'gageSclFlowlineCurv',
           'nrrdMeasureProduct', 'echoThreadState',
           'tenDwiGageTensorNLS', 'nrrdResampleRangeFullSet',
           'limnPolyDataSize', 'airMyQNaNHiBit', 'airDioWrite',
           'tend_gradsCmd', 'tenFiberParmVerbose',
           'limnObjectLookAdd', 'tenModelNllFit',
           'limnEdgeTypeBorder', 'nrrdSpaceOriginGet',
           'nrrdBoundaryUnknown', 'tenInv_f', 'pullInfoAnswerLen',
           'tenGageCl1HessianEval2', 'tenTripleTypeUnknown',
           'nrrdSpaceRightAnteriorSuperiorTime',
           'tenGageCl1HessianEval1', 'tenFiberContextDwiNew',
           'tenFiberStopUnknown', 'airThreadBarrierNix',
           'meetPullInfoNew', 'gageSclHessEval0',
           'nrrdMeasureLineIntercept', 'airNoDioErr',
           'nrrdResample_t', 'nrrdUnaryOpRoundDown',
           'airArrayPointerCB', 'nrrdOriginStatusUnknown',
           'nrrdKind2Vector', 'nrrdEnvVarDefaultKernelParm0',
           'nrrdAxisInfoIdxRange', 'echoMatterLight',
           'gageDeconvolve', 'gageSclHessEval1', 'echoObjectNew',
           'gageSclHessEval2', 'tenEstimateLinear4D',
           'nrrdDefaultGetenv']

# its nice to have these FILE*s around, and this is exactly the
# reason for these air functions
stderr = airStderr()
stdout = airStdout()
stdin = airStdin()
