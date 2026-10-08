#!/bin/bash
# Stratara OS Post-Install Script
# Runs after Calamares installation to configure the system

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as root
if [[ $EUID -ne 0 ]]; then
    log_error "This script must be run as root"
    exit 1
fi

log_info "Starting Stratara OS post-install configuration..."

# Enable systemd services
log_info "Enabling systemd services..."
systemctl enable NetworkManager.service
systemctl enable bluetooth.service
systemctl enable pipewire.service
systemctl enable pipewire-pulse.service
systemctl enable dbus.service

# Enable Xodus system service
systemctl enable xodus.service

# Enable Stratara user service (will be enabled per-user)
log_info "Stratara user service will be enabled on first login"

# Configure GRUB for Stratara
log_info "Configuring GRUB..."
if [[ -f /etc/default/grub ]]; then
    # Backup original
    cp /etc/default/grub /etc/default/grub.backup
    
    # Update GRUB settings for Stratara
    sed -i 's/^GRUB_TIMEOUT=.*/GRUB_TIMEOUT=5/' /etc/default/grub
    sed -i 's/^GRUB_GFXMODE=.*/GRUB_GFXMODE=1920x1080/' /etc/default/grub
    
    # Add Stratara theme if not present
    if ! grep -q "GRUB_THEME" /etc/default/grub; then
        echo 'GRUB_THEME="/usr/share/grub/themes/stratara/theme.txt"' >> /etc/default/grub
    fi
    
    # Regenerate GRUB config
    grub-mkconfig -o /boot/grub/grub.cfg
    log_success "GRUB configured"
fi

# Configure initramfs
log_info "Configuring initramfs..."
if [[ -f /etc/mkinitcpio.conf ]]; then
    # Add Stratara hooks if needed
    sed -i 's/^HOOKS=.*/HOOKS=(base udev autodetect modconf block filesystems keyboard fsck)/' /etc/mkinitcpio.conf
    mkinitcpio -P
    log_success "Initramfs regenerated"
elif [[ -f /etc/dracut.conf ]]; then
    dracut --force
    log_success "Initramfs regenerated (dracut)"
fi

# Set up Stratara user directories
log_info "Setting up Stratara user directories..."
mkdir -p /etc/skel/.config/stratara
mkdir -p /etc/skel/.local/share/stratara
mkdir -p /etc/skel/.cache/stratara

# Copy default Stratara config if it exists
if [[ -f /usr/share/stratara/default-config.ini ]]; then
    cp /usr/share/stratara/default-config.ini /etc/skel/.config/stratara/config.ini
fi

# Configure NetworkManager for Stratara
log_info "Configuring NetworkManager..."
mkdir -p /etc/NetworkManager/conf.d
cat > /etc/NetworkManager/conf.d/stratara.conf << 'EOF'
[main]
dhcp=internal

[device]
wifi.scan-rand-mac-address=no

[connection]
ipv6.ip6-privacy=0
EOF

# Configure Bluetooth for Stratara
log_info "Configuring Bluetooth..."
mkdir -p /etc/bluetooth
if [[ ! -f /etc/bluetooth/main.conf ]]; then
    cat > /etc/bluetooth/main.conf << 'EOF'
[General]
Name = Stratara Living Room
Class = 0x000408
DiscoverableTimeout = 0
AlwaysPairable = true
AutoEnable = true

[Policy]
AutoEnable = true
EOF
fi

# Configure PipeWire for Stratara
log_info "Configuring PipeWire..."
mkdir -p /etc/pipewire/pipewire.conf.d
cat > /etc/pipewire/pipewire.conf.d/stratara.conf << 'EOF'
context.properties = {
    default.clock.rate = 48000
    default.clock.allowed-rates = [ 48000 ]
}
EOF

# Set up KWin for Stratara
log_info "Configuring KWin..."
mkdir -p /etc/kwin
if [[ -f /usr/share/stratara/kwinrc ]]; then
    cp /usr/share/stratara/kwinrc /etc/kwin/kwinrc
fi
if [[ -f /usr/share/stratara/windowrulesrc ]]; then
    cp /usr/share/stratara/windowrulesrc /etc/kwin/windowrulesrc
fi

# Configure systemd-logind for Stratara
log_info "Configuring systemd-logind..."
mkdir -p /etc/systemd/logind.conf.d
cat > /etc/systemd/logind.conf.d/stratara.conf << 'EOF'
[Login]
HandlePowerKey=poweroff
HandleSuspendKey=suspend
HandleHibernateKey=hibernate
HandleLidSwitch=ignore
IdleAction=ignore
IdleActionSec=0
EOF

# Set up Xodus configuration
log_info "Setting up Xodus..."
mkdir -p /etc/xodus
mkdir -p /var/lib/xodus

# Create default Xodus config
if [[ ! -f /etc/xodus/settings.conf ]]; then
    cat > /etc/xodus/settings.conf << 'EOF'
[Xodus]
version=0.1.0
hostname=stratara-livingroom
timezone=UTC
locale=en_US.UTF-8
keyboardLayout=us
keyboardVariant=
EOF
fi

# Set up Stratara systemd service overrides
log_info "Configuring Stratara systemd services..."
mkdir -p /etc/systemd/system/stratara.service.d
cat > /etc/systemd/system/stratara.service.d/override.conf << 'EOF'
[Service]
Environment=QT_QPA_PLATFORM=wayland
Environment=QT_WAYLAND_DISABLE_WINDOWDECORATION=1
Environment=XDG_SESSION_TYPE=wayland
EOF

# Reload systemd
systemctl daemon-reload

log_success "Stratara OS post-install configuration completed!"
log_info "The system is ready for first boot."