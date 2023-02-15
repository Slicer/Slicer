__all__ = ["Default"]


class Default:
    """
    Annotation to denote the default value for a parameter.
    """

    def __init__(self, value=None, *, generator=None):
        """
        If generator is not None, it will be called each time the default is requested via the self.value
        property.
        Otherwise, value will be returned.
        """
        self._value = value
        self._generator = generator

    @property
    def value(self):
        if self._generator:
            return self._generator()
        else:
            return self._value

    def __repr__(self) -> str:
        return f"Default(value={self._value}, generator={self._generator})"

    def __eq__(self, other):
        if isinstance(other, Default):
            return self._value == other._value and self._generator == other._generator
        else:
            return False


def extractDefault(annotations):
    """
    Given a list of annotations, returns the first Default annotation, if any
    """
    defaults = [x for x in annotations if isinstance(x, Default)]
    nonDefaults = [x for x in annotations if not isinstance(x, Default)]
    if len(defaults) > 1:
        raise Exception("Multiple defaults found")
    return (defaults[0] if defaults else None, nonDefaults)
