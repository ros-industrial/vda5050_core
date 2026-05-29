"""Bootstrap imports from autoxing-spellbook-cli."""

from __future__ import annotations

import os
import sys
from pathlib import Path

_DEFAULT_SPELLBOOK_DIR = Path.home() / "autoxing_spellbook" / "autoxing-spellbook-cli"
_SPELLBOOK_CONFIGURED = False


def spellbook_root() -> Path:
    """Return resolved autoxing-spellbook-cli directory."""
    env = os.environ.get("AUTOXING_SPELLBOOK_DIR", "").strip()
    root = Path(env).expanduser() if env else _DEFAULT_SPELLBOOK_DIR
    return root.resolve()


def _prepend_import_paths(root: Path, autoxing_pkg: Path) -> None:
    """Spellbook venv deps first (websockets>=10), then ``autoxing/`` scripts."""
    to_add: list[str] = []
    venv_lib = root / ".venv" / "lib"
    if venv_lib.is_dir():
        for site_packages in sorted(venv_lib.glob("python*/site-packages")):
            to_add.append(str(site_packages.resolve()))
    to_add.append(str(autoxing_pkg.resolve()))
    for path in reversed(to_add):
        if path not in sys.path:
            sys.path.insert(0, path)


def ensure_spellbook() -> Path:
    """Add spellbook venv + ``autoxing/`` to ``sys.path`` and verify config exists."""
    global _SPELLBOOK_CONFIGURED
    root = spellbook_root()
    autoxing_pkg = root / "autoxing"
    credentials_yml = autoxing_pkg / "credentials" / "CONSTANTS.yml"
    credentials_example = autoxing_pkg / "credentials" / "CONSTANTS.example.yml"

    if not root.is_dir():
        raise RuntimeError(
            f"Spellbook directory not found: {root}\n"
            "Set AUTOXING_SPELLBOOK_DIR or clone autoxing-spellbook-cli."
        )
    if not autoxing_pkg.is_dir():
        raise RuntimeError(
            f"Missing autoxing package under spellbook root: {autoxing_pkg}"
        )

    _prepend_import_paths(root, autoxing_pkg)

    venv_site = root / ".venv" / "lib"
    if not any(venv_site.glob("python*/site-packages")):
        raise RuntimeError(
            f"Spellbook venv not found under {root / '.venv'}.\n"
            "Run: cd autoxing-spellbook-cli && uv sync"
        )

    if not credentials_yml.is_file():
        hint = (
            f"Copy {credentials_example.name} to CONSTANTS.yml and set robot_ip."
            if credentials_example.is_file()
            else "Create CONSTANTS.yml with active robot IP."
        )
        raise RuntimeError(
            f"Spellbook robot config not found: {credentials_yml}\n{hint}"
        )

    _SPELLBOOK_CONFIGURED = True
    return root
