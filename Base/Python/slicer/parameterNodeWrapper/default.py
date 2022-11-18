class Default:
    """
    Annotation to denote the default value for a parameter.
    """

    def __init__(self, value):
        self.value = value

    def __repr__(self) -> str:
        return f"Default({self.value})"


def extractDefault(annotations):
    """
    Given a list of annotations, returns the first Default annotation, if any
    """
    defaults = [x for x in annotations if isinstance(x, Default)]
    nonDefaults = [x for x in annotations if not isinstance(x, Default)]
    if len(defaults) > 1:
        raise Exception("Multiple defaults found")
    return (defaults[0] if defaults else None, nonDefaults)
