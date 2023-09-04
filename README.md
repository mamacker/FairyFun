# FairyFun

Project creating a bit of fun around detecting "near" fingers and lighting up LEDs. Feels like magic.
Based on the Seeed Studio XIAO SAMD21 or the Adafruit QT Py - SAMD21 Dev Board with STEMMA QT.

## Novel bit

The touch capacitance is variable due to a variety of factors, including the size of the touch pad, the material, the moisture content of the air, etc. To reliably detect the proximity of a finger for the magic effect - we need to "learn" the baseline. This code incorporates that learning using a simple averaging over time.

Also, note that detecting fingers "near" works through some materials, like wood, plastic, and glass. It does not work through metal... unless the metal is incontact with the detector. This provides lots of magical potential.

## Hardware

This uses a little "antenna" off of pin A0, where the antenna can be soldered to anything conductive to create a "touch" or "nearness detector" area. I've done it with jewelry boxes, and nickle painted 3D prints.

## Why?

This is a fun project for those studying computer science because it combines a number of different concepts:

- C++ programming
- Microcontroller programming
- Hardware interfacing
- Capacitive touch
- LED control
- Serial communication
- Debugging

It also allows the creator to incorporate a little magic. One could imagine using this to create a hidden lock, or a cute night light.
