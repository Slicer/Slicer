from __future__ import annotations
import re
from docutils import nodes
from sphinx.transforms.post_transforms import SphinxPostTransform
from sphinx.util import logging

logger = logging.getLogger(__name__)

# Matches "[!KIND]" optionally followed by an inline title on the same line.
# Examples:
#   [!NOTE]
#   [!WARNING] Read this first
_ALERT_HEADER = re.compile(r"^\s*\[\!(?P<kind>[A-Za-z]+)\]\s*(?P<title>.*)")

# Map GitHub kinds to Docutils admonition node classes (extend as you like)
_KIND_MAP = {
    "note": nodes.note,
    "tip": nodes.tip,
    "hint": nodes.hint,
    "important": nodes.important,
    "warning": nodes.warning,
    "caution": nodes.caution,
    "attention": nodes.attention,
    "danger": nodes.danger,
    "error": nodes.error,
    # Aliases
    "info": nodes.note,
    "success": nodes.tip,
}

def _convert_blockquotes(doctree: nodes.document, *, debug: bool = False) -> int:
    """Convert GitHub alert blockquotes into admonitions. Returns count converted."""
    converted = 0

    candidates = list(doctree.traverse(nodes.block_quote))
    for cont in doctree.traverse(nodes.container):
        if "quote" in cont.get("classes", []):
            candidates.append(cont)

    seen = set()
    for bq in candidates:
        if id(bq) in seen or not bq.children:
            continue
        seen.add(id(bq))

        first_para = next((c for c in bq.children if isinstance(c, nodes.paragraph)), None)
        if not first_para:
            continue

        head_text = first_para.astext().strip()
        m = _ALERT_HEADER.match(head_text)
        if not m:
            continue

        kind_raw = m.group("kind")  # preserve original case for prefix removal
        kind = kind_raw.lower()

        NodeClass = _KIND_MAP.get(kind)
        admon = nodes.admonition() if NodeClass is None else NodeClass()
        if NodeClass is None:
            admon += nodes.title(text=kind.title())

        # Inline text = everything after the marker on the first paragraph (possibly multi-line)
        prefix = f"[!{kind_raw}]"
        inline_text = head_text.removeprefix(prefix)

        # Keep inline text as the first paragraph of the body (if present)
        if inline_text:
            admon += nodes.paragraph(text=inline_text)

        # Append remaining children from the blockquote (skip the first paragraph)
        for child in bq.children[1:]:
            admon += child.deepcopy()

        if debug:
            logger.info(
                f"[github_alerts] converted {kind!r} (inline->body: {bool(inline_text)})",
            )

        bq.replace_self(admon)
        converted += 1

    return converted

class GithubAlertsToAdmonitions(SphinxPostTransform):
    default_priority = 700  # after MyST parsing, before writers

    def run(self):
        debug = bool(getattr(self.app.config, "github_alerts_debug", False))
        _convert_blockquotes(self.document, debug=debug)

def setup(app):
    app.add_config_value("github_alerts_debug", False, "env")
    app.add_post_transform(GithubAlertsToAdmonitions)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
