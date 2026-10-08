#!/bin/bash
# Stratara OS OEM First-Boot Setup Script
# Runs on first boot after OEM installation

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

# Check if already run
OEM_MARKER="/var/lib/stratara/oem-setup-complete"
if [[ -f "$OEM_MARKER" ]]; then
    log_info "OEM setup already completed, skipping"
    exit 0
fi

# Check if running as root
if [[ $EUID -ne 0 ]]; then
    log_error "This script must be run as root"
    exit 1
fi

log_info "Starting Stratara OS OEM first-boot setup..."

# Create marker directory
mkdir -p /var/lib/stratara

# Generate machine ID if not present
if [[ ! -f /etc/machine-id ]]; then
    log_info "Generating machine ID..."
    systemd-machine-id-setup
fi

# Generate hostname
HOSTNAME="stratara-$(cat /etc/machine-id | cut -c1-8)"
log_info "Setting hostname to: $HOSTNAME"
hostnamectl set-hostname "$HOSTNAME"

# Set up timezone
log_info "Setting timezone to UTC..."
timedatectl set-timezone UTC

# Set up locale
log_info "Setting locale to en_US.UTF-8..."
localectl set-locale LANG=en_US.UTF-8

# Set up keyboard
log_info "Setting keyboard layout to US..."
localectl set-keymap us

# Configure NetworkManager
log_info "Configuring NetworkManager..."
systemctl enable NetworkManager.service
systemctl start NetworkManager.service

# Wait for NetworkManager to be ready
sleep 2

# Configure Bluetooth
log_info "Configuring Bluetooth..."
systemctl enable bluetooth.service
systemctl start bluetooth.service

# Configure PipeWire
log_info "Configuring PipeWire..."
systemctl --global enable pipewire.service
systemctl --global enable pipewire-pulse.service
systemctl --global enable wireplumber.service

# Enable Xodus service
log_info "Enabling Xodus service..."
systemctl enable xodus.service
systemctl start xodus.service

# Wait for Xodus to be ready
sleep 3

# Enroll with Xodus
log_info "Enrolling with Xodus..."
if command -v xodusctl &> /dev/null; then
    xodusctl enroll --name "Stratara Living Room" --type "living-room-shell" || log_warning "Xodus enrollment failed, will retry on next boot"
else
    log_warning "xodusctl not found, skipping Xodus enrollment"
fi

# Create default user if not exists
DEFAULT_USER="stratara"
if ! id "$DEFAULT_USER" &>/dev/null; then
    log_info "Creating default user: $DEFAULT_USER"
    useradd -m -G wheel,audio,video,network,input,kvm,render -s /bin/bash "$DEFAULT_USER"
    
    # Set up auto-login for SDDM/GDM
    log_info "Configuring auto-login for $DEFAULT_USER..."
    
    # For SDDM
    if [[ -f /etc/sddm.conf ]]; then
        mkdir -p /etc/sddm.conf.d
        cat > /etc/sddm.conf.d/autologin.conf << EOF
[Autologin]
User=$DEFAULT_USER
Session=wayland
Relogin=false
EOF
    fi
    
    # For GDM
    if [[ -f /etc/gdm/custom.conf ]]; then
        sed -i "s/^#  AutomaticLoginEnable =.*/  AutomaticLoginEnable = true/" /etc/gdm/custom.conf
        sed -i "s/^#  AutomaticLogin =.*/  AutomaticLogin = $DEFAULT_USER/" /etc/gdm/custom.conf
    fi
fi

# Set up Stratara user directories
log_info "Setting up Stratara user directories..."
mkdir -p "/home/$DEFAULT_USER/.config/stratara"
mkdir -p "/home/$DEFAULT_USER/.local/share/stratara"
mkdir -p "/home/$DEFAULT_USER/.cache/stratara"

# Copy default config
if [[ -f /usr/share/stratara/default-config.ini ]]; then
    cp /usr/share/stratara/default-config.ini "/home/$DEFAULT_USER/.config/stratara/config.ini"
    chown -R "$DEFAULT_USER:$DEFAULT_USER" "/home/$DEFAULT_USER/.config/stratara"
fi

# Configure KWin for the user
log_info "Configuring KWin for user..."
mkdir -p "/home/$DEFAULT_USER/.config"
if [[ -f /etc/kwin/kwinrc ]]; then
    cp /etc/kwin/kwinrc "/home/$DEFAULT_USER/.config/kwinrc"
    chown "$DEFAULT_USER:$DEFAULT_USER" "/home/$DEFAULT_USER/.config/kwinrc"
fi
if [[ -f /etc/kwin/windowrulesrc ]]; then
    cp /etc/kwin/windowrulesrc "/home/$DEFAULT_USER/.config/windowrulesrc"
    chown "$DEFAULT_USER:$DEFAULT_USER" "/home/$DEFAULT_USER/.config/windowrulesrc"
fi

# Enable Stratara user service
log_info "Enabling Stratara user service..."
systemctl --user --machine="$DEFAULT_USER@" enable stratara.service 2>/dev/null || true

# Set up XDG autostart for Stratara
log_info "Setting up Stratara autostart..."
mkdir -p "/home/$DEFAULT_USER/.config/autostart"
cat > "/home/$DEFAULT_USER/.config/autostart/stratara.desktop" << EOF
[Desktop Entry]
Type=Application
Name=Stratara Shell
Exec=stratara
Hidden=false
NoDisplay=false
X-GNOME-Autostart-enabled=true
OnlyShowIn=KDE;XFCE;GNOME;
EOF
chown "$DEFAULT_USER:$DEFAULT_USER" "/home/$DEFAULT_USER/.config/autostart/stratara.desktop"

# Configure display settings
log_info "Configuring display..."
mkdir -p /etc/X11/xorg.conf.d
cat > /etc/X11/xorg.conf.d/90-stratara.conf << 'EOF'
Section "Monitor"
    Identifier "HDMI-1"
    Option "PreferredMode" "1920x1080"
    Option "DPMS" "true"
EndSection

Section "Device"
    Identifier "GPU"
    Driver "modesetting"
    Option "AccelMethod" "glamor"
    Option "TearFree" "true"
EndSection

Section "Screen"
    Identifier "Screen0"
    Device "GPU"
    Monitor "HDMI-1"
    DefaultDepth 24
    SubSection "Display"
        Depth 24
        Modes "1920x1080" "1280x720"
    EndSubSection
EndSection
EOF

# Create marker file
touch "$OEM_MARKER"

log_success "Stratara OS OEM first-boot setup completed!"
log_info "The system will now reboot to apply all changes."

# Reboot
systemctl reboot