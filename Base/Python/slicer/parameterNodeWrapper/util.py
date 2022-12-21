import typing
from typing import Annotated

__all__ = ["splitAnnotations", "unannotatedType", "findFirstAnnotation"]


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
