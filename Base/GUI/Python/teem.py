from ctypes import *

import sys, os
if sys.platform == 'win32':
  teemdir = os.environ['TEEM_DIR']
  libteem = teemdir + '/teem.dll'
elif sys.platform == 'linux2':
  teemdir = os.environ['Teem_DIR']
  libteem = teemdir + '/libteem.so'
elif sys.platform == 'darwin':
  teemdir = os.environ['Teem_DIR']
  libteem = teemdir + '/libteem.dylib'

_libraries = {}
_libraries[libteem] = CDLL(libteem)
STRING = c_char_p

# this is a manual inclusion of GLK's stdio.py ---------
# Teem never cares about what's inside FILE, so hide that
class FILE(Structure):
    pass

# We do need ptrdiff_t, which itself is platform-dependent
if sizeof(c_void_p) == 4:
    ptrdiff_t = c_int32
elif sizeof(c_void_p) == 8:
    ptrdiff_t = c_int64
# end manual inclusion of stdio.py ---------------------

tenFiberParmWPunct = 3
gageVecVector1 = 3
tenGageBHessian = 83
nrrdUnaryOpNegative = 1
dyeSpaceHSL = 2
airFP_QNAN = 2
nrrdBasicInfoComments = 14
tenDwiGageTensorMLEError = 19
nrrdBasicInfoOldMin = 12
gageParmVerbose = 1
tenEstimate1MethodWLS = 2
tenGageDet = 6
tenDwiGageTensorWLS = 10
nrrdBoundaryWrap = 3
tenGageInvarRGradMags = 69
limnSpaceScreen = 3
miteValGTdotV = 16
tenGageSGradMag = 46
limnQNUnknown = 0
tenDwiGageTensorLLSLikelihood = 9
tenAniso_VF = 16
dyeSpaceXYZ = 4
tenGageEval2 = 18
seekTypeValleyLine = 5
tenAnisoUnknown = 0
nrrdBasicInfoData = 1
nrrdTernaryOpClamp = 5
airFP_POS_NORM = 5
gageSclHessRidgeness = 31
tenDwiGageADC = 4
nrrdKindDomain = 1
gageSclHessMode = 32
nrrdSpace3DRightHandedTime = 11
miteValUnknown = 0
tenGageFADiffusionAlign = 136
airMopAlways = 3
tenGageFAHessianEval2 = 91
nrrdSpaceLeftAnteriorSuperiorTime = 5
tenGageOmegaGradVec = 63
nrrdTypeULLong = 8
tenGageTraceNormal = 35
limnSplineTypeTimeWarp = 2
miteStageOpAdd = 3
miteValXi = 2
tenFiberTypeZhukov = 6
tenDwiFiberType1Evec0 = 1
nrrdMeasureUnknown = 0
tenGageTraceHessian = 82
tenAniso_Cs2 = 12
nrrdMeasureSD = 12
limnCameraPathTrackBoth = 3
nrrdUnaryOpLog10 = 12
tenGageCp1GradVec = 153
tenGageDelNormPhi3 = 29
nrrdFormatTypeEPS = 6
tenGageClpmin2 = 80
nrrdKind3Normal = 21
tenGageOmegaHessianEvec = 126
limnSplineTypeLast = 6
nrrdHasNonExistTrue = 1
limnPrimitiveLineStrip = 6
nrrdOriginStatusNoMaxOrSpacing = 3
nrrdUnaryOpLast = 29
tenGageOmegaHessianEval2 = 125
nrrdUnaryOpFloor = 18
echoTypeLast = 12
nrrdEncodingTypeLast = 6
echoMatterUnknown = 0
nrrdKindNormal = 8
seekTypeRidgeSurface = 2
nrrdBasicInfoSpaceUnits = 9
tenGageModeHessianEvec1 = 119
tenGageDetGradMag = 43
limnEdgeTypeBorder = 6
gageSclHessEval = 9
nrrdZlibStrategyUnknown = 0
miteRangeRed = 1
tenGageEval = 15
nrrdMeasureLast = 27
echoMatterPhongSp = 3
limnPrimitiveQuads = 5
echoJitterLast = 4
nrrdIoStateZlibLevel = 7
nrrdField_comment = 1
tenAniso_RA = 14
tenGageThetaGradVec = 60
limnSpaceUnknown = 0
nrrdMeasureHistoMin = 17
tenGageTensorGradMagMag = 32
echoTypeRectangle = 5
gagePvlFlagVolume = 1
gageVecMGEval = 28
echoJittableMotionB = 6
nrrdHasNonExistUnknown = 3
airNoDio_std = 3
limnSpaceDevice = 4
nrrdBinaryOpDivide = 4
nrrdBinaryOpMod = 7
airNoDio_test = 11
nrrdUnaryOpAtan = 8
gageScl2ndDD = 17
tenGageAniso = 150
gageParmGradMagCurvMin = 5
nrrdField_old_min = 21
tenGageUnknown = 0
nrrdMeasureHistoSum = 23
echoMatterPhong = 1
hooverErrRayEnd = 7
tenGageCl2 = 77
tenGageModeHessianEval2 = 116
tenGageModeHessianEval1 = 115
nrrdBasicInfoLast = 16
nrrdTypeDouble = 10
limnSplineTypeHermite = 3
nrrdBinaryOpMultiply = 3
tenFiberStopMinNumSteps = 10
tenGageModeHessianEvec2 = 120
miteRangeLast = 9
limnSplineInfoLast = 7
limnDeviceGL = 2
tenFiberTypeTensorLine = 4
nrrdAxisInfoThickness = 3
nrrdUnaryOpIf = 26
tenDwiGageTensorLLS = 6
tenGageTensorGradRotE = 157
nrrdUnaryOpRoundUp = 19
airFP_SNAN = 1
limnEdgeTypeBackCrease = 2
tenGageTheta = 12
nrrdFormatTypePNG = 3
nrrdKind3Vector = 19
gageCtxFlagNeedK = 3
tenGageCa1GradMag = 156
nrrdBlind8BitRangeTrue = 1
dyeSpaceLAB = 5
echoJitterNone = 0
gageVecCurl = 9
nrrdTernaryOpInOpen = 9
nrrdKind2DMatrix = 26
echoMatterGlassIndex = 0
tenGageOmegaLaplacian = 130
nrrdCenterCell = 2
tenGageSHessian = 85
nrrdFormatTypeNRRD = 1
nrrdBasicInfoUnknown = 0
nrrdSpacingStatusScalarWithSpace = 3
tenGageModeGradVec = 57
tenTripleTypeRThetaPhi = 5
tenAniso_Cl2 = 8
nrrdTypeDefault = 0
nrrdSpacingStatusNone = 1
echoMatterMetalKd = 2
tenGageModeHessian = 112
tenInterpTypeLoxK = 7
tenTripleTypeMoment = 2
nrrdField_block_size = 5
hooverErrNone = 0
tenGageCp1GradMag = 154
gageErrStackIntegral = 4
nrrdUnaryOpCos = 4
tenGageTensorLogEuclidean = 147
tenGageFAKappa1 = 102
nrrdSpace3DLeftHanded = 10
hooverErrThreadCreate = 3
nrrdField_centers = 15
tenGageCp1 = 74
gageKernel10 = 2
airEndianBig = 4321
nrrdZlibStrategyLast = 4
hooverErrLast = 11
gageKernelUnknown = 0
tenFiberStopBounds = 6
nrrdIoStateBareText = 2
tenDwiGage2TensorQSegAndError = 31
tenFiberParmStepSize = 1
nrrdUnaryOpExists = 23
tenDwiGageB0 = 2
gageParmUnknown = 0
tenGageConfidence = 2
tenDwiGage2TensorQSeg = 29
tenDwiGageTensorErrorLog = 24
gageCtxFlagKernel = 4
nrrdField_thicknesses = 11
nrrdEncodingTypeBzip2 = 5
nrrdKindStub = 9
nrrdBasicInfoContent = 5
miteValNormal = 12
limnDeviceUnknown = 0
airInsane_UCSize = 10
nrrdOriginStatusUnknown = 0
dyeSpaceLUV = 6
tenEstimate1MethodLast = 5
airNoDio_arch = 1
nrrdIoStateZlibStrategy = 8
tenDwiFiberTypeUnknown = 0
limnPrimitiveLast = 8
tenGageOmegaHessianEvec0 = 127
airInsane_FISize = 11
seekTypeValleySurface = 3
tenGageRGradVec = 54
tenDwiGage2TensorPeledLevmarInfo = 35
tenGageDelNormR2 = 26
tenGageTraceGradMag = 34
nrrdCenterUnknown = 0
tenFiberIntgEuler = 1
echoTypeAABBox = 8
airTypeDouble = 7
nrrdBinaryOpIf = 20
tenGageFAGaussCurv = 107
nrrdSpaceLeftAnteriorSuperior = 2
nrrdMeasureL1 = 8
tenGageCovariance = 144
tenGageFAHessianEvalMode = 99
tenInterpTypeLast = 11
echoTypeCylinder = 1
nrrdBinaryOpSgnPow = 6
miteRangeKd = 6
gageVecGradient2 = 25
miteValYw = 3
echoTypeSplit = 9
nrrdKindVector = 6
nrrdTypeBlock = 11
tenAnisoLast = 30
nrrdBasicInfoSpaceOrigin = 10
tenFiberStopUnknown = 0
tenDwiGageTensorLikelihood = 25
nrrdField_kinds = 16
gageSclLast = 33
nrrdMeasureHistoProduct = 22
nrrdBoundaryBleed = 2
limnQN8octa = 16
gageParmDefaultCenter = 11
limnQN16border1 = 2
echoJittableNormalA = 3
airInsane_dio = 8
nrrdField_sizes = 9
tenAniso_FA = 15
nrrdKind4Vector = 22
miteRangeEmissivity = 4
nrrdKindList = 4
seekTypeMaximalSurface = 7
nrrdUnaryOpNormalRand = 25
tenGageFAGradVec = 51
gageCtxFlagUnknown = 0
nrrdKind2DSymMatrix = 24
gageVecDivGradient = 16
tenGageDelNormK2 = 23
nrrdKindSpace = 2
nrrdFormatTypeUnknown = 0
nrrdSpace3DLeftHandedTime = 12
nrrdField_space = 7
tenFiberTypeUnknown = 0
airFP_Unknown = 0
airMopOnOkay = 2
echoTypeSphere = 0
airTypeUInt = 3
limnSplineInfo4Vector = 5
gageErrLast = 5
tenGageConfGradVecDotEvec0 = 141
limnQN12checker = 9
nrrdField_endian = 23
tenGageFARidgeSurfaceStrength = 96
tenTripleTypeUnknown = 0
gageVecHelicity = 11
ell_cubic_root_unknown = 0
tenGlyphTypeLast = 5
tenGageInvarRGrads = 68
nrrdSpaceLeftPosteriorSuperior = 3
nrrdBinaryOpSubtract = 2
echoMatterGlass = 2
dyeSpaceLast = 7
gageSclShapeTrace = 22
gageSclHessValleyness = 30
gageSclMeanCurv = 24
ell_cubic_root_last = 5
gageSclHessEvec1 = 15
airFP_NEG_DENORM = 8
tenAniso_Mode = 22
nrrdUnaryOpCeil = 17
tenGageModeHessianEval = 113
nrrdMeasureMean = 3
gageSclMedian = 29
tenDwiGageTensorWLSErrorLog = 12
tenFiberStopLast = 11
tenAniso_Ct1 = 7
miteShadeMethodLitTen = 3
nrrdOriginStatusNoMin = 2
limnPolyDataInfoRGBA = 1
tenGageRHessian = 111
gageParmRequireEqualCenters = 10
limnSplineInfoUnknown = 0
tenAniso_Cl1 = 2
nrrdSpacingStatusLast = 5
limnEdgeTypeFrontFacet = 5
tenEstimate1MethodMLE = 4
miteRangeAlpha = 0
tenGageQGradVec = 48
tenAniso_R = 19
nrrdUnaryOpRoundDown = 20
miteShadeMethodNone = 1
tenGageEvec0 = 20
nrrdIoStateSkipData = 5
nrrdField_space_units = 29
nrrdTernaryOpExists = 8
nrrdKind2DMaskedMatrix = 27
echoTypeUnknown = -1
tenEstimate2MethodLast = 3
limnQNLast = 17
airMopNever = 0
nrrdSpaceRightAnteriorSuperior = 1
tenGageOmegaGradVecDotEvec0 = 138
limnQN9octa = 14
tenGageFAHessianEvec0 = 93
echoMatterLightUnit = 1
gageVecDivergence = 8
tenGlyphTypeBox = 1
tenGageOmegaNormal = 65
airNoDio_format = 2
tenGageNormGradMag = 37
nrrdBinaryOpFmod = 8
tenGageOmegaDiffusionAlign = 139
nrrdTypeUInt = 6
nrrdUnaryOpAcos = 7
tenGageTensorQuatGeoLoxR = 149
airFP_Last = 11
nrrdSpaceLeftPosteriorSuperiorTime = 6
ell_cubic_root_single = 1
nrrdAxisInfoLabel = 9
echoMatterMetal = 3
hooverErrSample = 6
seekTypeLast = 8
airFP_NEG_INF = 4
nrrdUnaryOpSqrt = 14
tenGageCa1GradVec = 155
tenGageFAFlowlineCurv = 110
nrrdField_keyvalue = 27
nrrdAxisInfoUnits = 10
limnSpaceView = 2
tenGageFAKappa2 = 103
limnSplineTypeCubicBezier = 4
tenDwiGageFA = 27
tenFiberStopStub = 8
limnPolyDataInfoTex2 = 3
dyeSpaceRGB = 3
nrrdMeasureLineIntercept = 15
tenDwiGageMeanDWIValue = 5
nrrdField_measurement_frame = 31
tenGageFADiffusionFraction = 137
airNoDio_ptr = 8
tenGageB = 5
gageVecCurlGradient = 17
miteValVdefTdotV = 18
nrrdEncodingTypeRaw = 1
limnEdgeTypeBackFacet = 1
gageVecLambda2 = 13
nrrdBasicInfoSpaceDimension = 8
tenGageSGradVec = 45
limnPrimitiveLines = 7
airEndianLittle = 1234
miteRangeSP = 8
tenGageEval1 = 17
nrrdUnaryOpCbrt = 15
nrrdKindComplex = 11
nrrdSpacingStatusScalarNoSpace = 2
nrrdTernaryOpMax = 4
gageVecUnknown = 0
limnCameraPathTrackFrom = 1
echoMatterMetalR0 = 0
limnPrimitiveTriangles = 2
tenGageFAGradVecDotEvec0 = 135
tenGageFAHessianEval1 = 90
seekTypeUnknown = 0
miteValTi = 10
tenGageThetaNormal = 62
echoMatterMetalKa = 1
tenAniso_Clpmin2 = 11
limnQN15octa = 5
tenInterpTypeGeoLoxR = 6
tenTripleTypeEigenvalue = 1
gagePvlFlagLast = 4
gageVecMGFrob = 27
gageSclGaussCurv = 25
gageErrBoundsStack = 3
nrrdUnaryOpSin = 3
airTypeUnknown = 0
hooverErrRenderBegin = 2
gageKernel00 = 1
nrrdUnaryOpLog2 = 11
tenGageCl1GradMag = 152
seekTypeIsocontour = 1
nrrdZlibStrategyDefault = 1
nrrdAxisInfoSpacing = 2
tenGageCa2 = 79
tenGageTraceDiffusionAlign = 133
tenDwiGageTensorWLSLikelihood = 13
nrrdKindHSVColor = 15
tenDwiGageTensorAllDWIError = 28
tenFiberStopMinLength = 9
nrrdMeasureMax = 2
tenDwiGageAll = 1
airFP_POS_ZERO = 9
nrrdIoStateUnknown = 0
nrrdTernaryOpLast = 11
tenGageQ = 8
nrrdUnaryOpOne = 28
nrrdBoundaryLast = 5
tenGageQGradMag = 49
tenGageOmegaHessianEval1 = 124
nrrdBasicInfoOldMax = 13
tenEstimate2MethodQSegLLS = 1
nrrdFormatTypeText = 5
gageSclGradVec = 2
nrrdBinaryOpNotEqual = 18
nrrdField_spacings = 10
echoJittableMotionA = 5
nrrdTypeShort = 3
tenGageOmega = 14
nrrdBlind8BitRangeUnknown = 0
nrrdIoStateBzip2BlockSize = 9
nrrdBasicInfoKeyValuePairs = 15
airInsane_AIR_NAN = 7
tenAniso_S = 20
miteStageOpLast = 5
echoJitterRandom = 3
gageSclFlowlineCurv = 28
nrrdIoStateValsPerLine = 4
nrrdKind3DSymMatrix = 28
tenGageFAHessian = 87
tenTripleTypeLast = 10
tenGageModeNormal = 59
tenGageTensor = 1
echoMatterPhongKa = 0
limnQN8checker = 15
tenInterpTypeLogLinear = 2
airTypeFloat = 6
tenAniso_Ca2 = 10
limnQN11octa = 11
echoJittableNormalB = 4
airNoDio_small = 6
nrrdField_dimension = 6
tenGageBGradVec = 39
tenGageFATotalCurv = 104
tenGageFAShapeIndex = 105
gageVecHessian = 15
miteValYi = 4
nrrdField_space_origin = 30
tenDwiGageTensorNLS = 14
nrrdMeasureHistoMax = 18
miteValView = 11
tenDwiGageTensorError = 23
nrrdBinaryOpGTE = 15
limnEdgeTypeLast = 8
tenInterpTypeQuatGeoLoxR = 10
nrrdField_sample_units = 28
tenGageOmegaHessian = 121
nrrdField_data_file = 32
limnQN16octa = 4
nrrdMeasureSum = 7
airTypeEnum = 10
tenGageFA = 9
echoTypeIsosurface = 7
tenAniso_Clpmin1 = 5
gageVecVector2 = 4
nrrdHasNonExistLast = 4
tenGageMode = 11
nrrdBasicInfoSpace = 7
echoMatterMetalFuzzy = 3
gageSclHessEval2 = 12
tenEstimate2MethodUnknown = 0
nrrdBasicInfoDimension = 4
gageParmDefaultSpacing = 6
tenGageDetHessian = 84
nrrdBinaryOpLTE = 13
tenGageRNormal = 56
limnSpaceWorld = 1
tenGageDelNormPhi2 = 28
airMopOnError = 1
nrrdHasNonExistFalse = 0
airTypeInt = 2
nrrdKind2DMaskedSymMatrix = 25
gageVecLength = 5
nrrdBinaryOpNormalRandScaleAdd = 21
gageParmK3Pack = 4
limnSplineInfoQuaternion = 6
airNoDio_dioinfo = 5
tenGageCovarianceKGRT = 146
dyeSpaceUnknown = 0
limnSplineInfoScalar = 1
nrrdAxisInfoSpaceDirection = 6
gageSclHessEvec0 = 14
miteValRi = 8
tenGageCl1 = 73
airTypeLongInt = 4
nrrdSpaceUnknown = 0
tenDwiGageTensorNLSError = 15
tenDwiGage2TensorQSegError = 30
nrrdField_line_skip = 25
gageVecVector0 = 2
airFP_NEG_NORM = 6
hooverErrRenderEnd = 10
nrrdUnaryOpUnknown = 0
tenGageHessian = 81
miteValZi = 6
gageSclGeomTens = 18
tenAniso_Omega = 24
nrrdUnaryOpSgn = 22
tenGageModeHessianEvec0 = 118
tenFiberStopFraction = 7
airNoDio_okay = 0
tenEstimate1MethodNLS = 3
nrrdOriginStatusDirection = 1
tenGageTraceGradVecDotEvec0 = 132
tenFiberStopConfidence = 4
nrrdEncodingTypeGzip = 4
miteShadeMethodLast = 4
nrrdAxisInfoUnknown = 0
gageSclHessFrob = 8
tenAniso_Ca1 = 4
nrrdAxisInfoMax = 5
miteValVdefT = 17
tenFiberTypeEvec1 = 2
nrrdEncodingTypeAscii = 2
tenAniso_Skew = 21
nrrdBinaryOpLast = 22
nrrdKindTime = 3
gageParmLast = 14
nrrdIoStateLast = 10
limnQN13octa = 8
tenAniso_Q = 18
gageParmStackUse = 12
nrrdMeasureHistoSD = 26
tenGageFACurvDir1 = 108
miteRangeKs = 7
nrrdIoStateCharsPerLine = 3
tenFiberParmUnknown = 0
nrrdKind3Gradient = 20
nrrdTernaryOpLerp = 7
limnCameraPathTrackUnknown = 0
tenGageFANormal = 53
airInsane_pInfExists = 2
limnPrimitiveNoop = 1
tenGageRGradMag = 55
tenGageDelNormR1 = 25
tenAniso_Cp2 = 9
nrrdTernaryOpInClosed = 10
echoMatterLast = 5
miteValTw = 9
echoMatterGlassFuzzy = 3
echoJittableLast = 7
tenGageConfDiffusionFraction = 143
limnQN10checker = 12
tenGageFALaplacian = 98
nrrdField_last = 33
limnSplineInfo3Vector = 3
tenGageRotTans = 70
nrrdTypeLLong = 7
echoMatterGlassKd = 2
nrrdUnaryOpAsin = 6
tenDwiGageTensorNLSErrorLog = 16
echoMatterLightPower = 0
tenGageThetaGradMag = 61
nrrdSpace3DRightHanded = 9
nrrdAxisInfoMin = 4
gageVecLast = 30
hooverErrRayBegin = 5
miteRangeBlue = 3
airNoDio_setfl = 10
nrrdMeasureMode = 5
tenFiberTypeEvec2 = 3
nrrdBinaryOpLT = 12
tenDwiGageTensorWLSError = 11
nrrdKindRGBAColor = 18
nrrdBasicInfoMeasurementFrame = 11
nrrdField_type = 4
tenFiberIntgRK4 = 3
nrrdFormatTypeVTK = 4
tenFiberStopNumSteps = 3
ell_cubic_root_single_double = 3
limnEdgeTypeUnknown = 0
miteValNdotV = 13
nrrdMeasureLinf = 10
tenGageTensorGradMag = 31
gageSclNPerp = 5
tenGageR = 10
nrrdField_units = 18
tenGageClpmin1 = 76
nrrdTernaryOpUnknown = 0
gageParmCheckIntegrals = 3
echoJittablePixel = 0
gageVecGradient0 = 23
tenInterpTypeGeoLoxK = 5
airNoDio_size = 7
nrrdTernaryOpMin = 3
tenGageQNormal = 50
nrrdCenterNode = 1
airInsane_FltDblFPClass = 5
tenGageEvec2 = 22
limnPolyDataInfoLast = 4
nrrdKindLast = 32
echoMatterGlassKa = 1
tenInterpTypeWang = 4
gagePvlFlagUnknown = 0
nrrdBinaryOpExists = 19
nrrdAxisInfoKind = 8
tenGageOmegaDiffusionFraction = 140
gageSclCurvDir2 = 27
tenGageFAHessianEvec2 = 95
nrrdSpaceScannerXYZTime = 8
gagePvlFlagNeedD = 3
tenGlyphTypeSuperquad = 4
tenGageInvarKGradMags = 67
tenInterpTypeAffineInvariant = 3
airNoDio_fpos = 9
tenDwiGageTensorNLSLikelihood = 17
tenDwiGage2TensorPeled = 32
hooverErrInit = 1
gageSclHessEval0 = 10
nrrdField_byte_skip = 26
limnEdgeTypeFrontCrease = 4
ell_cubic_root_three = 4
tenDwiGageUnknown = 0
gageKernelLast = 8
tenGageRotTanMags = 71
nrrdUnaryOpErf = 16
nrrdBinaryOpAtan2 = 9
tenEstimate1MethodUnknown = 0
nrrdKindXYZColor = 16
tenFiberStopLength = 2
echoTypeInstance = 11
gageCtxFlagNeedD = 1
gageVecCurlNorm = 10
airFP_POS_DENORM = 7
nrrdTypeChar = 1
gageVecProjHelGradient = 22
tenAniso_Conf = 1
nrrdField_max = 20
tenGageSNormal = 47
airInsane_QNaNHiBit = 6
gageSclValue = 1
nrrdMeasureSkew = 13
tenGageEvec = 19
echoJitterJitter = 2
seekTypeRidgeLine = 4
limnDeviceLast = 3
miteStageOpMultiply = 4
tenGageFAHessianEvec = 92
nrrdKindUnknown = 0
echoMatterPhongKd = 1
echoMatterLight = 4
airTypeSize_t = 5
tenGageOmegaGradMag = 64
tenGageNormGradVec = 36
tenDwiGage2TensorPeledError = 33
tenInterpTypeLinear = 1
gageVecMultiGrad = 26
nrrdTypeInt = 5
nrrdField_number = 3
miteValLast = 20
nrrdKindScalar = 10
nrrdSpacingStatusDirection = 4
hooverErrThreadJoin = 9
tenInterpTypeUnknown = 0
gageKernel20 = 4
tenGageBGradMag = 40
nrrdTypeFloat = 9
nrrdUnaryOpLog1p = 13
tenGageFACurvDir2 = 109
tenGageInvarKGrads = 66
nrrdMeasureMin = 1
limnSpaceLast = 5
limnPolyDataInfoUnknown = 0
nrrdKindRGBColor = 14
nrrdBasicInfoBlocksize = 3
nrrdHasNonExistOnly = 2
nrrdBlind8BitRangeLast = 4
tenGageNorm = 4
nrrdBlind8BitRangeState = 3
airTypeOther = 11
gageSclHessEvec = 13
airInsane_not = 0
gageCtxFlagK3Pack = 2
tenGageEval0 = 16
airInsane_NaNExists = 4
nrrdKind3DMaskedSymMatrix = 29
gageVecNormalized = 6
nrrdBoundaryUnknown = 0
nrrdMeasureHistoVariance = 25
gageParmCurvNormalSide = 7
nrrdBinaryOpCompare = 16
nrrdIoStateKeepNrrdDataFileOpen = 6
nrrdField_labels = 17
gageErrBoundsSpace = 2
limnPrimitiveTriangleStrip = 3
gageSclNormal = 4
tenGageTraceDiffusionFraction = 134
tenGageFAHessianEval0 = 89
tenTripleTypeR = 8
airTypeBool = 1
tenGageTraceGradVec = 33
limnQN14checker = 6
tenDwiGage2TensorPeledAndError = 34
tenInterpTypeQuatGeoLoxK = 9
nrrdBinaryOpAdd = 1
tenGageFAHessianEval = 88
limnCameraPathTrackLast = 4
tenGageLast = 158
limnCameraPathTrackAt = 2
tenFiberTypeLast = 7
seekTypeMinimalSurface = 6
echoTypeTriMesh = 6
airNoDio_disable = 12
gageSclShapeIndex = 23
nrrdUnaryOpLog = 10
tenGageCl1GradVec = 151
nrrdField_old_max = 22
airFP_POS_INF = 3
tenGageFAMeanCurv = 106
nrrdZlibStrategyFiltered = 3
miteValXw = 1
nrrdField_content = 2
tenGageCp2 = 78
gageSclTotalCurv = 21
limnPolyDataInfoNorm = 2
nrrdMeasureHistoMean = 19
tenFiberStopRadius = 5
nrrdUnaryOpReciprocal = 2
tenGlyphTypeSphere = 2
limnSplineTypeBC = 5
gageCtxFlagShape = 6
nrrdMeasureProduct = 6
tenAniso_eval2 = 29
nrrdUnaryOpZero = 27
gageSclHessian = 6
tenDwiGageTensorLLSErrorLog = 8
tenGageOmegaHessianEval0 = 123
gageCtxFlagLast = 7
tenAniso_Tr = 26
tenGageS = 7
tenGageOmegaHessianEvec2 = 129
limnQN16simple = 1
tenAniso_Cp1 = 3
tenGageModeWarp = 13
nrrdTypeUChar = 2
gageParmKernelIntegralNearZero = 8
nrrdKindCovariantVector = 7
tenDwiGageConfidence = 26
gageVecJacobian = 7
gageCtxFlagRadius = 5
gageVecNormHelicity = 12
tenDwiGageTensorMLE = 18
gageVecHelGradient = 20
gageSclLaplacian = 7
nrrdBasicInfoType = 2
nrrdTernaryOpIfElse = 6
tenGageOmega2ndDD = 131
tenEstimate2MethodPeled = 2
gageParmRenormalize = 2
gageErrUnknown = 0
tenGageQHessian = 86
airEndianUnknown = 0
limnPrimitiveUnknown = 0
nrrdBinaryOpUnknown = 0
tenGageModeGradMag = 58
tenDwiGageTensorMLEErrorLog = 20
tenGageTensorGrad = 30
echoTypeCube = 3
airTypeString = 9
gageVecMGEvec = 29
limnSplineInfoNormal = 4
gageKernel22 = 6
miteStageOpMin = 1
nrrdUnaryOpTan = 5
tenGageTensorQuatGeoLoxK = 148
limnQN14octa = 7
miteValZw = 5
tenGageCa1 = 75
airEndianLast = 4322
nrrdField_min = 19
nrrdKind2Vector = 12
tenGageOmegaHessianEvec1 = 128
nrrdKind3DMaskedMatrix = 31
echoTypeList = 10
tenFiberParmLast = 5
nrrdTernaryOpAdd = 1
nrrdUnaryOpRand = 24
tenAniso_eval1 = 28
miteShadeMethodPhong = 2
nrrdOriginStatusOkay = 4
nrrdMeasureHistoMedian = 20
tenDwiFiberType12BlendEvec0 = 3
tenGageConfDiffusionAlign = 142
miteValWdotD = 19
gageSclHessEvec2 = 16
nrrdField_axis_mins = 12
tenAniso_Th = 23
nrrdEncodingTypeUnknown = 0
tenAniso_Det = 25
gageVecNCurlNormGrad = 19
nrrdField_unknown = 0
gageVecVector = 1
nrrdBasicInfoSampleUnits = 6
tenFiberTypeEvec0 = 1
limnQN16checker = 3
limnEdgeTypeLone = 7
miteRangeGreen = 2
echoJittableLight = 1
nrrdAxisInfoCenter = 7
nrrdTernaryOpMultiply = 2
nrrdCenterLast = 3
gageSclGradMag = 3
limnQN10octa = 13
echoJittableLens = 2
nrrdIoStateDetachedHeader = 1
nrrdSpaceRightAnteriorSuperiorTime = 4
gageVecImaginaryPart = 14
nrrdMeasureLineSlope = 14
tenDwiGageTensorMLELikelihood = 21
nrrdTypeUnknown = 0
tenGageDelNormPhi1 = 27
tenDwiGageLast = 36
nrrdBinaryOpGT = 14
tenTripleTypeK = 7
gageSclK1 = 19
tenGlyphTypeUnknown = 0
miteValVrefN = 15
nrrdMeasureHistoMode = 21
tenTripleTypeJ = 6
echoTypeTriangle = 4
echoMatterPhongKs = 2
gagePvlFlagQuery = 2
gageSclK2 = 20
tenGlyphTypeCylinder = 3
nrrdSpacingStatusUnknown = 0
tenGageFAGradMag = 52
airNoDio_fd = 4
gageErrNone = 1
echoTypeSuperquad = 2
tenGageCovarianceRGRT = 145
tenGageFA2ndDD = 100
gageSclUnknown = 0
tenTripleTypeXYZ = 3
tenGageEvalGrads = 72
ell_cubic_root_triple = 2
tenGageFAGeomTens = 101
miteStageOpUnknown = 0
miteValRw = 7
limnSplineInfo2Vector = 2
nrrdAxisInfoSize = 1
hooverErrThreadEnd = 8
gageKernelStack = 7
tenTripleTypeRThetaZ = 4
nrrdKindPoint = 5
limnSplineTypeLinear = 1
nrrdUnaryOpAbs = 21
nrrdKind3DMatrix = 30
tenGageModeHessianEvec = 117
nrrdMeasureMedian = 4
dyeSpaceHSV = 1
nrrdKindQuaternion = 23
tenDwiGageTensorLLSError = 7
nrrdBinaryOpMin = 10
nrrdField_space_directions = 14
nrrdEncodingTypeHex = 3
tenAniso_eval0 = 27
nrrdMeasureVariance = 11
nrrdBlind8BitRangeFalse = 2
gageParmStackRenormalize = 13
nrrdFormatTypeLast = 7
gageVecGradient1 = 24
nrrdField_axis_maxs = 13
limnEdgeTypeContour = 3
nrrdFormatTypePNM = 2
gageVecDirHelDeriv = 21
airInsane_endian = 1
echoJitterGrid = 1
nrrdSpaceLast = 13
airInsane_32Bit = 9
airInsane_nInfExists = 3
tenDwiFiberTypeLast = 4
tenAniso_B = 17
tenDwiGageTensor = 22
tenGageDelNormK3 = 24
airInsane_DLSize = 12
miteShadeMethodUnknown = 0
tenFiberTypePureLine = 5
tenGageDetNormal = 44
gageSclHessEval1 = 11
tenGageOmegaHessianEval = 122
miteRangeKa = 5
nrrdTypeUShort = 4
tenGageFAValleySurfaceStrength = 97
tenInterpTypeLoxR = 8
airFP_NEG_ZERO = 10
miteStageOpMax = 2
nrrdUnaryOpExp = 9
tenGageBNormal = 41
gageParmRequireAllSpacings = 9
hooverErrThreadBegin = 4
gageKernel11 = 3
nrrdTypeLast = 12
nrrdBinaryOpMax = 11
nrrdField_space_dimension = 8
echoJitterUnknown = -1
nrrdBoundaryPad = 1
limnDevicePS = 1
gageSclCurvDir1 = 26
nrrdOriginStatusLast = 5
tenGageModeHessianEval0 = 114
gageVecCurlNormGrad = 18
nrrdKind3Color = 13
nrrdZlibStrategyHuffman = 2
limnSplineTypeUnknown = 0
limnPrimitiveTriangleFan = 4
tenDwiGageJustDWI = 3
nrrdKind4Color = 17
tenFiberIntgUnknown = 0
nrrdBinaryOpEqual = 17
tenGageTrace = 3
nrrdBoundaryWeight = 4
nrrdMeasureL2 = 9
miteRangeUnknown = -1
nrrdField_encoding = 24
echoJittableUnknown = -1
tenFiberIntgMidpoint = 2
nrrdAxisInfoLast = 11
nrrdMeasureHistoL2 = 24
tenAniso_Cs1 = 6
airTypeLast = 12
nrrdBinaryOpPow = 5
tenTripleTypeWheelParm = 9
tenGageEvec1 = 21
tenEstimate1MethodLLS = 1
tenFiberParmVerbose = 4
tenFiberIntgLast = 4
miteValNdotL = 14
tenAniso_Ct2 = 13
tenGageFAHessianEvec1 = 94
tenFiberStopAniso = 1
nrrdSpaceScannerXYZ = 7
airTypeChar = 8
tenGageDetGradVec = 42
gageKernel21 = 5
tenGageNormNormal = 38
limnQN12octa = 10
tenFiberParmUseIndexSpace = 2
nrrdMeasureLineError = 16
tenDwiFiberType2Evec0 = 2
airLLong = c_longlong
airULLong = c_ulonglong
class airEnum(Structure):
    pass
airEnum._fields_ = [
    ('name', c_char * 129),
    ('M', c_uint),
    ('str', POINTER(c_char * 129)),
    ('val', POINTER(c_int)),
    ('desc', POINTER(c_char * 257)),
    ('strEqv', POINTER(c_char * 129)),
    ('valEqv', POINTER(c_int)),
    ('sense', c_int),
]
airEnumUnknown = _libraries[libteem].airEnumUnknown
airEnumUnknown.restype = c_int
airEnumUnknown.argtypes = [POINTER(airEnum)]
airEnumLast = _libraries[libteem].airEnumLast
airEnumLast.restype = c_int
airEnumLast.argtypes = [POINTER(airEnum)]
airEnumValCheck = _libraries[libteem].airEnumValCheck
airEnumValCheck.restype = c_int
airEnumValCheck.argtypes = [POINTER(airEnum), c_int]
airEnumStr = _libraries[libteem].airEnumStr
airEnumStr.restype = STRING
airEnumStr.argtypes = [POINTER(airEnum), c_int]
airEnumDesc = _libraries[libteem].airEnumDesc
airEnumDesc.restype = STRING
airEnumDesc.argtypes = [POINTER(airEnum), c_int]
airEnumVal = _libraries[libteem].airEnumVal
airEnumVal.restype = c_int
airEnumVal.argtypes = [POINTER(airEnum), STRING]
airEnumFmtDesc = _libraries[libteem].airEnumFmtDesc
airEnumFmtDesc.restype = STRING
airEnumFmtDesc.argtypes = [POINTER(airEnum), c_int, c_int, STRING]
airEndian = (POINTER(airEnum)).in_dll(_libraries[libteem], 'airEndian')
airMyEndian = (c_int).in_dll(_libraries[libteem], 'airMyEndian')
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
airArrayNew = _libraries[libteem].airArrayNew
airArrayNew.restype = POINTER(airArray)
airArrayNew.argtypes = [POINTER(c_void_p), POINTER(c_uint), size_t, c_uint]
airArrayStructCB = _libraries[libteem].airArrayStructCB
airArrayStructCB.restype = None
airArrayStructCB.argtypes = [POINTER(airArray), CFUNCTYPE(None, c_void_p), CFUNCTYPE(None, c_void_p)]
airArrayPointerCB = _libraries[libteem].airArrayPointerCB
airArrayPointerCB.restype = None
airArrayPointerCB.argtypes = [POINTER(airArray), CFUNCTYPE(c_void_p), CFUNCTYPE(c_void_p, c_void_p)]
airArrayLenSet = _libraries[libteem].airArrayLenSet
airArrayLenSet.restype = None
airArrayLenSet.argtypes = [POINTER(airArray), c_uint]
airArrayLenPreSet = _libraries[libteem].airArrayLenPreSet
airArrayLenPreSet.restype = None
airArrayLenPreSet.argtypes = [POINTER(airArray), c_uint]
airArrayLenIncr = _libraries[libteem].airArrayLenIncr
airArrayLenIncr.restype = c_uint
airArrayLenIncr.argtypes = [POINTER(airArray), c_int]
airArrayNix = _libraries[libteem].airArrayNix
airArrayNix.restype = POINTER(airArray)
airArrayNix.argtypes = [POINTER(airArray)]
airArrayNuke = _libraries[libteem].airArrayNuke
airArrayNuke.restype = POINTER(airArray)
airArrayNuke.argtypes = [POINTER(airArray)]
airThreadCapable = (c_int).in_dll(_libraries[libteem], 'airThreadCapable')
airThreadNoopWarning = (c_int).in_dll(_libraries[libteem], 'airThreadNoopWarning')
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
airThreadNew = _libraries[libteem].airThreadNew
airThreadNew.restype = POINTER(airThread)
airThreadNew.argtypes = []
airThreadStart = _libraries[libteem].airThreadStart
airThreadStart.restype = c_int
airThreadStart.argtypes = [POINTER(airThread), CFUNCTYPE(c_void_p, c_void_p), c_void_p]
airThreadJoin = _libraries[libteem].airThreadJoin
airThreadJoin.restype = c_int
airThreadJoin.argtypes = [POINTER(airThread), POINTER(c_void_p)]
airThreadNix = _libraries[libteem].airThreadNix
airThreadNix.restype = POINTER(airThread)
airThreadNix.argtypes = [POINTER(airThread)]
airThreadMutexNew = _libraries[libteem].airThreadMutexNew
airThreadMutexNew.restype = POINTER(airThreadMutex)
airThreadMutexNew.argtypes = []
airThreadMutexLock = _libraries[libteem].airThreadMutexLock
airThreadMutexLock.restype = c_int
airThreadMutexLock.argtypes = [POINTER(airThreadMutex)]
airThreadMutexUnlock = _libraries[libteem].airThreadMutexUnlock
airThreadMutexUnlock.restype = c_int
airThreadMutexUnlock.argtypes = [POINTER(airThreadMutex)]
airThreadMutexNix = _libraries[libteem].airThreadMutexNix
airThreadMutexNix.restype = POINTER(airThreadMutex)
airThreadMutexNix.argtypes = [POINTER(airThreadMutex)]
airThreadCondNew = _libraries[libteem].airThreadCondNew
airThreadCondNew.restype = POINTER(airThreadCond)
airThreadCondNew.argtypes = []
airThreadCondWait = _libraries[libteem].airThreadCondWait
airThreadCondWait.restype = c_int
airThreadCondWait.argtypes = [POINTER(airThreadCond), POINTER(airThreadMutex)]
airThreadCondSignal = _libraries[libteem].airThreadCondSignal
airThreadCondSignal.restype = c_int
airThreadCondSignal.argtypes = [POINTER(airThreadCond)]
airThreadCondBroadcast = _libraries[libteem].airThreadCondBroadcast
airThreadCondBroadcast.restype = c_int
airThreadCondBroadcast.argtypes = [POINTER(airThreadCond)]
airThreadCondNix = _libraries[libteem].airThreadCondNix
airThreadCondNix.restype = POINTER(airThreadCond)
airThreadCondNix.argtypes = [POINTER(airThreadCond)]
airThreadBarrierNew = _libraries[libteem].airThreadBarrierNew
airThreadBarrierNew.restype = POINTER(airThreadBarrier)
airThreadBarrierNew.argtypes = [c_uint]
airThreadBarrierWait = _libraries[libteem].airThreadBarrierWait
airThreadBarrierWait.restype = c_int
airThreadBarrierWait.argtypes = [POINTER(airThreadBarrier)]
airThreadBarrierNix = _libraries[libteem].airThreadBarrierNix
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
airDouble._fields_ = [
    ('i', airULLong),
    ('d', c_double),
]
airMyQNaNHiBit = (c_int).in_dll(_libraries[libteem], 'airMyQNaNHiBit')
airFPPartsToVal_f = _libraries[libteem].airFPPartsToVal_f
airFPPartsToVal_f.restype = c_float
airFPPartsToVal_f.argtypes = [c_uint, c_uint, c_uint]
airFPValToParts_f = _libraries[libteem].airFPValToParts_f
airFPValToParts_f.restype = None
airFPValToParts_f.argtypes = [POINTER(c_uint), POINTER(c_uint), POINTER(c_uint), c_float]
airFPPartsToVal_d = _libraries[libteem].airFPPartsToVal_d
airFPPartsToVal_d.restype = c_double
airFPPartsToVal_d.argtypes = [c_uint, c_uint, c_uint, c_uint]
airFPValToParts_d = _libraries[libteem].airFPValToParts_d
airFPValToParts_d.restype = None
airFPValToParts_d.argtypes = [POINTER(c_uint), POINTER(c_uint), POINTER(c_uint), POINTER(c_uint), c_double]
airFPGen_f = _libraries[libteem].airFPGen_f
airFPGen_f.restype = c_float
airFPGen_f.argtypes = [c_int]
airFPGen_d = _libraries[libteem].airFPGen_d
airFPGen_d.restype = c_double
airFPGen_d.argtypes = [c_int]
airFPClass_f = _libraries[libteem].airFPClass_f
airFPClass_f.restype = c_int
airFPClass_f.argtypes = [c_float]
airFPClass_d = _libraries[libteem].airFPClass_d
airFPClass_d.restype = c_int
airFPClass_d.argtypes = [c_double]
airFPFprintf_f = _libraries[libteem].airFPFprintf_f
airFPFprintf_f.restype = None
airFPFprintf_f.argtypes = [POINTER(FILE), c_float]
airFPFprintf_d = _libraries[libteem].airFPFprintf_d
airFPFprintf_d.restype = None
airFPFprintf_d.argtypes = [POINTER(FILE), c_double]
airFloatQNaN = (airFloat).in_dll(_libraries[libteem], 'airFloatQNaN')
airFloatSNaN = (airFloat).in_dll(_libraries[libteem], 'airFloatSNaN')
airFloatPosInf = (airFloat).in_dll(_libraries[libteem], 'airFloatPosInf')
airFloatNegInf = (airFloat).in_dll(_libraries[libteem], 'airFloatNegInf')
airNaN = _libraries[libteem].airNaN
airNaN.restype = c_float
airNaN.argtypes = []
airIsNaN = _libraries[libteem].airIsNaN
airIsNaN.restype = c_int
airIsNaN.argtypes = [c_double]
airIsInf_f = _libraries[libteem].airIsInf_f
airIsInf_f.restype = c_int
airIsInf_f.argtypes = [c_float]
airIsInf_d = _libraries[libteem].airIsInf_d
airIsInf_d.restype = c_int
airIsInf_d.argtypes = [c_double]
airExists = _libraries[libteem].airExists
airExists.restype = c_int
airExists.argtypes = [c_double]
class airRandMTState(Structure):
    pass
airRandMTState._fields_ = [
    ('state', c_uint * 624),
    ('pNext', POINTER(c_uint)),
    ('left', c_uint),
]
airRandMTStateGlobal = (POINTER(airRandMTState)).in_dll(_libraries[libteem], 'airRandMTStateGlobal')
airRandMTStateNew = _libraries[libteem].airRandMTStateNew
airRandMTStateNew.restype = POINTER(airRandMTState)
airRandMTStateNew.argtypes = [c_uint]
airRandMTStateNix = _libraries[libteem].airRandMTStateNix
airRandMTStateNix.restype = POINTER(airRandMTState)
airRandMTStateNix.argtypes = [POINTER(airRandMTState)]
airSrandMT_r = _libraries[libteem].airSrandMT_r
airSrandMT_r.restype = None
airSrandMT_r.argtypes = [POINTER(airRandMTState), c_uint]
airDrandMT_r = _libraries[libteem].airDrandMT_r
airDrandMT_r.restype = c_double
airDrandMT_r.argtypes = [POINTER(airRandMTState)]
airUIrandMT_r = _libraries[libteem].airUIrandMT_r
airUIrandMT_r.restype = c_uint
airUIrandMT_r.argtypes = [POINTER(airRandMTState)]
airDrandMT53_r = _libraries[libteem].airDrandMT53_r
airDrandMT53_r.restype = c_double
airDrandMT53_r.argtypes = [POINTER(airRandMTState)]
airRandInt = _libraries[libteem].airRandInt
airRandInt.restype = c_uint
airRandInt.argtypes = [c_uint]
airRandInt_r = _libraries[libteem].airRandInt_r
airRandInt_r.restype = c_uint
airRandInt_r.argtypes = [POINTER(airRandMTState), c_uint]
airSrandMT = _libraries[libteem].airSrandMT
airSrandMT.restype = None
airSrandMT.argtypes = [c_uint]
airDrandMT = _libraries[libteem].airDrandMT
airDrandMT.restype = c_double
airDrandMT.argtypes = []
airAtod = _libraries[libteem].airAtod
airAtod.restype = c_double
airAtod.argtypes = [STRING]
airSingleSscanf = _libraries[libteem].airSingleSscanf
airSingleSscanf.restype = c_int
airSingleSscanf.argtypes = [STRING, STRING, c_void_p]
airBool = (POINTER(airEnum)).in_dll(_libraries[libteem], 'airBool')
airParseStrB = _libraries[libteem].airParseStrB
airParseStrB.restype = c_uint
airParseStrB.argtypes = [POINTER(c_int), STRING, STRING, c_uint]
airParseStrI = _libraries[libteem].airParseStrI
airParseStrI.restype = c_uint
airParseStrI.argtypes = [POINTER(c_int), STRING, STRING, c_uint]
airParseStrUI = _libraries[libteem].airParseStrUI
airParseStrUI.restype = c_uint
airParseStrUI.argtypes = [POINTER(c_uint), STRING, STRING, c_uint]
airParseStrZ = _libraries[libteem].airParseStrZ
airParseStrZ.restype = c_uint
airParseStrZ.argtypes = [POINTER(size_t), STRING, STRING, c_uint]
airParseStrF = _libraries[libteem].airParseStrF
airParseStrF.restype = c_uint
airParseStrF.argtypes = [POINTER(c_float), STRING, STRING, c_uint]
airParseStrD = _libraries[libteem].airParseStrD
airParseStrD.restype = c_uint
airParseStrD.argtypes = [POINTER(c_double), STRING, STRING, c_uint]
airParseStrC = _libraries[libteem].airParseStrC
airParseStrC.restype = c_uint
airParseStrC.argtypes = [STRING, STRING, STRING, c_uint]
airParseStrS = _libraries[libteem].airParseStrS
airParseStrS.restype = c_uint
airParseStrS.argtypes = [POINTER(STRING), STRING, STRING, c_uint]
airParseStrE = _libraries[libteem].airParseStrE
airParseStrE.restype = c_uint
airParseStrE.argtypes = [POINTER(c_int), STRING, STRING, c_uint]
airParseStr = (CFUNCTYPE(c_uint, c_void_p, STRING, STRING, c_uint) * 12).in_dll(_libraries[libteem], 'airParseStr')
airStrdup = _libraries[libteem].airStrdup
airStrdup.restype = STRING
airStrdup.argtypes = [STRING]
airStrlen = _libraries[libteem].airStrlen
airStrlen.restype = size_t
airStrlen.argtypes = [STRING]
airStrtokQuoting = (c_int).in_dll(_libraries[libteem], 'airStrtokQuoting')
airStrtok = _libraries[libteem].airStrtok
airStrtok.restype = STRING
airStrtok.argtypes = [STRING, STRING, POINTER(STRING)]
airStrntok = _libraries[libteem].airStrntok
airStrntok.restype = c_uint
airStrntok.argtypes = [STRING, STRING]
airStrtrans = _libraries[libteem].airStrtrans
airStrtrans.restype = STRING
airStrtrans.argtypes = [STRING, c_char, c_char]
airEndsWith = _libraries[libteem].airEndsWith
airEndsWith.restype = c_int
airEndsWith.argtypes = [STRING, STRING]
airUnescape = _libraries[libteem].airUnescape
airUnescape.restype = STRING
airUnescape.argtypes = [STRING]
airOneLinify = _libraries[libteem].airOneLinify
airOneLinify.restype = STRING
airOneLinify.argtypes = [STRING]
airToLower = _libraries[libteem].airToLower
airToLower.restype = STRING
airToLower.argtypes = [STRING]
airToUpper = _libraries[libteem].airToUpper
airToUpper.restype = STRING
airToUpper.argtypes = [STRING]
airOneLine = _libraries[libteem].airOneLine
airOneLine.restype = c_uint
airOneLine.argtypes = [POINTER(FILE), STRING, c_int]
airInsaneErr = _libraries[libteem].airInsaneErr
airInsaneErr.restype = STRING
airInsaneErr.argtypes = [c_int]
airSanity = _libraries[libteem].airSanity
airSanity.restype = c_int
airSanity.argtypes = []
airTeemVersion = (STRING).in_dll(_libraries[libteem], 'airTeemVersion')
airTeemReleaseDate = (STRING).in_dll(_libraries[libteem], 'airTeemReleaseDate')
airNull = _libraries[libteem].airNull
airNull.restype = c_void_p
airNull.argtypes = []
airSetNull = _libraries[libteem].airSetNull
airSetNull.restype = c_void_p
airSetNull.argtypes = [POINTER(c_void_p)]
airFree = _libraries[libteem].airFree
airFree.restype = c_void_p
airFree.argtypes = [c_void_p]
airFopen = _libraries[libteem].airFopen
airFopen.restype = POINTER(FILE)
airFopen.argtypes = [STRING, POINTER(FILE), STRING]
airFclose = _libraries[libteem].airFclose
airFclose.restype = POINTER(FILE)
airFclose.argtypes = [POINTER(FILE)]
airSinglePrintf = _libraries[libteem].airSinglePrintf
airSinglePrintf.restype = c_int
airSinglePrintf.argtypes = [POINTER(FILE), STRING, STRING]
airMy32Bit = (c_int).in_dll(_libraries[libteem], 'airMy32Bit')
airIndex = _libraries[libteem].airIndex
airIndex.restype = c_uint
airIndex.argtypes = [c_double, c_double, c_double, c_uint]
airIndexClamp = _libraries[libteem].airIndexClamp
airIndexClamp.restype = c_uint
airIndexClamp.argtypes = [c_double, c_double, c_double, c_uint]
airIndexULL = _libraries[libteem].airIndexULL
airIndexULL.restype = airULLong
airIndexULL.argtypes = [c_double, c_double, c_double, airULLong]
airIndexClampULL = _libraries[libteem].airIndexClampULL
airIndexClampULL.restype = airULLong
airIndexClampULL.argtypes = [c_double, c_double, c_double, airULLong]
airMyFmt_size_t = (c_char * 0).in_dll(_libraries[libteem], 'airMyFmt_size_t')
airDoneStr = _libraries[libteem].airDoneStr
airDoneStr.restype = STRING
airDoneStr.argtypes = [c_double, c_double, c_double, STRING]
airTime = _libraries[libteem].airTime
airTime.restype = c_double
airTime.argtypes = []
airBinaryPrintUInt = _libraries[libteem].airBinaryPrintUInt
airBinaryPrintUInt.restype = None
airBinaryPrintUInt.argtypes = [POINTER(FILE), c_int, c_uint]
airTypeStr = (c_char * 129 * 12).in_dll(_libraries[libteem], 'airTypeStr')
airTypeSize = (size_t * 12).in_dll(_libraries[libteem], 'airTypeSize')
airILoad = _libraries[libteem].airILoad
airILoad.restype = c_int
airILoad.argtypes = [c_void_p, c_int]
airFLoad = _libraries[libteem].airFLoad
airFLoad.restype = c_float
airFLoad.argtypes = [c_void_p, c_int]
airDLoad = _libraries[libteem].airDLoad
airDLoad.restype = c_double
airDLoad.argtypes = [c_void_p, c_int]
airIStore = _libraries[libteem].airIStore
airIStore.restype = c_int
airIStore.argtypes = [c_void_p, c_int, c_int]
airFStore = _libraries[libteem].airFStore
airFStore.restype = c_float
airFStore.argtypes = [c_void_p, c_int, c_float]
airDStore = _libraries[libteem].airDStore
airDStore.restype = c_double
airDStore.argtypes = [c_void_p, c_int, c_double]
airEqvAdd = _libraries[libteem].airEqvAdd
airEqvAdd.restype = None
airEqvAdd.argtypes = [POINTER(airArray), c_uint, c_uint]
airEqvMap = _libraries[libteem].airEqvMap
airEqvMap.restype = c_uint
airEqvMap.argtypes = [POINTER(airArray), POINTER(c_uint), c_uint]
airEqvSettle = _libraries[libteem].airEqvSettle
airEqvSettle.restype = c_uint
airEqvSettle.argtypes = [POINTER(c_uint), c_uint]
airFastExp = _libraries[libteem].airFastExp
airFastExp.restype = c_double
airFastExp.argtypes = [c_double]
airNormalRand = _libraries[libteem].airNormalRand
airNormalRand.restype = None
airNormalRand.argtypes = [POINTER(c_double), POINTER(c_double)]
airNormalRand_r = _libraries[libteem].airNormalRand_r
airNormalRand_r.restype = None
airNormalRand_r.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(airRandMTState)]
airShuffle = _libraries[libteem].airShuffle
airShuffle.restype = None
airShuffle.argtypes = [POINTER(c_uint), c_uint, c_int]
airShuffle_r = _libraries[libteem].airShuffle_r
airShuffle_r.restype = None
airShuffle_r.argtypes = [POINTER(airRandMTState), POINTER(c_uint), c_uint, c_int]
airCbrt = _libraries[libteem].airCbrt
airCbrt.restype = c_double
airCbrt.argtypes = [c_double]
airMode3 = _libraries[libteem].airMode3
airMode3.restype = c_double
airMode3.argtypes = [c_double, c_double, c_double]
airMode3_d = _libraries[libteem].airMode3_d
airMode3_d.restype = c_double
airMode3_d.argtypes = [POINTER(c_double)]
airSgnPow = _libraries[libteem].airSgnPow
airSgnPow.restype = c_double
airSgnPow.argtypes = [c_double, c_double]
airIntPow = _libraries[libteem].airIntPow
airIntPow.restype = c_double
airIntPow.argtypes = [c_double, c_int]
airSgn = _libraries[libteem].airSgn
airSgn.restype = c_int
airSgn.argtypes = [c_double]
airLog2 = _libraries[libteem].airLog2
airLog2.restype = c_int
airLog2.argtypes = [c_double]
airErfc = _libraries[libteem].airErfc
airErfc.restype = c_double
airErfc.argtypes = [c_double]
airErf = _libraries[libteem].airErf
airErf.restype = c_double
airErf.argtypes = [c_double]
airGaussian = _libraries[libteem].airGaussian
airGaussian.restype = c_double
airGaussian.argtypes = [c_double, c_double, c_double]
airBesselI0 = _libraries[libteem].airBesselI0
airBesselI0.restype = c_double
airBesselI0.argtypes = [c_double]
airBesselI1 = _libraries[libteem].airBesselI1
airBesselI1.restype = c_double
airBesselI1.argtypes = [c_double]
airBesselI0ExpScaled = _libraries[libteem].airBesselI0ExpScaled
airBesselI0ExpScaled.restype = c_double
airBesselI0ExpScaled.argtypes = [c_double]
airBesselI1ExpScaled = _libraries[libteem].airBesselI1ExpScaled
airBesselI1ExpScaled.restype = c_double
airBesselI1ExpScaled.argtypes = [c_double]
airLogBesselI0 = _libraries[libteem].airLogBesselI0
airLogBesselI0.restype = c_double
airLogBesselI0.argtypes = [c_double]
airBesselI1By0 = _libraries[libteem].airBesselI1By0
airBesselI1By0.restype = c_double
airBesselI1By0.argtypes = [c_double]
airBesselIn = _libraries[libteem].airBesselIn
airBesselIn.restype = c_double
airBesselIn.argtypes = [c_int, c_double]
airBesselInExpScaled = _libraries[libteem].airBesselInExpScaled
airBesselInExpScaled.restype = c_double
airBesselInExpScaled.argtypes = [c_int, c_double]
airNoDioErr = _libraries[libteem].airNoDioErr
airNoDioErr.restype = STRING
airNoDioErr.argtypes = [c_int]
airMyDio = (c_int).in_dll(_libraries[libteem], 'airMyDio')
airDisableDio = (c_int).in_dll(_libraries[libteem], 'airDisableDio')
airDioInfo = _libraries[libteem].airDioInfo
airDioInfo.restype = None
airDioInfo.argtypes = [POINTER(c_int), POINTER(c_int), POINTER(c_int), c_int]
airDioTest = _libraries[libteem].airDioTest
airDioTest.restype = c_int
airDioTest.argtypes = [c_int, c_void_p, size_t]
airDioMalloc = _libraries[libteem].airDioMalloc
airDioMalloc.restype = c_void_p
airDioMalloc.argtypes = [size_t, c_int]
airDioRead = _libraries[libteem].airDioRead
airDioRead.restype = size_t
airDioRead.argtypes = [c_int, c_void_p, size_t]
airDioWrite = _libraries[libteem].airDioWrite
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
airMopNew = _libraries[libteem].airMopNew
airMopNew.restype = POINTER(airArray)
airMopNew.argtypes = []
airMopAdd = _libraries[libteem].airMopAdd
airMopAdd.restype = None
airMopAdd.argtypes = [POINTER(airArray), c_void_p, airMopper, c_int]
airMopSub = _libraries[libteem].airMopSub
airMopSub.restype = None
airMopSub.argtypes = [POINTER(airArray), c_void_p, airMopper]
airMopMem = _libraries[libteem].airMopMem
airMopMem.restype = None
airMopMem.argtypes = [POINTER(airArray), c_void_p, c_int]
airMopUnMem = _libraries[libteem].airMopUnMem
airMopUnMem.restype = None
airMopUnMem.argtypes = [POINTER(airArray), c_void_p]
airMopPrint = _libraries[libteem].airMopPrint
airMopPrint.restype = None
airMopPrint.argtypes = [POINTER(airArray), c_void_p, c_int]
airMopDone = _libraries[libteem].airMopDone
airMopDone.restype = None
airMopDone.argtypes = [POINTER(airArray), c_int]
airMopError = _libraries[libteem].airMopError
airMopError.restype = None
airMopError.argtypes = [POINTER(airArray)]
airMopOkay = _libraries[libteem].airMopOkay
airMopOkay.restype = None
airMopOkay.argtypes = [POINTER(airArray)]
airMopDebug = _libraries[libteem].airMopDebug
airMopDebug.restype = None
airMopDebug.argtypes = [POINTER(airArray)]
biffAdd = _libraries[libteem].biffAdd
biffAdd.restype = None
biffAdd.argtypes = [STRING, STRING]
biffMaybeAdd = _libraries[libteem].biffMaybeAdd
biffMaybeAdd.restype = None
biffMaybeAdd.argtypes = [STRING, STRING, c_int]
biffCheck = _libraries[libteem].biffCheck
biffCheck.restype = c_int
biffCheck.argtypes = [STRING]
biffDone = _libraries[libteem].biffDone
biffDone.restype = None
biffDone.argtypes = [STRING]
biffMove = _libraries[libteem].biffMove
biffMove.restype = None
biffMove.argtypes = [STRING, STRING, STRING]
biffGet = _libraries[libteem].biffGet
biffGet.restype = STRING
biffGet.argtypes = [STRING]
biffGetStrlen = _libraries[libteem].biffGetStrlen
biffGetStrlen.restype = c_int
biffGetStrlen.argtypes = [STRING]
biffSetStr = _libraries[libteem].biffSetStr
biffSetStr.restype = None
biffSetStr.argtypes = [STRING, STRING]
biffGetDone = _libraries[libteem].biffGetDone
biffGetDone.restype = STRING
biffGetDone.argtypes = [STRING]
biffSetStrDone = _libraries[libteem].biffSetStrDone
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
dyeBiffKey = (STRING).in_dll(_libraries[libteem], 'dyeBiffKey')
dyeSpaceToStr = (c_char * 129 * 0).in_dll(_libraries[libteem], 'dyeSpaceToStr')
dyeStrToSpace = _libraries[libteem].dyeStrToSpace
dyeStrToSpace.restype = c_int
dyeStrToSpace.argtypes = [STRING]
dyeColorInit = _libraries[libteem].dyeColorInit
dyeColorInit.restype = POINTER(dyeColor)
dyeColorInit.argtypes = [POINTER(dyeColor)]
dyeColorSet = _libraries[libteem].dyeColorSet
dyeColorSet.restype = POINTER(dyeColor)
dyeColorSet.argtypes = [POINTER(dyeColor), c_int, c_float, c_float, c_float]
dyeColorGet = _libraries[libteem].dyeColorGet
dyeColorGet.restype = c_int
dyeColorGet.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), POINTER(dyeColor)]
dyeColorGetAs = _libraries[libteem].dyeColorGetAs
dyeColorGetAs.restype = c_int
dyeColorGetAs.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), POINTER(dyeColor), c_int]
dyeColorNew = _libraries[libteem].dyeColorNew
dyeColorNew.restype = POINTER(dyeColor)
dyeColorNew.argtypes = []
dyeColorCopy = _libraries[libteem].dyeColorCopy
dyeColorCopy.restype = POINTER(dyeColor)
dyeColorCopy.argtypes = [POINTER(dyeColor), POINTER(dyeColor)]
dyeColorNix = _libraries[libteem].dyeColorNix
dyeColorNix.restype = POINTER(dyeColor)
dyeColorNix.argtypes = [POINTER(dyeColor)]
dyeColorParse = _libraries[libteem].dyeColorParse
dyeColorParse.restype = c_int
dyeColorParse.argtypes = [POINTER(dyeColor), STRING]
dyeColorSprintf = _libraries[libteem].dyeColorSprintf
dyeColorSprintf.restype = STRING
dyeColorSprintf.argtypes = [STRING, POINTER(dyeColor)]
dyeConverter = CFUNCTYPE(None, POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float)
dyeRGBtoHSV = _libraries[libteem].dyeRGBtoHSV
dyeRGBtoHSV.restype = None
dyeRGBtoHSV.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeHSVtoRGB = _libraries[libteem].dyeHSVtoRGB
dyeHSVtoRGB.restype = None
dyeHSVtoRGB.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeRGBtoHSL = _libraries[libteem].dyeRGBtoHSL
dyeRGBtoHSL.restype = None
dyeRGBtoHSL.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeHSLtoRGB = _libraries[libteem].dyeHSLtoRGB
dyeHSLtoRGB.restype = None
dyeHSLtoRGB.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeRGBtoXYZ = _libraries[libteem].dyeRGBtoXYZ
dyeRGBtoXYZ.restype = None
dyeRGBtoXYZ.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeXYZtoRGB = _libraries[libteem].dyeXYZtoRGB
dyeXYZtoRGB.restype = None
dyeXYZtoRGB.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeXYZtoLAB = _libraries[libteem].dyeXYZtoLAB
dyeXYZtoLAB.restype = None
dyeXYZtoLAB.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeXYZtoLUV = _libraries[libteem].dyeXYZtoLUV
dyeXYZtoLUV.restype = None
dyeXYZtoLUV.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeLABtoXYZ = _libraries[libteem].dyeLABtoXYZ
dyeLABtoXYZ.restype = None
dyeLABtoXYZ.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeLUVtoXYZ = _libraries[libteem].dyeLUVtoXYZ
dyeLUVtoXYZ.restype = None
dyeLUVtoXYZ.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), c_float, c_float, c_float]
dyeSimpleConvert = (dyeConverter * 7 * 7).in_dll(_libraries[libteem], 'dyeSimpleConvert')
dyeConvert = _libraries[libteem].dyeConvert
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
echoJitter = (POINTER(airEnum)).in_dll(_libraries[libteem], 'echoJitter')
echoType = (POINTER(airEnum)).in_dll(_libraries[libteem], 'echoType')
echoMatter = (POINTER(airEnum)).in_dll(_libraries[libteem], 'echoMatter')
echoBiffKey = (STRING).in_dll(_libraries[libteem], 'echoBiffKey')
echoRTParmNew = _libraries[libteem].echoRTParmNew
echoRTParmNew.restype = POINTER(echoRTParm)
echoRTParmNew.argtypes = []
echoRTParmNix = _libraries[libteem].echoRTParmNix
echoRTParmNix.restype = POINTER(echoRTParm)
echoRTParmNix.argtypes = [POINTER(echoRTParm)]
echoGlobalStateNew = _libraries[libteem].echoGlobalStateNew
echoGlobalStateNew.restype = POINTER(echoGlobalState)
echoGlobalStateNew.argtypes = []
echoGlobalStateNix = _libraries[libteem].echoGlobalStateNix
echoGlobalStateNix.restype = POINTER(echoGlobalState)
echoGlobalStateNix.argtypes = [POINTER(echoGlobalState)]
echoThreadStateNew = _libraries[libteem].echoThreadStateNew
echoThreadStateNew.restype = POINTER(echoThreadState)
echoThreadStateNew.argtypes = []
echoThreadStateNix = _libraries[libteem].echoThreadStateNix
echoThreadStateNix.restype = POINTER(echoThreadState)
echoThreadStateNix.argtypes = [POINTER(echoThreadState)]
echoSceneNew = _libraries[libteem].echoSceneNew
echoSceneNew.restype = POINTER(echoScene)
echoSceneNew.argtypes = []
echoSceneNix = _libraries[libteem].echoSceneNix
echoSceneNix.restype = POINTER(echoScene)
echoSceneNix.argtypes = [POINTER(echoScene)]
echoObjectNew = _libraries[libteem].echoObjectNew
echoObjectNew.restype = POINTER(echoObject)
echoObjectNew.argtypes = [POINTER(echoScene), c_byte]
echoObjectAdd = _libraries[libteem].echoObjectAdd
echoObjectAdd.restype = c_int
echoObjectAdd.argtypes = [POINTER(echoScene), POINTER(echoObject)]
echoObjectNix = _libraries[libteem].echoObjectNix
echoObjectNix.restype = POINTER(echoObject)
echoObjectNix.argtypes = [POINTER(echoObject)]
echoRoughSphereNew = _libraries[libteem].echoRoughSphereNew
echoRoughSphereNew.restype = POINTER(echoObject)
echoRoughSphereNew.argtypes = [POINTER(echoScene), c_int, c_int, POINTER(echoPos_t)]
echoBoundsGet = _libraries[libteem].echoBoundsGet
echoBoundsGet.restype = None
echoBoundsGet.argtypes = [POINTER(echoPos_t), POINTER(echoPos_t), POINTER(echoObject)]
echoListAdd = _libraries[libteem].echoListAdd
echoListAdd.restype = None
echoListAdd.argtypes = [POINTER(echoObject), POINTER(echoObject)]
echoListSplit = _libraries[libteem].echoListSplit
echoListSplit.restype = POINTER(echoObject)
echoListSplit.argtypes = [POINTER(echoScene), POINTER(echoObject), c_int]
echoListSplit3 = _libraries[libteem].echoListSplit3
echoListSplit3.restype = POINTER(echoObject)
echoListSplit3.argtypes = [POINTER(echoScene), POINTER(echoObject), c_int]
echoSphereSet = _libraries[libteem].echoSphereSet
echoSphereSet.restype = None
echoSphereSet.argtypes = [POINTER(echoObject), echoPos_t, echoPos_t, echoPos_t, echoPos_t]
echoCylinderSet = _libraries[libteem].echoCylinderSet
echoCylinderSet.restype = None
echoCylinderSet.argtypes = [POINTER(echoObject), c_int]
echoSuperquadSet = _libraries[libteem].echoSuperquadSet
echoSuperquadSet.restype = None
echoSuperquadSet.argtypes = [POINTER(echoObject), c_int, echoPos_t, echoPos_t]
echoRectangleSet = _libraries[libteem].echoRectangleSet
echoRectangleSet.restype = None
echoRectangleSet.argtypes = [POINTER(echoObject), echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t]
echoTriangleSet = _libraries[libteem].echoTriangleSet
echoTriangleSet.restype = None
echoTriangleSet.argtypes = [POINTER(echoObject), echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t, echoPos_t]
echoTriMeshSet = _libraries[libteem].echoTriMeshSet
echoTriMeshSet.restype = None
echoTriMeshSet.argtypes = [POINTER(echoObject), c_int, POINTER(echoPos_t), c_int, POINTER(c_int)]
echoInstanceSet = _libraries[libteem].echoInstanceSet
echoInstanceSet.restype = None
echoInstanceSet.argtypes = [POINTER(echoObject), POINTER(echoPos_t), POINTER(echoObject)]
echoObjectHasMatter = (c_int * 12).in_dll(_libraries[libteem], 'echoObjectHasMatter')
echoColorSet = _libraries[libteem].echoColorSet
echoColorSet.restype = None
echoColorSet.argtypes = [POINTER(echoObject), echoCol_t, echoCol_t, echoCol_t, echoCol_t]
echoMatterPhongSet = _libraries[libteem].echoMatterPhongSet
echoMatterPhongSet.restype = None
echoMatterPhongSet.argtypes = [POINTER(echoScene), POINTER(echoObject), echoCol_t, echoCol_t, echoCol_t, echoCol_t]
echoMatterGlassSet = _libraries[libteem].echoMatterGlassSet
echoMatterGlassSet.restype = None
echoMatterGlassSet.argtypes = [POINTER(echoScene), POINTER(echoObject), echoCol_t, echoCol_t, echoCol_t, echoCol_t]
echoMatterMetalSet = _libraries[libteem].echoMatterMetalSet
echoMatterMetalSet.restype = None
echoMatterMetalSet.argtypes = [POINTER(echoScene), POINTER(echoObject), echoCol_t, echoCol_t, echoCol_t, echoCol_t]
echoMatterLightSet = _libraries[libteem].echoMatterLightSet
echoMatterLightSet.restype = None
echoMatterLightSet.argtypes = [POINTER(echoScene), POINTER(echoObject), echoCol_t, echoCol_t]
echoMatterTextureSet = _libraries[libteem].echoMatterTextureSet
echoMatterTextureSet.restype = None
echoMatterTextureSet.argtypes = [POINTER(echoScene), POINTER(echoObject), POINTER(Nrrd)]
echoLightPosition = _libraries[libteem].echoLightPosition
echoLightPosition.restype = None
echoLightPosition.argtypes = [POINTER(echoPos_t), POINTER(echoObject), POINTER(echoThreadState)]
echoLightColor = _libraries[libteem].echoLightColor
echoLightColor.restype = None
echoLightColor.argtypes = [POINTER(echoCol_t), echoPos_t, POINTER(echoObject), POINTER(echoRTParm), POINTER(echoThreadState)]
echoEnvmapLookup = _libraries[libteem].echoEnvmapLookup
echoEnvmapLookup.restype = None
echoEnvmapLookup.argtypes = [POINTER(echoCol_t), POINTER(echoPos_t), POINTER(Nrrd)]
echoTextureLookup = _libraries[libteem].echoTextureLookup
echoTextureLookup.restype = None
echoTextureLookup.argtypes = [POINTER(echoCol_t), POINTER(Nrrd), echoPos_t, echoPos_t, POINTER(echoRTParm)]
echoIntxMaterialColor = _libraries[libteem].echoIntxMaterialColor
echoIntxMaterialColor.restype = None
echoIntxMaterialColor.argtypes = [POINTER(echoCol_t), POINTER(echoIntx), POINTER(echoRTParm)]
echoIntxLightColor = _libraries[libteem].echoIntxLightColor
echoIntxLightColor.restype = None
echoIntxLightColor.argtypes = [POINTER(echoCol_t), POINTER(echoCol_t), POINTER(echoCol_t), echoCol_t, POINTER(echoIntx), POINTER(echoScene), POINTER(echoRTParm), POINTER(echoThreadState)]
echoIntxFuzzify = _libraries[libteem].echoIntxFuzzify
echoIntxFuzzify.restype = None
echoIntxFuzzify.argtypes = [POINTER(echoIntx), echoCol_t, POINTER(echoThreadState)]
echoRayIntx = _libraries[libteem].echoRayIntx
echoRayIntx.restype = c_int
echoRayIntx.argtypes = [POINTER(echoIntx), POINTER(echoRay), POINTER(echoScene), POINTER(echoRTParm), POINTER(echoThreadState)]
echoIntxColor = _libraries[libteem].echoIntxColor
echoIntxColor.restype = None
echoIntxColor.argtypes = [POINTER(echoCol_t), POINTER(echoIntx), POINTER(echoScene), POINTER(echoRTParm), POINTER(echoThreadState)]
echoThreadStateInit = _libraries[libteem].echoThreadStateInit
echoThreadStateInit.restype = c_int
echoThreadStateInit.argtypes = [c_int, POINTER(echoThreadState), POINTER(echoRTParm), POINTER(echoGlobalState)]
echoJitterCompute = _libraries[libteem].echoJitterCompute
echoJitterCompute.restype = None
echoJitterCompute.argtypes = [POINTER(echoRTParm), POINTER(echoThreadState)]
echoRayColor = _libraries[libteem].echoRayColor
echoRayColor.restype = None
echoRayColor.argtypes = [POINTER(echoCol_t), POINTER(echoRay), POINTER(echoScene), POINTER(echoRTParm), POINTER(echoThreadState)]
echoChannelAverage = _libraries[libteem].echoChannelAverage
echoChannelAverage.restype = None
echoChannelAverage.argtypes = [POINTER(echoCol_t), POINTER(echoRTParm), POINTER(echoThreadState)]
echoRTRenderCheck = _libraries[libteem].echoRTRenderCheck
echoRTRenderCheck.restype = c_int
echoRTRenderCheck.argtypes = [POINTER(Nrrd), POINTER(limnCamera), POINTER(echoScene), POINTER(echoRTParm), POINTER(echoGlobalState)]
echoRTRender = _libraries[libteem].echoRTRender
echoRTRender.restype = c_int
echoRTRender.argtypes = [POINTER(Nrrd), POINTER(limnCamera), POINTER(echoScene), POINTER(echoRTParm), POINTER(echoGlobalState)]
ell_biff_key = (STRING).in_dll(_libraries[libteem], 'ell_biff_key')
ell_cubic_root = (POINTER(airEnum)).in_dll(_libraries[libteem], 'ell_cubic_root')
ell_debug = (c_int).in_dll(_libraries[libteem], 'ell_debug')
ell_3m_print_f = _libraries[libteem].ell_3m_print_f
ell_3m_print_f.restype = None
ell_3m_print_f.argtypes = [POINTER(FILE), POINTER(c_float)]
ell_3v_print_f = _libraries[libteem].ell_3v_print_f
ell_3v_print_f.restype = None
ell_3v_print_f.argtypes = [POINTER(FILE), POINTER(c_float)]
ell_3m_print_d = _libraries[libteem].ell_3m_print_d
ell_3m_print_d.restype = None
ell_3m_print_d.argtypes = [POINTER(FILE), POINTER(c_double)]
ell_3v_print_d = _libraries[libteem].ell_3v_print_d
ell_3v_print_d.restype = None
ell_3v_print_d.argtypes = [POINTER(FILE), POINTER(c_double)]
ell_4m_print_f = _libraries[libteem].ell_4m_print_f
ell_4m_print_f.restype = None
ell_4m_print_f.argtypes = [POINTER(FILE), POINTER(c_float)]
ell_4v_print_f = _libraries[libteem].ell_4v_print_f
ell_4v_print_f.restype = None
ell_4v_print_f.argtypes = [POINTER(FILE), POINTER(c_float)]
ell_4m_print_d = _libraries[libteem].ell_4m_print_d
ell_4m_print_d.restype = None
ell_4m_print_d.argtypes = [POINTER(FILE), POINTER(c_double)]
ell_4v_print_d = _libraries[libteem].ell_4v_print_d
ell_4v_print_d.restype = None
ell_4v_print_d.argtypes = [POINTER(FILE), POINTER(c_double)]
ell_3v_perp_f = _libraries[libteem].ell_3v_perp_f
ell_3v_perp_f.restype = None
ell_3v_perp_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3v_perp_d = _libraries[libteem].ell_3v_perp_d
ell_3v_perp_d.restype = None
ell_3v_perp_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_3mv_mul_f = _libraries[libteem].ell_3mv_mul_f
ell_3mv_mul_f.restype = None
ell_3mv_mul_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_3mv_mul_d = _libraries[libteem].ell_3mv_mul_d
ell_3mv_mul_d.restype = None
ell_3mv_mul_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_4mv_mul_f = _libraries[libteem].ell_4mv_mul_f
ell_4mv_mul_f.restype = None
ell_4mv_mul_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_4mv_mul_d = _libraries[libteem].ell_4mv_mul_d
ell_4mv_mul_d.restype = None
ell_4mv_mul_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_3v_angle_f = _libraries[libteem].ell_3v_angle_f
ell_3v_angle_f.restype = c_float
ell_3v_angle_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3v_angle_d = _libraries[libteem].ell_3v_angle_d
ell_3v_angle_d.restype = c_double
ell_3v_angle_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_3m_mul_f = _libraries[libteem].ell_3m_mul_f
ell_3m_mul_f.restype = None
ell_3m_mul_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_3m_mul_d = _libraries[libteem].ell_3m_mul_d
ell_3m_mul_d.restype = None
ell_3m_mul_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_3m_pre_mul_f = _libraries[libteem].ell_3m_pre_mul_f
ell_3m_pre_mul_f.restype = None
ell_3m_pre_mul_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3m_pre_mul_d = _libraries[libteem].ell_3m_pre_mul_d
ell_3m_pre_mul_d.restype = None
ell_3m_pre_mul_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_3m_post_mul_f = _libraries[libteem].ell_3m_post_mul_f
ell_3m_post_mul_f.restype = None
ell_3m_post_mul_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3m_post_mul_d = _libraries[libteem].ell_3m_post_mul_d
ell_3m_post_mul_d.restype = None
ell_3m_post_mul_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_3m_det_f = _libraries[libteem].ell_3m_det_f
ell_3m_det_f.restype = c_float
ell_3m_det_f.argtypes = [POINTER(c_float)]
ell_3m_det_d = _libraries[libteem].ell_3m_det_d
ell_3m_det_d.restype = c_double
ell_3m_det_d.argtypes = [POINTER(c_double)]
ell_3m_inv_f = _libraries[libteem].ell_3m_inv_f
ell_3m_inv_f.restype = None
ell_3m_inv_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3m_inv_d = _libraries[libteem].ell_3m_inv_d
ell_3m_inv_d.restype = None
ell_3m_inv_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_4m_mul_f = _libraries[libteem].ell_4m_mul_f
ell_4m_mul_f.restype = None
ell_4m_mul_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_4m_mul_d = _libraries[libteem].ell_4m_mul_d
ell_4m_mul_d.restype = None
ell_4m_mul_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_4m_pre_mul_f = _libraries[libteem].ell_4m_pre_mul_f
ell_4m_pre_mul_f.restype = None
ell_4m_pre_mul_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_4m_post_mul_f = _libraries[libteem].ell_4m_post_mul_f
ell_4m_post_mul_f.restype = None
ell_4m_post_mul_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_4m_post_mul_d = _libraries[libteem].ell_4m_post_mul_d
ell_4m_post_mul_d.restype = None
ell_4m_post_mul_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_4m_det_f = _libraries[libteem].ell_4m_det_f
ell_4m_det_f.restype = c_float
ell_4m_det_f.argtypes = [POINTER(c_float)]
ell_4m_det_d = _libraries[libteem].ell_4m_det_d
ell_4m_det_d.restype = c_double
ell_4m_det_d.argtypes = [POINTER(c_double)]
ell_4m_inv_f = _libraries[libteem].ell_4m_inv_f
ell_4m_inv_f.restype = None
ell_4m_inv_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_4m_inv_d = _libraries[libteem].ell_4m_inv_d
ell_4m_inv_d.restype = None
ell_4m_inv_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_6m_mul_d = _libraries[libteem].ell_6m_mul_d
ell_6m_mul_d.restype = None
ell_6m_mul_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_3m_rotate_between_d = _libraries[libteem].ell_3m_rotate_between_d
ell_3m_rotate_between_d.restype = None
ell_3m_rotate_between_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_3m_to_q_f = _libraries[libteem].ell_3m_to_q_f
ell_3m_to_q_f.restype = None
ell_3m_to_q_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3m_to_q_d = _libraries[libteem].ell_3m_to_q_d
ell_3m_to_q_d.restype = None
ell_3m_to_q_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_4m_to_q_f = _libraries[libteem].ell_4m_to_q_f
ell_4m_to_q_f.restype = None
ell_4m_to_q_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_4m_to_q_d = _libraries[libteem].ell_4m_to_q_d
ell_4m_to_q_d.restype = None
ell_4m_to_q_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_to_3m_f = _libraries[libteem].ell_q_to_3m_f
ell_q_to_3m_f.restype = None
ell_q_to_3m_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_q_to_3m_d = _libraries[libteem].ell_q_to_3m_d
ell_q_to_3m_d.restype = None
ell_q_to_3m_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_to_4m_f = _libraries[libteem].ell_q_to_4m_f
ell_q_to_4m_f.restype = None
ell_q_to_4m_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_q_to_4m_d = _libraries[libteem].ell_q_to_4m_d
ell_q_to_4m_d.restype = None
ell_q_to_4m_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_to_aa_f = _libraries[libteem].ell_q_to_aa_f
ell_q_to_aa_f.restype = c_float
ell_q_to_aa_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_q_to_aa_d = _libraries[libteem].ell_q_to_aa_d
ell_q_to_aa_d.restype = c_double
ell_q_to_aa_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_aa_to_q_f = _libraries[libteem].ell_aa_to_q_f
ell_aa_to_q_f.restype = None
ell_aa_to_q_f.argtypes = [POINTER(c_float), c_float, POINTER(c_float)]
ell_aa_to_q_d = _libraries[libteem].ell_aa_to_q_d
ell_aa_to_q_d.restype = None
ell_aa_to_q_d.argtypes = [POINTER(c_double), c_double, POINTER(c_double)]
ell_aa_to_3m_f = _libraries[libteem].ell_aa_to_3m_f
ell_aa_to_3m_f.restype = None
ell_aa_to_3m_f.argtypes = [POINTER(c_float), c_float, POINTER(c_float)]
ell_aa_to_3m_d = _libraries[libteem].ell_aa_to_3m_d
ell_aa_to_3m_d.restype = None
ell_aa_to_3m_d.argtypes = [POINTER(c_double), c_double, POINTER(c_double)]
ell_aa_to_4m_f = _libraries[libteem].ell_aa_to_4m_f
ell_aa_to_4m_f.restype = None
ell_aa_to_4m_f.argtypes = [POINTER(c_float), c_float, POINTER(c_float)]
ell_aa_to_4m_d = _libraries[libteem].ell_aa_to_4m_d
ell_aa_to_4m_d.restype = None
ell_aa_to_4m_d.argtypes = [POINTER(c_double), c_double, POINTER(c_double)]
ell_3m_to_aa_f = _libraries[libteem].ell_3m_to_aa_f
ell_3m_to_aa_f.restype = c_float
ell_3m_to_aa_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_3m_to_aa_d = _libraries[libteem].ell_3m_to_aa_d
ell_3m_to_aa_d.restype = c_double
ell_3m_to_aa_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_4m_to_aa_f = _libraries[libteem].ell_4m_to_aa_f
ell_4m_to_aa_f.restype = c_float
ell_4m_to_aa_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_4m_to_aa_d = _libraries[libteem].ell_4m_to_aa_d
ell_4m_to_aa_d.restype = c_double
ell_4m_to_aa_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_mul_f = _libraries[libteem].ell_q_mul_f
ell_q_mul_f.restype = None
ell_q_mul_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_q_mul_d = _libraries[libteem].ell_q_mul_d
ell_q_mul_d.restype = None
ell_q_mul_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_q_inv_f = _libraries[libteem].ell_q_inv_f
ell_q_inv_f.restype = None
ell_q_inv_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_q_inv_d = _libraries[libteem].ell_q_inv_d
ell_q_inv_d.restype = None
ell_q_inv_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_pow_f = _libraries[libteem].ell_q_pow_f
ell_q_pow_f.restype = None
ell_q_pow_f.argtypes = [POINTER(c_float), POINTER(c_float), c_float]
ell_q_pow_d = _libraries[libteem].ell_q_pow_d
ell_q_pow_d.restype = None
ell_q_pow_d.argtypes = [POINTER(c_double), POINTER(c_double), c_double]
ell_q_div_f = _libraries[libteem].ell_q_div_f
ell_q_div_f.restype = None
ell_q_div_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_q_div_d = _libraries[libteem].ell_q_div_d
ell_q_div_d.restype = None
ell_q_div_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_q_exp_f = _libraries[libteem].ell_q_exp_f
ell_q_exp_f.restype = None
ell_q_exp_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_q_exp_d = _libraries[libteem].ell_q_exp_d
ell_q_exp_d.restype = None
ell_q_exp_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_log_f = _libraries[libteem].ell_q_log_f
ell_q_log_f.restype = None
ell_q_log_f.argtypes = [POINTER(c_float), POINTER(c_float)]
ell_q_log_d = _libraries[libteem].ell_q_log_d
ell_q_log_d.restype = None
ell_q_log_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_q_3v_rotate_f = _libraries[libteem].ell_q_3v_rotate_f
ell_q_3v_rotate_f.restype = None
ell_q_3v_rotate_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_q_3v_rotate_d = _libraries[libteem].ell_q_3v_rotate_d
ell_q_3v_rotate_d.restype = None
ell_q_3v_rotate_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_q_4v_rotate_f = _libraries[libteem].ell_q_4v_rotate_f
ell_q_4v_rotate_f.restype = None
ell_q_4v_rotate_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
ell_q_4v_rotate_d = _libraries[libteem].ell_q_4v_rotate_d
ell_q_4v_rotate_d.restype = None
ell_q_4v_rotate_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_q_avg4_d = _libraries[libteem].ell_q_avg4_d
ell_q_avg4_d.restype = c_int
ell_q_avg4_d.argtypes = [POINTER(c_double), POINTER(c_uint), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_double, c_uint]
ell_q_avgN_d = _libraries[libteem].ell_q_avgN_d
ell_q_avgN_d.restype = c_int
ell_q_avgN_d.argtypes = [POINTER(c_double), POINTER(c_uint), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_uint, c_double, c_uint]
ell_Nm_check = _libraries[libteem].ell_Nm_check
ell_Nm_check.restype = c_int
ell_Nm_check.argtypes = [POINTER(Nrrd), c_int]
ell_Nm_tran = _libraries[libteem].ell_Nm_tran
ell_Nm_tran.restype = c_int
ell_Nm_tran.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
ell_Nm_mul = _libraries[libteem].ell_Nm_mul
ell_Nm_mul.restype = c_int
ell_Nm_mul.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
ell_Nm_inv = _libraries[libteem].ell_Nm_inv
ell_Nm_inv.restype = c_int
ell_Nm_inv.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
ell_Nm_pseudo_inv = _libraries[libteem].ell_Nm_pseudo_inv
ell_Nm_pseudo_inv.restype = c_int
ell_Nm_pseudo_inv.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
ell_Nm_wght_pseudo_inv = _libraries[libteem].ell_Nm_wght_pseudo_inv
ell_Nm_wght_pseudo_inv.restype = c_int
ell_Nm_wght_pseudo_inv.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
ell_cubic = _libraries[libteem].ell_cubic
ell_cubic.restype = c_int
ell_cubic.argtypes = [POINTER(c_double), c_double, c_double, c_double, c_int]
ell_3m_1d_nullspace_d = _libraries[libteem].ell_3m_1d_nullspace_d
ell_3m_1d_nullspace_d.restype = None
ell_3m_1d_nullspace_d.argtypes = [POINTER(c_double), POINTER(c_double)]
ell_3m_2d_nullspace_d = _libraries[libteem].ell_3m_2d_nullspace_d
ell_3m_2d_nullspace_d.restype = None
ell_3m_2d_nullspace_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
ell_3m_eigenvalues_d = _libraries[libteem].ell_3m_eigenvalues_d
ell_3m_eigenvalues_d.restype = c_int
ell_3m_eigenvalues_d.argtypes = [POINTER(c_double), POINTER(c_double), c_int]
ell_3m_eigensolve_d = _libraries[libteem].ell_3m_eigensolve_d
ell_3m_eigensolve_d.restype = c_int
ell_3m_eigensolve_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int]
ell_3m_svd_d = _libraries[libteem].ell_3m_svd_d
ell_3m_svd_d.restype = c_int
ell_3m_svd_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int]
ell_6ms_eigensolve_d = _libraries[libteem].ell_6ms_eigensolve_d
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
    ('center', c_int),
    ('fromOrientation', c_int),
    ('size', c_uint * 3),
    ('spacing', c_double * 3),
    ('volHalfLen', c_double * 3),
    ('voxLen', c_double * 3),
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
    ('defaultSpacing', c_double),
    ('curvNormalSide', c_int),
    ('requireAllSpacings', c_int),
    ('requireEqualCenters', c_int),
    ('defaultCenter', c_int),
    ('stackUse', c_int),
    ('stackRenormalize', c_int),
]
gageParm = gageParm_t
class gagePoint_t(Structure):
    pass
gagePoint_t._fields_ = [
    ('xf', c_double),
    ('yf', c_double),
    ('zf', c_double),
    ('xi', c_uint),
    ('yi', c_uint),
    ('zi', c_uint),
]
gagePoint = gagePoint_t
gageQuery = c_ubyte * 24
class gageContext_t(Structure):
    pass
class NrrdKernelSpec(Structure):
    pass
class gagePerVolume_t(Structure):
    pass
gageContext_t._fields_ = [
    ('verbose', c_int),
    ('parm', gageParm),
    ('ksp', POINTER(NrrdKernelSpec) * 8),
    ('pvl', POINTER(POINTER(gagePerVolume_t))),
    ('pvlNum', c_uint),
    ('pvlArr', POINTER(airArray)),
    ('shape', POINTER(gageShape)),
    ('stackPos', POINTER(c_double)),
    ('stackFslw', POINTER(c_double)),
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
gageBiffKey = (STRING).in_dll(_libraries[libteem], 'gageBiffKey')
gageDefVerbose = (c_int).in_dll(_libraries[libteem], 'gageDefVerbose')
gageDefGradMagCurvMin = (c_double).in_dll(_libraries[libteem], 'gageDefGradMagCurvMin')
gageDefRenormalize = (c_int).in_dll(_libraries[libteem], 'gageDefRenormalize')
gageDefCheckIntegrals = (c_int).in_dll(_libraries[libteem], 'gageDefCheckIntegrals')
gageDefK3Pack = (c_int).in_dll(_libraries[libteem], 'gageDefK3Pack')
gageDefDefaultSpacing = (c_double).in_dll(_libraries[libteem], 'gageDefDefaultSpacing')
gageDefCurvNormalSide = (c_int).in_dll(_libraries[libteem], 'gageDefCurvNormalSide')
gageDefKernelIntegralNearZero = (c_double).in_dll(_libraries[libteem], 'gageDefKernelIntegralNearZero')
gageDefRequireAllSpacings = (c_int).in_dll(_libraries[libteem], 'gageDefRequireAllSpacings')
gageDefRequireEqualCenters = (c_int).in_dll(_libraries[libteem], 'gageDefRequireEqualCenters')
gageDefDefaultCenter = (c_int).in_dll(_libraries[libteem], 'gageDefDefaultCenter')
gageDefStackUse = (c_int).in_dll(_libraries[libteem], 'gageDefStackUse')
gageDefStackRenormalize = (c_int).in_dll(_libraries[libteem], 'gageDefStackRenormalize')
gageZeroNormal = (c_double * 3).in_dll(_libraries[libteem], 'gageZeroNormal')
gageErr = (POINTER(airEnum)).in_dll(_libraries[libteem], 'gageErr')
gageKernel = (POINTER(airEnum)).in_dll(_libraries[libteem], 'gageKernel')
gageParmReset = _libraries[libteem].gageParmReset
gageParmReset.restype = None
gageParmReset.argtypes = [POINTER(gageParm)]
gagePointReset = _libraries[libteem].gagePointReset
gagePointReset.restype = None
gagePointReset.argtypes = [POINTER(gagePoint)]
gageItemSpecNew = _libraries[libteem].gageItemSpecNew
gageItemSpecNew.restype = POINTER(gageItemSpec)
gageItemSpecNew.argtypes = []
gageItemSpecInit = _libraries[libteem].gageItemSpecInit
gageItemSpecInit.restype = None
gageItemSpecInit.argtypes = [POINTER(gageItemSpec)]
gageItemSpecNix = _libraries[libteem].gageItemSpecNix
gageItemSpecNix.restype = POINTER(gageItemSpec)
gageItemSpecNix.argtypes = [POINTER(gageItemSpec)]
gageKindCheck = _libraries[libteem].gageKindCheck
gageKindCheck.restype = c_int
gageKindCheck.argtypes = [POINTER(gageKind)]
gageKindTotalAnswerLength = _libraries[libteem].gageKindTotalAnswerLength
gageKindTotalAnswerLength.restype = c_int
gageKindTotalAnswerLength.argtypes = [POINTER(gageKind)]
gageKindAnswerLength = _libraries[libteem].gageKindAnswerLength
gageKindAnswerLength.restype = c_uint
gageKindAnswerLength.argtypes = [POINTER(gageKind), c_int]
gageKindAnswerOffset = _libraries[libteem].gageKindAnswerOffset
gageKindAnswerOffset.restype = c_int
gageKindAnswerOffset.argtypes = [POINTER(gageKind), c_int]
gageKindVolumeCheck = _libraries[libteem].gageKindVolumeCheck
gageKindVolumeCheck.restype = c_int
gageKindVolumeCheck.argtypes = [POINTER(gageKind), POINTER(Nrrd)]
gageQueryPrint = _libraries[libteem].gageQueryPrint
gageQueryPrint.restype = None
gageQueryPrint.argtypes = [POINTER(FILE), POINTER(gageKind), POINTER(c_ubyte)]
gageScl3PFilter_t = CFUNCTYPE(None, POINTER(gageShape), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int, c_int)
gageScl3PFilter2 = _libraries[libteem].gageScl3PFilter2
gageScl3PFilter2.restype = None
gageScl3PFilter2.argtypes = [POINTER(gageShape), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int, c_int]
gageScl3PFilter4 = _libraries[libteem].gageScl3PFilter4
gageScl3PFilter4.restype = None
gageScl3PFilter4.argtypes = [POINTER(gageShape), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int, c_int]
gageScl3PFilter6 = _libraries[libteem].gageScl3PFilter6
gageScl3PFilter6.restype = None
gageScl3PFilter6.argtypes = [POINTER(gageShape), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int, c_int]
gageScl3PFilter8 = _libraries[libteem].gageScl3PFilter8
gageScl3PFilter8.restype = None
gageScl3PFilter8.argtypes = [POINTER(gageShape), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int, c_int]
gageScl3PFilterN = _libraries[libteem].gageScl3PFilterN
gageScl3PFilterN.restype = None
gageScl3PFilterN.argtypes = [POINTER(gageShape), c_int, POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_int, c_int]
gageScl = (POINTER(airEnum)).in_dll(_libraries[libteem], 'gageScl')
gageKindScl = (POINTER(gageKind)).in_dll(_libraries[libteem], 'gageKindScl')
gageVec = (POINTER(airEnum)).in_dll(_libraries[libteem], 'gageVec')
gageKindVec = (POINTER(gageKind)).in_dll(_libraries[libteem], 'gageKindVec')
gageShapeReset = _libraries[libteem].gageShapeReset
gageShapeReset.restype = None
gageShapeReset.argtypes = [POINTER(gageShape)]
gageShapeNew = _libraries[libteem].gageShapeNew
gageShapeNew.restype = POINTER(gageShape)
gageShapeNew.argtypes = []
gageShapeCopy = _libraries[libteem].gageShapeCopy
gageShapeCopy.restype = POINTER(gageShape)
gageShapeCopy.argtypes = [POINTER(gageShape)]
gageShapeNix = _libraries[libteem].gageShapeNix
gageShapeNix.restype = POINTER(gageShape)
gageShapeNix.argtypes = [POINTER(gageShape)]
gageShapeSet = _libraries[libteem].gageShapeSet
gageShapeSet.restype = c_int
gageShapeSet.argtypes = [POINTER(gageShape), POINTER(Nrrd), c_int]
gageShapeWtoI = _libraries[libteem].gageShapeWtoI
gageShapeWtoI.restype = None
gageShapeWtoI.argtypes = [POINTER(gageShape), POINTER(c_double), POINTER(c_double)]
gageShapeItoW = _libraries[libteem].gageShapeItoW
gageShapeItoW.restype = None
gageShapeItoW.argtypes = [POINTER(gageShape), POINTER(c_double), POINTER(c_double)]
gageShapeEqual = _libraries[libteem].gageShapeEqual
gageShapeEqual.restype = c_int
gageShapeEqual.argtypes = [POINTER(gageShape), STRING, POINTER(gageShape), STRING]
gageShapeBoundingBox = _libraries[libteem].gageShapeBoundingBox
gageShapeBoundingBox.restype = None
gageShapeBoundingBox.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(gageShape)]
gageVolumeCheck = _libraries[libteem].gageVolumeCheck
gageVolumeCheck.restype = c_int
gageVolumeCheck.argtypes = [POINTER(gageContext), POINTER(Nrrd), POINTER(gageKind)]
gagePerVolumeNew = _libraries[libteem].gagePerVolumeNew
gagePerVolumeNew.restype = POINTER(gagePerVolume)
gagePerVolumeNew.argtypes = [POINTER(gageContext), POINTER(Nrrd), POINTER(gageKind)]
gagePerVolumeNix = _libraries[libteem].gagePerVolumeNix
gagePerVolumeNix.restype = POINTER(gagePerVolume)
gagePerVolumeNix.argtypes = [POINTER(gagePerVolume)]
gageAnswerPointer = _libraries[libteem].gageAnswerPointer
gageAnswerPointer.restype = POINTER(c_double)
gageAnswerPointer.argtypes = [POINTER(gageContext), POINTER(gagePerVolume), c_int]
gageAnswerLength = _libraries[libteem].gageAnswerLength
gageAnswerLength.restype = c_uint
gageAnswerLength.argtypes = [POINTER(gageContext), POINTER(gagePerVolume), c_int]
gageQueryReset = _libraries[libteem].gageQueryReset
gageQueryReset.restype = c_int
gageQueryReset.argtypes = [POINTER(gageContext), POINTER(gagePerVolume)]
gageQuerySet = _libraries[libteem].gageQuerySet
gageQuerySet.restype = c_int
gageQuerySet.argtypes = [POINTER(gageContext), POINTER(gagePerVolume), POINTER(c_ubyte)]
gageQueryAdd = _libraries[libteem].gageQueryAdd
gageQueryAdd.restype = c_int
gageQueryAdd.argtypes = [POINTER(gageContext), POINTER(gagePerVolume), POINTER(c_ubyte)]
gageQueryItemOn = _libraries[libteem].gageQueryItemOn
gageQueryItemOn.restype = c_int
gageQueryItemOn.argtypes = [POINTER(gageContext), POINTER(gagePerVolume), c_int]
gageTauOfTee = _libraries[libteem].gageTauOfTee
gageTauOfTee.restype = c_double
gageTauOfTee.argtypes = [c_double]
gageTeeOfTau = _libraries[libteem].gageTeeOfTau
gageTeeOfTau.restype = c_double
gageTeeOfTau.argtypes = [c_double]
gageSigOfTau = _libraries[libteem].gageSigOfTau
gageSigOfTau.restype = c_double
gageSigOfTau.argtypes = [c_double]
gageTauOfSig = _libraries[libteem].gageTauOfSig
gageTauOfSig.restype = c_double
gageTauOfSig.argtypes = [c_double]
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
gageStackBlur = _libraries[libteem].gageStackBlur
gageStackBlur.restype = c_int
gageStackBlur.argtypes = [POINTER(POINTER(Nrrd)), POINTER(c_double), c_uint, POINTER(Nrrd), c_uint, POINTER(NrrdKernelSpec), c_int, c_int, c_int]
gageStackPerVolumeNew = _libraries[libteem].gageStackPerVolumeNew
gageStackPerVolumeNew.restype = c_int
gageStackPerVolumeNew.argtypes = [POINTER(gageContext), POINTER(POINTER(POINTER(gagePerVolume))), POINTER(POINTER(Nrrd)), c_uint, POINTER(gageKind)]
gageStackPerVolumeAttach = _libraries[libteem].gageStackPerVolumeAttach
gageStackPerVolumeAttach.restype = c_int
gageStackPerVolumeAttach.argtypes = [POINTER(gageContext), POINTER(gagePerVolume), POINTER(POINTER(gagePerVolume)), POINTER(c_double), c_uint]
gageContextNew = _libraries[libteem].gageContextNew
gageContextNew.restype = POINTER(gageContext)
gageContextNew.argtypes = []
gageContextCopy = _libraries[libteem].gageContextCopy
gageContextCopy.restype = POINTER(gageContext)
gageContextCopy.argtypes = [POINTER(gageContext)]
gageContextNix = _libraries[libteem].gageContextNix
gageContextNix.restype = POINTER(gageContext)
gageContextNix.argtypes = [POINTER(gageContext)]
gageParmSet = _libraries[libteem].gageParmSet
gageParmSet.restype = None
gageParmSet.argtypes = [POINTER(gageContext), c_int, c_double]
gagePerVolumeIsAttached = _libraries[libteem].gagePerVolumeIsAttached
gagePerVolumeIsAttached.restype = c_int
gagePerVolumeIsAttached.argtypes = [POINTER(gageContext), POINTER(gagePerVolume)]
gagePerVolumeAttach = _libraries[libteem].gagePerVolumeAttach
gagePerVolumeAttach.restype = c_int
gagePerVolumeAttach.argtypes = [POINTER(gageContext), POINTER(gagePerVolume)]
gagePerVolumeDetach = _libraries[libteem].gagePerVolumeDetach
gagePerVolumeDetach.restype = c_int
gagePerVolumeDetach.argtypes = [POINTER(gageContext), POINTER(gagePerVolume)]
gageKernelSet = _libraries[libteem].gageKernelSet
gageKernelSet.restype = c_int
gageKernelSet.argtypes = [POINTER(gageContext), c_int, POINTER(NrrdKernel), POINTER(c_double)]
gageKernelReset = _libraries[libteem].gageKernelReset
gageKernelReset.restype = None
gageKernelReset.argtypes = [POINTER(gageContext)]
gageProbe = _libraries[libteem].gageProbe
gageProbe.restype = c_int
gageProbe.argtypes = [POINTER(gageContext), c_double, c_double, c_double]
gageProbeSpace = _libraries[libteem].gageProbeSpace
gageProbeSpace.restype = c_int
gageProbeSpace.argtypes = [POINTER(gageContext), c_double, c_double, c_double, c_int, c_int]
gageStackProbe = _libraries[libteem].gageStackProbe
gageStackProbe.restype = c_int
gageStackProbe.argtypes = [POINTER(gageContext), c_double, c_double, c_double, c_double]
gageStackProbeSpace = _libraries[libteem].gageStackProbeSpace
gageStackProbeSpace.restype = c_int
gageStackProbeSpace.argtypes = [POINTER(gageContext), c_double, c_double, c_double, c_double, c_int, c_int]
gageUpdate = _libraries[libteem].gageUpdate
gageUpdate.restype = c_int
gageUpdate.argtypes = [POINTER(gageContext)]
gageStructureTensor = _libraries[libteem].gageStructureTensor
gageStructureTensor.restype = c_int
gageStructureTensor.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, c_int, c_int]
gageDeconvolve = _libraries[libteem].gageDeconvolve
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
hestVerbosity = (c_int).in_dll(_libraries[libteem], 'hestVerbosity')
hestRespFileEnable = (c_int).in_dll(_libraries[libteem], 'hestRespFileEnable')
hestElideSingleEnumType = (c_int).in_dll(_libraries[libteem], 'hestElideSingleEnumType')
hestElideSingleOtherType = (c_int).in_dll(_libraries[libteem], 'hestElideSingleOtherType')
hestElideSingleOtherDefault = (c_int).in_dll(_libraries[libteem], 'hestElideSingleOtherDefault')
hestElideSingleNonExistFloatDefault = (c_int).in_dll(_libraries[libteem], 'hestElideSingleNonExistFloatDefault')
hestElideMultipleNonExistFloatDefault = (c_int).in_dll(_libraries[libteem], 'hestElideMultipleNonExistFloatDefault')
hestElideSingleEmptyStringDefault = (c_int).in_dll(_libraries[libteem], 'hestElideSingleEmptyStringDefault')
hestElideMultipleEmptyStringDefault = (c_int).in_dll(_libraries[libteem], 'hestElideMultipleEmptyStringDefault')
hestGreedySingleString = (c_int).in_dll(_libraries[libteem], 'hestGreedySingleString')
hestCleverPluralizeOtherY = (c_int).in_dll(_libraries[libteem], 'hestCleverPluralizeOtherY')
hestColumns = (c_int).in_dll(_libraries[libteem], 'hestColumns')
hestRespFileFlag = (c_char).in_dll(_libraries[libteem], 'hestRespFileFlag')
hestRespFileComment = (c_char).in_dll(_libraries[libteem], 'hestRespFileComment')
hestVarParamStopFlag = (c_char).in_dll(_libraries[libteem], 'hestVarParamStopFlag')
hestMultiFlagSep = (c_char).in_dll(_libraries[libteem], 'hestMultiFlagSep')
hestParmNew = _libraries[libteem].hestParmNew
hestParmNew.restype = POINTER(hestParm)
hestParmNew.argtypes = []
hestParmFree = _libraries[libteem].hestParmFree
hestParmFree.restype = POINTER(hestParm)
hestParmFree.argtypes = [POINTER(hestParm)]
hestOptAdd = _libraries[libteem].hestOptAdd
hestOptAdd.restype = None
hestOptAdd.argtypes = [POINTER(POINTER(hestOpt)), STRING, STRING, c_int, c_int, c_int, c_void_p, STRING, STRING]
hestOptFree = _libraries[libteem].hestOptFree
hestOptFree.restype = POINTER(hestOpt)
hestOptFree.argtypes = [POINTER(hestOpt)]
hestOptCheck = _libraries[libteem].hestOptCheck
hestOptCheck.restype = c_int
hestOptCheck.argtypes = [POINTER(hestOpt), POINTER(STRING)]
hestParse = _libraries[libteem].hestParse
hestParse.restype = c_int
hestParse.argtypes = [POINTER(hestOpt), c_int, POINTER(STRING), POINTER(STRING), POINTER(hestParm)]
hestParseFree = _libraries[libteem].hestParseFree
hestParseFree.restype = c_void_p
hestParseFree.argtypes = [POINTER(hestOpt)]
hestParseOrDie = _libraries[libteem].hestParseOrDie
hestParseOrDie.restype = None
hestParseOrDie.argtypes = [POINTER(hestOpt), c_int, POINTER(STRING), POINTER(hestParm), STRING, STRING, c_int, c_int, c_int]
hestMinNumArgs = _libraries[libteem].hestMinNumArgs
hestMinNumArgs.restype = c_int
hestMinNumArgs.argtypes = [POINTER(hestOpt)]
hestUsage = _libraries[libteem].hestUsage
hestUsage.restype = None
hestUsage.argtypes = [POINTER(FILE), POINTER(hestOpt), STRING, POINTER(hestParm)]
hestGlossary = _libraries[libteem].hestGlossary
hestGlossary.restype = None
hestGlossary.argtypes = [POINTER(FILE), POINTER(hestOpt), POINTER(hestParm)]
hestInfo = _libraries[libteem].hestInfo
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
hooverBiffKey = (STRING).in_dll(_libraries[libteem], 'hooverBiffKey')
hooverDefVolCentering = (c_int).in_dll(_libraries[libteem], 'hooverDefVolCentering')
hooverDefImgCentering = (c_int).in_dll(_libraries[libteem], 'hooverDefImgCentering')
hooverErr = (POINTER(airEnum)).in_dll(_libraries[libteem], 'hooverErr')
hooverContextNew = _libraries[libteem].hooverContextNew
hooverContextNew.restype = POINTER(hooverContext)
hooverContextNew.argtypes = []
hooverContextCheck = _libraries[libteem].hooverContextCheck
hooverContextCheck.restype = c_int
hooverContextCheck.argtypes = [POINTER(hooverContext)]
hooverContextNix = _libraries[libteem].hooverContextNix
hooverContextNix.restype = None
hooverContextNix.argtypes = [POINTER(hooverContext)]
hooverRender = _libraries[libteem].hooverRender
hooverRender.restype = c_int
hooverRender.argtypes = [POINTER(hooverContext), POINTER(c_int), POINTER(c_int)]
hooverStubRenderBegin = _libraries[libteem].hooverStubRenderBegin
hooverStubRenderBegin.restype = c_int
hooverStubRenderBegin.argtypes = [POINTER(c_void_p), c_void_p]
hooverStubThreadBegin = _libraries[libteem].hooverStubThreadBegin
hooverStubThreadBegin.restype = c_int
hooverStubThreadBegin.argtypes = [POINTER(c_void_p), c_void_p, c_void_p, c_int]
hooverStubRayBegin = _libraries[libteem].hooverStubRayBegin
hooverStubRayBegin.restype = c_int
hooverStubRayBegin.argtypes = [c_void_p, c_void_p, c_void_p, c_int, c_int, c_double, POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double)]
hooverStubSample = _libraries[libteem].hooverStubSample
hooverStubSample.restype = c_double
hooverStubSample.argtypes = [c_void_p, c_void_p, c_void_p, c_int, c_double, c_int, POINTER(c_double), POINTER(c_double)]
hooverStubRayEnd = _libraries[libteem].hooverStubRayEnd
hooverStubRayEnd.restype = c_int
hooverStubRayEnd.argtypes = [c_void_p, c_void_p, c_void_p]
hooverStubThreadEnd = _libraries[libteem].hooverStubThreadEnd
hooverStubThreadEnd.restype = c_int
hooverStubThreadEnd.argtypes = [c_void_p, c_void_p, c_void_p]
hooverStubRenderEnd = _libraries[libteem].hooverStubRenderEnd
hooverStubRenderEnd.restype = c_int
hooverStubRenderEnd.argtypes = [c_void_p, c_void_p]
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
limnBiffKey = (STRING).in_dll(_libraries[libteem], 'limnBiffKey')
limnDefCameraAtRelative = (c_int).in_dll(_libraries[libteem], 'limnDefCameraAtRelative')
limnDefCameraOrthographic = (c_int).in_dll(_libraries[libteem], 'limnDefCameraOrthographic')
limnDefCameraRightHanded = (c_int).in_dll(_libraries[libteem], 'limnDefCameraRightHanded')
limnSpace = (POINTER(airEnum)).in_dll(_libraries[libteem], 'limnSpace')
limnPolyDataInfo = (POINTER(airEnum)).in_dll(_libraries[libteem], 'limnPolyDataInfo')
limnCameraPathTrack = (POINTER(airEnum)).in_dll(_libraries[libteem], 'limnCameraPathTrack')
limnPrimitive = (POINTER(airEnum)).in_dll(_libraries[libteem], 'limnPrimitive')
limnQNBins = (c_int * 17).in_dll(_libraries[libteem], 'limnQNBins')
limnQNtoV_f = (CFUNCTYPE(None, POINTER(c_float), c_int) * 17).in_dll(_libraries[libteem], 'limnQNtoV_f')
limnQNtoV_d = (CFUNCTYPE(None, POINTER(c_double), c_int) * 17).in_dll(_libraries[libteem], 'limnQNtoV_d')
limnVtoQN_f = (CFUNCTYPE(c_int, POINTER(c_float)) * 17).in_dll(_libraries[libteem], 'limnVtoQN_f')
limnVtoQN_d = (CFUNCTYPE(c_int, POINTER(c_double)) * 17).in_dll(_libraries[libteem], 'limnVtoQN_d')
limnLightSet = _libraries[libteem].limnLightSet
limnLightSet.restype = None
limnLightSet.argtypes = [POINTER(limnLight), c_int, c_int, c_float, c_float, c_float, c_float, c_float, c_float]
limnLightAmbientSet = _libraries[libteem].limnLightAmbientSet
limnLightAmbientSet.restype = None
limnLightAmbientSet.argtypes = [POINTER(limnLight), c_float, c_float, c_float]
limnLightSwitch = _libraries[libteem].limnLightSwitch
limnLightSwitch.restype = None
limnLightSwitch.argtypes = [POINTER(limnLight), c_int, c_int]
limnLightReset = _libraries[libteem].limnLightReset
limnLightReset.restype = None
limnLightReset.argtypes = [POINTER(limnLight)]
limnLightUpdate = _libraries[libteem].limnLightUpdate
limnLightUpdate.restype = c_int
limnLightUpdate.argtypes = [POINTER(limnLight), POINTER(limnCamera)]
limnEnvMapCB = CFUNCTYPE(None, POINTER(c_float), POINTER(c_float), c_void_p)
limnEnvMapFill = _libraries[libteem].limnEnvMapFill
limnEnvMapFill.restype = c_int
limnEnvMapFill.argtypes = [POINTER(Nrrd), limnEnvMapCB, c_int, c_void_p]
limnLightDiffuseCB = _libraries[libteem].limnLightDiffuseCB
limnLightDiffuseCB.restype = None
limnLightDiffuseCB.argtypes = [POINTER(c_float), POINTER(c_float), c_void_p]
limnEnvMapCheck = _libraries[libteem].limnEnvMapCheck
limnEnvMapCheck.restype = c_int
limnEnvMapCheck.argtypes = [POINTER(Nrrd)]
limnLightNew = _libraries[libteem].limnLightNew
limnLightNew.restype = POINTER(limnLight)
limnLightNew.argtypes = []
limnCameraInit = _libraries[libteem].limnCameraInit
limnCameraInit.restype = None
limnCameraInit.argtypes = [POINTER(limnCamera)]
limnLightNix = _libraries[libteem].limnLightNix
limnLightNix.restype = POINTER(limnLight)
limnLightNix.argtypes = [POINTER(limnLight)]
limnCameraNew = _libraries[libteem].limnCameraNew
limnCameraNew.restype = POINTER(limnCamera)
limnCameraNew.argtypes = []
limnCameraNix = _libraries[libteem].limnCameraNix
limnCameraNix.restype = POINTER(limnCamera)
limnCameraNix.argtypes = [POINTER(limnCamera)]
limnWindowNew = _libraries[libteem].limnWindowNew
limnWindowNew.restype = POINTER(limnWindow)
limnWindowNew.argtypes = [c_int]
limnWindowNix = _libraries[libteem].limnWindowNix
limnWindowNix.restype = POINTER(limnWindow)
limnWindowNix.argtypes = [POINTER(limnWindow)]
limnHestCameraOptAdd = _libraries[libteem].limnHestCameraOptAdd
limnHestCameraOptAdd.restype = None
limnHestCameraOptAdd.argtypes = [POINTER(POINTER(hestOpt)), POINTER(limnCamera), STRING, STRING, STRING, STRING, STRING, STRING, STRING, STRING, STRING]
limnCameraAspectSet = _libraries[libteem].limnCameraAspectSet
limnCameraAspectSet.restype = c_int
limnCameraAspectSet.argtypes = [POINTER(limnCamera), c_int, c_int, c_int]
limnCameraUpdate = _libraries[libteem].limnCameraUpdate
limnCameraUpdate.restype = c_int
limnCameraUpdate.argtypes = [POINTER(limnCamera)]
limnCameraPathMake = _libraries[libteem].limnCameraPathMake
limnCameraPathMake.restype = c_int
limnCameraPathMake.argtypes = [POINTER(limnCamera), c_int, POINTER(limnCamera), POINTER(c_double), c_int, c_int, POINTER(limnSplineTypeSpec), POINTER(limnSplineTypeSpec), POINTER(limnSplineTypeSpec), POINTER(limnSplineTypeSpec)]
limnObjectLookAdd = _libraries[libteem].limnObjectLookAdd
limnObjectLookAdd.restype = c_int
limnObjectLookAdd.argtypes = [POINTER(limnObject)]
limnObjectNew = _libraries[libteem].limnObjectNew
limnObjectNew.restype = POINTER(limnObject)
limnObjectNew.argtypes = [c_int, c_int]
limnObjectNix = _libraries[libteem].limnObjectNix
limnObjectNix.restype = POINTER(limnObject)
limnObjectNix.argtypes = [POINTER(limnObject)]
limnObjectEmpty = _libraries[libteem].limnObjectEmpty
limnObjectEmpty.restype = None
limnObjectEmpty.argtypes = [POINTER(limnObject)]
limnObjectPreSet = _libraries[libteem].limnObjectPreSet
limnObjectPreSet.restype = c_int
limnObjectPreSet.argtypes = [POINTER(limnObject), c_uint, c_uint, c_uint, c_uint, c_uint]
limnObjectPartAdd = _libraries[libteem].limnObjectPartAdd
limnObjectPartAdd.restype = c_int
limnObjectPartAdd.argtypes = [POINTER(limnObject)]
limnObjectVertexNumPreSet = _libraries[libteem].limnObjectVertexNumPreSet
limnObjectVertexNumPreSet.restype = c_int
limnObjectVertexNumPreSet.argtypes = [POINTER(limnObject), c_uint, c_uint]
limnObjectVertexAdd = _libraries[libteem].limnObjectVertexAdd
limnObjectVertexAdd.restype = c_int
limnObjectVertexAdd.argtypes = [POINTER(limnObject), c_uint, c_float, c_float, c_float]
limnObjectEdgeAdd = _libraries[libteem].limnObjectEdgeAdd
limnObjectEdgeAdd.restype = c_int
limnObjectEdgeAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_uint, c_uint, c_uint]
limnObjectFaceNumPreSet = _libraries[libteem].limnObjectFaceNumPreSet
limnObjectFaceNumPreSet.restype = c_int
limnObjectFaceNumPreSet.argtypes = [POINTER(limnObject), c_uint, c_uint]
limnObjectFaceAdd = _libraries[libteem].limnObjectFaceAdd
limnObjectFaceAdd.restype = c_int
limnObjectFaceAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_uint, POINTER(c_uint)]
limnPolyDataNew = _libraries[libteem].limnPolyDataNew
limnPolyDataNew.restype = POINTER(limnPolyData)
limnPolyDataNew.argtypes = []
limnPolyDataNix = _libraries[libteem].limnPolyDataNix
limnPolyDataNix.restype = POINTER(limnPolyData)
limnPolyDataNix.argtypes = [POINTER(limnPolyData)]
limnPolyDataInfoBitFlag = _libraries[libteem].limnPolyDataInfoBitFlag
limnPolyDataInfoBitFlag.restype = c_uint
limnPolyDataInfoBitFlag.argtypes = [POINTER(limnPolyData)]
limnPolyDataAlloc = _libraries[libteem].limnPolyDataAlloc
limnPolyDataAlloc.restype = c_int
limnPolyDataAlloc.argtypes = [POINTER(limnPolyData), c_uint, c_uint, c_uint, c_uint]
limnPolyDataSize = _libraries[libteem].limnPolyDataSize
limnPolyDataSize.restype = size_t
limnPolyDataSize.argtypes = [POINTER(limnPolyData)]
limnPolyDataCopy = _libraries[libteem].limnPolyDataCopy
limnPolyDataCopy.restype = c_int
limnPolyDataCopy.argtypes = [POINTER(limnPolyData), POINTER(limnPolyData)]
limnPolyDataCopyN = _libraries[libteem].limnPolyDataCopyN
limnPolyDataCopyN.restype = c_int
limnPolyDataCopyN.argtypes = [POINTER(limnPolyData), POINTER(limnPolyData), c_uint]
limnPolyDataTransform_f = _libraries[libteem].limnPolyDataTransform_f
limnPolyDataTransform_f.restype = None
limnPolyDataTransform_f.argtypes = [POINTER(limnPolyData), POINTER(c_float)]
limnPolyDataTransform_d = _libraries[libteem].limnPolyDataTransform_d
limnPolyDataTransform_d.restype = None
limnPolyDataTransform_d.argtypes = [POINTER(limnPolyData), POINTER(c_double)]
limnPolyDataPolygonNumber = _libraries[libteem].limnPolyDataPolygonNumber
limnPolyDataPolygonNumber.restype = c_uint
limnPolyDataPolygonNumber.argtypes = [POINTER(limnPolyData)]
limnPolyDataVertexNormals = _libraries[libteem].limnPolyDataVertexNormals
limnPolyDataVertexNormals.restype = c_int
limnPolyDataVertexNormals.argtypes = [POINTER(limnPolyData)]
limnPolyDataPrimitiveTypes = _libraries[libteem].limnPolyDataPrimitiveTypes
limnPolyDataPrimitiveTypes.restype = c_uint
limnPolyDataPrimitiveTypes.argtypes = [POINTER(limnPolyData)]
limnPolyDataPrimitiveVertexNumber = _libraries[libteem].limnPolyDataPrimitiveVertexNumber
limnPolyDataPrimitiveVertexNumber.restype = c_int
limnPolyDataPrimitiveVertexNumber.argtypes = [POINTER(Nrrd), POINTER(limnPolyData)]
limnPolyDataPrimitiveArea = _libraries[libteem].limnPolyDataPrimitiveArea
limnPolyDataPrimitiveArea.restype = c_int
limnPolyDataPrimitiveArea.argtypes = [POINTER(Nrrd), POINTER(limnPolyData)]
limnPolyDataRasterize = _libraries[libteem].limnPolyDataRasterize
limnPolyDataRasterize.restype = c_int
limnPolyDataRasterize.argtypes = [POINTER(Nrrd), POINTER(limnPolyData), POINTER(c_double), POINTER(c_double), POINTER(size_t), c_int]
limnPolyDataColorSet = _libraries[libteem].limnPolyDataColorSet
limnPolyDataColorSet.restype = None
limnPolyDataColorSet.argtypes = [POINTER(limnPolyData), c_ubyte, c_ubyte, c_ubyte, c_ubyte]
limnPolyDataCube = _libraries[libteem].limnPolyDataCube
limnPolyDataCube.restype = c_int
limnPolyDataCube.argtypes = [POINTER(limnPolyData), c_uint, c_int]
limnPolyDataCone = _libraries[libteem].limnPolyDataCone
limnPolyDataCone.restype = c_int
limnPolyDataCone.argtypes = [POINTER(limnPolyData), c_uint, c_uint, c_int]
limnPolyDataCylinder = _libraries[libteem].limnPolyDataCylinder
limnPolyDataCylinder.restype = c_int
limnPolyDataCylinder.argtypes = [POINTER(limnPolyData), c_uint, c_uint, c_int]
limnPolyDataSuperquadric = _libraries[libteem].limnPolyDataSuperquadric
limnPolyDataSuperquadric.restype = c_int
limnPolyDataSuperquadric.argtypes = [POINTER(limnPolyData), c_uint, c_float, c_float, c_uint, c_uint]
limnPolyDataSpiralSuperquadric = _libraries[libteem].limnPolyDataSpiralSuperquadric
limnPolyDataSpiralSuperquadric.restype = c_int
limnPolyDataSpiralSuperquadric.argtypes = [POINTER(limnPolyData), c_uint, c_float, c_float, c_uint, c_uint]
limnPolyDataPolarSphere = _libraries[libteem].limnPolyDataPolarSphere
limnPolyDataPolarSphere.restype = c_int
limnPolyDataPolarSphere.argtypes = [POINTER(limnPolyData), c_uint, c_uint, c_uint]
limnPolyDataSpiralSphere = _libraries[libteem].limnPolyDataSpiralSphere
limnPolyDataSpiralSphere.restype = c_int
limnPolyDataSpiralSphere.argtypes = [POINTER(limnPolyData), c_uint, c_uint, c_uint]
limnPolyDataPlane = _libraries[libteem].limnPolyDataPlane
limnPolyDataPlane.restype = c_int
limnPolyDataPlane.argtypes = [POINTER(limnPolyData), c_uint, c_uint, c_uint]
limnPolyDataVertexWindingFix = _libraries[libteem].limnPolyDataVertexWindingFix
limnPolyDataVertexWindingFix.restype = c_int
limnPolyDataVertexWindingFix.argtypes = [POINTER(limnPolyData), c_int]
limnPolyDataClip = _libraries[libteem].limnPolyDataClip
limnPolyDataClip.restype = c_int
limnPolyDataClip.argtypes = [POINTER(limnPolyData), POINTER(Nrrd), c_double]
limnPolyDataVertexWindingFlip = _libraries[libteem].limnPolyDataVertexWindingFlip
limnPolyDataVertexWindingFlip.restype = c_int
limnPolyDataVertexWindingFlip.argtypes = [POINTER(limnPolyData)]
limnPolyDataCCFind = _libraries[libteem].limnPolyDataCCFind
limnPolyDataCCFind.restype = c_int
limnPolyDataCCFind.argtypes = [POINTER(limnPolyData)]
limnPolyDataPrimitiveSort = _libraries[libteem].limnPolyDataPrimitiveSort
limnPolyDataPrimitiveSort.restype = c_int
limnPolyDataPrimitiveSort.argtypes = [POINTER(limnPolyData), POINTER(Nrrd)]
limnPolyDataPrimitiveSelect = _libraries[libteem].limnPolyDataPrimitiveSelect
limnPolyDataPrimitiveSelect.restype = c_int
limnPolyDataPrimitiveSelect.argtypes = [POINTER(limnPolyData), POINTER(limnPolyData), POINTER(Nrrd)]
limnPolyDataSpiralTubeWrap = _libraries[libteem].limnPolyDataSpiralTubeWrap
limnPolyDataSpiralTubeWrap.restype = c_int
limnPolyDataSpiralTubeWrap.argtypes = [POINTER(limnPolyData), POINTER(limnPolyData), c_uint, POINTER(Nrrd), c_uint, c_uint, c_double]
limnObjectDescribe = _libraries[libteem].limnObjectDescribe
limnObjectDescribe.restype = c_int
limnObjectDescribe.argtypes = [POINTER(FILE), POINTER(limnObject)]
limnObjectReadOFF = _libraries[libteem].limnObjectReadOFF
limnObjectReadOFF.restype = c_int
limnObjectReadOFF.argtypes = [POINTER(limnObject), POINTER(FILE)]
limnObjectWriteOFF = _libraries[libteem].limnObjectWriteOFF
limnObjectWriteOFF.restype = c_int
limnObjectWriteOFF.argtypes = [POINTER(FILE), POINTER(limnObject)]
limnPolyDataWriteIV = _libraries[libteem].limnPolyDataWriteIV
limnPolyDataWriteIV.restype = c_int
limnPolyDataWriteIV.argtypes = [POINTER(FILE), POINTER(limnPolyData)]
limnPolyDataWriteLMPD = _libraries[libteem].limnPolyDataWriteLMPD
limnPolyDataWriteLMPD.restype = c_int
limnPolyDataWriteLMPD.argtypes = [POINTER(FILE), POINTER(limnPolyData)]
limnPolyDataReadLMPD = _libraries[libteem].limnPolyDataReadLMPD
limnPolyDataReadLMPD.restype = c_int
limnPolyDataReadLMPD.argtypes = [POINTER(limnPolyData), POINTER(FILE)]
limnPolyDataWriteVTK = _libraries[libteem].limnPolyDataWriteVTK
limnPolyDataWriteVTK.restype = c_int
limnPolyDataWriteVTK.argtypes = [POINTER(FILE), POINTER(limnPolyData)]
limnPolyDataReadOFF = _libraries[libteem].limnPolyDataReadOFF
limnPolyDataReadOFF.restype = c_int
limnPolyDataReadOFF.argtypes = [POINTER(limnPolyData), POINTER(FILE)]
limnPolyDataSave = _libraries[libteem].limnPolyDataSave
limnPolyDataSave.restype = c_int
limnPolyDataSave.argtypes = [STRING, POINTER(limnPolyData)]
limnHestPolyDataLMPD = (POINTER(hestCB)).in_dll(_libraries[libteem], 'limnHestPolyDataLMPD')
limnHestPolyDataOFF = (POINTER(hestCB)).in_dll(_libraries[libteem], 'limnHestPolyDataOFF')
limnObjectCubeAdd = _libraries[libteem].limnObjectCubeAdd
limnObjectCubeAdd.restype = c_int
limnObjectCubeAdd.argtypes = [POINTER(limnObject), c_uint]
limnObjectSquareAdd = _libraries[libteem].limnObjectSquareAdd
limnObjectSquareAdd.restype = c_int
limnObjectSquareAdd.argtypes = [POINTER(limnObject), c_uint]
limnObjectCylinderAdd = _libraries[libteem].limnObjectCylinderAdd
limnObjectCylinderAdd.restype = c_int
limnObjectCylinderAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_uint]
limnObjectPolarSphereAdd = _libraries[libteem].limnObjectPolarSphereAdd
limnObjectPolarSphereAdd.restype = c_int
limnObjectPolarSphereAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_uint, c_uint]
limnObjectConeAdd = _libraries[libteem].limnObjectConeAdd
limnObjectConeAdd.restype = c_int
limnObjectConeAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_uint]
limnObjectPolarSuperquadAdd = _libraries[libteem].limnObjectPolarSuperquadAdd
limnObjectPolarSuperquadAdd.restype = c_int
limnObjectPolarSuperquadAdd.argtypes = [POINTER(limnObject), c_uint, c_uint, c_float, c_float, c_uint, c_uint]
limnObjectWorldHomog = _libraries[libteem].limnObjectWorldHomog
limnObjectWorldHomog.restype = c_int
limnObjectWorldHomog.argtypes = [POINTER(limnObject)]
limnObjectFaceNormals = _libraries[libteem].limnObjectFaceNormals
limnObjectFaceNormals.restype = c_int
limnObjectFaceNormals.argtypes = [POINTER(limnObject), c_int]
limnObjectVertexNormals = _libraries[libteem].limnObjectVertexNormals
limnObjectVertexNormals.restype = c_int
limnObjectVertexNormals.argtypes = [POINTER(limnObject)]
limnObjectSpaceTransform = _libraries[libteem].limnObjectSpaceTransform
limnObjectSpaceTransform.restype = c_int
limnObjectSpaceTransform.argtypes = [POINTER(limnObject), POINTER(limnCamera), POINTER(limnWindow), c_int]
limnObjectPartTransform = _libraries[libteem].limnObjectPartTransform
limnObjectPartTransform.restype = c_int
limnObjectPartTransform.argtypes = [POINTER(limnObject), c_uint, POINTER(c_float)]
limnObjectDepthSortParts = _libraries[libteem].limnObjectDepthSortParts
limnObjectDepthSortParts.restype = c_int
limnObjectDepthSortParts.argtypes = [POINTER(limnObject)]
limnObjectDepthSortFaces = _libraries[libteem].limnObjectDepthSortFaces
limnObjectDepthSortFaces.restype = c_int
limnObjectDepthSortFaces.argtypes = [POINTER(limnObject)]
limnObjectFaceReverse = _libraries[libteem].limnObjectFaceReverse
limnObjectFaceReverse.restype = c_int
limnObjectFaceReverse.argtypes = [POINTER(limnObject)]
limnObjectRender = _libraries[libteem].limnObjectRender
limnObjectRender.restype = c_int
limnObjectRender.argtypes = [POINTER(limnObject), POINTER(limnCamera), POINTER(limnWindow)]
limnObjectPSDraw = _libraries[libteem].limnObjectPSDraw
limnObjectPSDraw.restype = c_int
limnObjectPSDraw.argtypes = [POINTER(limnObject), POINTER(limnCamera), POINTER(Nrrd), POINTER(limnWindow)]
limnObjectPSDrawConcave = _libraries[libteem].limnObjectPSDrawConcave
limnObjectPSDrawConcave.restype = c_int
limnObjectPSDrawConcave.argtypes = [POINTER(limnObject), POINTER(limnCamera), POINTER(Nrrd), POINTER(limnWindow)]
limnSplineTypeSpecNew = _libraries[libteem].limnSplineTypeSpecNew
limnSplineTypeSpecNew.restype = POINTER(limnSplineTypeSpec)
limnSplineTypeSpecNew.argtypes = [c_int]
limnSplineTypeSpecNix = _libraries[libteem].limnSplineTypeSpecNix
limnSplineTypeSpecNix.restype = POINTER(limnSplineTypeSpec)
limnSplineTypeSpecNix.argtypes = [POINTER(limnSplineTypeSpec)]
limnSplineNew = _libraries[libteem].limnSplineNew
limnSplineNew.restype = POINTER(limnSpline)
limnSplineNew.argtypes = [POINTER(Nrrd), c_int, POINTER(limnSplineTypeSpec)]
limnSplineNix = _libraries[libteem].limnSplineNix
limnSplineNix.restype = POINTER(limnSpline)
limnSplineNix.argtypes = [POINTER(limnSpline)]
limnSplineNrrdCleverFix = _libraries[libteem].limnSplineNrrdCleverFix
limnSplineNrrdCleverFix.restype = c_int
limnSplineNrrdCleverFix.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, c_int]
limnSplineCleverNew = _libraries[libteem].limnSplineCleverNew
limnSplineCleverNew.restype = POINTER(limnSpline)
limnSplineCleverNew.argtypes = [POINTER(Nrrd), c_int, POINTER(limnSplineTypeSpec)]
limnSplineUpdate = _libraries[libteem].limnSplineUpdate
limnSplineUpdate.restype = c_int
limnSplineUpdate.argtypes = [POINTER(limnSpline), POINTER(Nrrd)]
limnSplineType = (POINTER(airEnum)).in_dll(_libraries[libteem], 'limnSplineType')
limnSplineInfo = (POINTER(airEnum)).in_dll(_libraries[libteem], 'limnSplineInfo')
limnSplineParse = _libraries[libteem].limnSplineParse
limnSplineParse.restype = POINTER(limnSpline)
limnSplineParse.argtypes = [STRING]
limnSplineTypeSpecParse = _libraries[libteem].limnSplineTypeSpecParse
limnSplineTypeSpecParse.restype = POINTER(limnSplineTypeSpec)
limnSplineTypeSpecParse.argtypes = [STRING]
limnHestSpline = (POINTER(hestCB)).in_dll(_libraries[libteem], 'limnHestSpline')
limnHestSplineTypeSpec = (POINTER(hestCB)).in_dll(_libraries[libteem], 'limnHestSplineTypeSpec')
limnSplineInfoSize = (c_uint * 7).in_dll(_libraries[libteem], 'limnSplineInfoSize')
limnSplineTypeHasImplicitTangents = (c_int * 6).in_dll(_libraries[libteem], 'limnSplineTypeHasImplicitTangents')
limnSplineNumPoints = _libraries[libteem].limnSplineNumPoints
limnSplineNumPoints.restype = c_int
limnSplineNumPoints.argtypes = [POINTER(limnSpline)]
limnSplineMinT = _libraries[libteem].limnSplineMinT
limnSplineMinT.restype = c_double
limnSplineMinT.argtypes = [POINTER(limnSpline)]
limnSplineMaxT = _libraries[libteem].limnSplineMaxT
limnSplineMaxT.restype = c_double
limnSplineMaxT.argtypes = [POINTER(limnSpline)]
limnSplineBCSet = _libraries[libteem].limnSplineBCSet
limnSplineBCSet.restype = None
limnSplineBCSet.argtypes = [POINTER(limnSpline), c_double, c_double]
limnSplineEvaluate = _libraries[libteem].limnSplineEvaluate
limnSplineEvaluate.restype = None
limnSplineEvaluate.argtypes = [POINTER(c_double), POINTER(limnSpline), c_double]
limnSplineNrrdEvaluate = _libraries[libteem].limnSplineNrrdEvaluate
limnSplineNrrdEvaluate.restype = c_int
limnSplineNrrdEvaluate.argtypes = [POINTER(Nrrd), POINTER(limnSpline), POINTER(Nrrd)]
limnSplineSample = _libraries[libteem].limnSplineSample
limnSplineSample.restype = c_int
limnSplineSample.argtypes = [POINTER(Nrrd), POINTER(limnSpline), c_double, size_t, c_double]
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
miteBiffKey = (STRING).in_dll(_libraries[libteem], 'miteBiffKey')
miteDefRefStep = (c_double).in_dll(_libraries[libteem], 'miteDefRefStep')
miteDefRenorm = (c_int).in_dll(_libraries[libteem], 'miteDefRenorm')
miteDefNormalSide = (c_int).in_dll(_libraries[libteem], 'miteDefNormalSide')
miteDefOpacNear1 = (c_double).in_dll(_libraries[libteem], 'miteDefOpacNear1')
miteDefOpacMatters = (c_double).in_dll(_libraries[libteem], 'miteDefOpacMatters')
miteVal = (POINTER(airEnum)).in_dll(_libraries[libteem], 'miteVal')
miteValGageKind = (POINTER(gageKind)).in_dll(_libraries[libteem], 'miteValGageKind')
miteStageOp = (POINTER(airEnum)).in_dll(_libraries[libteem], 'miteStageOp')
miteRangeChar = (c_char * 10).in_dll(_libraries[libteem], 'miteRangeChar')
miteVariableParse = _libraries[libteem].miteVariableParse
miteVariableParse.restype = c_int
miteVariableParse.argtypes = [POINTER(gageItemSpec), STRING]
miteVariablePrint = _libraries[libteem].miteVariablePrint
miteVariablePrint.restype = None
miteVariablePrint.argtypes = [STRING, POINTER(gageItemSpec)]
miteNtxfCheck = _libraries[libteem].miteNtxfCheck
miteNtxfCheck.restype = c_int
miteNtxfCheck.argtypes = [POINTER(Nrrd)]
miteQueryAdd = _libraries[libteem].miteQueryAdd
miteQueryAdd.restype = None
miteQueryAdd.argtypes = [POINTER(c_ubyte), POINTER(c_ubyte), POINTER(c_ubyte), POINTER(c_ubyte), POINTER(gageItemSpec)]
miteUserNew = _libraries[libteem].miteUserNew
miteUserNew.restype = POINTER(miteUser)
miteUserNew.argtypes = []
miteUserNix = _libraries[libteem].miteUserNix
miteUserNix.restype = POINTER(miteUser)
miteUserNix.argtypes = [POINTER(miteUser)]
miteShadeSpecNew = _libraries[libteem].miteShadeSpecNew
miteShadeSpecNew.restype = POINTER(miteShadeSpec)
miteShadeSpecNew.argtypes = []
miteShadeSpecNix = _libraries[libteem].miteShadeSpecNix
miteShadeSpecNix.restype = POINTER(miteShadeSpec)
miteShadeSpecNix.argtypes = [POINTER(miteShadeSpec)]
miteShadeSpecParse = _libraries[libteem].miteShadeSpecParse
miteShadeSpecParse.restype = c_int
miteShadeSpecParse.argtypes = [POINTER(miteShadeSpec), STRING]
miteShadeSpecPrint = _libraries[libteem].miteShadeSpecPrint
miteShadeSpecPrint.restype = None
miteShadeSpecPrint.argtypes = [STRING, POINTER(miteShadeSpec)]
miteShadeSpecQueryAdd = _libraries[libteem].miteShadeSpecQueryAdd
miteShadeSpecQueryAdd.restype = None
miteShadeSpecQueryAdd.argtypes = [POINTER(c_ubyte), POINTER(c_ubyte), POINTER(c_ubyte), POINTER(c_ubyte), POINTER(miteShadeSpec)]
miteRenderBegin = _libraries[libteem].miteRenderBegin
miteRenderBegin.restype = c_int
miteRenderBegin.argtypes = [POINTER(POINTER(miteRender)), POINTER(miteUser)]
miteRenderEnd = _libraries[libteem].miteRenderEnd
miteRenderEnd.restype = c_int
miteRenderEnd.argtypes = [POINTER(miteRender), POINTER(miteUser)]
miteThreadNew = _libraries[libteem].miteThreadNew
miteThreadNew.restype = POINTER(miteThread)
miteThreadNew.argtypes = []
miteThreadNix = _libraries[libteem].miteThreadNix
miteThreadNix.restype = POINTER(miteThread)
miteThreadNix.argtypes = [POINTER(miteThread)]
miteThreadBegin = _libraries[libteem].miteThreadBegin
miteThreadBegin.restype = c_int
miteThreadBegin.argtypes = [POINTER(POINTER(miteThread)), POINTER(miteRender), POINTER(miteUser), c_int]
miteThreadEnd = _libraries[libteem].miteThreadEnd
miteThreadEnd.restype = c_int
miteThreadEnd.argtypes = [POINTER(miteThread), POINTER(miteRender), POINTER(miteUser)]
miteRayBegin = _libraries[libteem].miteRayBegin
miteRayBegin.restype = c_int
miteRayBegin.argtypes = [POINTER(miteThread), POINTER(miteRender), POINTER(miteUser), c_int, c_int, c_double, POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double)]
miteSample = _libraries[libteem].miteSample
miteSample.restype = c_double
miteSample.argtypes = [POINTER(miteThread), POINTER(miteRender), POINTER(miteUser), c_int, c_double, c_int, POINTER(c_double), POINTER(c_double)]
miteRayEnd = _libraries[libteem].miteRayEnd
miteRayEnd.restype = c_int
miteRayEnd.argtypes = [POINTER(miteThread), POINTER(miteRender), POINTER(miteUser)]
nrrdDefaultWriteEncodingType = (c_int).in_dll(_libraries[libteem], 'nrrdDefaultWriteEncodingType')
nrrdDefaultWriteBareText = (c_int).in_dll(_libraries[libteem], 'nrrdDefaultWriteBareText')
nrrdDefaultWriteCharsPerLine = (c_uint).in_dll(_libraries[libteem], 'nrrdDefaultWriteCharsPerLine')
nrrdDefaultWriteValsPerLine = (c_uint).in_dll(_libraries[libteem], 'nrrdDefaultWriteValsPerLine')
nrrdDefaultResampleBoundary = (c_int).in_dll(_libraries[libteem], 'nrrdDefaultResampleBoundary')
nrrdDefaultResampleType = (c_int).in_dll(_libraries[libteem], 'nrrdDefaultResampleType')
nrrdDefaultResampleRenormalize = (c_int).in_dll(_libraries[libteem], 'nrrdDefaultResampleRenormalize')
nrrdDefaultResampleRound = (c_int).in_dll(_libraries[libteem], 'nrrdDefaultResampleRound')
nrrdDefaultResampleClamp = (c_int).in_dll(_libraries[libteem], 'nrrdDefaultResampleClamp')
nrrdDefaultResampleCheap = (c_int).in_dll(_libraries[libteem], 'nrrdDefaultResampleCheap')
nrrdDefaultResamplePadValue = (c_double).in_dll(_libraries[libteem], 'nrrdDefaultResamplePadValue')
nrrdDefaultKernelParm0 = (c_double).in_dll(_libraries[libteem], 'nrrdDefaultKernelParm0')
nrrdDefaultCenter = (c_int).in_dll(_libraries[libteem], 'nrrdDefaultCenter')
nrrdDefaultSpacing = (c_double).in_dll(_libraries[libteem], 'nrrdDefaultSpacing')
nrrdStateVerboseIO = (c_int).in_dll(_libraries[libteem], 'nrrdStateVerboseIO')
nrrdStateKeyValuePairsPropagate = (c_int).in_dll(_libraries[libteem], 'nrrdStateKeyValuePairsPropagate')
nrrdStateBlind8BitRange = (c_int).in_dll(_libraries[libteem], 'nrrdStateBlind8BitRange')
nrrdStateMeasureType = (c_int).in_dll(_libraries[libteem], 'nrrdStateMeasureType')
nrrdStateMeasureModeBins = (c_int).in_dll(_libraries[libteem], 'nrrdStateMeasureModeBins')
nrrdStateMeasureHistoType = (c_int).in_dll(_libraries[libteem], 'nrrdStateMeasureHistoType')
nrrdStateDisallowIntegerNonExist = (c_int).in_dll(_libraries[libteem], 'nrrdStateDisallowIntegerNonExist')
nrrdStateAlwaysSetContent = (c_int).in_dll(_libraries[libteem], 'nrrdStateAlwaysSetContent')
nrrdStateDisableContent = (c_int).in_dll(_libraries[libteem], 'nrrdStateDisableContent')
nrrdStateUnknownContent = (STRING).in_dll(_libraries[libteem], 'nrrdStateUnknownContent')
nrrdStateGrayscaleImage3D = (c_int).in_dll(_libraries[libteem], 'nrrdStateGrayscaleImage3D')
nrrdStateKeyValueReturnInternalPointers = (c_int).in_dll(_libraries[libteem], 'nrrdStateKeyValueReturnInternalPointers')
nrrdStateKindNoop = (c_int).in_dll(_libraries[libteem], 'nrrdStateKindNoop')
nrrdEnvVarDefaultWriteEncodingType = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarDefaultWriteEncodingType')
nrrdEnvVarDefaultWriteBareText = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarDefaultWriteBareText')
nrrdEnvVarDefaultWriteBareTextOld = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarDefaultWriteBareTextOld')
nrrdEnvVarDefaultCenter = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarDefaultCenter')
nrrdEnvVarDefaultCenterOld = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarDefaultCenterOld')
nrrdEnvVarDefaultWriteCharsPerLine = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarDefaultWriteCharsPerLine')
nrrdEnvVarDefaultWriteValsPerLine = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarDefaultWriteValsPerLine')
nrrdEnvVarDefaultKernelParm0 = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarDefaultKernelParm0')
nrrdEnvVarDefaultSpacing = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarDefaultSpacing')
nrrdEnvVarStateKindNoop = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarStateKindNoop')
nrrdEnvVarStateVerboseIO = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarStateVerboseIO')
nrrdEnvVarStateKeyValuePairsPropagate = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarStateKeyValuePairsPropagate')
nrrdEnvVarStateBlind8BitRange = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarStateBlind8BitRange')
nrrdEnvVarStateAlwaysSetContent = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarStateAlwaysSetContent')
nrrdEnvVarStateDisableContent = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarStateDisableContent')
nrrdEnvVarStateMeasureType = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarStateMeasureType')
nrrdEnvVarStateMeasureModeBins = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarStateMeasureModeBins')
nrrdEnvVarStateMeasureHistoType = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarStateMeasureHistoType')
nrrdEnvVarStateGrayscaleImage3D = (STRING).in_dll(_libraries[libteem], 'nrrdEnvVarStateGrayscaleImage3D')
nrrdGetenvBool = _libraries[libteem].nrrdGetenvBool
nrrdGetenvBool.restype = c_int
nrrdGetenvBool.argtypes = [POINTER(c_int), POINTER(STRING), STRING]
nrrdGetenvEnum = _libraries[libteem].nrrdGetenvEnum
nrrdGetenvEnum.restype = c_int
nrrdGetenvEnum.argtypes = [POINTER(c_int), POINTER(STRING), POINTER(airEnum), STRING]
nrrdGetenvInt = _libraries[libteem].nrrdGetenvInt
nrrdGetenvInt.restype = c_int
nrrdGetenvInt.argtypes = [POINTER(c_int), POINTER(STRING), STRING]
nrrdGetenvUInt = _libraries[libteem].nrrdGetenvUInt
nrrdGetenvUInt.restype = c_int
nrrdGetenvUInt.argtypes = [POINTER(c_uint), POINTER(STRING), STRING]
nrrdGetenvDouble = _libraries[libteem].nrrdGetenvDouble
nrrdGetenvDouble.restype = c_int
nrrdGetenvDouble.argtypes = [POINTER(c_double), POINTER(STRING), STRING]
nrrdDefaultGetenv = _libraries[libteem].nrrdDefaultGetenv
nrrdDefaultGetenv.restype = None
nrrdDefaultGetenv.argtypes = []
nrrdStateGetenv = _libraries[libteem].nrrdStateGetenv
nrrdStateGetenv.restype = None
nrrdStateGetenv.argtypes = []
nrrdFormatType = (POINTER(airEnum)).in_dll(_libraries[libteem], 'nrrdFormatType')
nrrdType = (POINTER(airEnum)).in_dll(_libraries[libteem], 'nrrdType')
nrrdEncodingType = (POINTER(airEnum)).in_dll(_libraries[libteem], 'nrrdEncodingType')
nrrdCenter = (POINTER(airEnum)).in_dll(_libraries[libteem], 'nrrdCenter')
nrrdKind = (POINTER(airEnum)).in_dll(_libraries[libteem], 'nrrdKind')
nrrdField = (POINTER(airEnum)).in_dll(_libraries[libteem], 'nrrdField')
nrrdSpace = (POINTER(airEnum)).in_dll(_libraries[libteem], 'nrrdSpace')
nrrdBoundary = (POINTER(airEnum)).in_dll(_libraries[libteem], 'nrrdBoundary')
nrrdMeasure = (POINTER(airEnum)).in_dll(_libraries[libteem], 'nrrdMeasure')
nrrdUnaryOp = (POINTER(airEnum)).in_dll(_libraries[libteem], 'nrrdUnaryOp')
nrrdBinaryOp = (POINTER(airEnum)).in_dll(_libraries[libteem], 'nrrdBinaryOp')
nrrdTernaryOp = (POINTER(airEnum)).in_dll(_libraries[libteem], 'nrrdTernaryOp')
nrrdTypePrintfStr = (c_char * 129 * 0).in_dll(_libraries[libteem], 'nrrdTypePrintfStr')
nrrdTypeSize = (size_t * 0).in_dll(_libraries[libteem], 'nrrdTypeSize')
nrrdTypeMin = (c_double * 0).in_dll(_libraries[libteem], 'nrrdTypeMin')
nrrdTypeMax = (c_double * 0).in_dll(_libraries[libteem], 'nrrdTypeMax')
nrrdTypeIsIntegral = (c_int * 0).in_dll(_libraries[libteem], 'nrrdTypeIsIntegral')
nrrdTypeIsUnsigned = (c_int * 0).in_dll(_libraries[libteem], 'nrrdTypeIsUnsigned')
nrrdTypeNumberOfValues = (c_double * 0).in_dll(_libraries[libteem], 'nrrdTypeNumberOfValues')
class NrrdIoState_t(Structure):
    pass
NrrdIoState = NrrdIoState_t
nrrdIoStateNew = _libraries[libteem].nrrdIoStateNew
nrrdIoStateNew.restype = POINTER(NrrdIoState)
nrrdIoStateNew.argtypes = []
nrrdIoStateInit = _libraries[libteem].nrrdIoStateInit
nrrdIoStateInit.restype = None
nrrdIoStateInit.argtypes = [POINTER(NrrdIoState)]
nrrdIoStateNix = _libraries[libteem].nrrdIoStateNix
nrrdIoStateNix.restype = POINTER(NrrdIoState)
nrrdIoStateNix.argtypes = [POINTER(NrrdIoState)]
class NrrdResampleInfo(Structure):
    pass
nrrdResampleInfoNew = _libraries[libteem].nrrdResampleInfoNew
nrrdResampleInfoNew.restype = POINTER(NrrdResampleInfo)
nrrdResampleInfoNew.argtypes = []
nrrdResampleInfoNix = _libraries[libteem].nrrdResampleInfoNix
nrrdResampleInfoNix.restype = POINTER(NrrdResampleInfo)
nrrdResampleInfoNix.argtypes = [POINTER(NrrdResampleInfo)]
nrrdKernelSpecNew = _libraries[libteem].nrrdKernelSpecNew
nrrdKernelSpecNew.restype = POINTER(NrrdKernelSpec)
nrrdKernelSpecNew.argtypes = []
nrrdKernelSpecCopy = _libraries[libteem].nrrdKernelSpecCopy
nrrdKernelSpecCopy.restype = POINTER(NrrdKernelSpec)
nrrdKernelSpecCopy.argtypes = [POINTER(NrrdKernelSpec)]
nrrdKernelSpecSet = _libraries[libteem].nrrdKernelSpecSet
nrrdKernelSpecSet.restype = None
nrrdKernelSpecSet.argtypes = [POINTER(NrrdKernelSpec), POINTER(NrrdKernel), POINTER(c_double)]
nrrdKernelParmSet = _libraries[libteem].nrrdKernelParmSet
nrrdKernelParmSet.restype = None
nrrdKernelParmSet.argtypes = [POINTER(POINTER(NrrdKernel)), POINTER(c_double), POINTER(NrrdKernelSpec)]
nrrdKernelSpecNix = _libraries[libteem].nrrdKernelSpecNix
nrrdKernelSpecNix.restype = POINTER(NrrdKernelSpec)
nrrdKernelSpecNix.argtypes = [POINTER(NrrdKernelSpec)]
nrrdInit = _libraries[libteem].nrrdInit
nrrdInit.restype = None
nrrdInit.argtypes = [POINTER(Nrrd)]
nrrdNew = _libraries[libteem].nrrdNew
nrrdNew.restype = POINTER(Nrrd)
nrrdNew.argtypes = []
nrrdNix = _libraries[libteem].nrrdNix
nrrdNix.restype = POINTER(Nrrd)
nrrdNix.argtypes = [POINTER(Nrrd)]
nrrdEmpty = _libraries[libteem].nrrdEmpty
nrrdEmpty.restype = POINTER(Nrrd)
nrrdEmpty.argtypes = [POINTER(Nrrd)]
nrrdNuke = _libraries[libteem].nrrdNuke
nrrdNuke.restype = POINTER(Nrrd)
nrrdNuke.argtypes = [POINTER(Nrrd)]
nrrdWrap_nva = _libraries[libteem].nrrdWrap_nva
nrrdWrap_nva.restype = c_int
nrrdWrap_nva.argtypes = [POINTER(Nrrd), c_void_p, c_int, c_uint, POINTER(size_t)]
nrrdWrap_va = _libraries[libteem].nrrdWrap_va
nrrdWrap_va.restype = c_int
nrrdWrap_va.argtypes = [POINTER(Nrrd), c_void_p, c_int, c_uint]
nrrdBasicInfoInit = _libraries[libteem].nrrdBasicInfoInit
nrrdBasicInfoInit.restype = None
nrrdBasicInfoInit.argtypes = [POINTER(Nrrd), c_int]
nrrdBasicInfoCopy = _libraries[libteem].nrrdBasicInfoCopy
nrrdBasicInfoCopy.restype = c_int
nrrdBasicInfoCopy.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdCopy = _libraries[libteem].nrrdCopy
nrrdCopy.restype = c_int
nrrdCopy.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
nrrdAlloc_nva = _libraries[libteem].nrrdAlloc_nva
nrrdAlloc_nva.restype = c_int
nrrdAlloc_nva.argtypes = [POINTER(Nrrd), c_int, c_uint, POINTER(size_t)]
nrrdAlloc_va = _libraries[libteem].nrrdAlloc_va
nrrdAlloc_va.restype = c_int
nrrdAlloc_va.argtypes = [POINTER(Nrrd), c_int, c_uint]
nrrdMaybeAlloc_nva = _libraries[libteem].nrrdMaybeAlloc_nva
nrrdMaybeAlloc_nva.restype = c_int
nrrdMaybeAlloc_nva.argtypes = [POINTER(Nrrd), c_int, c_uint, POINTER(size_t)]
nrrdMaybeAlloc_va = _libraries[libteem].nrrdMaybeAlloc_va
nrrdMaybeAlloc_va.restype = c_int
nrrdMaybeAlloc_va.argtypes = [POINTER(Nrrd), c_int, c_uint]
nrrdPPM = _libraries[libteem].nrrdPPM
nrrdPPM.restype = c_int
nrrdPPM.argtypes = [POINTER(Nrrd), size_t, size_t]
nrrdPGM = _libraries[libteem].nrrdPGM
nrrdPGM.restype = c_int
nrrdPGM.argtypes = [POINTER(Nrrd), size_t, size_t]
nrrdKindIsDomain = _libraries[libteem].nrrdKindIsDomain
nrrdKindIsDomain.restype = c_int
nrrdKindIsDomain.argtypes = [c_int]
nrrdKindSize = _libraries[libteem].nrrdKindSize
nrrdKindSize.restype = c_uint
nrrdKindSize.argtypes = [c_int]
nrrdAxisInfoCopy = _libraries[libteem].nrrdAxisInfoCopy
nrrdAxisInfoCopy.restype = c_int
nrrdAxisInfoCopy.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(c_int), c_int]
nrrdAxisInfoSet_nva = _libraries[libteem].nrrdAxisInfoSet_nva
nrrdAxisInfoSet_nva.restype = None
nrrdAxisInfoSet_nva.argtypes = [POINTER(Nrrd), c_int, c_void_p]
nrrdAxisInfoSet_va = _libraries[libteem].nrrdAxisInfoSet_va
nrrdAxisInfoSet_va.restype = None
nrrdAxisInfoSet_va.argtypes = [POINTER(Nrrd), c_int]
nrrdAxisInfoGet_nva = _libraries[libteem].nrrdAxisInfoGet_nva
nrrdAxisInfoGet_nva.restype = None
nrrdAxisInfoGet_nva.argtypes = [POINTER(Nrrd), c_int, c_void_p]
nrrdAxisInfoGet_va = _libraries[libteem].nrrdAxisInfoGet_va
nrrdAxisInfoGet_va.restype = None
nrrdAxisInfoGet_va.argtypes = [POINTER(Nrrd), c_int]
nrrdAxisInfoPos = _libraries[libteem].nrrdAxisInfoPos
nrrdAxisInfoPos.restype = c_double
nrrdAxisInfoPos.argtypes = [POINTER(Nrrd), c_uint, c_double]
nrrdAxisInfoIdx = _libraries[libteem].nrrdAxisInfoIdx
nrrdAxisInfoIdx.restype = c_double
nrrdAxisInfoIdx.argtypes = [POINTER(Nrrd), c_uint, c_double]
nrrdAxisInfoPosRange = _libraries[libteem].nrrdAxisInfoPosRange
nrrdAxisInfoPosRange.restype = None
nrrdAxisInfoPosRange.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(Nrrd), c_uint, c_double, c_double]
nrrdAxisInfoIdxRange = _libraries[libteem].nrrdAxisInfoIdxRange
nrrdAxisInfoIdxRange.restype = None
nrrdAxisInfoIdxRange.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(Nrrd), c_uint, c_double, c_double]
nrrdAxisInfoSpacingSet = _libraries[libteem].nrrdAxisInfoSpacingSet
nrrdAxisInfoSpacingSet.restype = None
nrrdAxisInfoSpacingSet.argtypes = [POINTER(Nrrd), c_uint]
nrrdAxisInfoMinMaxSet = _libraries[libteem].nrrdAxisInfoMinMaxSet
nrrdAxisInfoMinMaxSet.restype = None
nrrdAxisInfoMinMaxSet.argtypes = [POINTER(Nrrd), c_uint, c_int]
nrrdDomainAxesGet = _libraries[libteem].nrrdDomainAxesGet
nrrdDomainAxesGet.restype = c_uint
nrrdDomainAxesGet.argtypes = [POINTER(Nrrd), POINTER(c_uint)]
nrrdRangeAxesGet = _libraries[libteem].nrrdRangeAxesGet
nrrdRangeAxesGet.restype = c_uint
nrrdRangeAxesGet.argtypes = [POINTER(Nrrd), POINTER(c_uint)]
nrrdSpatialAxesGet = _libraries[libteem].nrrdSpatialAxesGet
nrrdSpatialAxesGet.restype = c_uint
nrrdSpatialAxesGet.argtypes = [POINTER(Nrrd), POINTER(c_uint)]
nrrdNonSpatialAxesGet = _libraries[libteem].nrrdNonSpatialAxesGet
nrrdNonSpatialAxesGet.restype = c_uint
nrrdNonSpatialAxesGet.argtypes = [POINTER(Nrrd), POINTER(c_uint)]
nrrdSpacingCalculate = _libraries[libteem].nrrdSpacingCalculate
nrrdSpacingCalculate.restype = c_int
nrrdSpacingCalculate.argtypes = [POINTER(Nrrd), c_uint, POINTER(c_double), POINTER(c_double)]
nrrdOrientationReduce = _libraries[libteem].nrrdOrientationReduce
nrrdOrientationReduce.restype = c_int
nrrdOrientationReduce.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdBiffKey = (STRING).in_dll(_libraries[libteem], 'nrrdBiffKey')
nrrdSpaceDimension = _libraries[libteem].nrrdSpaceDimension
nrrdSpaceDimension.restype = c_uint
nrrdSpaceDimension.argtypes = [c_int]
nrrdSpaceSet = _libraries[libteem].nrrdSpaceSet
nrrdSpaceSet.restype = c_int
nrrdSpaceSet.argtypes = [POINTER(Nrrd), c_int]
nrrdSpaceDimensionSet = _libraries[libteem].nrrdSpaceDimensionSet
nrrdSpaceDimensionSet.restype = c_int
nrrdSpaceDimensionSet.argtypes = [POINTER(Nrrd), c_uint]
nrrdSpaceOriginGet = _libraries[libteem].nrrdSpaceOriginGet
nrrdSpaceOriginGet.restype = c_uint
nrrdSpaceOriginGet.argtypes = [POINTER(Nrrd), POINTER(c_double)]
nrrdSpaceOriginSet = _libraries[libteem].nrrdSpaceOriginSet
nrrdSpaceOriginSet.restype = c_int
nrrdSpaceOriginSet.argtypes = [POINTER(Nrrd), POINTER(c_double)]
nrrdOriginCalculate = _libraries[libteem].nrrdOriginCalculate
nrrdOriginCalculate.restype = c_int
nrrdOriginCalculate.argtypes = [POINTER(Nrrd), POINTER(c_uint), c_uint, c_int, POINTER(c_double)]
nrrdContentSet_va = _libraries[libteem].nrrdContentSet_va
nrrdContentSet_va.restype = c_int
nrrdContentSet_va.argtypes = [POINTER(Nrrd), STRING, POINTER(Nrrd), STRING]
nrrdDescribe = _libraries[libteem].nrrdDescribe
nrrdDescribe.restype = None
nrrdDescribe.argtypes = [POINTER(FILE), POINTER(Nrrd)]
nrrdCheck = _libraries[libteem].nrrdCheck
nrrdCheck.restype = c_int
nrrdCheck.argtypes = [POINTER(Nrrd)]
nrrdElementSize = _libraries[libteem].nrrdElementSize
nrrdElementSize.restype = size_t
nrrdElementSize.argtypes = [POINTER(Nrrd)]
nrrdElementNumber = _libraries[libteem].nrrdElementNumber
nrrdElementNumber.restype = size_t
nrrdElementNumber.argtypes = [POINTER(Nrrd)]
nrrdSanity = _libraries[libteem].nrrdSanity
nrrdSanity.restype = c_int
nrrdSanity.argtypes = []
nrrdSameSize = _libraries[libteem].nrrdSameSize
nrrdSameSize.restype = c_int
nrrdSameSize.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdSpaceVecCopy = _libraries[libteem].nrrdSpaceVecCopy
nrrdSpaceVecCopy.restype = None
nrrdSpaceVecCopy.argtypes = [POINTER(c_double), POINTER(c_double)]
nrrdSpaceVecScaleAdd2 = _libraries[libteem].nrrdSpaceVecScaleAdd2
nrrdSpaceVecScaleAdd2.restype = None
nrrdSpaceVecScaleAdd2.argtypes = [POINTER(c_double), c_double, POINTER(c_double), c_double, POINTER(c_double)]
nrrdSpaceVecScale = _libraries[libteem].nrrdSpaceVecScale
nrrdSpaceVecScale.restype = None
nrrdSpaceVecScale.argtypes = [POINTER(c_double), c_double, POINTER(c_double)]
nrrdSpaceVecNorm = _libraries[libteem].nrrdSpaceVecNorm
nrrdSpaceVecNorm.restype = c_double
nrrdSpaceVecNorm.argtypes = [c_int, POINTER(c_double)]
nrrdSpaceVecSetNaN = _libraries[libteem].nrrdSpaceVecSetNaN
nrrdSpaceVecSetNaN.restype = None
nrrdSpaceVecSetNaN.argtypes = [POINTER(c_double)]
nrrdCommentAdd = _libraries[libteem].nrrdCommentAdd
nrrdCommentAdd.restype = c_int
nrrdCommentAdd.argtypes = [POINTER(Nrrd), STRING]
nrrdCommentClear = _libraries[libteem].nrrdCommentClear
nrrdCommentClear.restype = None
nrrdCommentClear.argtypes = [POINTER(Nrrd)]
nrrdCommentCopy = _libraries[libteem].nrrdCommentCopy
nrrdCommentCopy.restype = c_int
nrrdCommentCopy.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
nrrdKeyValueSize = _libraries[libteem].nrrdKeyValueSize
nrrdKeyValueSize.restype = c_uint
nrrdKeyValueSize.argtypes = [POINTER(Nrrd)]
nrrdKeyValueAdd = _libraries[libteem].nrrdKeyValueAdd
nrrdKeyValueAdd.restype = c_int
nrrdKeyValueAdd.argtypes = [POINTER(Nrrd), STRING, STRING]
nrrdKeyValueGet = _libraries[libteem].nrrdKeyValueGet
nrrdKeyValueGet.restype = STRING
nrrdKeyValueGet.argtypes = [POINTER(Nrrd), STRING]
nrrdKeyValueIndex = _libraries[libteem].nrrdKeyValueIndex
nrrdKeyValueIndex.restype = None
nrrdKeyValueIndex.argtypes = [POINTER(Nrrd), POINTER(STRING), POINTER(STRING), c_uint]
nrrdKeyValueErase = _libraries[libteem].nrrdKeyValueErase
nrrdKeyValueErase.restype = c_int
nrrdKeyValueErase.argtypes = [POINTER(Nrrd), STRING]
nrrdKeyValueClear = _libraries[libteem].nrrdKeyValueClear
nrrdKeyValueClear.restype = None
nrrdKeyValueClear.argtypes = [POINTER(Nrrd)]
nrrdKeyValueCopy = _libraries[libteem].nrrdKeyValueCopy
nrrdKeyValueCopy.restype = c_int
nrrdKeyValueCopy.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
nrrdSwapEndian = _libraries[libteem].nrrdSwapEndian
nrrdSwapEndian.restype = None
nrrdSwapEndian.argtypes = [POINTER(Nrrd)]
class NrrdFormat(Structure):
    pass
class NrrdEncoding_t(Structure):
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
nrrdFormatNRRD = (POINTER(NrrdFormat)).in_dll(_libraries[libteem], 'nrrdFormatNRRD')
nrrdFormatPNM = (POINTER(NrrdFormat)).in_dll(_libraries[libteem], 'nrrdFormatPNM')
nrrdFormatPNG = (POINTER(NrrdFormat)).in_dll(_libraries[libteem], 'nrrdFormatPNG')
nrrdFormatVTK = (POINTER(NrrdFormat)).in_dll(_libraries[libteem], 'nrrdFormatVTK')
nrrdFormatText = (POINTER(NrrdFormat)).in_dll(_libraries[libteem], 'nrrdFormatText')
nrrdFormatEPS = (POINTER(NrrdFormat)).in_dll(_libraries[libteem], 'nrrdFormatEPS')
nrrdFormatUnknown = (POINTER(NrrdFormat)).in_dll(_libraries[libteem], 'nrrdFormatUnknown')
nrrdFormatArray = (POINTER(NrrdFormat) * 7).in_dll(_libraries[libteem], 'nrrdFormatArray')
NrrdEncoding = NrrdEncoding_t
nrrdEncodingRaw = (POINTER(NrrdEncoding)).in_dll(_libraries[libteem], 'nrrdEncodingRaw')
nrrdEncodingAscii = (POINTER(NrrdEncoding)).in_dll(_libraries[libteem], 'nrrdEncodingAscii')
nrrdEncodingHex = (POINTER(NrrdEncoding)).in_dll(_libraries[libteem], 'nrrdEncodingHex')
nrrdEncodingGzip = (POINTER(NrrdEncoding)).in_dll(_libraries[libteem], 'nrrdEncodingGzip')
nrrdEncodingBzip2 = (POINTER(NrrdEncoding)).in_dll(_libraries[libteem], 'nrrdEncodingBzip2')
nrrdEncodingUnknown = (POINTER(NrrdEncoding)).in_dll(_libraries[libteem], 'nrrdEncodingUnknown')
nrrdEncodingArray = (POINTER(NrrdEncoding) * 6).in_dll(_libraries[libteem], 'nrrdEncodingArray')
nrrdFieldInfoParse = (CFUNCTYPE(c_int, POINTER(FILE), POINTER(Nrrd), POINTER(NrrdIoState), c_int) * 33).in_dll(_libraries[libteem], 'nrrdFieldInfoParse')
nrrdLineSkip = _libraries[libteem].nrrdLineSkip
nrrdLineSkip.restype = c_int
nrrdLineSkip.argtypes = [POINTER(FILE), POINTER(NrrdIoState)]
nrrdByteSkip = _libraries[libteem].nrrdByteSkip
nrrdByteSkip.restype = c_int
nrrdByteSkip.argtypes = [POINTER(FILE), POINTER(Nrrd), POINTER(NrrdIoState)]
nrrdLoad = _libraries[libteem].nrrdLoad
nrrdLoad.restype = c_int
nrrdLoad.argtypes = [POINTER(Nrrd), STRING, POINTER(NrrdIoState)]
nrrdRead = _libraries[libteem].nrrdRead
nrrdRead.restype = c_int
nrrdRead.argtypes = [POINTER(Nrrd), POINTER(FILE), POINTER(NrrdIoState)]
nrrdStringRead = _libraries[libteem].nrrdStringRead
nrrdStringRead.restype = c_int
nrrdStringRead.argtypes = [POINTER(Nrrd), STRING, POINTER(NrrdIoState)]
nrrdIoStateSet = _libraries[libteem].nrrdIoStateSet
nrrdIoStateSet.restype = c_int
nrrdIoStateSet.argtypes = [POINTER(NrrdIoState), c_int, c_int]
nrrdIoStateEncodingSet = _libraries[libteem].nrrdIoStateEncodingSet
nrrdIoStateEncodingSet.restype = c_int
nrrdIoStateEncodingSet.argtypes = [POINTER(NrrdIoState), POINTER(NrrdEncoding)]
nrrdIoStateFormatSet = _libraries[libteem].nrrdIoStateFormatSet
nrrdIoStateFormatSet.restype = c_int
nrrdIoStateFormatSet.argtypes = [POINTER(NrrdIoState), POINTER(NrrdFormat)]
nrrdIoStateGet = _libraries[libteem].nrrdIoStateGet
nrrdIoStateGet.restype = c_int
nrrdIoStateGet.argtypes = [POINTER(NrrdIoState), c_int]
nrrdIoStateEncodingGet = _libraries[libteem].nrrdIoStateEncodingGet
nrrdIoStateEncodingGet.restype = POINTER(NrrdEncoding)
nrrdIoStateEncodingGet.argtypes = [POINTER(NrrdIoState)]
nrrdIoStateFormatGet = _libraries[libteem].nrrdIoStateFormatGet
nrrdIoStateFormatGet.restype = POINTER(NrrdFormat)
nrrdIoStateFormatGet.argtypes = [POINTER(NrrdIoState)]
nrrdSave = _libraries[libteem].nrrdSave
nrrdSave.restype = c_int
nrrdSave.argtypes = [STRING, POINTER(Nrrd), POINTER(NrrdIoState)]
nrrdSaveMulti = _libraries[libteem].nrrdSaveMulti
nrrdSaveMulti.restype = c_int
nrrdSaveMulti.argtypes = [STRING, POINTER(POINTER(Nrrd)), c_uint, c_uint, POINTER(NrrdIoState)]
nrrdWrite = _libraries[libteem].nrrdWrite
nrrdWrite.restype = c_int
nrrdWrite.argtypes = [POINTER(FILE), POINTER(Nrrd), POINTER(NrrdIoState)]
nrrdStringWrite = _libraries[libteem].nrrdStringWrite
nrrdStringWrite.restype = c_int
nrrdStringWrite.argtypes = [POINTER(STRING), POINTER(Nrrd), POINTER(NrrdIoState)]
nrrdDLoad = (CFUNCTYPE(c_double, c_void_p) * 12).in_dll(_libraries[libteem], 'nrrdDLoad')
nrrdFLoad = (CFUNCTYPE(c_float, c_void_p) * 12).in_dll(_libraries[libteem], 'nrrdFLoad')
nrrdILoad = (CFUNCTYPE(c_int, c_void_p) * 12).in_dll(_libraries[libteem], 'nrrdILoad')
nrrdUILoad = (CFUNCTYPE(c_uint, c_void_p) * 12).in_dll(_libraries[libteem], 'nrrdUILoad')
nrrdDStore = (CFUNCTYPE(c_double, c_void_p, c_double) * 12).in_dll(_libraries[libteem], 'nrrdDStore')
nrrdFStore = (CFUNCTYPE(c_float, c_void_p, c_float) * 12).in_dll(_libraries[libteem], 'nrrdFStore')
nrrdIStore = (CFUNCTYPE(c_int, c_void_p, c_int) * 12).in_dll(_libraries[libteem], 'nrrdIStore')
nrrdUIStore = (CFUNCTYPE(c_uint, c_void_p, c_uint) * 12).in_dll(_libraries[libteem], 'nrrdUIStore')
nrrdDLookup = (CFUNCTYPE(c_double, c_void_p, size_t) * 12).in_dll(_libraries[libteem], 'nrrdDLookup')
nrrdFLookup = (CFUNCTYPE(c_float, c_void_p, size_t) * 12).in_dll(_libraries[libteem], 'nrrdFLookup')
nrrdILookup = (CFUNCTYPE(c_int, c_void_p, size_t) * 12).in_dll(_libraries[libteem], 'nrrdILookup')
nrrdUILookup = (CFUNCTYPE(c_uint, c_void_p, size_t) * 12).in_dll(_libraries[libteem], 'nrrdUILookup')
nrrdDInsert = (CFUNCTYPE(c_double, c_void_p, size_t, c_double) * 12).in_dll(_libraries[libteem], 'nrrdDInsert')
nrrdFInsert = (CFUNCTYPE(c_float, c_void_p, size_t, c_float) * 12).in_dll(_libraries[libteem], 'nrrdFInsert')
nrrdIInsert = (CFUNCTYPE(c_int, c_void_p, size_t, c_int) * 12).in_dll(_libraries[libteem], 'nrrdIInsert')
nrrdUIInsert = (CFUNCTYPE(c_uint, c_void_p, size_t, c_uint) * 12).in_dll(_libraries[libteem], 'nrrdUIInsert')
nrrdSprint = (CFUNCTYPE(c_int, STRING, c_void_p) * 12).in_dll(_libraries[libteem], 'nrrdSprint')
nrrdFprint = (CFUNCTYPE(c_int, POINTER(FILE), c_void_p) * 12).in_dll(_libraries[libteem], 'nrrdFprint')
nrrdMinMaxExactFind = (CFUNCTYPE(None, c_void_p, c_void_p, POINTER(c_int), POINTER(Nrrd)) * 12).in_dll(_libraries[libteem], 'nrrdMinMaxExactFind')
nrrdValCompare = (CFUNCTYPE(c_int, c_void_p, c_void_p) * 12).in_dll(_libraries[libteem], 'nrrdValCompare')
nrrdValCompareInv = (CFUNCTYPE(c_int, c_void_p, c_void_p) * 12).in_dll(_libraries[libteem], 'nrrdValCompareInv')
nrrdAxesInsert = _libraries[libteem].nrrdAxesInsert
nrrdAxesInsert.restype = c_int
nrrdAxesInsert.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdInvertPerm = _libraries[libteem].nrrdInvertPerm
nrrdInvertPerm.restype = c_int
nrrdInvertPerm.argtypes = [POINTER(c_uint), POINTER(c_uint), c_uint]
nrrdAxesPermute = _libraries[libteem].nrrdAxesPermute
nrrdAxesPermute.restype = c_int
nrrdAxesPermute.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(c_uint)]
nrrdShuffle = _libraries[libteem].nrrdShuffle
nrrdShuffle.restype = c_int
nrrdShuffle.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, POINTER(size_t)]
nrrdAxesSwap = _libraries[libteem].nrrdAxesSwap
nrrdAxesSwap.restype = c_int
nrrdAxesSwap.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, c_uint]
nrrdFlip = _libraries[libteem].nrrdFlip
nrrdFlip.restype = c_int
nrrdFlip.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdJoin = _libraries[libteem].nrrdJoin
nrrdJoin.restype = c_int
nrrdJoin.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), c_uint, c_uint, c_int]
nrrdReshape_va = _libraries[libteem].nrrdReshape_va
nrrdReshape_va.restype = c_int
nrrdReshape_va.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdReshape_nva = _libraries[libteem].nrrdReshape_nva
nrrdReshape_nva.restype = c_int
nrrdReshape_nva.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, POINTER(size_t)]
nrrdAxesSplit = _libraries[libteem].nrrdAxesSplit
nrrdAxesSplit.restype = c_int
nrrdAxesSplit.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, size_t, size_t]
nrrdAxesDelete = _libraries[libteem].nrrdAxesDelete
nrrdAxesDelete.restype = c_int
nrrdAxesDelete.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdAxesMerge = _libraries[libteem].nrrdAxesMerge
nrrdAxesMerge.restype = c_int
nrrdAxesMerge.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdBlock = _libraries[libteem].nrrdBlock
nrrdBlock.restype = c_int
nrrdBlock.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
nrrdUnblock = _libraries[libteem].nrrdUnblock
nrrdUnblock.restype = c_int
nrrdUnblock.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdTile2D = _libraries[libteem].nrrdTile2D
nrrdTile2D.restype = c_int
nrrdTile2D.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, c_uint, c_uint, size_t, size_t]
nrrdUntile2D = _libraries[libteem].nrrdUntile2D
nrrdUntile2D.restype = c_int
nrrdUntile2D.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, c_uint, c_uint, size_t, size_t]
nrrdHestNrrd = (POINTER(hestCB)).in_dll(_libraries[libteem], 'nrrdHestNrrd')
nrrdHestKernelSpec = (POINTER(hestCB)).in_dll(_libraries[libteem], 'nrrdHestKernelSpec')
nrrdHestIter = (POINTER(hestCB)).in_dll(_libraries[libteem], 'nrrdHestIter')
class NrrdIter(Structure):
    pass
nrrdIterNew = _libraries[libteem].nrrdIterNew
nrrdIterNew.restype = POINTER(NrrdIter)
nrrdIterNew.argtypes = []
nrrdIterSetValue = _libraries[libteem].nrrdIterSetValue
nrrdIterSetValue.restype = None
nrrdIterSetValue.argtypes = [POINTER(NrrdIter), c_double]
nrrdIterSetNrrd = _libraries[libteem].nrrdIterSetNrrd
nrrdIterSetNrrd.restype = None
nrrdIterSetNrrd.argtypes = [POINTER(NrrdIter), POINTER(Nrrd)]
nrrdIterSetOwnNrrd = _libraries[libteem].nrrdIterSetOwnNrrd
nrrdIterSetOwnNrrd.restype = None
nrrdIterSetOwnNrrd.argtypes = [POINTER(NrrdIter), POINTER(Nrrd)]
nrrdIterValue = _libraries[libteem].nrrdIterValue
nrrdIterValue.restype = c_double
nrrdIterValue.argtypes = [POINTER(NrrdIter)]
nrrdIterContent = _libraries[libteem].nrrdIterContent
nrrdIterContent.restype = STRING
nrrdIterContent.argtypes = [POINTER(NrrdIter)]
nrrdIterNix = _libraries[libteem].nrrdIterNix
nrrdIterNix.restype = POINTER(NrrdIter)
nrrdIterNix.argtypes = [POINTER(NrrdIter)]
class NrrdRange(Structure):
    pass
nrrdRangeNew = _libraries[libteem].nrrdRangeNew
nrrdRangeNew.restype = POINTER(NrrdRange)
nrrdRangeNew.argtypes = [c_double, c_double]
NrrdRange._fields_ = [
    ('min', c_double),
    ('max', c_double),
    ('hasNonExist', c_int),
]
nrrdRangeCopy = _libraries[libteem].nrrdRangeCopy
nrrdRangeCopy.restype = POINTER(NrrdRange)
nrrdRangeCopy.argtypes = [POINTER(NrrdRange)]
nrrdRangeNix = _libraries[libteem].nrrdRangeNix
nrrdRangeNix.restype = POINTER(NrrdRange)
nrrdRangeNix.argtypes = [POINTER(NrrdRange)]
nrrdRangeReset = _libraries[libteem].nrrdRangeReset
nrrdRangeReset.restype = None
nrrdRangeReset.argtypes = [POINTER(NrrdRange)]
nrrdRangeSet = _libraries[libteem].nrrdRangeSet
nrrdRangeSet.restype = None
nrrdRangeSet.argtypes = [POINTER(NrrdRange), POINTER(Nrrd), c_int]
nrrdRangeSafeSet = _libraries[libteem].nrrdRangeSafeSet
nrrdRangeSafeSet.restype = None
nrrdRangeSafeSet.argtypes = [POINTER(NrrdRange), POINTER(Nrrd), c_int]
nrrdRangeNewSet = _libraries[libteem].nrrdRangeNewSet
nrrdRangeNewSet.restype = POINTER(NrrdRange)
nrrdRangeNewSet.argtypes = [POINTER(Nrrd), c_int]
nrrdHasNonExist = _libraries[libteem].nrrdHasNonExist
nrrdHasNonExist.restype = c_int
nrrdHasNonExist.argtypes = [POINTER(Nrrd)]
nrrdFClamp = (CFUNCTYPE(c_float, c_float) * 12).in_dll(_libraries[libteem], 'nrrdFClamp')
nrrdDClamp = (CFUNCTYPE(c_double, c_double) * 12).in_dll(_libraries[libteem], 'nrrdDClamp')
nrrdConvert = _libraries[libteem].nrrdConvert
nrrdConvert.restype = c_int
nrrdConvert.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdClampConvert = _libraries[libteem].nrrdClampConvert
nrrdClampConvert.restype = c_int
nrrdClampConvert.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdQuantize = _libraries[libteem].nrrdQuantize
nrrdQuantize.restype = c_int
nrrdQuantize.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), c_uint]
nrrdUnquantize = _libraries[libteem].nrrdUnquantize
nrrdUnquantize.restype = c_int
nrrdUnquantize.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
nrrdHistoEq = _libraries[libteem].nrrdHistoEq
nrrdHistoEq.restype = c_int
nrrdHistoEq.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(POINTER(Nrrd)), c_uint, c_uint, c_float]
nrrdApply1DLut = _libraries[libteem].nrrdApply1DLut
nrrdApply1DLut.restype = c_int
nrrdApply1DLut.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), POINTER(Nrrd), c_int, c_int]
nrrdApplyMulti1DLut = _libraries[libteem].nrrdApplyMulti1DLut
nrrdApplyMulti1DLut.restype = c_int
nrrdApplyMulti1DLut.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), POINTER(Nrrd), c_int, c_int]
nrrdApply1DRegMap = _libraries[libteem].nrrdApply1DRegMap
nrrdApply1DRegMap.restype = c_int
nrrdApply1DRegMap.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), POINTER(Nrrd), c_int, c_int]
nrrdApplyMulti1DRegMap = _libraries[libteem].nrrdApplyMulti1DRegMap
nrrdApplyMulti1DRegMap.restype = c_int
nrrdApplyMulti1DRegMap.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), POINTER(Nrrd), c_int, c_int]
nrrd1DIrregMapCheck = _libraries[libteem].nrrd1DIrregMapCheck
nrrd1DIrregMapCheck.restype = c_int
nrrd1DIrregMapCheck.argtypes = [POINTER(Nrrd)]
nrrd1DIrregAclGenerate = _libraries[libteem].nrrd1DIrregAclGenerate
nrrd1DIrregAclGenerate.restype = c_int
nrrd1DIrregAclGenerate.argtypes = [POINTER(Nrrd), POINTER(Nrrd), size_t]
nrrd1DIrregAclCheck = _libraries[libteem].nrrd1DIrregAclCheck
nrrd1DIrregAclCheck.restype = c_int
nrrd1DIrregAclCheck.argtypes = [POINTER(Nrrd)]
nrrdApply1DIrregMap = _libraries[libteem].nrrdApply1DIrregMap
nrrdApply1DIrregMap.restype = c_int
nrrdApply1DIrregMap.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), POINTER(Nrrd), POINTER(Nrrd), c_int, c_int]
nrrdApply1DSubstitution = _libraries[libteem].nrrdApply1DSubstitution
nrrdApply1DSubstitution.restype = c_int
nrrdApply1DSubstitution.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
nrrdApply2DLut = _libraries[libteem].nrrdApply2DLut
nrrdApply2DLut.restype = c_int
nrrdApply2DLut.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, POINTER(NrrdRange), POINTER(NrrdRange), POINTER(Nrrd), c_int, c_int, c_int]
nrrdSlice = _libraries[libteem].nrrdSlice
nrrdSlice.restype = c_int
nrrdSlice.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, size_t]
nrrdCrop = _libraries[libteem].nrrdCrop
nrrdCrop.restype = c_int
nrrdCrop.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(size_t), POINTER(size_t)]
nrrdSample_nva = _libraries[libteem].nrrdSample_nva
nrrdSample_nva.restype = c_int
nrrdSample_nva.argtypes = [c_void_p, POINTER(Nrrd), POINTER(size_t)]
nrrdSample_va = _libraries[libteem].nrrdSample_va
nrrdSample_va.restype = c_int
nrrdSample_va.argtypes = [c_void_p, POINTER(Nrrd)]
nrrdSimpleCrop = _libraries[libteem].nrrdSimpleCrop
nrrdSimpleCrop.restype = c_int
nrrdSimpleCrop.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdSplice = _libraries[libteem].nrrdSplice
nrrdSplice.restype = c_int
nrrdSplice.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), c_uint, size_t]
nrrdPad_nva = _libraries[libteem].nrrdPad_nva
nrrdPad_nva.restype = c_int
nrrdPad_nva.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(ptrdiff_t), POINTER(ptrdiff_t), c_int, c_double]
nrrdPad_va = _libraries[libteem].nrrdPad_va
nrrdPad_va.restype = c_int
nrrdPad_va.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(ptrdiff_t), POINTER(ptrdiff_t), c_int]
nrrdSimplePad_nva = _libraries[libteem].nrrdSimplePad_nva
nrrdSimplePad_nva.restype = c_int
nrrdSimplePad_nva.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, c_int, c_double]
nrrdSimplePad_va = _libraries[libteem].nrrdSimplePad_va
nrrdSimplePad_va.restype = c_int
nrrdSimplePad_va.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, c_int]
nrrdInset = _libraries[libteem].nrrdInset
nrrdInset.restype = c_int
nrrdInset.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), POINTER(size_t)]
nrrdMeasureLine = (CFUNCTYPE(None, c_void_p, c_int, c_void_p, c_int, size_t, c_double, c_double) * 27).in_dll(_libraries[libteem], 'nrrdMeasureLine')
nrrdProject = _libraries[libteem].nrrdProject
nrrdProject.restype = c_int
nrrdProject.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, c_int, c_int]
nrrdHisto = _libraries[libteem].nrrdHisto
nrrdHisto.restype = c_int
nrrdHisto.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), POINTER(Nrrd), size_t, c_int]
nrrdHistoCheck = _libraries[libteem].nrrdHistoCheck
nrrdHistoCheck.restype = c_int
nrrdHistoCheck.argtypes = [POINTER(Nrrd)]
nrrdHistoDraw = _libraries[libteem].nrrdHistoDraw
nrrdHistoDraw.restype = c_int
nrrdHistoDraw.argtypes = [POINTER(Nrrd), POINTER(Nrrd), size_t, c_int, c_double]
nrrdHistoAxis = _libraries[libteem].nrrdHistoAxis
nrrdHistoAxis.restype = c_int
nrrdHistoAxis.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), c_uint, size_t, c_int]
nrrdHistoJoint = _libraries[libteem].nrrdHistoJoint
nrrdHistoJoint.restype = c_int
nrrdHistoJoint.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(POINTER(NrrdRange)), c_uint, POINTER(Nrrd), POINTER(size_t), c_int, POINTER(c_int)]
nrrdHistoThresholdOtsu = _libraries[libteem].nrrdHistoThresholdOtsu
nrrdHistoThresholdOtsu.restype = c_int
nrrdHistoThresholdOtsu.argtypes = [POINTER(c_double), POINTER(Nrrd), c_double]
nrrdArithGamma = _libraries[libteem].nrrdArithGamma
nrrdArithGamma.restype = c_int
nrrdArithGamma.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdRange), c_double]
nrrdArithUnaryOp = _libraries[libteem].nrrdArithUnaryOp
nrrdArithUnaryOp.restype = c_int
nrrdArithUnaryOp.argtypes = [POINTER(Nrrd), c_int, POINTER(Nrrd)]
nrrdArithBinaryOp = _libraries[libteem].nrrdArithBinaryOp
nrrdArithBinaryOp.restype = c_int
nrrdArithBinaryOp.argtypes = [POINTER(Nrrd), c_int, POINTER(Nrrd), POINTER(Nrrd)]
nrrdArithTernaryOp = _libraries[libteem].nrrdArithTernaryOp
nrrdArithTernaryOp.restype = c_int
nrrdArithTernaryOp.argtypes = [POINTER(Nrrd), c_int, POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
nrrdArithIterBinaryOp = _libraries[libteem].nrrdArithIterBinaryOp
nrrdArithIterBinaryOp.restype = c_int
nrrdArithIterBinaryOp.argtypes = [POINTER(Nrrd), c_int, POINTER(NrrdIter), POINTER(NrrdIter)]
nrrdArithIterTernaryOp = _libraries[libteem].nrrdArithIterTernaryOp
nrrdArithIterTernaryOp.restype = c_int
nrrdArithIterTernaryOp.argtypes = [POINTER(Nrrd), c_int, POINTER(NrrdIter), POINTER(NrrdIter), POINTER(NrrdIter)]
nrrdCheapMedian = _libraries[libteem].nrrdCheapMedian
nrrdCheapMedian.restype = c_int
nrrdCheapMedian.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, c_int, c_uint, c_float, c_uint]
nrrdDistanceL2 = _libraries[libteem].nrrdDistanceL2
nrrdDistanceL2.restype = c_int
nrrdDistanceL2.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, POINTER(c_int), c_double, c_int]
nrrdDistanceL2Signed = _libraries[libteem].nrrdDistanceL2Signed
nrrdDistanceL2Signed.restype = c_int
nrrdDistanceL2Signed.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, POINTER(c_int), c_double, c_int]
nrrdResample_t = c_double
class NrrdResampleContext(Structure):
    pass
nrrdResampleContextNew = _libraries[libteem].nrrdResampleContextNew
nrrdResampleContextNew.restype = POINTER(NrrdResampleContext)
nrrdResampleContextNew.argtypes = []
nrrdResampleContextNix = _libraries[libteem].nrrdResampleContextNix
nrrdResampleContextNix.restype = POINTER(NrrdResampleContext)
nrrdResampleContextNix.argtypes = [POINTER(NrrdResampleContext)]
nrrdResampleDefaultCenterSet = _libraries[libteem].nrrdResampleDefaultCenterSet
nrrdResampleDefaultCenterSet.restype = c_int
nrrdResampleDefaultCenterSet.argtypes = [POINTER(NrrdResampleContext), c_int]
nrrdResampleNrrdSet = _libraries[libteem].nrrdResampleNrrdSet
nrrdResampleNrrdSet.restype = c_int
nrrdResampleNrrdSet.argtypes = [POINTER(NrrdResampleContext), POINTER(Nrrd)]
nrrdResampleKernelSet = _libraries[libteem].nrrdResampleKernelSet
nrrdResampleKernelSet.restype = c_int
nrrdResampleKernelSet.argtypes = [POINTER(NrrdResampleContext), c_uint, POINTER(NrrdKernel), POINTER(c_double)]
nrrdResampleSamplesSet = _libraries[libteem].nrrdResampleSamplesSet
nrrdResampleSamplesSet.restype = c_int
nrrdResampleSamplesSet.argtypes = [POINTER(NrrdResampleContext), c_uint, size_t]
nrrdResampleRangeSet = _libraries[libteem].nrrdResampleRangeSet
nrrdResampleRangeSet.restype = c_int
nrrdResampleRangeSet.argtypes = [POINTER(NrrdResampleContext), c_uint, c_double, c_double]
nrrdResampleRangeFullSet = _libraries[libteem].nrrdResampleRangeFullSet
nrrdResampleRangeFullSet.restype = c_int
nrrdResampleRangeFullSet.argtypes = [POINTER(NrrdResampleContext), c_uint]
nrrdResampleBoundarySet = _libraries[libteem].nrrdResampleBoundarySet
nrrdResampleBoundarySet.restype = c_int
nrrdResampleBoundarySet.argtypes = [POINTER(NrrdResampleContext), c_int]
nrrdResamplePadValueSet = _libraries[libteem].nrrdResamplePadValueSet
nrrdResamplePadValueSet.restype = c_int
nrrdResamplePadValueSet.argtypes = [POINTER(NrrdResampleContext), c_double]
nrrdResampleTypeOutSet = _libraries[libteem].nrrdResampleTypeOutSet
nrrdResampleTypeOutSet.restype = c_int
nrrdResampleTypeOutSet.argtypes = [POINTER(NrrdResampleContext), c_int]
nrrdResampleRenormalizeSet = _libraries[libteem].nrrdResampleRenormalizeSet
nrrdResampleRenormalizeSet.restype = c_int
nrrdResampleRenormalizeSet.argtypes = [POINTER(NrrdResampleContext), c_int]
nrrdResampleRoundSet = _libraries[libteem].nrrdResampleRoundSet
nrrdResampleRoundSet.restype = c_int
nrrdResampleRoundSet.argtypes = [POINTER(NrrdResampleContext), c_int]
nrrdResampleClampSet = _libraries[libteem].nrrdResampleClampSet
nrrdResampleClampSet.restype = c_int
nrrdResampleClampSet.argtypes = [POINTER(NrrdResampleContext), c_int]
nrrdResampleExecute = _libraries[libteem].nrrdResampleExecute
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
nrrdSpatialResample = _libraries[libteem].nrrdSpatialResample
nrrdSpatialResample.restype = c_int
nrrdSpatialResample.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdResampleInfo)]
nrrdSimpleResample = _libraries[libteem].nrrdSimpleResample
nrrdSimpleResample.restype = c_int
nrrdSimpleResample.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(NrrdKernel), POINTER(c_double), POINTER(size_t), POINTER(c_double)]
nrrdCCValid = _libraries[libteem].nrrdCCValid
nrrdCCValid.restype = c_int
nrrdCCValid.argtypes = [POINTER(Nrrd)]
nrrdCCSize = _libraries[libteem].nrrdCCSize
nrrdCCSize.restype = c_uint
nrrdCCSize.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
nrrdCCMax = _libraries[libteem].nrrdCCMax
nrrdCCMax.restype = c_uint
nrrdCCMax.argtypes = [POINTER(Nrrd)]
nrrdCCNum = _libraries[libteem].nrrdCCNum
nrrdCCNum.restype = c_uint
nrrdCCNum.argtypes = [POINTER(Nrrd)]
nrrdCCFind = _libraries[libteem].nrrdCCFind
nrrdCCFind.restype = c_int
nrrdCCFind.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(Nrrd), c_int, c_uint]
nrrdCCAdjacency = _libraries[libteem].nrrdCCAdjacency
nrrdCCAdjacency.restype = c_int
nrrdCCAdjacency.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint]
nrrdCCMerge = _libraries[libteem].nrrdCCMerge
nrrdCCMerge.restype = c_int
nrrdCCMerge.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), c_int, c_uint, c_uint, c_uint]
nrrdCCRevalue = _libraries[libteem].nrrdCCRevalue
nrrdCCRevalue.restype = c_int
nrrdCCRevalue.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
nrrdCCSettle = _libraries[libteem].nrrdCCSettle
nrrdCCSettle.restype = c_int
nrrdCCSettle.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(Nrrd)]
nrrdKernelTMF = (POINTER(NrrdKernel) * 5 * 5 * 4).in_dll(_libraries[libteem], 'nrrdKernelTMF')
nrrdKernelTMF_maxD = (c_uint).in_dll(_libraries[libteem], 'nrrdKernelTMF_maxD')
nrrdKernelTMF_maxC = (c_uint).in_dll(_libraries[libteem], 'nrrdKernelTMF_maxC')
nrrdKernelTMF_maxA = (c_uint).in_dll(_libraries[libteem], 'nrrdKernelTMF_maxA')
nrrdKernelHann = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelHann')
nrrdKernelHannD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelHannD')
nrrdKernelHannDD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelHannDD')
nrrdKernelBlackman = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelBlackman')
nrrdKernelBlackmanD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelBlackmanD')
nrrdKernelBlackmanDD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelBlackmanDD')
nrrdKernelZero = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelZero')
nrrdKernelBox = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelBox')
nrrdKernelCheap = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelCheap')
nrrdKernelHermiteFlag = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelHermiteFlag')
nrrdKernelTent = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelTent')
nrrdKernelForwDiff = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelForwDiff')
nrrdKernelCentDiff = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelCentDiff')
nrrdKernelBCCubic = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelBCCubic')
nrrdKernelBCCubicD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelBCCubicD')
nrrdKernelBCCubicDD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelBCCubicDD')
nrrdKernelAQuartic = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelAQuartic')
nrrdKernelAQuarticD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelAQuarticD')
nrrdKernelAQuarticDD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelAQuarticDD')
nrrdKernelC3Quintic = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelC3Quintic')
nrrdKernelC3QuinticD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelC3QuinticD')
nrrdKernelC3QuinticDD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelC3QuinticDD')
nrrdKernelC4Hexic = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelC4Hexic')
nrrdKernelC4HexicD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelC4HexicD')
nrrdKernelC4HexicDD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelC4HexicDD')
nrrdKernelGaussian = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelGaussian')
nrrdKernelGaussianD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelGaussianD')
nrrdKernelGaussianDD = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelGaussianDD')
nrrdKernelDiscreteGaussian = (POINTER(NrrdKernel)).in_dll(_libraries[libteem], 'nrrdKernelDiscreteGaussian')
nrrdKernelParse = _libraries[libteem].nrrdKernelParse
nrrdKernelParse.restype = c_int
nrrdKernelParse.argtypes = [POINTER(POINTER(NrrdKernel)), POINTER(c_double), STRING]
nrrdKernelSpecParse = _libraries[libteem].nrrdKernelSpecParse
nrrdKernelSpecParse.restype = c_int
nrrdKernelSpecParse.argtypes = [POINTER(NrrdKernelSpec), STRING]
nrrdKernelSpecSprint = _libraries[libteem].nrrdKernelSpecSprint
nrrdKernelSpecSprint.restype = c_int
nrrdKernelSpecSprint.argtypes = [STRING, POINTER(NrrdKernelSpec)]
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
    ('lowerInside', c_int),
    ('normalsFind', c_int),
    ('strengthUse', c_int),
    ('strengthSign', c_int),
    ('isovalue', c_double),
    ('strength', c_double),
    ('strengthMin', c_double),
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
    ('sclv', POINTER(c_double)),
    ('grad', POINTER(c_double)),
    ('eval', POINTER(c_double)),
    ('evec', POINTER(c_double)),
    ('flip', POINTER(c_byte)),
    ('stng', POINTER(c_double)),
    ('nvidx', POINTER(Nrrd)),
    ('nsclv', POINTER(Nrrd)),
    ('ngrad', POINTER(Nrrd)),
    ('neval', POINTER(Nrrd)),
    ('nevec', POINTER(Nrrd)),
    ('nflip', POINTER(Nrrd)),
    ('nstng', POINTER(Nrrd)),
    ('voxNum', c_uint),
    ('vertNum', c_uint),
    ('faceNum', c_uint),
    ('strengthSeenMax', c_double),
    ('time', c_double),
]
seekBiffKey = (STRING).in_dll(_libraries[libteem], 'seekBiffKey')
seekType = (POINTER(airEnum)).in_dll(_libraries[libteem], 'seekType')
seekContour3DTopoHackEdge = (c_int * 256).in_dll(_libraries[libteem], 'seekContour3DTopoHackEdge')
seekContour3DTopoHackTriangle = (c_int * 19 * 256).in_dll(_libraries[libteem], 'seekContour3DTopoHackTriangle')
seekContextNew = _libraries[libteem].seekContextNew
seekContextNew.restype = POINTER(seekContext)
seekContextNew.argtypes = []
seekContextNix = _libraries[libteem].seekContextNix
seekContextNix.restype = POINTER(seekContext)
seekContextNix.argtypes = [POINTER(seekContext)]
seekVerboseSet = _libraries[libteem].seekVerboseSet
seekVerboseSet.restype = None
seekVerboseSet.argtypes = [POINTER(seekContext), c_int]
seekDataSet = _libraries[libteem].seekDataSet
seekDataSet.restype = c_int
seekDataSet.argtypes = [POINTER(seekContext), POINTER(Nrrd), POINTER(gageContext), c_uint]
seekNormalsFindSet = _libraries[libteem].seekNormalsFindSet
seekNormalsFindSet.restype = c_int
seekNormalsFindSet.argtypes = [POINTER(seekContext), c_int]
seekStrengthUseSet = _libraries[libteem].seekStrengthUseSet
seekStrengthUseSet.restype = c_int
seekStrengthUseSet.argtypes = [POINTER(seekContext), c_int]
seekStrengthSet = _libraries[libteem].seekStrengthSet
seekStrengthSet.restype = c_int
seekStrengthSet.argtypes = [POINTER(seekContext), c_int, c_double, c_double]
seekSamplesSet = _libraries[libteem].seekSamplesSet
seekSamplesSet.restype = c_int
seekSamplesSet.argtypes = [POINTER(seekContext), POINTER(size_t)]
seekTypeSet = _libraries[libteem].seekTypeSet
seekTypeSet.restype = c_int
seekTypeSet.argtypes = [POINTER(seekContext), c_int]
seekLowerInsideSet = _libraries[libteem].seekLowerInsideSet
seekLowerInsideSet.restype = c_int
seekLowerInsideSet.argtypes = [POINTER(seekContext), c_int]
seekItemScalarSet = _libraries[libteem].seekItemScalarSet
seekItemScalarSet.restype = c_int
seekItemScalarSet.argtypes = [POINTER(seekContext), c_int]
seekItemStrengthSet = _libraries[libteem].seekItemStrengthSet
seekItemStrengthSet.restype = c_int
seekItemStrengthSet.argtypes = [POINTER(seekContext), c_int]
seekItemNormalSet = _libraries[libteem].seekItemNormalSet
seekItemNormalSet.restype = c_int
seekItemNormalSet.argtypes = [POINTER(seekContext), c_int]
seekItemGradientSet = _libraries[libteem].seekItemGradientSet
seekItemGradientSet.restype = c_int
seekItemGradientSet.argtypes = [POINTER(seekContext), c_int]
seekItemEigensystemSet = _libraries[libteem].seekItemEigensystemSet
seekItemEigensystemSet.restype = c_int
seekItemEigensystemSet.argtypes = [POINTER(seekContext), c_int, c_int]
seekIsovalueSet = _libraries[libteem].seekIsovalueSet
seekIsovalueSet.restype = c_int
seekIsovalueSet.argtypes = [POINTER(seekContext), c_double]
seekUpdate = _libraries[libteem].seekUpdate
seekUpdate.restype = c_int
seekUpdate.argtypes = [POINTER(seekContext)]
seekExtract = _libraries[libteem].seekExtract
seekExtract.restype = c_int
seekExtract.argtypes = [POINTER(seekContext), POINTER(limnPolyData)]
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
tenBiffKey = (STRING).in_dll(_libraries[libteem], 'tenBiffKey')
tenDefFiberKernel = (c_char * 0).in_dll(_libraries[libteem], 'tenDefFiberKernel')
tenDefFiberStepSize = (c_double).in_dll(_libraries[libteem], 'tenDefFiberStepSize')
tenDefFiberUseIndexSpace = (c_int).in_dll(_libraries[libteem], 'tenDefFiberUseIndexSpace')
tenDefFiberMaxNumSteps = (c_int).in_dll(_libraries[libteem], 'tenDefFiberMaxNumSteps')
tenDefFiberMaxHalfLen = (c_double).in_dll(_libraries[libteem], 'tenDefFiberMaxHalfLen')
tenDefFiberAnisoStopType = (c_int).in_dll(_libraries[libteem], 'tenDefFiberAnisoStopType')
tenDefFiberAnisoThresh = (c_double).in_dll(_libraries[libteem], 'tenDefFiberAnisoThresh')
tenDefFiberIntg = (c_int).in_dll(_libraries[libteem], 'tenDefFiberIntg')
tenDefFiberWPunct = (c_double).in_dll(_libraries[libteem], 'tenDefFiberWPunct')
tenTripleConvertSingle_d = _libraries[libteem].tenTripleConvertSingle_d
tenTripleConvertSingle_d.restype = None
tenTripleConvertSingle_d.argtypes = [POINTER(c_double), c_int, POINTER(c_double), c_int]
tenTripleConvertSingle_f = _libraries[libteem].tenTripleConvertSingle_f
tenTripleConvertSingle_f.restype = None
tenTripleConvertSingle_f.argtypes = [POINTER(c_float), c_int, POINTER(c_float), c_int]
tenTripleCalcSingle_d = _libraries[libteem].tenTripleCalcSingle_d
tenTripleCalcSingle_d.restype = None
tenTripleCalcSingle_d.argtypes = [POINTER(c_double), c_int, POINTER(c_double)]
tenTripleCalcSingle_f = _libraries[libteem].tenTripleCalcSingle_f
tenTripleCalcSingle_f.restype = None
tenTripleCalcSingle_f.argtypes = [POINTER(c_float), c_int, POINTER(c_float)]
tenTripleCalc = _libraries[libteem].tenTripleCalc
tenTripleCalc.restype = c_int
tenTripleCalc.argtypes = [POINTER(Nrrd), c_int, POINTER(Nrrd)]
tenTripleConvert = _libraries[libteem].tenTripleConvert
tenTripleConvert.restype = c_int
tenTripleConvert.argtypes = [POINTER(Nrrd), c_int, POINTER(Nrrd), c_int]
tenGradientParmNew = _libraries[libteem].tenGradientParmNew
tenGradientParmNew.restype = POINTER(tenGradientParm)
tenGradientParmNew.argtypes = []
tenGradientParmNix = _libraries[libteem].tenGradientParmNix
tenGradientParmNix.restype = POINTER(tenGradientParm)
tenGradientParmNix.argtypes = [POINTER(tenGradientParm)]
tenGradientCheck = _libraries[libteem].tenGradientCheck
tenGradientCheck.restype = c_int
tenGradientCheck.argtypes = [POINTER(Nrrd), c_int, c_uint]
tenGradientRandom = _libraries[libteem].tenGradientRandom
tenGradientRandom.restype = c_int
tenGradientRandom.argtypes = [POINTER(Nrrd), c_uint, c_uint]
tenGradientIdealEdge = _libraries[libteem].tenGradientIdealEdge
tenGradientIdealEdge.restype = c_double
tenGradientIdealEdge.argtypes = [c_uint, c_int]
tenGradientJitter = _libraries[libteem].tenGradientJitter
tenGradientJitter.restype = c_int
tenGradientJitter.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double]
tenGradientBalance = _libraries[libteem].tenGradientBalance
tenGradientBalance.restype = c_int
tenGradientBalance.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(tenGradientParm)]
tenGradientMeasure = _libraries[libteem].tenGradientMeasure
tenGradientMeasure.restype = None
tenGradientMeasure.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(Nrrd), POINTER(tenGradientParm), c_int]
tenGradientDistribute = _libraries[libteem].tenGradientDistribute
tenGradientDistribute.restype = c_int
tenGradientDistribute.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(tenGradientParm)]
tenGradientGenerate = _libraries[libteem].tenGradientGenerate
tenGradientGenerate.restype = c_int
tenGradientGenerate.argtypes = [POINTER(Nrrd), c_uint, POINTER(tenGradientParm)]
tenAniso = (POINTER(airEnum)).in_dll(_libraries[libteem], 'tenAniso')
tenInterpType = (POINTER(airEnum)).in_dll(_libraries[libteem], 'tenInterpType')
tenGage = (POINTER(airEnum)).in_dll(_libraries[libteem], 'tenGage')
tenFiberType = (POINTER(airEnum)).in_dll(_libraries[libteem], 'tenFiberType')
tenDwiFiberType = (POINTER(airEnum)).in_dll(_libraries[libteem], 'tenDwiFiberType')
tenFiberStop = (POINTER(airEnum)).in_dll(_libraries[libteem], 'tenFiberStop')
tenFiberIntg = (POINTER(airEnum)).in_dll(_libraries[libteem], 'tenFiberIntg')
tenGlyphType = (POINTER(airEnum)).in_dll(_libraries[libteem], 'tenGlyphType')
tenEstimate1Method = (POINTER(airEnum)).in_dll(_libraries[libteem], 'tenEstimate1Method')
tenEstimate2Method = (POINTER(airEnum)).in_dll(_libraries[libteem], 'tenEstimate2Method')
tenTripleType = (POINTER(airEnum)).in_dll(_libraries[libteem], 'tenTripleType')
tenInterpParmNew = _libraries[libteem].tenInterpParmNew
tenInterpParmNew.restype = POINTER(tenInterpParm)
tenInterpParmNew.argtypes = []
tenInterpParmCopy = _libraries[libteem].tenInterpParmCopy
tenInterpParmCopy.restype = POINTER(tenInterpParm)
tenInterpParmCopy.argtypes = [POINTER(tenInterpParm)]
tenInterpParmBufferAlloc = _libraries[libteem].tenInterpParmBufferAlloc
tenInterpParmBufferAlloc.restype = c_int
tenInterpParmBufferAlloc.argtypes = [POINTER(tenInterpParm), c_uint]
tenInterpParmNix = _libraries[libteem].tenInterpParmNix
tenInterpParmNix.restype = POINTER(tenInterpParm)
tenInterpParmNix.argtypes = [POINTER(tenInterpParm)]
tenInterpTwo_d = _libraries[libteem].tenInterpTwo_d
tenInterpTwo_d.restype = None
tenInterpTwo_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), c_int, c_double, POINTER(tenInterpParm)]
tenInterpN_d = _libraries[libteem].tenInterpN_d
tenInterpN_d.restype = c_int
tenInterpN_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), c_uint, c_int, POINTER(tenInterpParm)]
tenInterpPathLength = _libraries[libteem].tenInterpPathLength
tenInterpPathLength.restype = c_double
tenInterpPathLength.argtypes = [POINTER(Nrrd), c_int, c_int, c_int]
tenInterpTwoDiscrete_d = _libraries[libteem].tenInterpTwoDiscrete_d
tenInterpTwoDiscrete_d.restype = c_int
tenInterpTwoDiscrete_d.argtypes = [POINTER(Nrrd), POINTER(c_double), POINTER(c_double), c_int, c_uint, POINTER(tenInterpParm)]
tenInterpDistanceTwo_d = _libraries[libteem].tenInterpDistanceTwo_d
tenInterpDistanceTwo_d.restype = c_double
tenInterpDistanceTwo_d.argtypes = [POINTER(c_double), POINTER(c_double), c_int, POINTER(tenInterpParm)]
tenInterpMulti3D = _libraries[libteem].tenInterpMulti3D
tenInterpMulti3D.restype = c_int
tenInterpMulti3D.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(c_double), c_uint, c_int, POINTER(tenInterpParm)]
tenGlyphParmNew = _libraries[libteem].tenGlyphParmNew
tenGlyphParmNew.restype = POINTER(tenGlyphParm)
tenGlyphParmNew.argtypes = []
tenGlyphParmNix = _libraries[libteem].tenGlyphParmNix
tenGlyphParmNix.restype = POINTER(tenGlyphParm)
tenGlyphParmNix.argtypes = [POINTER(tenGlyphParm)]
tenGlyphParmCheck = _libraries[libteem].tenGlyphParmCheck
tenGlyphParmCheck.restype = c_int
tenGlyphParmCheck.argtypes = [POINTER(tenGlyphParm), POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
tenGlyphGen = _libraries[libteem].tenGlyphGen
tenGlyphGen.restype = c_int
tenGlyphGen.argtypes = [POINTER(limnObject), POINTER(echoScene), POINTER(tenGlyphParm), POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
tenVerbose = (c_int).in_dll(_libraries[libteem], 'tenVerbose')
tenTensorCheck = _libraries[libteem].tenTensorCheck
tenTensorCheck.restype = c_int
tenTensorCheck.argtypes = [POINTER(Nrrd), c_int, c_int, c_int]
tenMeasurementFrameReduce = _libraries[libteem].tenMeasurementFrameReduce
tenMeasurementFrameReduce.restype = c_int
tenMeasurementFrameReduce.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
tenExpand = _libraries[libteem].tenExpand
tenExpand.restype = c_int
tenExpand.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double, c_double]
tenShrink = _libraries[libteem].tenShrink
tenShrink.restype = c_int
tenShrink.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
tenEigensolve_f = _libraries[libteem].tenEigensolve_f
tenEigensolve_f.restype = c_int
tenEigensolve_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float)]
tenEigensolve_d = _libraries[libteem].tenEigensolve_d
tenEigensolve_d.restype = c_int
tenEigensolve_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
tenMakeSingle_f = _libraries[libteem].tenMakeSingle_f
tenMakeSingle_f.restype = None
tenMakeSingle_f.argtypes = [POINTER(c_float), c_float, POINTER(c_float), POINTER(c_float)]
tenMakeSingle_d = _libraries[libteem].tenMakeSingle_d
tenMakeSingle_d.restype = None
tenMakeSingle_d.argtypes = [POINTER(c_double), c_double, POINTER(c_double), POINTER(c_double)]
tenMake = _libraries[libteem].tenMake
tenMake.restype = c_int
tenMake.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd)]
tenSlice = _libraries[libteem].tenSlice
tenSlice.restype = c_int
tenSlice.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_uint, size_t, c_uint]
tenInvariantGradientsK_d = _libraries[libteem].tenInvariantGradientsK_d
tenInvariantGradientsK_d.restype = None
tenInvariantGradientsK_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_double]
tenInvariantGradientsR_d = _libraries[libteem].tenInvariantGradientsR_d
tenInvariantGradientsR_d.restype = None
tenInvariantGradientsR_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_double]
tenRotationTangents_d = _libraries[libteem].tenRotationTangents_d
tenRotationTangents_d.restype = None
tenRotationTangents_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double)]
tenLogSingle_d = _libraries[libteem].tenLogSingle_d
tenLogSingle_d.restype = None
tenLogSingle_d.argtypes = [POINTER(c_double), POINTER(c_double)]
tenLogSingle_f = _libraries[libteem].tenLogSingle_f
tenLogSingle_f.restype = None
tenLogSingle_f.argtypes = [POINTER(c_float), POINTER(c_float)]
tenExpSingle_d = _libraries[libteem].tenExpSingle_d
tenExpSingle_d.restype = None
tenExpSingle_d.argtypes = [POINTER(c_double), POINTER(c_double)]
tenExpSingle_f = _libraries[libteem].tenExpSingle_f
tenExpSingle_f.restype = None
tenExpSingle_f.argtypes = [POINTER(c_float), POINTER(c_float)]
tenSqrtSingle_d = _libraries[libteem].tenSqrtSingle_d
tenSqrtSingle_d.restype = None
tenSqrtSingle_d.argtypes = [POINTER(c_double), POINTER(c_double)]
tenSqrtSingle_f = _libraries[libteem].tenSqrtSingle_f
tenSqrtSingle_f.restype = None
tenSqrtSingle_f.argtypes = [POINTER(c_float), POINTER(c_float)]
tenPowSingle_d = _libraries[libteem].tenPowSingle_d
tenPowSingle_d.restype = None
tenPowSingle_d.argtypes = [POINTER(c_double), POINTER(c_double), c_double]
tenPowSingle_f = _libraries[libteem].tenPowSingle_f
tenPowSingle_f.restype = None
tenPowSingle_f.argtypes = [POINTER(c_float), POINTER(c_float), c_float]
tenInv_f = _libraries[libteem].tenInv_f
tenInv_f.restype = None
tenInv_f.argtypes = [POINTER(c_float), POINTER(c_float)]
tenInv_d = _libraries[libteem].tenInv_d
tenInv_d.restype = None
tenInv_d.argtypes = [POINTER(c_double), POINTER(c_double)]
tenDoubleContract_d = _libraries[libteem].tenDoubleContract_d
tenDoubleContract_d.restype = c_double
tenDoubleContract_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double)]
tenDWMRIModalityKey = (STRING).in_dll(_libraries[libteem], 'tenDWMRIModalityKey')
tenDWMRIModalityVal = (STRING).in_dll(_libraries[libteem], 'tenDWMRIModalityVal')
tenDWMRINAVal = (STRING).in_dll(_libraries[libteem], 'tenDWMRINAVal')
tenDWMRIBValueKey = (STRING).in_dll(_libraries[libteem], 'tenDWMRIBValueKey')
tenDWMRIGradKeyFmt = (STRING).in_dll(_libraries[libteem], 'tenDWMRIGradKeyFmt')
tenDWMRIBmatKeyFmt = (STRING).in_dll(_libraries[libteem], 'tenDWMRIBmatKeyFmt')
tenDWMRINexKeyFmt = (STRING).in_dll(_libraries[libteem], 'tenDWMRINexKeyFmt')
tenDWMRISkipKeyFmt = (STRING).in_dll(_libraries[libteem], 'tenDWMRISkipKeyFmt')
tenDWMRIKeyValueParse = _libraries[libteem].tenDWMRIKeyValueParse
tenDWMRIKeyValueParse.restype = c_int
tenDWMRIKeyValueParse.argtypes = [POINTER(POINTER(Nrrd)), POINTER(POINTER(Nrrd)), POINTER(c_double), POINTER(POINTER(c_uint)), POINTER(c_uint), POINTER(Nrrd)]
tenBMatrixCalc = _libraries[libteem].tenBMatrixCalc
tenBMatrixCalc.restype = c_int
tenBMatrixCalc.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
tenEMatrixCalc = _libraries[libteem].tenEMatrixCalc
tenEMatrixCalc.restype = c_int
tenEMatrixCalc.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int]
tenEstimateLinearSingle_f = _libraries[libteem].tenEstimateLinearSingle_f
tenEstimateLinearSingle_f.restype = None
tenEstimateLinearSingle_f.argtypes = [POINTER(c_float), POINTER(c_float), POINTER(c_float), POINTER(c_double), POINTER(c_double), c_uint, c_int, c_float, c_float, c_float]
tenEstimateLinearSingle_d = _libraries[libteem].tenEstimateLinearSingle_d
tenEstimateLinearSingle_d.restype = None
tenEstimateLinearSingle_d.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_uint, c_int, c_double, c_double, c_double]
tenEstimateLinear3D = _libraries[libteem].tenEstimateLinear3D
tenEstimateLinear3D.restype = c_int
tenEstimateLinear3D.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(POINTER(Nrrd)), POINTER(POINTER(Nrrd)), c_uint, POINTER(Nrrd), c_int, c_double, c_double, c_double]
tenEstimateLinear4D = _libraries[libteem].tenEstimateLinear4D
tenEstimateLinear4D.restype = c_int
tenEstimateLinear4D.argtypes = [POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(POINTER(Nrrd)), POINTER(Nrrd), POINTER(Nrrd), c_int, c_double, c_double, c_double]
tenSimulateSingle_f = _libraries[libteem].tenSimulateSingle_f
tenSimulateSingle_f.restype = None
tenSimulateSingle_f.argtypes = [POINTER(c_float), c_float, POINTER(c_float), POINTER(c_double), c_uint, c_float]
tenSimulate = _libraries[libteem].tenSimulate
tenSimulate.restype = c_int
tenSimulate.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), c_double]
tenEstimateContextNew = _libraries[libteem].tenEstimateContextNew
tenEstimateContextNew.restype = POINTER(tenEstimateContext)
tenEstimateContextNew.argtypes = []
tenEstimateVerboseSet = _libraries[libteem].tenEstimateVerboseSet
tenEstimateVerboseSet.restype = None
tenEstimateVerboseSet.argtypes = [POINTER(tenEstimateContext), c_int]
tenEstimateNegEvalShiftSet = _libraries[libteem].tenEstimateNegEvalShiftSet
tenEstimateNegEvalShiftSet.restype = None
tenEstimateNegEvalShiftSet.argtypes = [POINTER(tenEstimateContext), c_int]
tenEstimateMethodSet = _libraries[libteem].tenEstimateMethodSet
tenEstimateMethodSet.restype = c_int
tenEstimateMethodSet.argtypes = [POINTER(tenEstimateContext), c_int]
tenEstimateSigmaSet = _libraries[libteem].tenEstimateSigmaSet
tenEstimateSigmaSet.restype = c_int
tenEstimateSigmaSet.argtypes = [POINTER(tenEstimateContext), c_double]
tenEstimateValueMinSet = _libraries[libteem].tenEstimateValueMinSet
tenEstimateValueMinSet.restype = c_int
tenEstimateValueMinSet.argtypes = [POINTER(tenEstimateContext), c_double]
tenEstimateGradientsSet = _libraries[libteem].tenEstimateGradientsSet
tenEstimateGradientsSet.restype = c_int
tenEstimateGradientsSet.argtypes = [POINTER(tenEstimateContext), POINTER(Nrrd), c_double, c_int]
tenEstimateBMatricesSet = _libraries[libteem].tenEstimateBMatricesSet
tenEstimateBMatricesSet.restype = c_int
tenEstimateBMatricesSet.argtypes = [POINTER(tenEstimateContext), POINTER(Nrrd), c_double, c_int]
tenEstimateSkipSet = _libraries[libteem].tenEstimateSkipSet
tenEstimateSkipSet.restype = c_int
tenEstimateSkipSet.argtypes = [POINTER(tenEstimateContext), c_uint, c_int]
tenEstimateSkipReset = _libraries[libteem].tenEstimateSkipReset
tenEstimateSkipReset.restype = c_int
tenEstimateSkipReset.argtypes = [POINTER(tenEstimateContext)]
tenEstimateThresholdSet = _libraries[libteem].tenEstimateThresholdSet
tenEstimateThresholdSet.restype = c_int
tenEstimateThresholdSet.argtypes = [POINTER(tenEstimateContext), c_double, c_double]
tenEstimateUpdate = _libraries[libteem].tenEstimateUpdate
tenEstimateUpdate.restype = c_int
tenEstimateUpdate.argtypes = [POINTER(tenEstimateContext)]
tenEstimate1TensorSimulateSingle_f = _libraries[libteem].tenEstimate1TensorSimulateSingle_f
tenEstimate1TensorSimulateSingle_f.restype = c_int
tenEstimate1TensorSimulateSingle_f.argtypes = [POINTER(tenEstimateContext), POINTER(c_float), c_float, c_float, c_float, POINTER(c_float)]
tenEstimate1TensorSimulateSingle_d = _libraries[libteem].tenEstimate1TensorSimulateSingle_d
tenEstimate1TensorSimulateSingle_d.restype = c_int
tenEstimate1TensorSimulateSingle_d.argtypes = [POINTER(tenEstimateContext), POINTER(c_double), c_double, c_double, c_double, POINTER(c_double)]
tenEstimate1TensorSimulateVolume = _libraries[libteem].tenEstimate1TensorSimulateVolume
tenEstimate1TensorSimulateVolume.restype = c_int
tenEstimate1TensorSimulateVolume.argtypes = [POINTER(tenEstimateContext), POINTER(Nrrd), c_double, c_double, POINTER(Nrrd), POINTER(Nrrd), c_int, c_int]
tenEstimate1TensorSingle_f = _libraries[libteem].tenEstimate1TensorSingle_f
tenEstimate1TensorSingle_f.restype = c_int
tenEstimate1TensorSingle_f.argtypes = [POINTER(tenEstimateContext), POINTER(c_float), POINTER(c_float)]
tenEstimate1TensorSingle_d = _libraries[libteem].tenEstimate1TensorSingle_d
tenEstimate1TensorSingle_d.restype = c_int
tenEstimate1TensorSingle_d.argtypes = [POINTER(tenEstimateContext), POINTER(c_double), POINTER(c_double)]
tenEstimate1TensorVolume4D = _libraries[libteem].tenEstimate1TensorVolume4D
tenEstimate1TensorVolume4D.restype = c_int
tenEstimate1TensorVolume4D.argtypes = [POINTER(tenEstimateContext), POINTER(Nrrd), POINTER(POINTER(Nrrd)), POINTER(POINTER(Nrrd)), POINTER(Nrrd), c_int]
tenEstimateContextNix = _libraries[libteem].tenEstimateContextNix
tenEstimateContextNix.restype = POINTER(tenEstimateContext)
tenEstimateContextNix.argtypes = [POINTER(tenEstimateContext)]
tenAnisoEval_f = _libraries[libteem].tenAnisoEval_f
tenAnisoEval_f.restype = c_float
tenAnisoEval_f.argtypes = [POINTER(c_float), c_int]
tenAnisoEval_d = _libraries[libteem].tenAnisoEval_d
tenAnisoEval_d.restype = c_double
tenAnisoEval_d.argtypes = [POINTER(c_double), c_int]
tenAnisoTen_f = _libraries[libteem].tenAnisoTen_f
tenAnisoTen_f.restype = c_float
tenAnisoTen_f.argtypes = [POINTER(c_float), c_int]
tenAnisoTen_d = _libraries[libteem].tenAnisoTen_d
tenAnisoTen_d.restype = c_double
tenAnisoTen_d.argtypes = [POINTER(c_double), c_int]
tenAnisoPlot = _libraries[libteem].tenAnisoPlot
tenAnisoPlot.restype = c_int
tenAnisoPlot.argtypes = [POINTER(Nrrd), c_int, c_uint, c_int, c_int, c_int]
tenAnisoVolume = _libraries[libteem].tenAnisoVolume
tenAnisoVolume.restype = c_int
tenAnisoVolume.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, c_double]
tenAnisoHistogram = _libraries[libteem].tenAnisoHistogram
tenAnisoHistogram.restype = c_int
tenAnisoHistogram.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), c_int, c_int, c_uint]
tenEvecRGBParmNew = _libraries[libteem].tenEvecRGBParmNew
tenEvecRGBParmNew.restype = POINTER(tenEvecRGBParm)
tenEvecRGBParmNew.argtypes = []
tenEvecRGBParmNix = _libraries[libteem].tenEvecRGBParmNix
tenEvecRGBParmNix.restype = POINTER(tenEvecRGBParm)
tenEvecRGBParmNix.argtypes = [POINTER(tenEvecRGBParm)]
tenEvecRGBParmCheck = _libraries[libteem].tenEvecRGBParmCheck
tenEvecRGBParmCheck.restype = c_int
tenEvecRGBParmCheck.argtypes = [POINTER(tenEvecRGBParm)]
tenEvecRGBSingle_f = _libraries[libteem].tenEvecRGBSingle_f
tenEvecRGBSingle_f.restype = None
tenEvecRGBSingle_f.argtypes = [POINTER(c_float), c_float, POINTER(c_float), POINTER(c_float), POINTER(tenEvecRGBParm)]
tenEvecRGBSingle_d = _libraries[libteem].tenEvecRGBSingle_d
tenEvecRGBSingle_d.restype = None
tenEvecRGBSingle_d.argtypes = [POINTER(c_double), c_double, POINTER(c_double), POINTER(c_double), POINTER(tenEvecRGBParm)]
tenEvecRGB = _libraries[libteem].tenEvecRGB
tenEvecRGB.restype = c_int
tenEvecRGB.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(tenEvecRGBParm)]
tenEvqVolume = _libraries[libteem].tenEvqVolume
tenEvqVolume.restype = c_int
tenEvqVolume.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_int, c_int, c_int]
tenBMatrixCheck = _libraries[libteem].tenBMatrixCheck
tenBMatrixCheck.restype = c_int
tenBMatrixCheck.argtypes = [POINTER(Nrrd), c_int, c_uint]
tenFiberSingleInit = _libraries[libteem].tenFiberSingleInit
tenFiberSingleInit.restype = None
tenFiberSingleInit.argtypes = [POINTER(tenFiberSingle)]
tenFiberSingleDone = _libraries[libteem].tenFiberSingleDone
tenFiberSingleDone.restype = None
tenFiberSingleDone.argtypes = [POINTER(tenFiberSingle)]
tenFiberSingleNew = _libraries[libteem].tenFiberSingleNew
tenFiberSingleNew.restype = POINTER(tenFiberSingle)
tenFiberSingleNew.argtypes = []
tenFiberSingleNix = _libraries[libteem].tenFiberSingleNix
tenFiberSingleNix.restype = POINTER(tenFiberSingle)
tenFiberSingleNix.argtypes = [POINTER(tenFiberSingle)]
tenFiberContextNew = _libraries[libteem].tenFiberContextNew
tenFiberContextNew.restype = POINTER(tenFiberContext)
tenFiberContextNew.argtypes = [POINTER(Nrrd)]
tenFiberContextDwiNew = _libraries[libteem].tenFiberContextDwiNew
tenFiberContextDwiNew.restype = POINTER(tenFiberContext)
tenFiberContextDwiNew.argtypes = [POINTER(Nrrd), c_double, c_double, c_double, c_int, c_int]
tenFiberVerboseSet = _libraries[libteem].tenFiberVerboseSet
tenFiberVerboseSet.restype = None
tenFiberVerboseSet.argtypes = [POINTER(tenFiberContext), c_int]
tenFiberTypeSet = _libraries[libteem].tenFiberTypeSet
tenFiberTypeSet.restype = c_int
tenFiberTypeSet.argtypes = [POINTER(tenFiberContext), c_int]
tenFiberKernelSet = _libraries[libteem].tenFiberKernelSet
tenFiberKernelSet.restype = c_int
tenFiberKernelSet.argtypes = [POINTER(tenFiberContext), POINTER(NrrdKernel), POINTER(c_double)]
tenFiberIntgSet = _libraries[libteem].tenFiberIntgSet
tenFiberIntgSet.restype = c_int
tenFiberIntgSet.argtypes = [POINTER(tenFiberContext), c_int]
tenFiberStopSet = _libraries[libteem].tenFiberStopSet
tenFiberStopSet.restype = c_int
tenFiberStopSet.argtypes = [POINTER(tenFiberContext), c_int]
tenFiberStopAnisoSet = _libraries[libteem].tenFiberStopAnisoSet
tenFiberStopAnisoSet.restype = c_int
tenFiberStopAnisoSet.argtypes = [POINTER(tenFiberContext), c_int, c_double]
tenFiberStopDoubleSet = _libraries[libteem].tenFiberStopDoubleSet
tenFiberStopDoubleSet.restype = c_int
tenFiberStopDoubleSet.argtypes = [POINTER(tenFiberContext), c_int, c_double]
tenFiberStopUIntSet = _libraries[libteem].tenFiberStopUIntSet
tenFiberStopUIntSet.restype = c_int
tenFiberStopUIntSet.argtypes = [POINTER(tenFiberContext), c_int, c_uint]
tenFiberStopOn = _libraries[libteem].tenFiberStopOn
tenFiberStopOn.restype = None
tenFiberStopOn.argtypes = [POINTER(tenFiberContext), c_int]
tenFiberStopOff = _libraries[libteem].tenFiberStopOff
tenFiberStopOff.restype = None
tenFiberStopOff.argtypes = [POINTER(tenFiberContext), c_int]
tenFiberStopReset = _libraries[libteem].tenFiberStopReset
tenFiberStopReset.restype = None
tenFiberStopReset.argtypes = [POINTER(tenFiberContext)]
tenFiberAnisoSpeedSet = _libraries[libteem].tenFiberAnisoSpeedSet
tenFiberAnisoSpeedSet.restype = c_int
tenFiberAnisoSpeedSet.argtypes = [POINTER(tenFiberContext), c_int, c_double, c_double, c_double]
tenFiberAnisoSpeedReset = _libraries[libteem].tenFiberAnisoSpeedReset
tenFiberAnisoSpeedReset.restype = c_int
tenFiberAnisoSpeedReset.argtypes = [POINTER(tenFiberContext)]
tenFiberParmSet = _libraries[libteem].tenFiberParmSet
tenFiberParmSet.restype = c_int
tenFiberParmSet.argtypes = [POINTER(tenFiberContext), c_int, c_double]
tenFiberUpdate = _libraries[libteem].tenFiberUpdate
tenFiberUpdate.restype = c_int
tenFiberUpdate.argtypes = [POINTER(tenFiberContext)]
tenFiberContextCopy = _libraries[libteem].tenFiberContextCopy
tenFiberContextCopy.restype = POINTER(tenFiberContext)
tenFiberContextCopy.argtypes = [POINTER(tenFiberContext)]
tenFiberContextNix = _libraries[libteem].tenFiberContextNix
tenFiberContextNix.restype = POINTER(tenFiberContext)
tenFiberContextNix.argtypes = [POINTER(tenFiberContext)]
tenFiberTraceSet = _libraries[libteem].tenFiberTraceSet
tenFiberTraceSet.restype = c_int
tenFiberTraceSet.argtypes = [POINTER(tenFiberContext), POINTER(Nrrd), POINTER(c_double), c_uint, POINTER(c_uint), POINTER(c_uint), POINTER(c_double)]
tenFiberTrace = _libraries[libteem].tenFiberTrace
tenFiberTrace.restype = c_int
tenFiberTrace.argtypes = [POINTER(tenFiberContext), POINTER(Nrrd), POINTER(c_double)]
tenFiberDirectionNumber = _libraries[libteem].tenFiberDirectionNumber
tenFiberDirectionNumber.restype = c_uint
tenFiberDirectionNumber.argtypes = [POINTER(tenFiberContext), POINTER(c_double)]
tenFiberSingleTrace = _libraries[libteem].tenFiberSingleTrace
tenFiberSingleTrace.restype = c_int
tenFiberSingleTrace.argtypes = [POINTER(tenFiberContext), POINTER(tenFiberSingle), POINTER(c_double), c_uint]
tenFiberMultiNew = _libraries[libteem].tenFiberMultiNew
tenFiberMultiNew.restype = POINTER(tenFiberMulti)
tenFiberMultiNew.argtypes = []
tenFiberMultiNix = _libraries[libteem].tenFiberMultiNix
tenFiberMultiNix.restype = POINTER(tenFiberMulti)
tenFiberMultiNix.argtypes = [POINTER(tenFiberMulti)]
tenFiberMultiTrace = _libraries[libteem].tenFiberMultiTrace
tenFiberMultiTrace.restype = c_int
tenFiberMultiTrace.argtypes = [POINTER(tenFiberContext), POINTER(tenFiberMulti), POINTER(Nrrd)]
tenFiberMultiPolyData = _libraries[libteem].tenFiberMultiPolyData
tenFiberMultiPolyData.restype = c_int
tenFiberMultiPolyData.argtypes = [POINTER(tenFiberContext), POINTER(limnPolyData), POINTER(tenFiberMulti)]
tenEpiRegister3D = _libraries[libteem].tenEpiRegister3D
tenEpiRegister3D.restype = c_int
tenEpiRegister3D.argtypes = [POINTER(POINTER(Nrrd)), POINTER(POINTER(Nrrd)), c_uint, POINTER(Nrrd), c_int, c_double, c_double, c_double, c_double, c_int, POINTER(NrrdKernel), POINTER(c_double), c_int, c_int]
tenEpiRegister4D = _libraries[libteem].tenEpiRegister4D
tenEpiRegister4D.restype = c_int
tenEpiRegister4D.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(Nrrd), c_int, c_double, c_double, c_double, c_double, c_int, POINTER(NrrdKernel), POINTER(c_double), c_int, c_int]
tenSizeNormalize = _libraries[libteem].tenSizeNormalize
tenSizeNormalize.restype = c_int
tenSizeNormalize.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(c_double), c_double, c_double]
tenSizeScale = _libraries[libteem].tenSizeScale
tenSizeScale.restype = c_int
tenSizeScale.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double]
tenAnisoScale = _libraries[libteem].tenAnisoScale
tenAnisoScale.restype = c_int
tenAnisoScale.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double, c_int, c_int]
tenEigenvaluePower = _libraries[libteem].tenEigenvaluePower
tenEigenvaluePower.restype = c_int
tenEigenvaluePower.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double]
tenEigenvalueClamp = _libraries[libteem].tenEigenvalueClamp
tenEigenvalueClamp.restype = c_int
tenEigenvalueClamp.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double, c_double]
tenEigenvalueAdd = _libraries[libteem].tenEigenvalueAdd
tenEigenvalueAdd.restype = c_int
tenEigenvalueAdd.argtypes = [POINTER(Nrrd), POINTER(Nrrd), c_double]
tenLog = _libraries[libteem].tenLog
tenLog.restype = c_int
tenLog.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
tenExp = _libraries[libteem].tenExp
tenExp.restype = c_int
tenExp.argtypes = [POINTER(Nrrd), POINTER(Nrrd)]
tenBVecNonLinearFit = _libraries[libteem].tenBVecNonLinearFit
tenBVecNonLinearFit.restype = c_int
tenBVecNonLinearFit.argtypes = [POINTER(Nrrd), POINTER(Nrrd), POINTER(c_double), POINTER(c_double), c_int, c_double]
tenGageKind = (POINTER(gageKind)).in_dll(_libraries[libteem], 'tenGageKind')
tenDwiGage = (POINTER(airEnum)).in_dll(_libraries[libteem], 'tenDwiGage')
tenDwiGageKindNew = _libraries[libteem].tenDwiGageKindNew
tenDwiGageKindNew.restype = POINTER(gageKind)
tenDwiGageKindNew.argtypes = []
tenDwiGageKindNix = _libraries[libteem].tenDwiGageKindNix
tenDwiGageKindNix.restype = POINTER(gageKind)
tenDwiGageKindNix.argtypes = [POINTER(gageKind)]
tenDwiGageKindSet = _libraries[libteem].tenDwiGageKindSet
tenDwiGageKindSet.restype = c_int
tenDwiGageKindSet.argtypes = [POINTER(gageKind), c_double, c_double, c_double, c_double, POINTER(Nrrd), POINTER(Nrrd), c_int, c_int, c_uint]
tenDwiGageKindCheck = _libraries[libteem].tenDwiGageKindCheck
tenDwiGageKindCheck.restype = c_int
tenDwiGageKindCheck.argtypes = [POINTER(gageKind)]
tenEMBimodalParmNew = _libraries[libteem].tenEMBimodalParmNew
tenEMBimodalParmNew.restype = POINTER(tenEMBimodalParm)
tenEMBimodalParmNew.argtypes = []
tenEMBimodalParmNix = _libraries[libteem].tenEMBimodalParmNix
tenEMBimodalParmNix.restype = POINTER(tenEMBimodalParm)
tenEMBimodalParmNix.argtypes = [POINTER(tenEMBimodalParm)]
tenEMBimodal = _libraries[libteem].tenEMBimodal
tenEMBimodal.restype = c_int
tenEMBimodal.argtypes = [POINTER(tenEMBimodalParm), POINTER(Nrrd)]
class unrrduCmd(Structure):
    pass
unrrduCmd._fields_ = [
    ('name', STRING),
    ('info', STRING),
    ('main', CFUNCTYPE(c_int, c_int, POINTER(STRING), STRING, POINTER(hestParm))),
]
tend_shrinkCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_shrinkCmd')
tend_anscaleCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_anscaleCmd')
tend_sliceCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_sliceCmd')
tend_anhistCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_anhistCmd')
tend_ellipseCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_ellipseCmd')
tend_evalCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_evalCmd')
tend_evqCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_evqCmd')
tend_simCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_simCmd')
tend_tripleCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_tripleCmd')
tend_expCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_expCmd')
tend_logCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_logCmd')
tend_satinCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_satinCmd')
tend_stenCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_stenCmd')
tend_glyphCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_glyphCmd')
tend_anvolCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_anvolCmd')
tend_evalpowCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_evalpowCmd')
tend_evecrgbCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_evecrgbCmd')
tend_fiberCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_fiberCmd')
tend_expandCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_expandCmd')
tend_anplotCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_anplotCmd')
tend_aboutCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_aboutCmd')
tend_bfitCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_bfitCmd')
tend_helixCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_helixCmd')
tend_bmatCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_bmatCmd')
tend_avgCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_avgCmd')
tend_evaladdCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_evaladdCmd')
tend_evalclampCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_evalclampCmd')
tend_normCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_normCmd')
tend_makeCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_makeCmd')
tend_estimCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_estimCmd')
tend_unmfCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_unmfCmd')
tend_epiregCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_epiregCmd')
tend_evecCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_evecCmd')
tend_pointCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_pointCmd')
tend_gradsCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_gradsCmd')
tend_tconvCmd = (unrrduCmd).in_dll(_libraries[libteem], 'tend_tconvCmd')
tendCmdList = (POINTER(unrrduCmd) * 0).in_dll(_libraries[libteem], 'tendCmdList')
tendUsage = _libraries[libteem].tendUsage
tendUsage.restype = None
tendUsage.argtypes = [STRING, POINTER(hestParm)]
tendFiberStopCB = (POINTER(hestCB)).in_dll(_libraries[libteem], 'tendFiberStopCB')
unrrdu_joinCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_joinCmd')
unrrdu_jhistoCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_jhistoCmd')
unrrdu_makeCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_makeCmd')
unrrdu_flipCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_flipCmd')
unrrdu_axmergeCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_axmergeCmd')
unrrdu_cropCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_cropCmd')
unrrdu_mlutCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_mlutCmd')
unrrdu_axinfoCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_axinfoCmd')
unrrdu_axinsertCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_axinsertCmd')
unrrdu_untileCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_untileCmd')
unrrdu_insetCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_insetCmd')
unrrdu_shuffleCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_shuffleCmd')
unrrdu_headCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_headCmd')
unrrdu_histaxCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_histaxCmd')
unrrdu_ccfindCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_ccfindCmd')
unrrdu_ccadjCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_ccadjCmd')
unrrdu_distCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_distCmd')
unrrdu_2opCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_2opCmd')
unrrdu_mrmapCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_mrmapCmd')
unrrdu_histoCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_histoCmd')
unrrdu_rmapCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_rmapCmd')
unrrdu_axdeleteCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_axdeleteCmd')
unrrdu_ccmergeCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_ccmergeCmd')
unrrdu_unorientCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_unorientCmd')
unrrdu_permuteCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_permuteCmd')
unrrdu_tileCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_tileCmd')
unrrdu_substCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_substCmd')
unrrdu_dhistoCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_dhistoCmd')
unrrdu_aboutCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_aboutCmd')
unrrdu_gammaCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_gammaCmd')
unrrdu_padCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_padCmd')
unrrdu_ccsettleCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_ccsettleCmd')
unrrdu_spliceCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_spliceCmd')
unrrdu_minmaxCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_minmaxCmd')
unrrdu_swapCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_swapCmd')
unrrdu_sliceCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_sliceCmd')
unrrdu_lut2Cmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_lut2Cmd')
unrrdu_3opCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_3opCmd')
unrrdu_imapCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_imapCmd')
unrrdu_resampleCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_resampleCmd')
unrrdu_heqCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_heqCmd')
unrrdu_projectCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_projectCmd')
unrrdu_cmedianCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_cmedianCmd')
unrrdu_diceCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_diceCmd')
unrrdu_quantizeCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_quantizeCmd')
unrrdu_unquantizeCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_unquantizeCmd')
unrrdu_saveCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_saveCmd')
unrrdu_convertCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_convertCmd')
unrrdu_lutCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_lutCmd')
unrrdu_1opCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_1opCmd')
unrrdu_reshapeCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_reshapeCmd')
unrrdu_dataCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_dataCmd')
unrrdu_envCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_envCmd')
unrrdu_axsplitCmd = (unrrduCmd).in_dll(_libraries[libteem], 'unrrdu_axsplitCmd')
unrrduBiffKey = (STRING).in_dll(_libraries[libteem], 'unrrduBiffKey')
unrrduDefNumColumns = (c_int).in_dll(_libraries[libteem], 'unrrduDefNumColumns')
unrrduCmdList = (POINTER(unrrduCmd) * 0).in_dll(_libraries[libteem], 'unrrduCmdList')
unrrduUsage = _libraries[libteem].unrrduUsage
unrrduUsage.restype = None
unrrduUsage.argtypes = [STRING, POINTER(hestParm)]
unrrduHestPosCB = (hestCB).in_dll(_libraries[libteem], 'unrrduHestPosCB')
unrrduHestMaybeTypeCB = (hestCB).in_dll(_libraries[libteem], 'unrrduHestMaybeTypeCB')
unrrduHestScaleCB = (hestCB).in_dll(_libraries[libteem], 'unrrduHestScaleCB')
unrrduHestBitsCB = (hestCB).in_dll(_libraries[libteem], 'unrrduHestBitsCB')
unrrduHestFileCB = (hestCB).in_dll(_libraries[libteem], 'unrrduHestFileCB')
unrrduHestEncodingCB = (hestCB).in_dll(_libraries[libteem], 'unrrduHestEncodingCB')
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
__all__ = ['tenFiberStopUIntSet', 'limnCameraPathTrack',
           'gageVecLambda2', 'miteUserNix', 'tenFiberIntgLast',
           'ell_4m_post_mul_f', 'nrrdArithGamma', 'ell_Nm_check',
           'tenEstimate1MethodMLE', 'nrrdHasNonExistOnly',
           'nrrdFormatPNG', 'miteBiffKey', 'limnEdgeTypeLast',
           'tenGageFAShapeIndex', 'nrrdOriginStatusNoMin',
           'airRandMTStateNix', 'miteValVdefTdotV',
           'nrrdKernelC4Hexic', 'tenEstimateContextNix',
           'tenGageFAGeomTens', 'airEndsWith', 'miteUser',
           'ell_4m_to_q_d', 'nrrdField_block_size',
           'tenEstimateSkipReset', 'limnSpaceView', 'limnSplineMinT',
           'nrrdFieldInfoParse', 'nrrdField_space',
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
           'seekTypeMaximalSurface', 'unrrdu_flipCmd',
           'gageErrUnknown', 'nrrdBinaryOpSgnPow', 'nrrdKeyValueGet',
           'limnQN16octa', 'nrrdKernelC3QuinticD', 'airTypeFloat',
           'limnObjectVertexNumPreSet', 'tenGlyphTypeBox',
           'limnLightReset', 'nrrdField_space_units',
           'gageSclShapeTrace', 'tenDwiGage2TensorPeled',
           'nrrdMeasureMax', 'miteShadeMethodLast',
           'unrrdu_minmaxCmd', 'unrrdu_substCmd',
           'limnHestPolyDataLMPD', 'nrrdUnaryOpExists',
           'nrrdSpace3DRightHandedTime', 'airArrayNix',
           'airNoDio_format', 'gageCtxFlagLast', 'dyeConvert',
           'limnQNLast', 'nrrdBasicInfoMeasurementFrame',
           'limnQN8octa', 'limnPolyDataNix', 'tenInterpTypeWang',
           'unrrdu_envCmd', 'nrrdSpace3DLeftHanded',
           'nrrdUnaryOpCbrt', 'nrrdAxisInfoLast', 'airFStore',
           'hooverThreadBegin_t', 'airFastExp', 'tenGageQGradVec',
           'tenGageOmegaLaplacian', 'seekTypeMinimalSurface',
           'tenGlyphType', 'tenFiberSingleTrace',
           'tenGageOmegaDiffusionFraction', 'gageVecProjHelGradient',
           'gageDefRequireEqualCenters', 'nrrdStateMeasureType',
           'tenTripleConvert', 'hestElideSingleOtherDefault',
           'echoTypeRectangle', 'nrrdUntile2D',
           'nrrdSpaceLeftAnteriorSuperior', 'nrrdTypeULLong',
           'airInsane_UCSize', 'limnSplineTypeSpecParse',
           'limnSplineInfo4Vector', 'tenInterpParmBufferAlloc',
           'nrrdResampleRangeSet', 'unrrdu_dhistoCmd',
           'tenFiberIntgRK4', 'tenGageModeWarp', 'nrrdFormatUnknown',
           'tenInterpTwoDiscrete_d', 'tenFiberStopMinNumSteps',
           'gageKindVolumeCheck', 'tenGageNormGradVec',
           'tenGageQHessian', 'tenEstimate2MethodUnknown',
           'limnPolyDataCone', 'hestParmNew', 'tenEstimate2Method',
           'nrrdUnaryOpAcos', 'tendCmdList', 'airMopAlways',
           'gageKindAnswerLength', 'nrrdMeasureSD', 'tenBMatrixCalc',
           'tenDwiGageKindSet', 'miteVariableParse',
           'echoThreadStateNew', 'nrrdBinaryOpGT',
           'tenDefFiberMaxHalfLen', 'unrrdu_3opCmd', 'airFloatQNaN',
           'nrrdSplice', 'tenEigensolve_f', 'tend_evqCmd',
           'tenEigensolve_d', 'tenGageQGradMag', 'nrrdArithUnaryOp',
           'nrrdStateDisableContent', 'echoColorSet', 'tenGageEval2',
           'nrrdEnvVarStateMeasureModeBins', 'miteRayBegin',
           'gageParmDefaultCenter', 'unrrdu_ccmergeCmd',
           'miteThreadNew', 'miteStage', 'airDrandMT53_r',
           'echoChannelAverage', 'tenFiberTypeEvec1',
           'tenFiberTypeEvec0', 'tenFiberTypeEvec2',
           'gageVecCurlNorm', 'ell_4m_to_q_f', 'ell_3v_print_d',
           'nrrdMeasureHistoMedian', 'ell_3v_print_f',
           'unrrdu_ccsettleCmd', 'tenGageFADiffusionFraction',
           'limnPolyDataSpiralSuperquadric', 'nrrdArithIterBinaryOp',
           'nrrdBasicInfoSpaceOrigin', 'airUnescape', 'nrrdPPM',
           'biffGetDone', 'airFPGen_f', 'airFPGen_d',
           'airInsane_FltDblFPClass', 'nrrdBinaryOpIf',
           'nrrdAxisInfoSpacing', 'airExists', 'tenDWMRINexKeyFmt',
           'nrrdBinaryOpLTE', 'nrrdSpaceLeftAnteriorSuperiorTime',
           'limnPolyDataCopy', 'nrrdAxisInfoUnknown',
           'miteThreadBegin', 'limnPrimitiveLines', 'unrrduCmdList',
           'airSinglePrintf', 'gageShapeCopy', 'tenFiberVerboseSet',
           'nrrdSanity', 'nrrdUnaryOpTan', 'tenAniso_RA',
           'tenTripleTypeRThetaZ', 'tenGageRHessian', 'nrrdFlip',
           'airNoDio_size', 'seekTypeRidgeSurface',
           'gagePerVolumeNew', 'nrrdAxesSwap', 'gageItemSpec',
           'nrrdDefaultWriteBareText', 'airShuffle_r',
           'limnObjectRender', 'miteValXw',
           'nrrdSpacingStatusScalarWithSpace', 'tenDWMRIModalityKey',
           'gageSclK1', 'miteValXi', 'nrrdOriginStatusDirection',
           'tend_normCmd', 'nrrdWrite', 'ell_biff_key',
           'nrrdAlloc_va', 'limnSplineUpdate', 'tenFiberParmStepSize',
           'limnObjectSpaceTransform', 'nrrdKeyValueErase',
           'limnPolyDataPrimitiveArea', 'nrrdResampleTypeOutSet',
           'unrrduHestScaleCB', 'nrrdArithBinaryOp',
           'nrrdZlibStrategyDefault', 'tenEMBimodalParmNix',
           'tenAniso_Cp1', 'nrrdIterNix', 'tenAniso_Cp2',
           'nrrdUIInsert', 'limnQN15octa', 'tend_shrinkCmd',
           'tend_expCmd', 'ell_3m_svd_d', 'limnQN9octa',
           'unrrdu_axinsertCmd', 'airBesselIn',
           'nrrdEnvVarDefaultSpacing', 'nrrdTypeUChar',
           'airNoDio_ptr', 'tenGageFALaplacian', 'tend_ellipseCmd',
           'airTypeLast', 'nrrdBasicInfoSpace', 'miteStageOpLast',
           'tenDwiGageConfidence', 'ell_cubic_root_last', 'gageProbe',
           'nrrdMeasureL2', 'nrrdMeasureL1', 'miteShadeMethodLitTen',
           'nrrdCenterUnknown', 'limnHestCameraOptAdd', 'airBesselI0',
           'airBesselI1', 'tenGradientParmNew', 'nrrdIterValue',
           'nrrdKind2DMaskedMatrix', 'airSgnPow',
           'nrrdBasicInfoOldMin', 'limnSpaceDevice',
           'gageVecHelicity', 'airEndianLast', 'gageSclHessFrob',
           'airThreadNew', 'tenGageOmegaHessianEval2',
           'echoListSplit3', 'tenGageOmegaHessianEval0',
           'tenDwiGageTensorAllDWIError',
           'nrrdSpacingStatusScalarNoSpace', 'tend_glyphCmd',
           'nrrdHistoEq', 'limnSpline_t', 'biffDone', 'tenGage',
           'nrrdBinaryOpExists', 'nrrdArithTernaryOp', 'tenFiberStop',
           'airDStore', 'echoGlobalStateNix', 'limnLook',
           'limnLightSet', 'seekStrengthSet', 'nrrdUnaryOpExp',
           'limnObjectReadOFF', 'tenGradientParm', 'hooverContextNew',
           'tenRotationTangents_d', 'limnPolyDataVertexWindingFlip',
           'nrrdTypeNumberOfValues', 'nrrdBoundaryBleed',
           'ell_cubic_root_unknown', 'echoJitterUnknown',
           'nrrdResampleRenormalizeSet', 'ell_aa_to_q_f',
           'ell_aa_to_q_d', 'miteDefRefStep',
           'limnSplineNrrdCleverFix', 'miteValGageKind',
           'nrrdEnvVarDefaultWriteEncodingType', 'echoSuperquadSet',
           'nrrdSpaceOriginSet', 'nrrdResampleInfoNew', 'dyeLUVtoXYZ',
           'tenAnisoUnknown', 'nrrdKernelCentDiff', 'miteThread',
           'tenGageFAHessianEvec', 'limnSplineInfoLast',
           'nrrdMeasureLineSlope', 'tenDwiGageKindData',
           'tenFiberKernelSet', 'nrrdTypeIsUnsigned',
           'airNoDio_setfl', 'tend_anhistCmd', 'ell_3m_print_f',
           'ell_3m_print_d', 'tenGageTraceDiffusionAlign',
           'gagePvlFlagNeedD', 'nrrdBasicInfoComments',
           'echoSuperquad', 'nrrdKind3DMaskedSymMatrix',
           'limnPolyDataTransform_f', 'tenGlyphTypeCylinder',
           'tenEstimate1TensorSimulateSingle_f',
           'tenEstimate1TensorSimulateSingle_d', 'nrrdHistoAxis',
           'tenDWMRINAVal', 'tenDwiGageTensorWLS',
           'tenInterpTypeQuatGeoLoxR', 'limnPrimitiveNoop',
           'tenGageTensorGradMag', 'unrrdu_sliceCmd',
           'nrrdIoStateSkipData', 'echoJittableLast',
           'nrrdCenterNode', 'echoCylinder', 'limnQN12checker',
           'airDLoad', 'airUIrandMT_r', 'nrrdDStore', 'nrrdRangeNix',
           'tenGageFAKappa2', 'nrrdUnaryOpLast', 'tenGageFAKappa1',
           'unrrdu_imapCmd', 'tenDwiGageTensorLLSErrorLog',
           'limnPrimitive', 'nrrdKind2DSymMatrix', 'tenAniso_Ca2',
           'tenAniso_Ca1', 'tenGageModeHessian',
           'tenInvariantGradientsK_d', 'NrrdResampleInfo',
           'gageParmVerbose', 'airTypeUInt', 'tenInterpTypeLinear',
           'unrrduCmd', 'limnVertex', 'nrrdCheck',
           'limnPolyDataTransform_d', 'gageParmRenormalize',
           'nrrdKernelGaussianDD', 'nrrdKindUnknown',
           'ell_cubic_root', 'tenEstimateMethodSet', 'nrrdMeasureSum',
           'airNoDio_arch', 'tenInterpPathLength',
           'echoMatterGlassFuzzy', 'nrrdRangeSafeSet',
           'tenInterpParmNew', 'airCbrt', 'airTypeEnum',
           'nrrdApply1DRegMap', 'tenGageCl2', 'tenGageCl1',
           'limnObjectPartTransform', 'tenFiberStopMinLength',
           'nrrdKernelBlackman', 'echoInstance',
           'nrrdMeasureHistoMax', 'limnPolyDataPlane',
           'limnSplineSample', 'nrrdFormatText',
           'nrrdResampleNrrdSet', 'nrrdEnvVarDefaultWriteValsPerLine',
           'miteStageOpMax', 'airFP_POS_NORM', 'nrrdKernelHann',
           'ell_q_pow_d', 'nrrdBinaryOpGTE', 'ell_q_pow_f',
           'unrrdu_mrmapCmd', 'tenGageFAGradVecDotEvec0',
           'nrrdKindPoint', 'limnPolyDataPolygonNumber', 'tenBiffKey',
           'nrrdFormatType', 'seekSamplesSet', 'nrrdUnaryOpLog',
           'limnPolyDataWriteLMPD', 'nrrdUnaryOpNegative',
           'nrrdFInsert', 'gageDefStackRenormalize',
           'limnCameraPathTrackFrom', 'hestRespFileFlag',
           'nrrdMeasureMode', 'nrrdDefaultWriteValsPerLine',
           'gageSclLaplacian', 'tenGlyphParm', 'nrrdConvert',
           'tenGradientDistribute', 'tenGlyphTypeSuperquad',
           'nrrdFormatTypeText', 'miteValYw', 'airDioRead',
           'hooverContext', 'echoMatterLast', 'airNoDio_dioinfo',
           'miteValYi', 'tenGageOmegaDiffusionAlign',
           'nrrdMeasureHistoSD', 'tenEstimateContextNew',
           'tenAnisoPlot', 'nrrdIoStateInit', 'dyeHSVtoRGB',
           'nrrdEnvVarStateMeasureHistoType', 'nrrdAxisInfoKind',
           'airTime', 'limnFace', 'limnPolyDataInfoNorm',
           'tenGageQNormal', 'limnPolyDataInfoTex2', 'nrrdHisto',
           'nrrdHasNonExistFalse', 'miteVariablePrint',
           'tenFiberParmUseIndexSpace', 'nrrdKind3Gradient',
           'gageAnswerPointer', 'nrrdCCFind', 'nrrdKeyValueClear',
           'nrrdCenter', 'airBesselI1ExpScaled',
           'nrrdKernelC3Quintic', 'gageShapeNix',
           'echoJittableNormalA', 'echoJittableNormalB',
           'nrrdResampleDefaultCenterSet', 'tenGageSNormal',
           'miteShadeSpecNix', 'nrrdTypeShort',
           'ell_cubic_root_single', 'gageShapeReset',
           'tenGageBGradMag', 'limnCameraPathTrackUnknown',
           'ell_3m_1d_nullspace_d', 'nrrdGetenvInt',
           'nrrdKindIsDomain', 'ell_3mv_mul_d', 'dyeXYZtoLUV',
           'ell_3mv_mul_f', 'nrrdType', 'echoThreadStateInit',
           'NrrdResampleAxis', 'nrrdAxisInfoCenter',
           'hestElideSingleOtherType', 'tenEMBimodalParm',
           'tenInterpTypeQuatGeoLoxK', 'echoAABBox', 'NrrdIter',
           'gageDefDefaultCenter', 'airThreadMutexNew',
           'tenGageTensorGrad', 'limnHestSplineTypeSpec',
           'limnSplineTypeUnknown', 'airMopDone', 'tend_unmfCmd',
           'tenMeasurementFrameReduce', 'nrrdTernaryOpLerp',
           'NrrdIoState', 'airErfc', 'airSrandMT_r', 'biffSetStr',
           'nrrdBinaryOpDivide', 'nrrdFprint', 'NrrdResampleContext',
           'nrrdMeasureVariance', 'nrrdHistoThresholdOtsu',
           'airEnumVal', 'nrrdSpaceLeftPosteriorSuperiorTime',
           'airTypeStr', 'tenFiberStopOn', 'hooverErrSample',
           'hooverStubSample', 'limnEdge', 'nrrdSprint',
           'hooverBiffKey', 'nrrdTypeMax', 'gageParmLast',
           'airParseStrF', 'airParseStrD', 'airParseStrE',
           'airParseStrB', 'airParseStrC', 'nrrdDistanceL2',
           'nrrdEnvVarStateKeyValuePairsPropagate', 'gageSclMeanCurv',
           'airParseStrI', 'airParseStrS', 'nrrdTernaryOpMin',
           'unrrdu_convertCmd', 'tenGageEval1', 'tenGageEval0',
           'tenAniso_Cs1', 'tenGageAniso', 'tenDwiGageTensor',
           'echoRTRender', 'dyeXYZtoRGB', 'gageKindTotalAnswerLength',
           'airBinaryPrintUInt', 'dyeSpaceLUV', 'nrrdKernelGaussianD',
           'airAtod', 'nrrdField_number', 'tenGageModeGradMag',
           'tenAniso_Conf', 'gageAnswerLength', 'nrrdByteSkip',
           'nrrdBasicInfoData', 'hestColumns', 'gageKernelReset',
           'airThreadCapable', 'nrrdFormatTypeLast', 'dyeColorCopy',
           'nrrdEnvVarStateVerboseIO', 'gageDefVerbose',
           'echoThreadStateNix', 'tenGageDelNormR2', 'miteNtxfCheck',
           'tenAniso_Omega', 'hooverStubThreadBegin', 'unrrdu_1opCmd',
           'nrrdBinaryOpNormalRandScaleAdd', 'tenGageFACurvDir1',
           'tenGageFACurvDir2', 'hooverErrThreadCreate',
           'limnSplineNumPoints', 'nrrdZlibStrategyHuffman',
           'nrrdDLookup', 'ell_q_3v_rotate_f', 'nrrdDInsert',
           'ell_q_3v_rotate_d', 'tenGageCa1', 'ell_3m_eigenvalues_d',
           'hooverDefVolCentering', 'gageVecHessian', 'limnObject',
           'tenEpiRegister4D', 'tenFiberStopFraction', 'airToLower',
           'nrrd1DIrregAclCheck', 'miteValZw', 'airThreadCond',
           'tenMake', 'unrrdu_makeCmd', 'miteValZi', 'ell_cubic',
           'nrrdKind3Normal', 'airMyDio', 'nrrdUnaryOpCeil',
           'nrrdUnaryOpReciprocal', 'limnObjectPreSet', 'gageShape',
           'tenDefFiberIntg', 'limnCameraNix',
           'tenDwiFiberType2Evec0', 'tenMakeSingle_d',
           'tenMakeSingle_f', 'nrrdUILookup', 'tenEstimate1MethodWLS',
           'nrrdAxisInfoPos', 'hooverErrNone', 'tend_fiberCmd',
           'nrrdIoStateNix', 'gageVecVector1', 'nrrdBinaryOpMin',
           'tenBVecNonLinearFit', 'hooverSample_t',
           'tenEstimateNegEvalShiftSet', 'nrrdCCAdjacency',
           'airEnumDesc', 'unrrdu_gammaCmd', 'nrrdWrap_va',
           'nrrdStateKeyValueReturnInternalPointers',
           'unrrduHestBitsCB', 'nrrdEncodingBzip2', 'miteValGTdotV',
           'tenSqrtSingle_d', 'nrrdKindTime', 'tenDwiFiberType',
           'airArrayLenPreSet', 'nrrdMeasure', 'airParseStrZ',
           'airMopAdd', 'limnObjectEmpty', 'airNormalRand_r',
           'ell_3m_inv_d', 'ell_3m_inv_f', 'miteValNormal',
           'limnQN12octa', 'unrrdu_lutCmd', 'tenDwiGageUnknown',
           'miteValTw', 'tenFiberIntg', 'miteValTi',
           'nrrdFormatTypeEPS', 'tend_tripleCmd', 'dyeColorParse',
           'airIndexClamp', 'nrrdEncodingTypeLast', 'nrrdTypeDefault',
           'nrrdEncodingTypeAscii', 'tenAniso_FA', 'tenGageFAHessian',
           'limnPolyDataReadOFF', 'gageSclValue', 'airEqvSettle',
           'echoIsosurface', 'echoMatterPhongSp', 'nrrdField_max',
           'tenGageEvec2', 'nrrdFLookup', 'unrrdu_diceCmd',
           'limnPolyDataInfoLast', 'nrrdResampleSamplesSet',
           'gageVecHelGradient', 'miteShadeMethodNone',
           'gagePvlFlagUnknown', 'nrrdIterContent', 'gageContextNix',
           'nrrdRangeCopy', 'miteStageOpAdd', 'tenFiberStopLast',
           'tenGradientGenerate', 'gagePerVolumeDetach', 'airBool',
           'nrrdBinaryOpSubtract', 'limnSplineNix', 'tenEvecRGB',
           'hestGlossary', 'nrrdUILoad', 'airNull',
           'nrrdField_keyvalue', 'unrrdu_2opCmd', 'airTypeOther',
           'nrrdIoStateFormatSet', 'airFLoad', 'ell_3v_angle_d',
           'ell_3v_angle_f', 'airEnumUnknown', 'nrrdFormatVTK',
           'nrrdField_space_dimension', 'limnObjectPartAdd',
           'tenEvecRGBParmCheck', 'limnSplineNrrdEvaluate',
           'limnPolyData', 'seekBiffKey', 'nrrdDefaultResampleCheap',
           'airOneLine', 'nrrdKernelForwDiff', 'miteShadeSpecNew',
           'hestCB', 'tenEstimate2MethodLast', 'tenAniso_eval2',
           'tenAniso_eval0', 'tenAniso_eval1', 'ell_4m_det_f',
           'ell_4m_det_d', 'tenEstimate2MethodPeled',
           'gageVecNCurlNormGrad', 'nrrdDefaultSpacing',
           'tenFiberTraceSet', 'nrrdApplyMulti1DLut',
           'nrrdMeasureHistoMode', 'nrrdBinaryOpNotEqual',
           'nrrdBinaryOpMultiply', 'tenInterpTypeUnknown',
           'biffMaybeAdd', 'hooverContextNix', 'nrrdSpatialResample',
           'echoSplit', 'dyeColorGetAs', 'tenDefFiberStepSize',
           'echoObjectNix', 'nrrdBiffKey', 'limnPrimitiveQuads',
           'limnPolyDataVertexNormals', 'limnLightNix',
           'unrrdu_untileCmd', 'gageVecCurl', 'nrrdInset',
           'dyeLABtoXYZ', 'nrrdBinaryOpMod', 'hestOptAdd',
           'tenGageDelNormR1', 'nrrdTypeMin', 'airStrdup',
           'echoRoughSphereNew', 'airThreadMutexLock', 'nrrdInit',
           'gageKernelStack', 'tenGradientIdealEdge',
           'nrrdIoStateEncodingGet', 'tenFiberMultiTrace',
           'dyeSpaceRGB', 'tenTripleCalcSingle_f', 'miteThread_t',
           'limnBiffKey', 'tenTripleCalcSingle_d', 'echoJitterJitter',
           'limnObjectVertexAdd', 'nrrdBinaryOpLT',
           'airStrtokQuoting', 'tenInterpN_d',
           'limnObjectFaceNumPreSet', 'gageSclMedian', 'miteQueryAdd',
           'airTypeSize', 'nrrdBinaryOpMax', 'airStrlen',
           'echoMatterUnknown', 'unrrdu_saveCmd', 'gageSclGradVec',
           'tenLog', 'nrrdAlloc_nva', 'nrrdSpaceDimension',
           'miteDefNormalSide', 'unrrdu_axmergeCmd',
           'nrrdField_labels', 'nrrdFStore', 'biffAdd', 'nrrdKind',
           'nrrdValCompare', 'gagePvlFlagQuery', 'tenDWMRISkipKeyFmt',
           'nrrdIterSetValue', 'limnCameraPathMake',
           'nrrdHasNonExistUnknown', 'miteRenderEnd', 'miteRender',
           'limnObjectWorldHomog', 'tenGradientParmNix',
           'nrrdBinaryOpEqual', 'limnQN10checker', 'miteUserNew',
           'tenFiberStopAnisoSet', 'tenEvqVolume', 'tenAnisoTen_d',
           'tenAnisoTen_f', 'echoMatterPhongSet', 'echoMatterMetal',
           'nrrdTernaryOpMultiply', 'tenFiberMultiNix',
           'nrrdBinaryOpAdd', 'size_t', 'nrrdTernaryOpExists',
           'limnObjectCubeAdd', 'nrrdKindVector', 'echoRTParm',
           'limnLightAmbientSet', 'gageShapeSet',
           'tenDefFiberUseIndexSpace', 'nrrdBlind8BitRangeFalse',
           'tenGageDetGradVec', 'airFP_NEG_INF', 'hestOptFree',
           'tenGageModeHessianEval', 'echoJitterGrid',
           'nrrdIoStateBzip2BlockSize', 'hooverErrRenderBegin',
           'airFPPartsToVal_d', 'airFPPartsToVal_f', 'hestVerbosity',
           'nrrdDefaultWriteEncodingType', 'limnLight',
           'tenAnisoVolume', 'echoMatterGlassKa',
           'nrrdKernelDiscreteGaussian', 'echoMatterGlassKd',
           'NrrdRange', 'limnObjectDescribe', 'nrrdIoStateSet',
           'tenFiberContextCopy', 'tend_helixCmd',
           'echoTypeIsosurface', 'tenEstimate1MethodUnknown',
           'echoMatterLightPower', 'nrrdSpaceRightAnteriorSuperior',
           'gageVecCurlNormGrad', 'gageVecNormHelicity',
           'nrrdContentSet_va', 'limnLightSwitch',
           'nrrdEncodingAscii', 'hooverErrRenderEnd',
           'echoRectangleSet', 'tenDoubleContract_d', 'ell_q_avgN_d',
           'airThreadCondNix', 'unrrdu_quantizeCmd', 'airInsane_not',
           'tenExp', 'miteValVdefT', 'nrrdField_line_skip',
           'tenFiberParmSet', 'airTypeChar', 'airRandInt_r',
           'nrrdResampleKernelSet', 'tenGageRNormal',
           'limnPolyDataReadLMPD', 'nrrdDefaultWriteCharsPerLine',
           'nrrdMaybeAlloc_va', 'nrrdKindQuaternion',
           'nrrdNonSpatialAxesGet', 'tenDwiGageTensorErrorLog',
           'limnPolyDataInfo', 'airFP_QNAN', 'nrrdProject',
           'nrrdField_endian', 'nrrdKindCovariantVector',
           'ell_4v_print_d', 'ell_4v_print_f', 'nrrdEncodingTypeGzip',
           'gageParm', 'tenGageOmegaHessian', 'unrrdu_lut2Cmd',
           'gageKindCheck', 'limnWindowNix',
           'nrrdEnvVarDefaultCenter', 'nrrdEncodingTypeRaw',
           'nrrdSpaceDimensionSet', 'nrrdFormatEPS',
           'gageCtxFlagKernel', 'tend_evaladdCmd',
           'limnObjectDepthSortParts', 'nrrdKindSize',
           'limnPrimitiveTriangleStrip', 'airThreadMutexNix',
           'tenGradientCheck', 'tenSimulateSingle_f',
           'tenGageFATotalCurv', 'airRandInt', 'miteRayEnd',
           'airFloatNegInf', 'nrrdKernelSpecSprint', 'tenGageClpmin2',
           'limnVtoQN_f', 'limnVtoQN_d', 'miteRangeSP',
           'airThreadCondSignal', 'airEndianBig', 'tenGageClpmin1',
           'hestOptCheck', 'hooverErr', 'hooverRayEnd_t',
           'echoTriMeshSet', 'nrrdIterSetOwnNrrd', 'limnEnvMapFill',
           'dyeSpaceLAB', 'nrrdIoStateNew', 'tenTripleType',
           'unrrdu_insetCmd', 'nrrdSample_nva', 'airTypeSize_t',
           'tenGradientRandom', 'tenAniso_VF', 'seekDataSet',
           'unrrdu_histaxCmd', 'tenInterpParm', 'gageBiffKey',
           'gageSclCurvDir2', 'tenGageRotTans', 'gageSclCurvDir1',
           'nrrdSpaceLeftPosteriorSuperior', 'nrrdHasNonExistTrue',
           'gageProbeSpace', 'tenDwiGageKindNix', 'limnSplineInfo',
           'gagePerVolumeNix', 'nrrdIoStateCharsPerLine',
           'NrrdEncoding_t', 'tenGageCa2', 'nrrdCCValid',
           'ell_4m_pre_mul_f', 'dyeBiffKey', 'miteVal',
           'nrrdAxisInfoSpacingSet', 'tenGageDet',
           'unrrdu_unorientCmd', 'tenAniso_Ct1', 'tenGageModeGradVec',
           'tenAniso_Ct2', 'limnEdgeTypeBackCrease',
           'hooverDefImgCentering', 'hooverErrThreadJoin', 'airFree',
           'unrrdu_jhistoCmd', 'hestRespFileEnable', 'nrrdSpaceSet',
           'limnSplineEvaluate', 'hooverStubRenderBegin',
           'biffSetStrDone', 'gageKernel10', 'gageKernel11',
           'tenFiberTypeTensorLine', 'airFPFprintf_f',
           'airFPFprintf_d', 'limnSpaceUnknown', 'tenAniso_Mode',
           'gageQueryItemOn', 'nrrdILoad', 'tenTensorCheck',
           'gageVecVector', 'tenExpSingle_d', 'airMopPrint',
           'tenExpSingle_f', 'Nrrd', 'tenInterpDistanceTwo_d',
           'ell_4m_to_aa_f', 'ell_4m_to_aa_d', 'tenPowSingle_f',
           'tenPowSingle_d', 'airFP_Last', 'limnPolyDataColorSet',
           'tenGageFAFlowlineCurv', 'echoRTRenderCheck',
           'nrrdAxisInfoMin', 'echoJittableMotionA',
           'echoJittableMotionB', 'echoInstanceSet',
           'gageStructureTensor', 'nrrdOrientationReduce',
           'tenGageRGradMag', 'ell_aa_to_4m_f',
           'tenDwiGage2TensorQSegAndError', 'ell_aa_to_4m_d',
           'tenGageB', 'nrrdPad_nva', 'nrrdKeyValueCopy', 'tenGageS',
           'tenGageR', 'tenGageQ', 'tenGageTheta', 'nrrdSave',
           'gageSclGaussCurv', 'limnObjectFaceAdd',
           'nrrdUnaryOpFloor', 'gageErrLast', 'tenAniso',
           'nrrdBasicInfoUnknown', 'tenEigenvaluePower',
           'tenSizeNormalize', 'tenEMBimodal',
           'tenFiberStopDoubleSet', 'gageParmRequireEqualCenters',
           'nrrdIInsert', 'airInsane_pInfExists',
           'limnObjectWriteOFF', 'tenDwiFiberType1Evec0',
           'airLogBesselI0', 'gageDefGradMagCurvMin', 'airMop',
           'limnPolyDataSuperquadric', 'nrrdTernaryOpUnknown',
           'nrrdHistoJoint', 'airInsane_QNaNHiBit',
           'airArrayStructCB', 'nrrdField_dimension',
           'nrrdBoundaryLast', 'miteStageOpMultiply', 'tenGageKind',
           'gageVec', 'nrrdEnvVarDefaultWriteBareTextOld',
           'nrrdTypeLast', 'nrrdStateUnknownContent', 'gageQuerySet',
           'tenDwiFiberType12BlendEvec0', 'echoMatterGlassIndex',
           'tenSimulate', 'nrrdEnvVarStateDisableContent',
           'gageVecDivGradient', 'nrrdKind3DSymMatrix',
           'nrrdBasicInfoSpaceDimension', 'nrrdAxisInfoIdx',
           'gageStackBlur', 'nrrd1DIrregMapCheck', 'echoRTParmNix',
           'echoRay', 'echoMatterGlassSet', 'unrrdu_cropCmd',
           'limnCameraAspectSet', 'tenInvariantGradientsR_d',
           'nrrdRangeNew', 'nrrdDefaultResampleType', 'ell_q_mul_d',
           'ell_q_mul_f', 'nrrdEncodingRaw', 'airInsane_NaNExists',
           'limnHestPolyDataOFF', 'nrrdQuantize',
           'nrrdSpacingStatusDirection', 'tenEstimateSigmaSet',
           'nrrdKernelHermiteFlag', 'tenTripleConvertSingle_f',
           'nrrdCopy', 'limnEdgeTypeLone', 'unrrdu_heqCmd',
           'airStrntok', 'nrrdTernaryOpInOpen', 'nrrdPGM',
           'gagePointReset', 'echoTypeAABBox',
           'nrrdZlibStrategyUnknown', 'tenGageFADiffusionAlign',
           'echoMatterLightSet', 'limnPrimitiveTriangles',
           'nrrdTernaryOpClamp', 'echoObjectHasMatter',
           'ell_3m_to_aa_f', 'ell_3m_to_aa_d', 'gageVecCurlGradient',
           'airMopMem', 'gageDefRenormalize',
           'limnObjectPolarSuperquadAdd', 'tenGageCp2', 'tenGageCp1',
           'limnObjectNew', 'nrrdBasicInfoLast', 'tenInterpParmNix',
           'tenAniso_Clpmin1', 'tenAniso_Clpmin2', 'echoCube',
           'echoMatterMetalR0', 'limnEdge_t',
           'limnEdgeTypeFrontFacet', 'tenFiberIntgUnknown',
           'limnObjectDepthSortFaces', 'tenGageOmegaNormal',
           'ell_6ms_eigensolve_d', 'gageTeeOfTau',
           'tenGageTraceDiffusionFraction', 'airThreadCondBroadcast',
           'nrrdBinaryOp', 'tenDwiGage2TensorPeledAndError',
           'nrrdMeasureMin', 'nrrdNuke', 'nrrdTypeIsIntegral',
           'airNoDio_disable', 'limnSplineTypeHermite',
           'tenEvecRGBSingle_f', 'tenDwiGagePvlData',
           'limnSpaceScreen', 'airThreadBarrierWait',
           'nrrdClampConvert', 'tenExpand', 'ell_3m_mul_f',
           'ell_3m_mul_d', 'hooverRayBegin_t', 'tenSlice',
           'airIndexULL', 'airEqvAdd', 'miteValView',
           'nrrdReshape_va', 'nrrdKindComplex', 'limnQN13octa',
           'tenInterpParmCopy', 'limnQN14octa', 'tenGageCp1GradVec',
           'echoTriangleSet', 'gageKernel00', 'nrrdEmpty',
           'limnSplineTypeTimeWarp', 'hestMinNumArgs',
           'limnObjectVertexNormals', 'tenTripleTypeEigenvalue',
           'nrrdKind4Vector', 'tenAnisoEval_f', 'tenAnisoEval_d',
           'tenDWMRIBValueKey', 'hooverErrInit', 'tend_avgCmd',
           'nrrdUnquantize', 'gageErr', 'tenGageOmegaHessianEval',
           'echoTypeTriangle', 'airFclose',
           'nrrdEnvVarDefaultWriteBareText',
           'limnPolyDataSpiralSphere', 'tenGageModeHessianEvec2',
           'tenGageModeHessianEvec1', 'tenGageModeHessianEvec0',
           'airFP_POS_INF', 'echoBoundsGet', 'gageShapeNew',
           '_airThread', 'nrrdKind3DMatrix', 'nrrdBasicInfoContent',
           'tenFiberStopAniso', 'tendUsage', 'gageShapeWtoI',
           'nrrdHestKernelSpec', 'nrrdMeasureLinf', 'limnPart',
           'gagePerVolume', 'tenGageCl1GradVec', 'seekUpdate',
           'tenGradientJitter', 'nrrdAxisInfoCopy',
           'gageParmGradMagCurvMin', 'miteDefOpacNear1',
           'nrrdEncodingTypeUnknown', 'tenTripleCalc', 'ell_debug',
           'tenDwiGageTensorNLSLikelihood',
           'limnSplineTypeCubicBezier', 'nrrdUnaryOpRand',
           'airArrayLenIncr', 'tenEstimate1TensorSingle_d',
           'tenEstimate1TensorSingle_f', 'ell_4mv_mul_f',
           'ell_4mv_mul_d', 'tenDefFiberAnisoStopType', 'nrrdSlice',
           'tenFiberMultiNew', 'nrrdKernelBlackmanDD',
           'echoTextureLookup', 'tenGageOmegaGradVecDotEvec0',
           'limnDefCameraRightHanded', 'nrrdKernelAQuarticD',
           'nrrdBoundaryWeight', 'tenGlyphParmCheck', 'ell_4m_inv_d',
           'nrrdSpaceVecScaleAdd2', 'nrrdFClamp',
           'nrrdBlind8BitRangeLast', 'nrrdField_centers', 'airLog2',
           'ell_q_inv_f', 'ell_q_inv_d', 'miteRangeGreen',
           'airFP_NEG_DENORM', 'nrrdAxisInfoGet_va', 'limnEnvMapCB',
           'tenGageOmegaGradVec', 'airFP_NEG_NORM', 'airToUpper',
           'tenEigenvalueClamp', 'nrrdApply1DIrregMap',
           'airFloatSNaN', 'nrrdKernelC3QuinticDD', 'airIntPow',
           'nrrdStateGrayscaleImage3D', 'hooverRenderEnd_t',
           'tenFiberMulti', 'ell_Nm_tran', 'nrrdUnaryOpLog10',
           'tenFiberMultiPolyData', 'miteValNdotL',
           'gageParmKernelIntegralNearZero', 'nrrdFormatTypePNM',
           'airErf', 'limnPolyDataWriteIV', 'miteValNdotV',
           'ell_3m_det_f', 'ell_3m_det_d', 'ell_q_4v_rotate_d',
           'ell_q_4v_rotate_f', 'gagePerVolumeIsAttached',
           'airMopOkay', 'limnCamera', 'echoTypeList',
           'nrrdMeasureLast', 'nrrdUnaryOpLog1p', 'airInsane_endian',
           'tenEMBimodalParmNew', 'tenFiberParmLast',
           'nrrdBasicInfoInit', 'nrrdIterSetNrrd',
           'tenGageFAHessianEvec2', 'tenGageFAHessianEvec1',
           'tenGageFAHessianEvec0', 'ell_4m_post_mul_d',
           'echoIntxMaterialColor', 'gageSclHessEval',
           'airThreadNoopWarning', 'limnDevicePS',
           'nrrdBasicInfoSpaceUnits', 'gageStackProbe',
           'nrrdBinaryOpAtan2', 'unrrdu_cmedianCmd', 'tenAniso_Tr',
           'biffMove', 'hooverRenderBegin_t', 'nrrdHestNrrd',
           'echoSceneNew', 'nrrdEnvVarStateAlwaysSetContent',
           'limnWindowNew', 'miteRangeChar', 'tenAniso_Th',
           'ell_3m_rotate_between_d', 'gageTauOfTee', 'nrrdTypeBlock',
           'nrrdSpatialAxesGet', 'seekItemGradientSet',
           'tenGageTensor', 'nrrdSpaceScannerXYZ',
           'airBesselInExpScaled', 'nrrdEncodingGzip',
           'airTeemVersion', 'airThreadMutex', 'unrrduUsage',
           'nrrdDistanceL2Signed', 'tenGageTensorQuatGeoLoxR',
           'tenGageInvarKGrads', 'tenSizeScale',
           'tenGageTensorQuatGeoLoxK', 'gageStackPerVolumeAttach',
           'biffGetStrlen', 'gageDefCheckIntegrals',
           'nrrdBinaryOpCompare', 'airNoDio_okay', 'airTypeUnknown',
           'tenAnisoHistogram', 'tenGageFA', 'nrrdSpaceVecNorm',
           'biffGet', 'tenDwiGageJustDWI', 'tenDwiGageTensorLLSError',
           'nrrdAxisInfoLabel', 'tenDwiGage', 'nrrdBinaryOpFmod',
           'nrrdKind3Vector', 'airStrtrans', 'tenBMatrixCheck',
           'nrrdTypeChar', 'nrrdCCRevalue', 'echoObject',
           'nrrdSpace3DLeftHandedTime', 'gageSclNormal',
           'unrrdu_axdeleteCmd', 'nrrdBasicInfoCopy', 'dyeConverter',
           'nrrdCrop', 'nrrdUnaryOpAbs', 'tenGageCovariance',
           'echoCylinderSet', 'nrrdElementNumber', 'tenEvecRGBParm',
           'hestMultiFlagSep', 'limnObjectPSDraw', 'nrrdJoin',
           'airInsane_FISize', 'hestParm', 'nrrdKeyValueIndex',
           'tenDefFiberKernel', 'seekType',
           'nrrdSpacingStatusUnknown', 'limnSplineTypeSpec',
           'airEnumStr', 'airArrayNuke', 'airDioTest', 'miteValVrefN',
           'tenFiberTrace', 'limnEdgeTypeFrontCrease', 'miteSample',
           'nrrdField_data_file', 'tenFiberIntgMidpoint',
           'limnWindow', 'nrrdField_old_min', 'ell_q_avg4_d',
           'nrrdTypeFloat', 'limnSplineNew', 'echoRTParmNew',
           'airEqvMap', 'seekTypeLast', 'gageKind_t',
           'gageDefKernelIntegralNearZero', 'nrrdEncodingTypeHex',
           'tenGageEvec0', 'tenGageEvec1', 'nrrdIoStateValsPerLine',
           'unrrdu_tileCmd', 'nrrdKind2DMaskedSymMatrix',
           'airMopOnOkay', 'limnPolyDataPrimitiveSort',
           'tenGageTraceHessian', 'nrrdField_space_directions',
           'gageContext', 'nrrdHasNonExist', 'tenGageOmegaGradMag',
           'airThreadMutexUnlock', 'tenFiberTypeUnknown',
           'nrrdMeasureHistoProduct', 'tenTripleConvertSingle_d',
           'nrrdUnaryOpIf', 'tenGageUnknown',
           'nrrdBasicInfoBlocksize', 'unrrdu_distCmd',
           'miteValUnknown', 'nrrdFormatNRRD', 'tenGageNormNormal',
           'nrrdKindRGBColor', 'airSingleSscanf', 'airThreadCondWait',
           'limnSplineTypeSpecNix', 'gageVecMGEval',
           'tenFiberIntgSet', 'nrrdField_unknown', 'nrrdCCNum',
           'gageParmRequireAllSpacings', 'tenGageOmega',
           'tend_logCmd', 'limnObjectNix', 'tenGageDetHessian',
           'tenGageBGradVec', 'gageErrNone', 'limnSplineMaxT',
           'seekStrengthUseSet', 'gageErrStackIntegral',
           'nrrdRangeNewSet', 'miteShadeSpecQueryAdd',
           'nrrdEnvVarStateKindNoop', 'tenGageInvarRGradMags',
           'nrrdSpaceVecSetNaN', 'limnCameraInit', 'tend_sliceCmd',
           'tenFiberTypePureLine', 'nrrdKindScalar',
           'tenFiberStopNumSteps', 'airTypeInt',
           'tenFiberAnisoSpeedSet', 'tenGlyphTypeSphere',
           'gageDefK3Pack', 'hestOpt', 'nrrdMeasureLineError',
           'nrrdKernelBCCubicDD', 'limnOptsPS',
           'nrrdSpacingStatusLast', 'tenFiberStopBounds',
           'limnSplineInfo2Vector', 'tenDwiGageAll',
           'nrrdKernelSpecNew', 'nrrdRangeSet', 'limnEdgeTypeContour',
           'ell_4m_inv_f', 'nrrdField_space_origin', 'dyeXYZtoLAB',
           'nrrdFormatTypeNRRD', 'echoLightPosition', 'tenAnisoScale',
           'echoScene_t', 'nrrdResampleClampSet', 'gageVecGradient2',
           'gageVecGradient0', 'gageVecGradient1', 'limnDeviceGL',
           'tenInterpTypeLast', 'tenGageFAHessianEval2',
           'tenGageFAHessianEval0', 'tenGageFAHessianEval1',
           'nrrdSpaceLast', 'tend_bmatCmd', 'nrrdAxisInfoThickness',
           'airEndian', 'nrrdIoStateBareText', 'ell_Nm_inv',
           'hestElideSingleEnumType', 'ell_q_log_d', 'ell_q_log_f',
           'airInsaneErr', 'tenGageCovarianceRGRT', 'gageKernel21',
           'gageKernel20', 'gageKernel22', 'echoMatterPhong',
           'tenGageSGradMag', 'tenGageEvalGrads', 'limnLightNew',
           'miteRangeEmissivity', 'nrrdAxisInfoPosRange',
           'gageItemEntry', 'echoJitter', 'nrrdFormatPNM',
           'nrrdReshape_nva', 'tenGlyphTypeUnknown',
           'airMyFmt_size_t', 'gageVecMGFrob',
           'tenGageModeHessianEvec', 'nrrdMeasureHistoMin',
           'nrrdUnaryOpRoundUp', 'tenFiberTypeLast', 'nrrdILookup',
           'gageParmCheckIntegrals', 'nrrdUnaryOpCos',
           'tenDwiGageTensorNLSError', 'ell_6m_mul_d', 'dyeRGBtoHSL',
           'limnPolyDataPrimitiveVertexNumber', 'nrrdField_content',
           'gageKernelUnknown', 'nrrdKindSpace', 'nrrdKindRGBAColor',
           'gageDefStackUse', 'nrrdEncodingArray', 'hestUsage',
           'nrrdHasNonExistLast', 'dyeRGBtoHSV', 'limnEnvMapCheck',
           'airEnumValCheck', 'echoEnvmapLookup', 'airThread',
           'tenGageMode', 'gageUpdate', 'nrrdSpacingCalculate',
           'miteDefRenorm', 'nrrdKernelHannD', 'seekTypeSet',
           'airThreadCondNew', 'nrrdStateGetenv', 'tenInterpType',
           'airEndianUnknown', 'nrrdMeasureMean', 'miteValWdotD',
           'tenGlyphGen', 'miteShadeSpecParse', 'airTypeBool',
           'tenDwiGage2TensorQSeg', 'nrrdStateKeyValuePairsPropagate',
           'gageShapeEqual', 'echoJittableLens',
           'nrrdStateMeasureHistoType', 'tenGlyphTypeLast',
           'limnSplineInfoQuaternion', 'tenGageTraceGradMag',
           'NrrdKernel', 'nrrdField_encoding', 'nrrdKernelGaussian',
           'echoScene', 'ell_Nm_pseudo_inv',
           'nrrdIoStateZlibStrategy', 'nrrdField_thicknesses',
           'echoTypeLast', 'gageStackPerVolumeNew', 'gageKernelSet',
           'nrrdKindList', 'limnCamera_t', 'echoSphereSet',
           'echoGlobalState', 'gageItemSpecNix', 'echoTypeSplit',
           'nrrdArithIterTernaryOp', 'tenDwiGageFA',
           'tenEstimateGradientsSet', 'nrrdUnaryOpZero',
           'airFloatPosInf', 'nrrdCommentAdd', 'limnSplineType',
           'nrrdCCSize', 'echoBiffKey', 'tenGageOmegaHessianEvec2',
           'tenGageOmegaHessianEvec0', 'tenGageOmegaHessianEvec1',
           'nrrdBoundary', 'nrrdAxesPermute', 'tenFiberParmWPunct',
           'limnCameraPathTrackBoth', 'echoMatterMetalSet',
           'limnSplineTypeLinear', 'gageParmCurvNormalSide',
           'gageSclHessMode', 'tenGageFAGradMag',
           'limnSplineInfoNormal', 'tenGageFAValleySurfaceStrength',
           'airFopen', 'airEnumLast', 'tenFiberAnisoSpeedReset',
           'nrrdKernelParse', 'tenAniso_Skew', 'nrrdMeasureHistoMean',
           'limnQN16border1', 'nrrdCenterLast', 'nrrdAxisInfoUnits',
           'miteRangeKa', 'tenFiberSingleDone', 'miteRangeKd',
           'nrrdMeasureMedian', 'nrrdMinMaxExactFind', 'ell_q_div_f',
           'ell_q_div_d', 'nrrdPad_va', 'nrrdAxisInfoSize',
           'miteRangeKs', 'nrrdStateAlwaysSetContent',
           'tenGageFAGaussCurv', 'NrrdFormat', 'nrrdKernelCheap',
           'tenDwiGage2TensorPeledError', 'airInsane_nInfExists',
           'nrrdTernaryOpMax', 'tenGageLast', 'gageScl3PFilter4',
           'tenGlyphParmNew', 'dyeColorNix', 'tenFiberTypeZhukov',
           'tenGageOmegaHessianEval1', 'tenDwiGageKindNew',
           'gageQueryPrint', 'tenEstimateVerboseSet', 'airSgn',
           'gageCtxFlagUnknown', 'echoJittableUnknown',
           'nrrdResampleRoundSet', 'nrrdKernelBCCubicD',
           'nrrdBasicInfoDimension', 'limnPolyDataCCFind',
           'airTeemReleaseDate', 'limnObjectFaceNormals',
           'nrrdKernelTMF_maxC', 'nrrdIoStateDetachedHeader',
           'tend_expandCmd', 'tenEstimate2MethodQSegLLS',
           'unrrduHestMaybeTypeCB', 'tenEvecRGBSingle_d',
           'dyeColorInit', 'tenDwiGageKindCheck', 'airMopError',
           'echoTriangle', 'limnPolyDataRasterize',
           'hestElideMultipleEmptyStringDefault', 'miteValRi',
           'echoPos_t', 'hestVarParamStopFlag', 'seekTypeValleyLine',
           'nrrdUIStore', 'miteRangeBlue', 'limnPolyDataInfoRGBA',
           'miteValRw', 'tend_stenCmd', 'limnEdgeTypeUnknown',
           'nrrdBasicInfoOldMax', 'unrrdu_shuffleCmd',
           'echoJittableLight', 'tenInterpMulti3D', 'seekIsovalueSet',
           'airMopDebug', 'nrrdEncodingType', 'tend_aboutCmd',
           'limnQN11octa', 'ell_q_exp_f', 'tenDwiGageTensorMLE',
           'airILoad', 'ell_q_to_aa_d', 'gagePoint_t',
           'ell_q_to_aa_f', 'unrrdu_padCmd', 'airMopOnError',
           'nrrdDefaultKernelParm0', 'tend_pointCmd',
           'echoTypeUnknown', 'airFPClass_f', 'airFPClass_d',
           'tenDwiGageMeanDWIValue', 'nrrdGetenvEnum',
           'nrrdUnaryOpSqrt', 'airFP_POS_ZERO', 'nrrdIoStateGet',
           'tenGageHessian', 'echoMatter', 'nrrdUnaryOpOne',
           'gageCtxFlagRadius', 'tenGageSGradVec',
           'tenInterpTypeLogLinear', 'tenTripleTypeWheelParm',
           'airMopNever', 'miteShadeSpec', 'gageKindAnswerOffset',
           'nrrdNew', 'nrrdEncodingTypeBzip2',
           'nrrdField_sample_units', 'hooverErrRayEnd',
           'tend_anplotCmd', 'gageShapeItoW', 'nrrdAxesSplit',
           'nrrdSameSize', 'tenEstimateContext', 'unrrdu_swapCmd',
           'seekItemEigensystemSet', 'airDioMalloc',
           'limnPrimitiveLineStrip', 'airThreadJoin',
           'tenDefFiberWPunct', 'nrrdTypeUShort',
           'hooverContextCheck', 'tenFiberStopSet',
           'echoMatterPhongKs', 'airMopUnMem', 'nrrdUnaryOpAsin',
           'tenGageInvarRGrads', 'echoMatterPhongKd', 'nrrdUnblock',
           'echoMatterPhongKa', 'tenFiberStopRadius',
           'limnPrimitiveUnknown', 'nrrdIterNew', 'airNoDio_std',
           'airFloat', 'tenFiberStopConfidence', 'airOneLinify',
           'tenGageModeHessianEval2', 'tenGageModeHessianEval0',
           'tenGageModeHessianEval1', 'gageKernelLast',
           'gageSclTotalCurv', 'nrrdUnaryOpErf', 'airGaussian',
           'nrrdResampleBoundarySet', 'nrrdBasicInfoType',
           'tenEigenvalueAdd', 'hooverErrLast', 'nrrdKernelC4HexicDD',
           'nrrdTile2D', 'airIndex', 'tenFiberStopStub',
           'limnCameraUpdate', 'dyeSpaceLast', 'airDrandMT_r',
           'airMode3_d', 'miteRangeAlpha', 'limnPolyDataSave',
           'nrrdStateVerboseIO', 'nrrdDefaultResamplePadValue',
           'nrrd1DIrregAclGenerate', 'tenGageModeNormal',
           'ell_3v_perp_d', 'ell_3v_perp_f', 'nrrdMeasureHistoL2',
           'airRandMTStateGlobal', 'airLLong', 'tenGageConfidence',
           'seekItemScalarSet', 'hooverStubThreadEnd', 'tenGageEvec',
           'tenDwiGageTensorLLS', 'limnSplineParse',
           'seekItemNormalSet', 'limnSpaceLast', 'miteRenderBegin',
           'airMyEndian', 'miteRangeRed', 'gageVecVector2',
           'echoObjectAdd', 'gageVecVector0', 'tenGageThetaNormal',
           'seekItemStrengthSet', 'nrrdSimplePad_nva',
           'gageSclHessEvec', 'airThreadStart', 'tenFiberSingle',
           'ell_3m_to_q_d', 'ell_3m_to_q_f', 'airFP_POS_DENORM',
           'nrrdAxesInsert', 'airThreadBarrierNew', 'gageSclHessian',
           'tenFiberUpdate', 'limnSpline', 'limnDeviceLast',
           'nrrdCCMerge', 'seekContext', 'nrrdAxisInfoSpaceDirection',
           'tenAniso_Det', 'tenGageFA2ndDD', 'tenDWMRIBmatKeyFmt',
           'tenGageCl1GradMag', 'tenFiberContextNew',
           '__darwin_size_t', 'gageParmK3Pack', 'tenGageBNormal',
           'tenEpiRegister3D', 'echoSphere', 'airEnumFmtDesc',
           'echoJittablePixel', 'ell_3m_eigensolve_d',
           'echoIntxLightColor', 'nrrdIoStateKeepNrrdDataFileOpen',
           'airIStore', 'seekExtract', 'nrrdField_last',
           'nrrdHestIter', 'gageVecImaginaryPart',
           'hooverErrThreadEnd', 'tenEstimateValueMinSet',
           'limnCameraPathTrackLast', 'echoJitterNone',
           'airRandMTStateNew', 'nrrdEnvVarDefaultCenterOld',
           'tenEstimateUpdate', 'nrrdLineSkip', 'nrrdField_kinds',
           'gageVecJacobian', 'nrrdBlind8BitRangeState',
           'tenEstimate1Method', 'airFP_NEG_ZERO', 'echoIntxColor',
           'tenFiberParmUnknown', 'unrrdu_aboutCmd',
           'unrrduDefNumColumns', 'airEndianLittle', 'nrrdKind4Color',
           'limnSpaceWorld', 'seekTypeUnknown', 'nrrdCommentClear',
           'limnPolyDataSpiralTubeWrap', 'tenGageRGradVec',
           'nrrdHistoCheck', 'tenDwiFiberTypeUnknown',
           'tenGageOmegaHessianEvec', 'seekContextNix',
           'nrrdTernaryOpAdd', 'ell_cubic_root_triple',
           'nrrdKernelSpecNix', 'nrrdDefaultResampleBoundary',
           'nrrdAxesDelete', 'airIsNaN', 'nrrdKernelSpecSet',
           'limnPolyDataInfoUnknown', 'echoJitterRandom',
           'gageItemSpecInit', 'tend_makeCmd', 'gagePvlFlagLast',
           'seekVerboseSet', 'airMopper', 'nrrdApply1DLut',
           'tenGradientMeasure', 'nrrdKindStub', 'nrrdTypeSize',
           'tenDwiGageTensorLLSLikelihood', 'nrrdBlock',
           'airTypeString', 'nrrdKernelC4HexicD', 'miteStageOpMin',
           'nrrdFormatTypeUnknown', 'dyeColorSprintf', 'tend_bfitCmd',
           'nrrdSpacingStatusNone', 'gageShape_t', 'limnPolyDataCube',
           'limnDefCameraAtRelative', 'ell_cubic_root_three',
           'tenGageCa1GradMag', 'limnPolyDataNew',
           'nrrdDefaultResampleRound', 'limnSplineTypeSpec_t',
           'gageSclHessEvec0', 'gageSclHessEvec1', 'gageSclHessEvec2',
           'hestElideSingleNonExistFloatDefault',
           'tenFiberStopLength', 'ell_4m_print_f',
           'limnSplineInfo3Vector', 'ell_4m_print_d',
           'tenDefFiberMaxNumSteps', 'limnLightDiffuseCB',
           'nrrdMeasureHistoSum', 'tenGageFARidgeSurfaceStrength',
           'nrrdField_axis_mins', 'tenDwiGageADC',
           'tenGradientBalance', 'nrrdOriginCalculate',
           'tenFiberContextNix', 'seekTypeRidgeLine',
           'nrrdGetenvBool', 'gageVecLength', 'limnSpace',
           'nrrdIoStateFormatGet', 'tenFiberDirectionNumber',
           'gageParmStackRenormalize', 'nrrdTypePrintfStr',
           'nrrdField_sizes', 'nrrdAxisInfoGet_nva',
           'nrrdSimplePad_va', 'tenDwiGageTensorLikelihood',
           'tenGageThetaGradMag', 'nrrdKindLast', 'gageParm_t',
           'nrrdIoStateLast', 'tenEvecRGBParmNew', 'limnFace_t',
           'nrrdField_axis_maxs', 'tenGageRotTanMags',
           'tenGageBHessian', 'limnObjectPSDrawConcave',
           'limnObjectPolarSphereAdd', 'tenGageCovarianceKGRT',
           'tenGageOmega2ndDD', 'nrrdDefaultResampleClamp',
           'hestElideSingleEmptyStringDefault',
           'tenDwiGage2TensorQSegError', 'gageScl2ndDD',
           'unrrdu_unquantizeCmd', 'airTypeDouble', 'tend_evalpowCmd',
           'gageParmDefaultSpacing', 'unrrdu_permuteCmd',
           'limnDeviceUnknown', 'nrrdUnaryOpAtan',
           'tenGageDelNormPhi3', 'tenGageDelNormPhi2',
           'tenGageDelNormPhi1', 'airRandMTState', 'NrrdKernelSpec',
           'nrrdEnvVarStateMeasureType', 'tenGageTensorGradRotE',
           'nrrdInvertPerm', 'nrrdTernaryOp', 'nrrdUnaryOpSgn',
           'hestElideMultipleNonExistFloatDefault',
           'gageCtxFlagK3Pack', 'dyeColorSet', 'nrrdKind2DMatrix',
           'limnPolyDataAlloc', 'hestGreedySingleString',
           'tenDwiGageTensorWLSErrorLog', 'nrrdHistoDraw',
           'nrrdGetenvDouble', 'nrrdSample_va',
           'echoMatterTextureSet', 'limnQNtoV_f', 'limnQNtoV_d',
           'nrrdApply1DSubstitution', 'echoSceneNix',
           'nrrdZlibStrategyFiltered', 'airMy32Bit',
           'nrrdSpaceVecCopy', 'unrrdu_ccfindCmd', 'nrrdKindHSVColor',
           'nrrdFormatTypeVTK', 'nrrdStateBlind8BitRange',
           'tend_evecrgbCmd', 'tenGageThetaGradVec',
           'tenGageDetGradMag', 'echoIntx', 'gageCtxFlagShape',
           'gageVecDivergence', 'limnCameraPathTrackAt',
           'limnPolyDataCopyN', 'nrrdKernelAQuartic',
           'nrrdSpace3DRightHanded', 'nrrdUnaryOpSin', 'echoType',
           'tenAniso_R', 'nrrdKindXYZColor', 'echoTypeCylinder',
           'nrrdAxisInfoMinMaxSet', 'gageVolumeCheck',
           'gageVecDirHelDeriv', 'nrrdStringWrite', 'tenAniso_B',
           'limnLightUpdate', 'nrrdOriginStatusOkay', 'tenAniso_Q',
           'ell_Nm_mul', 'tenAniso_S', 'limnQN16simple',
           'dyeColorGet', 'nrrdMeasureLine', 'tenGageFAMeanCurv',
           'ell_4m_mul_f', 'ell_4m_mul_d', 'tenDwiGageTensorWLSError',
           'miteRangeLast', 'tenTripleTypeLast', 'nrrdSwapEndian',
           'gageErrBoundsStack', 'tenEstimate1TensorSimulateVolume',
           'unrrdu_resampleCmd', 'limnPolyDataInfoBitFlag',
           'hooverStubRayBegin', 'nrrdRangeAxesGet',
           'tenGageConfDiffusionFraction', 'nrrdKernelBCCubic',
           'airDoneStr', 'gageDefRequireAllSpacings',
           'nrrdResamplePadValueSet', 'nrrdResampleExecute',
           'airParseStrUI', 'echoMatterMetalFuzzy', 'gageKindVec',
           'echoMatterMetalKd', 'tenGageCp1GradMag',
           'echoMatterMetalKa', 'gageKindScl',
           'hestCleverPluralizeOtherY', 'nrrdCheapMedian',
           'limnSplineInfoUnknown', 'tenEstimateLinearSingle_d',
           'tenEstimateLinearSingle_f', 'nrrdSaveMulti',
           'tenLogSingle_d', 'tend_satinCmd', 'nrrdIoStateZlibLevel',
           'gageQueryAdd', 'nrrdKeyValueSize', 'tenFiberSingleNix',
           'gageVecLast', 'limnQNUnknown', 'airIsInf_f',
           'tend_anscaleCmd', 'airIsInf_d', 'tenGageTensorGradMagMag',
           'tenDWMRIGradKeyFmt', 'seekTypeIsocontour',
           'airFP_Unknown', 'nrrdBasicInfoKeyValuePairs',
           'tenTripleTypeRThetaPhi', 'gageParmUnknown',
           'miteShadeMethodUnknown', 'nrrdBinaryOpPow',
           'nrrdTernaryOpLast', 'tenGageTensorLogEuclidean',
           'nrrdKeyValueAdd', 'airParseStr', 'tenInterpTypeGeoLoxR',
           'nrrdKernelZero', 'tenGageFAGradVec',
           'tenInterpTypeGeoLoxK', 'nrrdDefaultCenter',
           'limnPolyDataWriteVTK', 'ell_3m_post_mul_f',
           'ell_3m_post_mul_d', 'limnPart_t', 'gageScl',
           'tenDWMRIKeyValueParse', 'airIndexClampULL', 'nrrdFLoad',
           'echoGlobalStateNew', 'tenEstimateThresholdSet',
           'unrrdu_joinCmd', 'limnSplineTypeSpecNew', 'gageVecMGEvec',
           'echoRectangle', 'unrrdu_ccadjCmd', 'ell_q_to_4m_f',
           'nrrdStateDisallowIntegerNonExist', 'ell_q_to_4m_d',
           'gageContextCopy', 'ell_3m_2d_nullspace_d',
           'nrrdBinaryOpLast', 'tend_evalCmd', 'dyeSpaceUnknown',
           'nrrdDomainAxesGet', 'echoMatterLightUnit',
           'tenGageDetNormal', 'airDrandMT', 'nrrdUnaryOpLog2',
           'airMode3', 'limnObjectConeAdd', 'NrrdAxisInfo',
           'tenSqrtSingle_f', 'tenGageSHessian', 'nrrdField_min',
           'dyeStrToSpace', 'nrrdShuffle', 'nrrdLoad', 'mite_t',
           'airSetNull', 'airDioInfo', 'airArray',
           'nrrdResampleContextNew', 'echoListAdd',
           'tenDefFiberAnisoThresh', 'tenGageFAHessianEval',
           'tenGlyphParmNix', 'nrrdField_spacings',
           'nrrdKernelSpecCopy', 'gageParmStackUse', 'airNoDio_fpos',
           'nrrdOriginStatusNoMaxOrSpacing', 'gageSclHessRidgeness',
           'nrrdDClamp', 'unrrdu_reshapeCmd',
           'limnPolyDataPrimitiveTypes', 'tenTripleTypeMoment',
           'nrrdIStore', 'miteRangeUnknown', 'tenGageInvarKGradMags',
           'nrrdTypeInt', 'nrrdFormatArray', 'nrrdCCMax',
           'airBesselI0ExpScaled', 'tenTripleTypeK', 'tenTripleTypeJ',
           'tenTripleTypeR', 'tenVerbose', 'nrrdIoStateUnknown',
           'nrrdKernelSpecParse', 'gageDefCurvNormalSide',
           'tenGageCa1GradVec', 'tenGageDelNormK3',
           'tenGageDelNormK2', 'limnHestSpline',
           'nrrdEnvVarStateBlind8BitRange', 'echoLightColor',
           'tenFiberSingleNew', 'dyeRGBtoXYZ', 'unrrdu_mlutCmd',
           'hestParmFree', 'tenFiberIntgEuler', 'airNoDio_test',
           'gageDefDefaultSpacing', 'nrrdGetenvUInt',
           'nrrdTernaryOpInClosed', 'tenGageFAHessianEvalMode',
           'echoTypeSphere', 'gagePerVolumeAttach', 'nrrdField_units',
           'ell_aa_to_3m_f', 'ell_aa_to_3m_d',
           'seekContour3DTopoHackTriangle', 'tenGageTraceGradVec',
           'tenAniso_Cs2', 'nrrdSimpleCrop', 'tenEstimate1MethodNLS',
           'gageKind', 'tenDwiGageTensorNLSErrorLog', 'echoList',
           'nrrdOriginStatusLast', 'gageQueryReset',
           'unrrdu_axsplitCmd', 'tend_epiregCmd',
           'seekNormalsFindSet', 'tenFiberSingleInit',
           'unrrdu_projectCmd', 'biffCheck', 'tend_simCmd',
           'tenDwiGageTensorMLEErrorLog', 'hooverRender',
           'limnSplineTypeLast', 'tenEstimate1MethodLLS',
           'nrrdEnvVarDefaultWriteCharsPerLine', 'tenInterpTwo_d',
           'hestParseFree', 'hooverStubRenderEnd',
           'tend_evalclampCmd', 'tenGageTrace', 'echoRayColor',
           'gageSclGradMag', 'airInsane_32Bit', 'nrrdUnaryOp',
           'nrrdKernelTMF', 'limnSplineTypeBC', 'tenEstimateSkipSet',
           'airInsane_dio', 'tenDwiFiberTypeLast',
           'gageScl3PFilter_t', 'echoTriMesh',
           'tenDwiGageTensorError', 'airTypeLongInt',
           'nrrdKernelParmSet', 'limnPrimitiveLast',
           'gageVecMultiGrad', 'tenGageNorm',
           'tenDwiGage2TensorPeledLevmarInfo',
           'tenGageTraceGradVecDotEvec0',
           'tenDwiGageTensorWLSLikelihood', 'tenEMatrixCalc',
           'gageSigOfTau', 'nrrdRangeReset', 'nrrdKind3Color',
           'airSrandMT', 'tenGageConfGradVecDotEvec0',
           'airNoDio_small', 'tenDwiGageTensorNLS', 'airNoDio_fd',
           'tend_tconvCmd', 'nrrdAxisInfoSet_va', 'gageVecNormalized',
           'nrrdValCompareInv', 'nrrdStringRead', 'unrrdu_rmapCmd',
           'limnPolyDataPrimitiveSelect', 'airThreadBarrier',
           'miteThreadNix', 'nrrdSpace', 'tend_anvolCmd', 'hestParse',
           'limnObjectFaceReverse', 'miteShadeSpecPrint',
           'NrrdIoState_t', 'nrrdEncodingUnknown',
           'airFPValToParts_f', 'airFPValToParts_d', 'dyeColor',
           'tenFiberStopReset', 'gageCtxFlagNeedD',
           'limnPolyDataClip', 'gageCtxFlagNeedK', 'limnQN16checker',
           'gageZeroNormal', 'gageSclHessValleyness',
           '_airThreadCond', 'gageItemSpecNew', 'airBesselI1By0',
           'nrrdBasicInfoSampleUnits', 'nrrdIoStateEncodingSet',
           'nrrdMeasureUnknown', 'ell_3m_pre_mul_d',
           'ell_3m_pre_mul_f', 'unrrduBiffKey',
           'miteShadeMethodPhong', 'tenEstimateLinear3D',
           'ell_q_to_3m_f', 'nrrdSpaceVecScale', 'ell_q_to_3m_d',
           'nrrdDLoad', 'hestInfo', 'miteStageOpUnknown', 'nrrdField',
           'nrrdEncodingHex', 'nrrdStateMeasureModeBins',
           'gagePerVolume_t', 'limnPolyDataPolarSphere',
           'tenDwiGageB0', 'nrrdApplyMulti1DRegMap', 'gageSclNPerp',
           'echoTypeSuperquad', 'nrrdKindNormal', 'miteValLast',
           'tenGageTraceNormal', 'hooverErrRayBegin', 'nrrdAxesMerge',
           'echoJitterLast', 'echoTypeTriMesh', 'hooverThreadEnd_t',
           'limnQN8checker', 'nrrdAxisInfoSet_nva',
           'nrrdEnvVarStateGrayscaleImage3D',
           'tenEstimateBMatricesSet', 'unrrdu_histoCmd',
           'dyeColorNew', 'gageParmSet', 'nrrdRead',
           'echoJitterCompute', 'airInsane_AIR_NAN',
           'nrrdBinaryOpUnknown', 'nrrdDefaultResampleRenormalize',
           'tenEvecRGBParmNix', 'nrrdUnaryOpNormalRand',
           'seekTypeValleySurface', 'airShuffle', 'dyeSpaceXYZ',
           'nrrdApply2DLut', 'tenAniso_Cl1', 'tenAniso_Cl2',
           'nrrdFormatTypePNG', 'airULLong', 'gageKernel',
           'gageSclGeomTens', 'ell_cubic_root_single_double',
           'airEnum', 'nrrdCenterCell', 'hooverErrThreadBegin',
           'nrrdUnaryOpUnknown', 'airNaN',
           'limnPolyDataVertexWindingFix', 'miteStageOp',
           'dyeSpaceHSV', 'tenEstimate1MethodLast',
           'gageShapeBoundingBox', 'miteDefOpacMatters',
           'gageTauOfSig', 'airDisableDio', 'airArrayLenSet',
           'gageQuery', 'gageScl3PFilter8', 'hestParseOrDie',
           'tenFiberContext', 'gageScl3PFilter2',
           'nrrdBlind8BitRangeTrue', 'gageScl3PFilter6',
           'dyeSpaceHSL', 'nrrdSpaceScannerXYZTime', 'echoTypeCube',
           'nrrdCommentCopy', 'nrrdKernelTMF_maxA', 'dyeSpaceToStr',
           'nrrdKindDomain', 'nrrdKernelTMF_maxD', 'unrrduHestPosCB',
           'nrrdKernelTent', 'unrrduHestFileCB', 'gageParmReset',
           'airDouble', 'tenFiberType', 'airInsane_DLSize',
           'tenDwiGageLast', 'nrrdElementSize', 'limnCameraNew',
           'gageVecUnknown', 'nrrdMeasureSkew', 'echoCol_t',
           'echoListSplit', 'airNormalRand', 'airThreadNix',
           'airStrtok', 'seekContour3DTopoHackEdge',
           'nrrdBlind8BitRangeUnknown', 'gageScl3PFilterN',
           'unrrdu_spliceCmd', 'nrrdAxisInfoMax', 'nrrdTypeLLong',
           'echoMatterGlass', 'tenGageFANormal', 'dyeSimpleConvert',
           'nrrdMaybeAlloc_nva', 'nrrdSpaceUnknown',
           '_airThreadMutex', 'nrrdWrap_nva', 'nrrdNix',
           'nrrdResampleInfoNix', 'airSanity', 'nrrdCCSettle',
           'gagePoint', 'airArrayNew', 'limnPrimitiveTriangleFan',
           'nrrdField_type', 'miteThreadEnd', 'gageSclLast',
           'tenInterpTypeAffineInvariant', 'airMopSub',
           'echoTypeInstance', 'dyeHSLtoRGB', 'unrrdu_axinfoCmd',
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
           'nrrdField_old_max', 'gageSclFlowlineCurv',
           'nrrdMeasureProduct', 'echoThreadState', 'gageSclUnknown',
           'nrrdResampleRangeFullSet', 'limnPolyDataSize',
           'airMyQNaNHiBit', 'airDioWrite', 'tend_gradsCmd',
           'tenFiberParmVerbose', 'limnObjectLookAdd',
           'limnEdgeTypeBorder', 'nrrdSpaceOriginGet',
           'nrrdBoundaryUnknown', 'tenInv_f', 'tenInv_d',
           'nrrdField_byte_skip', 'tenTripleTypeUnknown',
           'nrrdSpaceRightAnteriorSuperiorTime', 'echoRayIntx',
           'tenFiberContextDwiNew', 'tenFiberStopUnknown',
           'airThreadBarrierNix', 'gageSclHessEval0',
           'nrrdMeasureLineIntercept', 'airNoDioErr',
           'nrrdResample_t', 'nrrdUnaryOpRoundDown',
           'airArrayPointerCB', 'nrrdOriginStatusUnknown',
           'nrrdKind2Vector', 'nrrdEnvVarDefaultKernelParm0',
           'nrrdAxisInfoIdxRange', 'echoMatterLight',
           'gageDeconvolve', 'gageSclHessEval1', 'echoObjectNew',
           'gageSclHessEval2', 'tenEstimateLinear4D',
           'nrrdDefaultGetenv']
