<div align="center">

# 🌌 Stratara

### A modern, tvOS-inspired Linux shell and operating system experience.

A controller-first, full-screen Linux interface inspired by modern tvOS, built around a clean and immersive living-room experience.

**Stratara is currently under active development.**

</div>

---

## ✨ What is Stratara?

Stratara is a **Linux-native operating system experience** designed around the living room.

It combines the flexibility and power of Linux with a polished, controller-first interface inspired by modern tvOS.

Instead of putting a traditional desktop in front of the user, Stratara aims to make the **shell itself the experience**.

```text
Linux
  ↓
KDE / KWin
  ↓
Stratara Shell
  ↓
Apps · Games · Media · Settings
```

The goal is simple:

> **A Linux computer that feels like a purpose-built modern TV operating system.**

---

## ✨ Design goals

* **Content first.** The interface should get out of the way when it isn't needed.
* **Controller first.** Every important part of the interface should be usable from a controller or remote.
* **Beautiful by default.** Large artwork, smooth motion, depth, blur and glass effects without unnecessary visual clutter.
* **Fast.** The shell should remain responsive on everything from low-power living-room PCs to high-end gaming systems.
* **Linux underneath.** Users still get the flexibility, applications, games and hardware support of Linux.
* **Modular.** System components should be replaceable without rebuilding the entire operating system.
* **Open.** Stratara is built as an open-source project by Astryon.

---

## 🎮 Home

The Stratara home screen is designed around the same principle as modern TV interfaces:

**the content is the interface.**

When idle, the UI should remain minimal and let the wallpaper or currently selected content take over the screen.

Navigation can reveal:

* Favorite applications
* Installed applications
* Games
* Media
* Recent applications
* System controls

Focus movement, transitions and animations are designed specifically for D-pad and controller navigation.

---

## 🪟 Liquid Glass

Stratara takes inspiration from the translucent, depth-based interfaces introduced in modern tvOS.

System surfaces can use:

* Frosted backgrounds
* Dynamic transparency
* Blur
* Refraction
* Depth
* Contextual lighting
* Smooth transitions

The implementation will be designed specifically for Linux hardware rather than relying on Android's rendering APIs.

---

## 🖼 Frame Art

Turn the display into a digital picture frame.

Frame Art will support:

* Local photo folders
* Individual images
* Wallpapers
* Artwork
* Slideshow playback
* Floating clock
* Configurable transitions
* Automatic idle activation

The goal is for the interface to completely disappear when the computer isn't being actively used.

---

## 🎛 Control Centre

Stratara will provide a dedicated system control surface for commonly used controls.

Planned controls include:

* Wi-Fi
* Bluetooth
* Audio
* Display
* Brightness
* Power
* Network
* Controller status
* Media playback
* User/session controls

Controls should appear contextually without completely taking over the current application.

---

## 🎮 Gaming

Stratara is designed to work as a normal Linux computer **and** a living-room gaming system.

The shell will be designed around:

* Xbox controllers
* PlayStation controllers
* Bluetooth remotes
* USB controllers
* Keyboard and mouse
* Steam
* Linux games
* Game launchers

Games should be able to run normally underneath the shell without requiring a separate operating system.

---

## 🖥 Desktop & Linux applications

Stratara is not intended to lock the user into a TV-only ecosystem.

Linux applications will remain normal Linux applications.

The shell will provide a TV-friendly way to launch and manage them while the underlying system remains a full Linux environment.

The long-term goal is to support switching between:

**Stratara Mode**

and

**Desktop Mode**

without requiring a separate installation.

---

## ⚙️ System architecture

Stratara is being developed as a shell and operating-system experience on top of established Linux technologies.

The current architectural direction is:

```text

│       Stratara Shell        │
├─────────────────────────────┤
│      KDE / Plasma / KWin    │
├─────────────────────────────┤
│      Linux system stack     │
├─────────────────────────────┤
│       Linux kernel          │
├─────────────────────────────┤
│          Hardware           │
└─────────────────────────────┘
```

This allows Stratara to focus on the user experience while relying on the mature Linux ecosystem for hardware, networking, audio, graphics and applications.

---

## 🛠 Development

Stratara is currently in **early development**.

The project originally started from the exploration of the Android TV launcher **Tarang**, whose tvOS-inspired interaction model provided inspiration for the project.

The Linux version is being substantially reworked into a native Linux shell rather than remaining an Android application.

### Planned technology

* Linux
* Wayland
* KDE / KWin
* Qt
* Qt Quick / QML
* KDE Frameworks
* PipeWire
* NetworkManager
* BlueZ
* systemd

The exact architecture is still evolving.

---

## 🚀 Development status

### Current

* [ ] Linux-native shell
* [ ] Basic full-screen home
* [ ] D-pad/controller navigation
* [ ] Linux application discovery
* [ ] Linux application launching
* [ ] Wallpaper system
* [ ] Basic system integration

### Planned

* [ ] App library
* [ ] Favorites
* [ ] Control Centre
* [ ] Settings
* [ ] Notifications
* [ ] Media controls
* [ ] Bluetooth controls
* [ ] Wi-Fi controls
* [ ] Power menu
* [ ] Frame Art
* [ ] Liquid Glass effects
* [ ] Gaming integration
* [ ] Desktop Mode
* [ ] Stratara installer
* [ ] Stratara OS image

---

## 🧭 Project direction

Stratara is intended to evolve from a Linux shell into a complete operating-system experience.

The long-term goal is:

```text
Install Stratara
       ↓
Boot
       ↓
Stratara Home
       ↓
Apps · Games · Media
       ↓
Everything just works
```

The user should never need to think about the underlying desktop environment unless they want to.

---

## 📜 Origin

Stratara's early UI direction was inspired by **Tarang**, an open-source tvOS-inspired Android TV launcher.

Tarang's original work remains credited to its respective authors.

Stratara is being developed as a separate **Linux-native project** and is not an Android TV launcher.

---

<div align="center">

### 🌌 Stratara

**A different kind of Linux experience.**

Built by **Astryon**.

</div>


<div align="center">
<sub><i>Tarang</i> (तरंग) — "wave." Built for the couch. 🛋️</sub>
</div>
