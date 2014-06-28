Prairie Avenue Live - Controller
================================

Terminology
-----------

- Mote: Arduino Fio and XBee collects sound levels from mic and transmits to controller.
  - There are multiple interior motes with one mic each and one exterior mote with multiple mics.
- Mic: the electret mics used by the motes.
- Controller: Teensy 3.1 and XBee that receive the sound data from the motes and control the LEDs.
- Node: the origin point (on the LED array) of the visualization for a particular mic.

Repos
-----

 - [PrairieAveLive_Controller](https://github.com/balbano/PrairieAveLive_Controller)
 - [PrairieAveLive_InteriorMote](https://github.com/balbano/PrairieAveLive_InteriorMote)
 - [PrairieAveLive_ExteriorMote](https://github.com/balbano/PrairieAveLive_ExteriorMote)
