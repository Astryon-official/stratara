# Stratara OS ISO Build

This directory contains the configuration and scripts for building the Stratara OS bootable ISO image.

## Build Methods

### Kiwi (openSUSE/SUSE)
Kiwi is the native image builder for openSUSE/SUSE distributions.

**Requirements:**
- kiwi-ng
- zypper
- qemu (for testing)

**Usage:**
```bash
sudo ./build-iso.sh kiwi
```

### Lorax (Fedora/RHEL)
Lorax is the image builder used by Fedora and RHEL.

**Requirements:**
- lorax
- dnf (or zypper with dnf plugin)
- qemu (for testing)

**Usage:**
```bash
sudo ./build-iso.sh lorax
```

## Configuration Files

| File | Purpose |
|------|---------|
| `kiwi/stratara-kiwi.xml` | Kiwi image configuration |
| `lorax/stratara-lorax.conf` | Lorax image configuration |
| `kernel/cmdline.txt` | Kernel command line parameters |
| `kernel/dracut-stratara.conf` | Dracut initramfs configuration |
| `secure-boot-setup.sh` | Secure Boot enrollment script |
| `build-iso.sh` | Main build script |

## Build Output

The build script creates:
- `Stratara-OS-0.1.0-x86_64.iso` - Bootable ISO image
- `Stratara-OS-0.1.0-x86_64.iso.sha256` - SHA256 checksum

## ISO Contents

The ISO includes:
- Stratara OS base system
- KWin Wayland compositor
- Plasma workspace
- Calamares installer with Stratara branding
- Xodus system service
- Stratara shell application
- GRUB theme with Stratara branding
- Secure Boot support (shim + MOK)
- OEM first-boot setup

## Testing the ISO

### QEMU/KVM
```bash
# UEFI boot with Secure Boot
qemu-system-x86_64 \
    -machine q35,accel=kvm \
    -cpu host \
    -m 4G \
    -bios /usr/share/ovmf/OVMF.fd \
    -drive file=Stratara-OS-0.1.0-x86_64.iso,format=raw,if=virtio,media=cdrom \
    -drive file=test-disk.qcow2,format=qcow2,if=virtio \
    -netdev user,id=net0 \
    -device virtio-net-pci,netdev=net0 \
    -vga virtio \
    -display gtk,gl=on

# Legacy BIOS boot
qemu-system-x86_64 \
    -machine pc,accel=kvm \
    -cpu host \
    -m 4G \
    -drive file=Stratara-OS-0.1.0-x86_64.iso,format=raw,if=virtio,media=cdrom \
    -drive file=test-disk.qcow2,format=qcow2,if=virtio \
    -netdev user,id=net0 \
    -device virtio-net-pci,netdev=net0 \
    -vga std
```

### VirtualBox
1. Create new VM with Linux 64-bit
2. Enable EFI in System → Motherboard
3. Attach ISO to optical drive
4. Start VM

### VMware
1. Create new VM with Linux 64-bit
2. Enable UEFI in Options → Advanced
4. Attach ISO to CD/DVD
5. Start VM

## Secure Boot

The ISO supports Secure Boot via shim:
1. Shim is installed as `BOOTX64.EFI`
2. MOK (Machine Owner Key) is enrolled on first boot
3. User must confirm MOK enrollment in shim blue screen
4. After enrollment, GRUB loads with Secure Boot enabled

## Kernel Parameters

The ISO uses these kernel parameters:
- `quiet` - Suppress kernel messages
- `splash=silent` - Silent splash screen
- `vt.global_cursor_default=0` - Hide cursor
- `loglevel=3` - Only errors/warnings
- `systemd.show_status=auto` - Auto status display
- `rd.udev.log_level=3` - Reduce initrd udev logging
- `module.sig_enforce=1` - Enforce module signatures
- `lock_down=1` - Lockdown mode for Secure Boot

## Dracut Configuration

The initramfs includes:
- Systemd modules
- Network support
- Crypt/LVM support
- DRM/KMS drivers
- Graphics drivers (amdgpu, i915, nouveau)
- Network drivers
- Input drivers
- Filesystem modules (ext4, btrfs, xfs, f2fs, vfat, ntfs3)

## GRUB Theme

The GRUB theme includes:
- Stratara branded background (1920x1080)
- Custom progress bar with glow effect
- 5-second timeout
- 1920x1080 resolution
- Stratara "S" logo with signal waves

## Wayland Session

The ISO provides a Wayland session:
- Session file: `/usr/share/wayland-sessions/stratara.desktop`
- Executes: `stratara`
- KWin Wayland compositor
- QtVirtualKeyboard for on-screen input

## OEM Installation

For OEM deployment:
1. Boot ISO in OEM mode
2. Install to target disk
3. On first boot, `oem-setup.sh` runs:
   - Generates machine ID
   - Sets hostname
   - Configures locale/keyboard/timezone
   - Creates default user with auto-login
   - Enrolls with Xodus
   - Configures display
   - Sets up autostart

## Requirements

### Build Host
- Root access (for loop devices, chroot)
- 20GB free disk space
- 4GB RAM minimum
- x86_64 architecture
- UEFI firmware (for Secure Boot testing)

### Target Hardware
- x86_64 CPU (Intel/AMD)
- 2GB RAM minimum (4GB recommended)
- 8GB storage minimum (20GB recommended)
- GPU with DRM/KMS support (Intel/AMD/NVIDIA)
- UEFI firmware (recommended)
- Network (WiFi/Ethernet) for setup

## Troubleshooting

### Build fails with "No space left on device"
Increase tmpdir or build on larger partition:
```bash
export TMPDIR=/path/to/large/partition
sudo ./build-iso.sh kiwi
```

### ISO doesn't boot
- Check UEFI vs BIOS mode matches
- Verify Secure Boot is disabled or MOK enrolled
- Try nomodeset kernel parameter

### Secure Boot fails
- Disable Secure Boot in firmware
- Or enroll MOK manually on first boot
- Check shim/mokutil versions

### Calamares doesn't start
- Check Wayland session is available
- Verify Stratara shell is in PATH
- Check Qt/Plasma dependencies

## Customization

To customize the ISO:
1. Edit `kiwi/stratara-kiwi.xml` or `lorax/stratara-lorax.conf`
2. Modify package lists
3. Change kernel parameters in `kernel/cmdline.txt`
4. Update GRUB theme in `../grub/themes/stratara/`
5. Modify Calamares branding in `../calamares/branding/stratara/`

## License

Stratara OS is licensed under GPL-3.0-or-later.
See LICENSE file in project root.