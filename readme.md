# Otter Firmware code

### Otter is an affordable conductive warming bassinet. This repository contains firmware code for the hardware.

### How to upload firmware to Otter.

Master branch contains most up-to-date Otter code for full system. Otter should work when `integratedOtter.ino` file is uploaded to the arduino on Otter, given that all hardware systems (UI, heating element) are functional.

### Branches

This repository is organized into different branches that have slightly different versions of Otter code for different purposes. Following is the list of important branches to note:

- `ADE_FinalCode` and `ADE_FinalCode_Pin10`: Legacy code from Summer 2017.
- `legacySpring2018`: Legacy code base from Spring 2018. Mirror of how this repository looked like in Spring 2018.
- `tests`: Different Otter codes used for testing
- `IntegratedOtter`: Current, most up-to-date Otter code for the functioning system.

### Note to future developers
- Currently, hard-coded version number is displayed on the UI (in `integratedOtter.ino`). This means that this version number should be incremented manually after there has been substantial update to the code.
- In the future, it would be necessary to come up with better version tracking system. 
