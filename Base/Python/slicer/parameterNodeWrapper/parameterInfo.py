from dataclasses import dataclass
import typing

from .serializers import Serializer

# this is totally private to the implementation, so not exposed
__all__ = []


@dataclass
class ParameterInfo:
    basename: str
    serializer: Serializer
    default: typing.Any
    unalteredType: typing.Any
