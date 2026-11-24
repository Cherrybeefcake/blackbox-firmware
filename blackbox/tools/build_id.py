# SPDX-License-Identifier: AGPL-3.0-or-later
"""Identify the committed firmware inputs. Build products live under PlatformIO's build directory."""
import subprocess
from pathlib import Path

Import("env")
project = Path(env.subst("$PROJECT_DIR"))
inputs = ["src", "platformio.ini", "tools", "../custom_8Mb.csv"]
def git(*args):
    return subprocess.check_output(["git", "-C", str(project), *args], text=True).strip()

try:
    revision = git("log", "-1", "--format=%H", "--", *inputs) or "unversioned"
    dirty = revision == "unversioned" or bool(git("status", "--porcelain", "--", *inputs))
except (OSError, subprocess.CalledProcessError):
    revision, dirty = "unversioned", True
build = Path(env.subst("$BUILD_DIR"))
build.mkdir(parents=True, exist_ok=True)
header = build / "blackbox_build.h"
content = '#define BLACKBOX_BUILD_ID "{}{}"\n'.format(revision[:12], "+" if dirty else "")
if not header.exists() or header.read_text() != content:
    header.write_text(content)
env.Append(CPPPATH=[str(build)])
env["BLACKBOX_REVISION"] = revision
env["BLACKBOX_DIRTY"] = dirty
