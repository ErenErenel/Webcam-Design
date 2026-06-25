# Custom Embedded Webcam (SAM4S8B + OV2640 + ESP32)

A custom webcam system that captures JPEG images on a SAM4S8B microcontroller and
streams them to a browser over Wi-Fi via an ESP32 module. This repository contains
both the **PCB hardware design** and the **microcontroller firmware**.

Developed for **EE 326: Electronic System Design 1** at Northwestern University.

## System Overview

```
 OV2640 camera ──(parallel + I²C)──► SAM4S8B MCU ──(UART + SPI)──► ESP32 Wi-Fi ──► Browser
```

- The **OV2640** captures JPEG frames at 320×240, clocked into the MCU over an
  8-bit parallel interface using the SAM4S Peripheral DMA Controller (PDC).
  Camera registers are configured over I²C (TWI) at 400 kHz.
- The **SAM4S8B** (Cortex-M4) initializes the camera, detects JPEG frame length,
  and pushes image data to the ESP32 over SPI.
- The **ESP32-WROOM-32E** runs Wi-Fi provisioning, hosts a websocket server, and
  serves the streamed images to connected web clients. The MCU controls it through
  UART commands and monitors network/client status over GPIO.

## Repository Structure

```
Webcam-Design/
├── Hardware/      KiCad PCB design (schematic, layout, design rules)
│   ├── Ereno2.kicad_sch     Schematic
│   ├── Ereno2.kicad_pcb     Board layout
│   ├── Ereno2.kicad_pro     KiCad project
│   └── Ereno2.kicad_dru     Custom design rules
└── Firmware/      Atmel Studio (ASF) firmware for the SAM4S8B
    └── ErenFirmware/
        └── ErenFirmware/src/
            ├── main.c                  Boot, peripheral init, stream loop
            ├── camera.c / camera.h     OV2640 capture (PDC, JPEG framing)
            ├── ov2640.c / ov2640.h     OV2640 register driver + register tables
            ├── wifi.c / wifi.h         ESP32 UART command + SPI image transfer
            ├── timer_interface.c/.h    Timer/counter setup
            └── config/                 Board, clock, and UART configuration
```

## Firmware Flow

1. **Init** — system clock, watchdog disable, I/O ports, timer, USART, SPI, camera.
2. **ESP32 setup** — hard-reset the ESP32, then assign its GPIO roles
   (comm, network status, client status, provisioning, indicator LEDs) and SPI baud
   rate via UART commands.
3. **Connect** — wait for the ESP32 to join a network, handling provisioning requests
   from the on-board setup button; retry with `test`/reset until the link reports `SUCCESS`.
4. **Stream** — while a network and at least one client are connected, capture a frame
   and transfer it to the ESP32 for web display.

## Building the Firmware

The firmware targets the **ATSAM4S8B** and is built with **Microchip (Atmel) Studio 7**
using the Advanced Software Framework (ASF).

1. Open `Firmware/ErenFirmware/ErenFirmware.atsln` in Microchip Studio.
2. Build (`F7`) — output is generated in `Debug/` (ignored by git).
3. Flash to the board using a SAM-compatible programmer/debugger.

## Hardware

Open `Hardware/Ereno2.kicad_pro` in **KiCad 7+** to view or edit the schematic and
board layout. Key components: SAM4S8B MCU, OV2640 camera module, and an
ESP32-WROOM-32E-N4 Wi-Fi module.

## License

Released under the [MIT License](LICENSE).
