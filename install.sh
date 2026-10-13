#!/usr/bin/env bash
# Installs or updates the Bridge mod in a DayZ server folder (Linux, macOS, Wine hosts).
#   curl -fsSL https://raw.githubusercontent.com/GallenteProt/Bridge/main/install.sh | bash -s -- /path/to/server
# Run the same command again to update: nothing is downloaded when the installed commit is the newest one.
set -euo pipefail

REPO="${BRIDGE_REPO:-GallenteProt/Bridge}"
BRANCH="${BRIDGE_BRANCH:-main}"
SERVER="${1:-$PWD}"

die() { echo "error: $*" >&2; exit 1; }
for tool in curl tar; do command -v "$tool" >/dev/null 2>&1 || die "$tool is required"; done
[ -d "$SERVER" ] || die "server folder not found: $SERVER"

remote="$(curl -fsSL -H 'Accept: application/vnd.github.sha' "https://api.github.com/repos/$REPO/commits/$BRANCH")"
[[ "$remote" =~ ^[0-9a-f]{40}$ ]] || die "cannot read the latest commit of $REPO@$BRANCH"
installed="$(cat "$SERVER/Bridge/.version" 2>/dev/null || true)"

if [ "$remote" = "$installed" ] && [ -f "$SERVER/Bridge/config.cpp" ]; then
  echo "Bridge is up to date (${remote:0:7})"
  exit 0
fi

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
curl -fsSL "https://codeload.github.com/$REPO/tar.gz/$remote" | tar -xz -C "$tmp"
src="$(echo "$tmp"/*/Bridge)"
[ -f "$src/config.cpp" ] || die "the downloaded archive has no Bridge/config.cpp"

rm -rf "$SERVER/Bridge.new"
cp -r "$src" "$SERVER/Bridge.new"
echo "$remote" > "$SERVER/Bridge.new/.version"
rm -rf "$SERVER/Bridge"
mv "$SERVER/Bridge.new" "$SERVER/Bridge"

if [ -n "$installed" ]; then echo "Bridge updated ${installed:0:7} -> ${remote:0:7}"; else echo "Bridge installed (${remote:0:7})"; fi
echo "Server launch parameters: -serverMod=Bridge -filePatching"
echo "Restart the server to load the new scripts."
