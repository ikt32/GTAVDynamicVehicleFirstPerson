# Changelog

## 1.0.0

Initial release after splitting off from Manual Transmission.

Changes since Manual Transmission 5.5.1:

Camera management:

* The "Vehicle1", "Vehicle2" and "Driver Head" cameras, and their "Bike" duplicates, are replaced by a system
with multiple user-addable cameras per vehicle configuration, with "Vehicle" and "Driver Head" mount point being a
property of the Camera.
* Cameras in a single configuration are now easier to switch between, in the main menu.

Camera functionality:

* Add camera shake from vehicle speed (configurable, enabled by default)
* Add camera shake from terrain (configurable, enabled by default)
* Add depth of field effect (Requires Very High PostFX, disabled by default)
* Add vertical and lateral movement in response to physics
* "Lean" (how far to move the camera while looking back) is now configurable
* Accessories (helmets, hats, glasses) are now removed when entering FPV and are restored when switching back
* Fix horizon lock for looking backwards
* Fix horizon lock jumpy behavior while rolled over
* Fix vehicles lacking seat bones non-functional vehicle mount
* Fix FPV visual glitches: Traffic lights, rain particles, etc.
* Fix FPV audio: Now uses normal FPV audio
