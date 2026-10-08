#!/bin/bash
# Stratara OS OEM First-Boot Setup Script
# Runs on first boot after OEM installation

set -euo pipefail

# Parse arguments
DATA_FILE=""
while [[ $# -gt 0 ]]; do
    case $1 in
        --data-file)
            DATA_FILE="$2"
            shift 2
            ;;
        *)
            shift
            ;;
    esac
done

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

# Load data from JSON file if provided
SETUP_HOSTNAME=""
SETUP_USERNAME="stratara"
SETUP_PASSWORD=""
SETUP_TIMEZONE="UTC"
SETUP_LOCALE="en_US.UTF-8"
SETUP_KEYBOARD="us"
SETUP_WIFI_SSID=""
SETUP_WIFI_PASSWORD=""
SETUP_USE_ETHERNET=false
SETUP_ENROLL_XODUS=true
SETUP_XODUS_NAME="Stratara Living Room"
SETUP_XODUS_TYPE="living-room-shell"
SETUP_DISPLAY_MODE="1920x1080"

if [[ -n "$DATA_FILE" && -f "$DATA_FILE" ]]; then
    log_info "Loading setup data from $DATA_FILE"
    if command -v jq &> /dev/null; then
        SETUP_HOSTNAME=$(jq -r '.hostname // ""' "$DATA_FILE")
        SETUP_USERNAME=$(jq -r '.username // "stratara"' "$DATA_FILE")
        SETUP_PASSWORD=$(jq -r '.password // ""' "$DATA_FILE")
        SETUP_TIMEZONE=$(jq -r '.timezone // "UTC"' "$DATA_FILE")
        SETUP_LOCALE=$(jq -r '.locale // "en_US.UTF-8"' "$DATA_FILE")
        SETUP_KEYBOARD=$(jq -r '.keyboardLayout // "us"' "$DATA_FILE")
        SETUP_WIFI_SSID=$(jq -r '.wifiSsid // ""' "$DATA_FILE")
        SETUP_WIFI_PASSWORD=$(jq -r '.wifiPassword // ""' "$DATA_FILE")
        SETUP_USE_ETHERNET=$(jq -r '.useEthernet // false' "$DATA_FILE")
        SETUP_ENROLL_XODUS=$(jq -r '.enrollXodus // true' "$DATA_FILE")
        SETUP_XODUS_NAME=$(jq -r '.xodusName // "Stratara Living Room"' "$DATA_FILE")
        SETUP_XODUS_TYPE=$(jq -r '.xodusType // "living-room-shell"' "$DATA_FILE")
        SETUP_DISPLAY_MODE=$(jq -r '.displayMode // "1920x1080"' "$DATA_FILE")
    else
        log_warning "jq not found, using defaults"
    fi
fi

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

# Set hostname
if [[ -n "$SETUP_HOSTNAME" ]]; then
    HOSTNAME="$SETUP_HOSTNAME"
else
    HOSTNAME="stratara-$(cat /etc/machine-id | cut -c1-8)"
fi
log_info "Setting hostname to: $HOSTNAME"
hostnamectl set-hostname "$HOSTNAME"

# Set up timezone
log_info "Setting timezone to: $SETUP_TIMEZONE"
timedatectl set-timezone "$SETUP_TIMEZONE"

# Set up locale
log_info "Setting locale to: $SETUP_LOCALE"
localectl set-locale LANG="$SETUP_LOCALE"

# Set up keyboard
log_info "Setting keyboard layout to: $SETUP_KEYBOARD"
localectl set-keymap "$SETUP_KEYBOARD"

# Configure NetworkManager
log_info "Configuring NetworkManager..."
systemctl enable NetworkManager.service
systemctl start NetworkManager.service

# Wait for NetworkManager to be ready
sleep 2

# Connect to WiFi if provided
if [[ "$SETUP_USE_ETHERNET" != "true" && -n "$SETUP_WIFI_SSID" && -n "$SETUP_WIFI_PASSWORD" ]]; then
    log_info "Connecting to WiFi: $SETUP_WIFI_SSID"
    nmcli device wifi connect "$SETUP_WIFI_SSID" password "$SETUP_WIFI_PASSWORD" || log_warning "WiFi connection failed, will retry later"
fi

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

# Enroll with Xodus if enabled
if [[ "$SETUP_ENROLL_XODUS" == "true" ]]; then
    log_info "Enrolling with Xodus as: $SETUP_XODUS_NAME ($SETUP_XODUS_TYPE)"
    if command -v xodusctl &> /dev/null; then
        xodusctl enroll --name "$SETUP_XODUS_NAME" --type "$SETUP_XODUS_TYPE" || log_warning "Xodus enrollment failed, will retry on next boot"
    else
        log_warning "xodusctl not found, skipping Xodus enrollment"
    fi
fi

# Create user with custom username
if ! id "$SETUP_USERNAME" &>/dev/null; then
    log_info "Creating user: $SETUP_USERNAME"
    useradd -m -G wheel,audio,video,network,input,kvm,render -s /bin/bash "$SETUP_USERNAME"
    
    # Set password if provided
    if [[ -n "$SETUP_PASSWORD" ]]; then
        echo "$SETUP_USERNAME:$SETUP_PASSWORD" | chpasswd
        log_info "Password set for user $SETUP_USERNAME"
    fi
    
    # Set up auto-login for SDDM/GDM
    log_info "Configuring auto-login for $SETUP_USERNAME..."
    
    # For SDDM
    if [[ -f /etc/sddm.conf ]]; then
        mkdir -p /etc/sddm.conf.d
        cat > /etc/sddm.conf.d/autologin.conf << EOF
[Autologin]
User=$SETUP_USERNAME
Session=wayland
Relogin=false
EOF
    fi
    
    # For GDM
    if [[ -f /etc/gdm/custom.conf ]]; then
        sed -i "s/^#  AutomaticLoginEnable =.*/  AutomaticLoginEnable = true/" /etc/gdm/custom.conf
        sed -i "s/^#  AutomaticLogin =.*/  AutomaticLogin = $SETUP_USERNAME/" /etc/gdm/custom.conf
    fi
fi

# Set up Stratara user directories
log_info "Setting up Stratara user directories..."
mkdir -p "/home/$SETUP_USERNAME/.config/stratara"
mkdir -p "/home/$SETUP_USERNAME/.local/share/stratara"
mkdir -p "/home/$SETUP_USERNAME/.cache/stratara"

# Copy default config
if [[ -f /usr/share/stratara/default-config.ini ]]; then
    cp /usr/share/stratara/default-config.ini "/home/$SETUP_USERNAME/.config/stratara/config.ini"
    chown -R "$SETUP_USERNAME:$SETUP_USERNAME" "/home/$SETUP_USERNAME/.config/stratara"
fi

# Configure KWin for the user
log_info "Configuring KWin for user..."
mkdir -p "/home/$SETUP_USERNAME/.config"
if [[ -f /etc/kwin/kwinrc ]]; then
    cp /etc/kwin/kwinrc "/home/$SETUP_USERNAME/.config/kwinrc"
    chown "$SETUP_USERNAME:$SETUP_USERNAME" "/home/$SETUP_USERNAME/.config/kwinrc"
fi
if [[ -f /etc/kwin/windowrulesrc ]]; then
    cp /etc/kwin/windowrulesrc "/home/$SETUP_USERNAME/.config/windowrulesrc"
    chown "$SETUP_USERNAME:$SETUP_USERNAME" "/home/$SETUP_USERNAME/.config/windowrulesrc"
fi

# Enable Stratara user service
log_info "Enabling Stratara user service..."
systemctl --user --machine="$SETUP_USERNAME@" enable stratara.service 2>/dev/null || true

# Set up XDG autostart for Stratara
log_info "Setting up Stratara autostart..."
mkdir -p "/home/$SETUP_USERNAME/.config/autostart"
cat > "/home/$SETUP_USERNAME/.config/autostart/stratara.desktop" << EOF
[Desktop Entry]
Type=Application
Name=Stratara Shell
Exec=stratara
Hidden=false
NoDisplay=false
X-GNOME-Autostart-enabled=true
OnlyShowIn=KDE;XFCE;GNOME;
EOF
chown "$SETUP_USERNAME:$SETUP_USERNAME" "/home/$SETUP_USERNAME/.config/autostart/stratara.desktop"

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