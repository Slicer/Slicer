"""The validators module is responsible for defining classes that can be used to place invariants on single parameters
of a parameterPack or parameterNodeWrapper."""

import abc

__all__ = [
    "Validator",
    "NotNone",
    "IsInstance",
    "WithinRange",
    "Minimum",
    "Maximum",
    "Choice",
    "Exclude",
    "RangeBounds",
]


class Validator(abc.ABC):
    """
    Base class from which all parameterNodeWrapper validators derive.
    Validators must derive from this class to be used.
    """
    @abc.abstractmethod
    def validate(self, value) -> None:
        """
        Validates that the input value is valid.
        Raises an exception (recommended ValueError) if it is not valid.
        """
        raise NotImplementedError("validate is not implemented")


def extractValidators(annotations):
    """
    Given a list of annotations, separates the annotations that are Validators from the ones that are not.
    """
    def isValidator(x) -> None:
        return isinstance(x, Validator) or (isinstance(x, type) and issubclass(x, Validator))

    return (
        [x for x in annotations if isValidator(x)],
        [x for x in annotations if not isValidator(x)]
    )


class IsNone(Validator):
    """
    Validates that the input value is None.
    """

    def __repr__(self) -> str:
        return f"IsNone()"

    def validate(self, value) -> None:
        if value is not None:
            raise ValueError("Value must be None")


class NotNone(Validator):
    """
    Validates that any input value is not None.
    """

    def __repr__(self) -> str:
        return f"NotNone()"

    def validate(self, value) -> None:
        if value is None:
            raise ValueError("Value must not be None")


class IsInstance(Validator):
    """
    Validates that any input value is an instance of a given type.
    """

    def __init__(self, classtype):
        self.classtype = classtype

    def __repr__(self) -> str:
        return f"IsInstance({self.classtype})"

    def validate(self, value) -> None:
        if value is not None and not isinstance(value, self.classtype):
            raise TypeError(f"Value must be of type '{self.classtype}', is type '{type(value)}' value '{value}'")


class WithinRange(Validator):
    """
    Validates that any input value is within the given range (inclusive).
    """

    def __init__(self, minimum, maximum):
        self.minimum = minimum
        self.maximum = maximum

    def __repr__(self) -> str:
        return f"WithinRange({self.minimum}, {self.maximum})"

    def validate(self, value) -> None:
        if not self.minimum <= value <= self.maximum:
            raise ValueError(f"Value must be within range [{self.minimum}, {self.maximum}], is {value}")


class Minimum(Validator):
    """
    Validates that any input value is greater than or equal to the given value.
    """

    def __init__(self, minimum):
        self.minimum = minimum

    def __repr__(self) -> str:
        return f"Minimum({self.minimum})"

    def validate(self, value) -> None:
        if value < self.minimum:
            raise ValueError(f"Value must be greater than {self.minimum}, is {value}")


class Maximum(Validator):
    """
    Validates that any input value is less than or equal to the given value.
    """

    def __init__(self, maximum):
        self.maximum = maximum

    def __repr__(self) -> str:
        return f"Maximum({self.maximum})"

    def validate(self, value) -> None:
        if value > self.maximum:
            raise ValueError(f"Value must be greater than {self.maximum}, is {value}")


class Choice(Validator):
    """
    Validates that any input value is in the list of valid choices.
    """

    def __init__(self, choices):
        self.choices = choices

    def __repr__(self) -> str:
        return f"Choice({self.choices})"

    def validate(self, value) -> None:
        if value not in self.choices:
            raise ValueError(f"Value is {value}, but must be in one of the following: {self.choices}")


class Exclude(Validator):
    """
    Validates that any input value is not in the list of invalid choices.
    """

    def __init__(self, excludedValues):
        self.excludedValues = excludedValues

    def __repr__(self) -> str:
        return f"Exclude({self.excludedValues})"

    def validate(self, value) -> None:
        if value in self.excludedValues:
            raise ValueError(f"Value is {value}, but must not be an excluded value: excluded values {self.excludedValues}")


class RangeBounds(Validator):
    """
    Validates that the values in a range are within the given overall bounds.
    """

    def __init__(self, minimum, maximum):
        self.minimum = minimum
        self.maximum = maximum

    def __repr__(self) -> str:
        return f"RangeBounds({self.minimum}, {self.maximum})"

    def validate(self, value) -> None:
        if not self.minimum <= value.minimum and value.maximum <= self.maximum:
            raise ValueError(f"Range must be within the bound of [{self.minimum}, {self.maximum}], is [{value.minimum}, {value.maximum}]")
