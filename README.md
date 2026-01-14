# TMS6100 milti-PHROM

This project contains the PCB design and firmware for a TMS6100 emulator, accommodating the following PHROM images:

* Acorn PHROMA (News presenter Kenneth Kendall's voice, Acorn specific format)
* VM61002 - the PHROM fitted to Acorn computer speech kits sold in the USA (American male voice, indexed phrases)
* VM10004 - military radio phrases (American male voice, non-indexed phrases)
* VM71003 - talking clock (American female voice, indexed phrases)

The PCB has the footprint of a DIP-28 and is intended to be plugged into the IC98 socket on the BBC Micro model A/B motherboard.

![PCB constructed](Images/V1_Top.JPG)

![PCB installed into computer](Images/Installed.JPG)

It is a fully-featured implementation, providing indirect addressing and multi-PHROM working.

The project is subdivided into multiple sections, stored in the following folders:

* PCB - The Kicad project for the PCB
* Firmware - The STM32CubeIDE project providing the firmware
* Scripts - Utilities for converting data
* BASIC2 - BBC BASIC programs for testing the multi-PHROM
* Images - Photos of the constructed PCB and installation

Each of these sections has their own README, providing more information.
