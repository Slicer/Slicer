import typing
from typing import Annotated

import slicer

__all__ = [
    "findFirstAnnotation",
    "getNodeTypes",
    "isNodeOrUnionOfNodes",
    "splitAnnotations",
    "unannotatedType",
]


def splitAnnotations(possiblyAnnotatedType):
    # Annotated types flatten, so
    #   Annotated[Annotated[int, 0], 1] == Annotated[int, 0, 1]
    # Wrapping the type (Annotated or not) in an Annotated gives a consistent interface
    # regardless of whether the type was Annotated or not, and it makes args[0] the same for both
    # list[int] and Annotated[list[int], annotations] (note that get_args(list[int]) == (int)).
    # We will drop the "0" off the end when we get the annotations. The value means nothing; there just
    # needs to be some annotation added for syntax reasons.
    annotatedType = Annotated[possiblyAnnotatedType, "stub annotation"]
    args = typing.get_args(annotatedType)

    actualtype = args[0]
    annotations = args[1:-1]
    return (actualtype, annotations)


def unannotatedType(possiblyAnnotatedType):
    return splitAnnotations(possiblyAnnotatedType)[0]


def findFirstAnnotation(annotationsList, annotationType):
    """
    Given a list of annotations, returns the first one of the given type
    """
    extracted = [annotation for annotation in annotationsList if isinstance(annotation, annotationType)]
    return extracted[0] if extracted else None


def splitPossiblyDottedName(possiblyDottedName):
    """
    Splits apart the name into a top level name, then the rest.

    E.g.
      "x" -> ("x", None)
      "x.y.z" -> ("x", "y.z")
    """
    if '.' in possiblyDottedName:
        split = possiblyDottedName.split('.', maxsplit=1)
        return split[0], split[1]
    else:
        return possiblyDottedName, None


def isNodeOrUnionOfNodes(datatype) -> bool:
    # Can handle a single node type, or a Union[NodeType1, NodeType2, ..., None]
    # Note the None is necessary for the parameterNodeWrapper
    underlyingDataType = unannotatedType(datatype)

    # check single type node case
    dataIsNode = issubclass(underlyingDataType, slicer.vtkMRMLNode) if type(underlyingDataType) == type else False
    if dataIsNode:
        return True

    # check union node case
    elif typing.get_origin(underlyingDataType) == typing.Union:
        underlyingArgTypes = [unannotatedType(arg) for arg in typing.get_args(underlyingDataType)]

        def validType(type_):
            return isinstance(None, type_) or issubclass(type_, slicer.vtkMRMLNode) if type(type_) == type else False
        return all([validType(t) for t in underlyingArgTypes])
    else:
        return False


def getNodeTypes(datatype):
    # datatype should be a node type or Union[NodeType1, NodeType2, ..., None]
    underlyingDataType = unannotatedType(datatype)

    # single node type case
    dataIsNode = issubclass(underlyingDataType, slicer.vtkMRMLNode) if type(underlyingDataType) == type else False
    if dataIsNode:
        return (underlyingDataType().GetClassName(), )
    # union case
    elif typing.get_origin(underlyingDataType) == typing.Union:
        nodeTypes = []
        for arg in typing.get_args(underlyingDataType):
            underlyingArgType = unannotatedType(arg)
            if type(underlyingArgType) == type and issubclass(underlyingArgType, slicer.vtkMRMLNode):
                nodeTypes.append(underlyingArgType)
        return tuple(nodeType().GetClassName() for nodeType in nodeTypes)
    else:
        raise TypeError(f"Cannot handle type {datatype}")
