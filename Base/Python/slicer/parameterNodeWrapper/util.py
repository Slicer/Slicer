import typing
from typing import Annotated


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
