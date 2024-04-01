# ESP32 Watchfaces

## Overview
This project showcases the rendering of prebuilt watch faces on ESP32 microcontrollers.

[`Demo Video`](https://youtu.be/6jkek5LbmeU)

[![screenshot](watchfaces.png?raw=true "screenshot")](https://youtu.be/6jkek5LbmeU)


## Devices Used
- ESP32 Devkit with WaveShare 240x240 LCD
- ESP32 C3 mini with a 240x240 touch display

## Usage Instructions
To use this project, follow these steps:
1. Get some of the available [`watch faces here`](https://github.com/fbiego/watch-face-wearfit/blob/main/dials/HW21/README.md).
2. Upload the binary watch faces to the SPIFFS partition on your ESP32 microcontroller.

## Disclaimer
This project renders watch faces with randomized numbers and cannot accurately display the current time for practical use as a watch.

Checkout [`esp32-lvgl-watchface`](https://github.com/fbiego/esp32-lvgl-watchface) to see how these fully functional watchfaces are rendered with LVGL

#### Preview

| | | |
| -- | -- | -- |
| !["Analog"](https://raw.githubusercontent.com/fbiego/esp32-lvgl-watchface/main/src/faces/75_2_dial/watchface.png?raw=true "75_2_dial") | !["Shadow"](https://raw.githubusercontent.com/fbiego/esp32-lvgl-watchface/main/src/faces/34_2_dial/watchface.png?raw=true "34_2_dial") | !["Blue"](https://raw.githubusercontent.com/fbiego/esp32-lvgl-watchface/main/src/faces/79_2_dial/watchface.png?raw=true "79_2_dial") |
| !["Radar"](https://raw.githubusercontent.com/fbiego/esp32-lvgl-watchface/main/src/faces/radar/watchface.png?raw=true "radar") | !["Outline"](https://raw.githubusercontent.com/fbiego/esp32-lvgl-watchface/main/src/faces/116_2_dial/watchface.png?raw=true "116_2_dial") | !["Red"](https://raw.githubusercontent.com/fbiego/esp32-lvgl-watchface/main/src/faces/756_2_dial/watchface.png?raw=true "756_2_dial") |
| !["Tix"](https://raw.githubusercontent.com/fbiego/esp32-lvgl-watchface/main/src/faces/tix_resized/watchface.png?raw=true "tix_resized") | !["Pixel"](https://raw.githubusercontent.com/fbiego/esp32-lvgl-watchface/main/src/faces/pixel_resized/watchface.png?raw=true "pixel_resized") | !["Smart"](https://raw.githubusercontent.com/fbiego/esp32-lvgl-watchface/main/src/faces/smart_resized/watchface.png?raw=true "smart_resized") |
