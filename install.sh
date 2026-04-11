#!/bin/bash
# Wallpaper Engine KDE Plugin - One-shot installer
# Detects distro, installs dependencies, builds and installs the plugin.

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

info()    { echo -e "${BLUE}[INFO]${NC} $*"; }
ok()      { echo -e "${GREEN}[ OK ]${NC} $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC} $*"; }
error()   { echo -e "${RED}[ERR ]${NC} $*" >&2; }
die()     { error "$*"; exit 1; }

# ── check we're in the project root ──────────────────────────────────────────
[[ -f CMakeLists.txt && -f plugin/metadata.json ]] \
    || die "Run this script from the wallpaper-engine-kde-plugin project root."

# ── detect KDE / Qt generation ───────────────────────────────────────────────
detect_kde_version() {
    if command -v plasmashell &>/dev/null; then
        local v
        v=$(plasmashell --version 2>/dev/null | grep -oP '\d+' | head -1)
        echo "${v:-5}"
    else
        echo "5"
    fi
}

KDE_VER=$(detect_kde_version)
info "Detected KDE ${KDE_VER}"

# ── detect distro ────────────────────────────────────────────────────────────
detect_distro() {
    if [[ -f /etc/os-release ]]; then
        # shellcheck disable=SC1091
        . /etc/os-release
        echo "${ID:-unknown}"
    else
        echo "unknown"
    fi
}

DISTRO=$(detect_distro)
info "Detected distro: ${DISTRO}"

# ── install dependencies ──────────────────────────────────────────────────────
install_deps() {
    case "${DISTRO}" in
        arch|cachyos|endeavouros|manjaro)
            info "Installing Arch-based dependencies…"
            sudo pacman -S --needed --noconfirm \
                extra-cmake-modules ninja base-devel cmake \
                vulkan-headers mpv sndio python-websockets \
                qt5-declarative qt5-websockets qt5-webchannel \
                gst-libav
            if [[ "${KDE_VER}" == "6" ]]; then
                sudo pacman -S --needed --noconfirm \
                    plasma-framework qt6-declarative qt6-websockets qt6-webchannel \
                    2>/dev/null || true
            else
                sudo pacman -S --needed --noconfirm plasma-framework5 2>/dev/null || true
            fi
            ;;
        debian|ubuntu|linuxmint|pop)
            info "Installing Debian-based dependencies…"
            sudo apt-get update -qq
            sudo apt-get install -y \
                build-essential cmake ninja-build \
                libvulkan-dev liblz4-dev libmpv-dev \
                python3-websockets \
                gstreamer1.0-libav \
                qtbase5-private-dev libqt5x11extras5-dev \
                qml-module-qtwebchannel qml-module-qtwebsockets \
                plasma-workspace-dev extra-cmake-modules
            ;;
        fedora|nobara|bazzite)
            info "Installing Fedora-based dependencies…"
            info "Note: RPM Fusion repository is required for mpv and gstreamer-libav."
            sudo dnf install -y \
                cmake ninja-build gcc-c++ \
                vulkan-headers lz4-devel mpv-libs-devel \
                python3-websockets \
                gstreamer1-libav \
                qt5-qtbase-private-devel qt5-qtx11extras-devel \
                qt5-qtwebchannel-devel qt5-qtwebsockets-devel \
                plasma-workspace-devel kf5-plasma-devel extra-cmake-modules
            ;;
        opensuse*|suse)
            info "Installing openSUSE dependencies…"
            info "Note: Packman repository is required for full codec support."
            sudo zypper install -y \
                cmake ninja gcc-c++ \
                vulkan-devel liblz4-devel mpv-devel \
                python3-websockets \
                gstreamer-plugins-libav \
                libqt5-qtbase-private-headers-devel libqt5-qtx11extras-devel \
                libqt5-qtwebsockets-devel \
                plasma-framework-devel plasma5-workspace-devel \
                extra-cmake-modules
            ;;
        void)
            info "Installing Void dependencies…"
            sudo xbps-install -Sy \
                cmake ninja base-devel \
                Vulkan-Headers liblz4-devel mpv-devel \
                python3-websockets \
                gst-libav \
                qt5-declarative qt5-websockets qt5-webchannel \
                plasma-framework plasma-workspace-devel \
                extra-cmake-modules
            ;;
        *)
            warn "Unknown distro '${DISTRO}'. Skipping automatic dependency installation."
            warn "Please install the required packages manually before continuing."
            warn "See README.md for the package list for your distro."
            read -rp "Continue anyway? [y/N] " ans
            [[ "${ans,,}" == "y" ]] || exit 0
            ;;
    esac
    ok "Dependencies installed."
}

# ── init git submodules ───────────────────────────────────────────────────────
init_submodules() {
    info "Initialising git submodules (scene renderer)…"
    git submodule update --init --force --recursive \
        || die "Failed to initialise submodules. Check your internet connection."
    ok "Submodules ready."
}

# ── build ─────────────────────────────────────────────────────────────────────
build_plugin() {
    info "Configuring build…"
    cmake -B build -S . -G Ninja \
        -DUSE_PLASMAPKG=ON \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        || die "CMake configuration failed. Check the output above for missing dependencies."

    info "Building plugin…"
    local jobs
    jobs=$(nproc 2>/dev/null || echo 4)
    cmake --build build --parallel "${jobs}" \
        || die "Build failed. Check the output above for errors."
    ok "Build complete."
}

# ── install ───────────────────────────────────────────────────────────────────
install_plugin() {
    info "Installing native library (may need sudo)…"
    sudo cmake --install build \
        || die "System install failed."

    info "Installing KDE plasma package…"
    cmake --build build --target install_pkg \
        || die "Plasma package install failed."

    ok "Plugin installed successfully."
}

# ── restart plasmashell ───────────────────────────────────────────────────────
restart_plasma() {
    info "Restarting plasmashell to load the new plugin…"
    if systemctl --user is-active plasma-plasmashell.service &>/dev/null; then
        systemctl --user restart plasma-plasmashell.service \
            && ok "plasmashell restarted." \
            || warn "Could not restart plasmashell automatically. Please log out and back in."
    else
        warn "plasma-plasmashell service not found. Please restart your KDE session manually."
    fi
}

# ── main ──────────────────────────────────────────────────────────────────────
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Wallpaper Engine KDE Plugin – Installer"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Parse flags
SKIP_DEPS=0
SKIP_RESTART=0
for arg in "$@"; do
    case "$arg" in
        --skip-deps)    SKIP_DEPS=1 ;;
        --skip-restart) SKIP_RESTART=1 ;;
        --help|-h)
            echo "Usage: $0 [--skip-deps] [--skip-restart]"
            echo ""
            echo "  --skip-deps     Skip dependency installation"
            echo "  --skip-restart  Skip restarting plasmashell after install"
            exit 0
            ;;
    esac
done

[[ "${SKIP_DEPS}" == "1" ]] || install_deps
init_submodules
build_plugin
install_plugin
[[ "${SKIP_RESTART}" == "1" ]] || restart_plasma

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
ok "Installation complete!"
echo ""
echo "  Next steps:"
echo "  1. Right-click your desktop → Configure Desktop and Wallpaper"
echo "  2. Select 'Wallpaper Engine for KDE' from the wallpaper type list"
echo "  3. Point the plugin at your Steam library folder"
echo "     (usually ~/.local/share/Steam)"
echo ""
echo "  If the plugin does not appear, restart your KDE session."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
