# Trainer Fault‑Code Diagnostic Gateway

A complete ESP32‑based **BLE/UART -> CAN -> Trainer** for extracting trainer diagnostic information (DTCs, live PIDs, etc.) and forwarding it to a website/display.

---

## 1. Quick‑Start (TL;DR)

```bash
# 1. Get the ESP‑IDF tool‑chain (≥ v5.1)
$ git clone https://github.com/espressif/esp-idf.git
$ cd esp-idf && ./install.sh esp32 && . export.sh

# 2. Pull the project & enter it
$ git clone <this‑repo>
$ cd trainer‑gateway

# 3. Configure your board (device name, GPIOs, …)
$ idf.py set-target esp32
$ idf.py menuconfig            # → Component config ▸ Trainer Gateway

# 4. Build, flash & monitor
$ idf.py build flash monitor   # UART @ 115200
```

 **Hardware hook‑up**: connect a **MCP2651** (or equivalent) CAN transceiver to the ESP32 – *TX* on GPIO21, *RX* on GPIO22 (defaults, configurable via *menuconfig*). Power the board from USB. Additionally, disable all Bluetooth classic related config and enable BLE only in menuconfig. Add this to Kconfig.projbuild:

```
menu "Trainer-Gateway"

config TRAINER_DEVICE_NAME
    string "BLE GAP device name"
    default "Trainer-01"
    help
        Advertised name shown in BLE scans (change per board or via NVS).

endmenu
```

---

## 2. Repository Layout (todo)

```
.
├── components/
│   ├── ble/        – BLE GATT service (files: ble.c/.h)
│   ├── twai/       – CAN (TWAI) driver & OBD‑II state‑machine (twai_tc.c, twai_obd.h)
│   ├── uart/       – UART framing & display protocol (uart_tc.c, UART_TC.h)
│   └── common/     – Shared types & helpers (tc_ref.h)
├── main/
│   └── main.c      – RTOS bootstrap & high‑level orchestration
└── README.md       – you’re here
```

---

## 3. Configuration Options

All knobs live under `` in *menuconfig*.

| Symbol                          | Default       | Description                                                                |
| ------------------------------- | ------------- | -------------------------------------------------------------------------- |
| `CONFIG_TRAINER_DEVICE_NAME`    | `"PowerSeat"` | BLE device name advertised to the app                                      |
| `CONFIG_EXAMPLE_UART_PORT_NUM`  | `1`           | UART peripheral used for the HMI display                                   |
| `CONFIG_EXAMPLE_UART_TXD / RXD` | `17 / 16`     | Pins for UART link                                                         |
| CAN GPIOs                       | 21 / 22       | Hard‑wired in `twai_tc.c`; override by editing `TX_GPIO_NUM`/`RX_GPIO_NUM` |

After changing any value run `idf.py build` again.

---


1. **BLE** (`ble.c`)

   - Exposes a 16‑bit GATT service `0xABCD` with three characteristics (important):
     - **TX** `0xAB01` – *Write No‑Rsp*: commands from the app .
     - **RX** `0xAB02` – *Notify/Read*: streamed data back to the app.
     - **CTRL** `0xAB03` – version/info.
   - Converts incoming bytes into high‑level requests (`ble_req_t`) and pushes them onto `ble_queue`.
   - Sends notifications (`BLE_push_dtcs`, `BLE_push_buf`) as CAN data arrives.

2. **TWAI / OBD‑II** (`twai_tc.c`)

   - Implements a state‑machine for ISO‑TP framing over CAN.
   - Three FreeRTOS tasks:
     - **RX** – parses ECU responses, assembles multi‑frame messages.
     - **TX** – issues requests and flow‑control frames.
     - **Services** – in/out interface that mirrors OBD service IDs (`SERV_*`).
   - Hardware abstraction: CAN is handled through ESP‑IDF’s **TWAI driver**.

3. **UART** (`uart_tc.c`)

   - Proprietary framed binary protocol for a simple TFT display.
   - Two tasks (**RX**/ **TX**) sharing `uart_send_queue`.
   - Capable of streaming live PIDs as well as DTC lists.

4. **main.c**

   - Boots NVS, instantiates queues/semaphores, and calls `BLE_init()` & `TWAI_INIT()`.
   - Translates BLE/UART requests into TWAI services via `Set_TWAI_Serv()`.

---

## 4. Workflows

### 4.1 Fetching Trouble Codes (For BLE)

| Step | UI   | API                          | Notes                                    |
| ---- | ------- | ---------------------------- | ---------------------------------------- |
| 1    | Web     | Write `0x01/02/03` to **TX** | Pending/Stored/Perm DTCs                 |
| 2    | Gateway | TWAI → ECU                   | Builds Dtc request (service 03/07/0A) |
| 3    | ECU     | CAN                          | Returns ISO‑TP‑framed payload            |
| 4    | Gateway | BLE `BLE_push_dtcs()`        | Sends 0‑N notifications, 20‑byte chunks  |

### 4.2 Live PID Streaming

1. App writes `0x06` (+ PID bitmask request).
2. Gateway replies with 7×4‑byte bitmask rows.
3. App enables streaming; Gateway keeps polling supported PIDs round‑robin every 100 ms.

---

## 5. Extending the System

- **Add a new OBD service**

  1. Define a new `SERV_*` value in `tc_ref.h`.
  2. Handle it in `ble_service_task()` and `TWAI_Services()`.
  3. Implement request/response parsing in `twai_tc.c`.

- **Support longer BLE packets** – bump MTU (already set to 247) and split payload accordingly.

---

## 6. Logging & Troubleshooting

| Module | Tag            | Verbosity                                |
| ------ | -------------- | ---------------------------------------- |
| BLE    | `PowerSeat`    | `esp_log_level_set(TAG, ESP_LOG_DEBUG);` |
| TWAI   | `Gateway TWAI` | Lots of `ESP_LOGI` + frame dumps         |
| UART   | `UART Service` | Frame validity checks                    |

---

## 7. Known Limitations

- UART protocol parity is **even**;
- Limited PIDs implemented.

---


