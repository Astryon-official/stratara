#!/bin/bash
# Stratara OS ISO Build Script
# This script builds the Stratara OS ISO using kiwi or lorax

set -euo pipefail

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
BUILD_DIR="/tmp/stratara-iso-build"
OUTPUT_DIR="$PROJECT_ROOT/build/iso"
ISO_NAME="Stratara-OS-0.1.0-x86_64"
BUILD_METHOD="${1:-kiwi}"  # kiwi or lorax

log_info "Starting Stratara OS ISO build..."
log_info "Build method: $BUILD_METHOD"
log_info "Project root: $PROJECT_ROOT"
log_info "Build directory: $BUILD_DIR"
log_info "Output directory: $OUTPUT_DIR"

# Check if running as root
if [[ $EUID -ne 0 ]]; then
    log_error "This script must be run as root"
    exit 1
fi

# Create directories
mkdir -p "$BUILD_DIR"
mkdir -p "$OUTPUT_DIR"

# Function to build with kiwi
build_with_kiwi() {
    log_info "Building ISO with kiwi..."
    
    # Check if kiwi is installed
    if ! command -v kiwi-ng &> /dev/null; then
        log_error "kiwi-ng not found. Install with: zypper install kiwi-ng"
        return 1
    fi
    
    # Create kiwi build directory
    local kiwi_dir="$BUILD_DIR/kiwi"
    mkdir -p "$kiwi_dir"
    
    # Copy kiwi config
    cp "$SCRIPT_DIR/kiwi/stratara-kiwi.xml" "$kiwi_dir/config.xml"
    
    # Copy required files
    log_info "Copying required files..."
    mkdir -p "$kiwi_dir/root/etc"
    mkdir -p "$kiwi_dir/root/usr/share/kwin/scripts"
    mkdir -p "$kiwi_dir/root/usr/share/wayland-sessions"
    mkdir -p "$kiwi_dir/root/usr/share/grub/themes"
    mkdir -p "$kiwi_dir/root/usr/lib/stratara"
    
    # Copy Stratara files (if they exist)
    if [[ -f "$PROJECT_ROOT/installer/kwin/kwinrc" ]]; then
        cp "$PROJECT_ROOT/installer/kwin/kwinrc" "$kiwi_dir/root/etc/"
    fi
    if [[ -f "$PROJECT_ROOT/installer/kwin/windowrulesrc" ]]; then
        cp "$PROJECT_ROOT/installer/kwin/windowrulesrc" "$kiwi_dir/root/etc/"
    fi
    if [[ -d "$PROJECT_ROOT/installer/kwin/scripts/stratara-shell" ]]; then
        cp -r "$PROJECT_ROOT/installer/kwin/scripts/stratara-shell" "$kiwi_dir/root/usr/share/kwin/scripts/"
    fi
    if [[ -f "$PROJECT_ROOT/installer/stratara.desktop" ]]; then
        cp "$PROJECT_ROOT/installer/stratara.desktop" "$kiwi_dir/root/usr/share/wayland-sessions/"
    fi
    if [[ -d "$PROJECT_ROOT/installer/grub/themes/stratara" ]]; then
        cp -r "$PROJECT_ROOT/installer/grub/themes/stratara" "$kiwi_dir/root/usr/share/grub/themes/"
    fi
    if [[ -f "$PROJECT_ROOT/installer/calamares/scripts/post-install.sh" ]]; then
        cp "$PROJECT_ROOT/installer/calamares/scripts/post-install.sh" "$kiwi_dir/root/usr/lib/stratara/"
    fi
    if [[ -f "$PROJECT_ROOT/installer/calamares/scripts/oem-setup.sh" ]]; then
        cp "$PROJECT_ROOT/installer/calamares/scripts/oem-setup.sh" "$kiwi_dir/root/usr/lib/stratara/"
    fi
    if [[ -f "$PROJECT_ROOT/installer/calamares/scripts/xodusctl" ]]; then
        cp "$PROJECT_ROOT/installer/calamares/scripts/xodusctl" "$kiwi_dir/root/usr/lib/stratara/"
    fi
    
    # Build ISO
    log_info "Running kiwi-ng build..."
    cd "$kiwi_dir"
    kiwi-ng --type iso system build \
        --description "$kiwi_dir" \
        --target-dir "$OUTPUT_DIR" \
        --profile default \
        --set-repo obs://repositories/BaseOS,obs://repositories/Updates,obs://repositories/KDE \
        --ignore-repos \
        --allow-existing-image \
        --force-new
    
    # Rename output
    local built_iso=$(find "$OUTPUT_DIR" -name "*.iso" -type f | head -1)
    if [[ -n "$built_iso" ]]; then
        mv "$built_iso" "$OUTPUT_DIR/${ISO_NAME}.iso"
        log_success "ISO built: $OUTPUT_DIR/${ISO_NAME}.iso"
        
        # Generate checksum
        cd "$OUTPUT_DIR"
        sha256sum "${ISO_NAME}.iso" > "${ISO_NAME}.iso.sha256"
        log_success "Checksum generated: ${ISO_NAME}.iso.sha256"
    else
        log_error "ISO build failed - no ISO found in output directory"
        return 1
    fi
}

# Function to build with lorax
build_with_lorax() {
    log_info "Building ISO with lorax..."
    
    # Check if lorax is installed
    if ! command -v lorax &> /dev/null; then
        log_error "lorax not found. Install with: zypper install lorax"
        return 1
    fi
    
    local lorax_dir="$BUILD_DIR/lorax"
    mkdir -p "$lorax_dir"
    
    # Copy lorax config
    cp "$SCRIPT_DIR/lorax/stratara-lorax.conf" "$lorax_dir/lorax.conf"
    
    # Copy required files
    log_info "Copying required files..."
    mkdir -p "$lorax_dir/root/etc"
    mkdir -p "$lorax_dir/root/usr/share/kwin/scripts"
    mkdir -p "$lorax_dir/root/usr/share/wayland-sessions"
    mkdir -p "$lorax_dir/root/usr/share/grub/themes"
    mkdir -p "$lorax_dir/root/usr/lib/stratara"
    
    # Copy Stratara files
    if [[ -f "$PROJECT_ROOT/installer/kwin/kwinrc" ]]; then
        cp "$PROJECT_ROOT/installer/kwin/kwinrc" "$lorax_dir/root/etc/"
    fi
    if [[ -f "$PROJECT_ROOT/installer/kwin/windowrulesrc" ]]; then
        cp "$PROJECT_ROOT/installer/kwin/windowrulesrc" "$lorax_dir/root/etc/"
    fi
    if [[ -d "$PROJECT_ROOT/installer/kwin/scripts/stratara-shell" ]]; then
        cp -r "$PROJECT_ROOT/installer/kwin/scripts/stratara-shell" "$lorax_dir/root/usr/share/kwin/scripts/"
    fi
    if [[ -f "$PROJECT_ROOT/installer/stratara.desktop" ]]; then
        cp "$PROJECT_ROOT/installer/stratara.desktop" "$lorax_dir/root/usr/share/wayland-sessions/"
    fi
    if [[ -d "$PROJECT_ROOT/installer/grub/themes/stratara" ]]; then
        cp -r "$PROJECT_ROOT/installer/grub/themes/stratara" "$lorax_dir/root/usr/share/grub/themes/"
    fi
    if [[ -f "$PROJECT_ROOT/installer/calamares/scripts/post-install.sh" ]]; then
        cp "$PROJECT_ROOT/installer/calamares/scripts/post-install.sh" "$lorax_dir/root/usr/lib/stratara/"
    fi
    if [[ -f "$PROJECT_ROOT/installer/calamares/scripts/oem-setup.sh" ]]; then
        cp "$PROJECT_ROOT/installer/calamares/scripts/oem-setup.sh" "$lorax_dir/root/usr/lib/stratara/"
    fi
    if [[ -f "$PROJECT_ROOT/installer/calamares/scripts/xodusctl" ]]; then
        cp "$PROJECT_ROOT/installer/calamares/scripts/xodusctl" "$lorax_dir/root/usr/lib/stratara/"
    fi
    
    # Copy kernel config
    cp "$SCRIPT_DIR/kernel/cmdline.txt" "$lorax_dir/root/etc/stratara/kernel-cmdline.txt"
    cp "$SCRIPT_DIR/kernel/dracut-stratara.conf" "$lorax_dir/root/etc/stratara/dracut-stratara.conf"
    
    # Build ISO
    log_info "Running lorax build..."
    cd "$lorax_dir"
    lorax -c "$lorax_dir/lorax.conf" \
        --output "$OUTPUT_DIR" \
        --volid "Stratara-OS" \
        --product "Stratara OS" \
        --version "0.1.0" \
        --release "1" \
        --arch "x86_64" \
        --nomacboot \
        --noverify \
        --buildarch x86_64 \
        --isfinal \
        "$OUTPUT_DIR/${ISO_NAME}"
    
    # Rename output
    local built_iso=$(find "$OUTPUT_DIR" -name "*.iso" -type f | head -1)
    if [[ -n "$built_iso" ]]; then
        mv "$built_iso" "$OUTPUT_DIR/${ISO_NAME}.iso"
        log_success "ISO built: $OUTPUT_DIR/${ISO_NAME}.iso"
        
        # Generate checksum
        cd "$OUTPUT_DIR"
        sha256sum "${ISO_NAME}.iso" > "${ISO_NAME}.iso.sha256"
        log_success "Checksum generated: ${ISO_NAME}.iso.sha256"
    else
        log_error "ISO build failed - no ISO found in output directory"
        return 1
    fi
}

# Main build logic
case "$BUILD_METHOD" in
    kiwi)
        build_with_kiwi
        ;;
    lorax)
        build_with_lorax
        ;;
    both)
        build_with_kiwi
        build_with_lorax
        ;;
    *)
        log_error "Unknown build method: $BUILD_METHOD"
        log_info "Usage: $0 [kiwi|lorax|both]"
        exit 1
        ;;
esac

# Run Secure Boot setup if ISO was built
if [[ -f "$OUTPUT_DIR/${ISO_NAME}.iso" ]]; then
    log_info "Running Secure Boot setup..."
    bash "$SCRIPT_DIR/secure-boot-setup.sh" || log_warning "Secure Boot setup had issues"
fi

log_success "ISO build completed!"
log_info "Output: $OUTPUT_DIR/${ISO_NAME}.iso"
log_info "Checksum: $OUTPUT_DIR/${ISO_NAME}.iso.sha256"