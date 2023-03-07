from .parameterPack import parameterPack

__all__ = [
    "FloatRange",
]


@parameterPack
class FloatRange:
    _minimum: float
    _maximum: float

    @staticmethod
    def checkRange(minimum, maximum):
        if not minimum <= maximum:
            raise ValueError(f"FloatRange must have minimum <= maximum: {minimum} <= {maximum}")

    def __init__(self, minimum=0, maximum=0) -> None:
        self.checkRange(minimum, maximum)
        self._minimum = minimum
        self._maximum = maximum

    def __str__(self) -> str:
        return f"FloatRange(minimum={self.minimum}, maximum={self.maximum})"
    
    def __repr__(self) -> str:
        return self.__str__()

    @property
    def minimum(self) -> float:
        return self._minimum

    @minimum.setter
    def minimum(self, value: float) -> None:
        self.checkRange(value, self.maximum)
        self._minimum = value

    @property
    def maximum(self) -> float:
        return self._maximum

    @maximum.setter
    def maximum(self, value: float) -> None:
        self.checkRange(self.minimum, value)
        self._maximum = value

    def setRange(self, minimum, maximum) -> None:
        self.checkRange(minimum, maximum)
        self._minimum = minimum
        self._maximum = maximum
