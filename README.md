# Barbell Power Meter (ESP32)

Turns the existing ESP32 motor/encoder rig into a **velocity-based training power meter**:

- a string runs from the spool to the barbell;
- the encoder measures string travel;
- the ESP32 converts travel into bar velocity;
- with the weight you enter, it calculates **concentric power in watts**;
- a phone-friendly website runs on the ESP32's own WiFi access point;
- sessions are saved to internal flash and can be exported as CSV.

The original PID position-control code is still in the project, but it is **not used** in training mode. Training mode only uses the motor for a light string take-up pull.

---

## Quick start

1. Build and upload:

   ```bash
   pio run -e esp32dev -t upload
   pio device monitor
   ```

   (Or use the PlatformIO buttons in CLion.)

2. On your phone, connect to WiFi:

   - **Network:** `BarbellPower`
   - **Password:** `barbell123`

3. Open **http://192.168.4.1** (or `http://barbell.local` if your phone supports mDNS). The captive-portal popup should also offer the page.

4. Open **Settings → 1 m calibration**:

   - Pull the string out **exactly 1.00 m** in the direction it moves when you **lower** the bar.
   - Press **Finish**.
   - The firmware learns both `counts per metre` and which encoder direction is "up".

5. Still in **Settings → String take-up**, press **Auto-detect take-up direction**. The motor runs for about half a second. Make sure the string can move freely and the bar is **not** attached.

6. Go to **Live**, enter the total moving weight (bar + plates), choose an exercise, and press **Start set**.

7. Do your reps. Press **Stop set**. The set summary appears and the raw data is saved.

8. Open the **History** tab to download raw CSV or the summary CSV.

---

## How the watts are calculated

For every sample:

```text
velocity (m/s) = change in string position / change in time
force (N)      = weight (kg) × 9.80665
power (W)      = force × velocity
```

Only the **lifting/concentric phase** produces positive watts. Eccentric velocity is shown on the chart but contributes 0 W, which is the standard convention for velocity-based training.

Per-rep and per-set values include:

| Metric | Meaning |
| --- | --- |
| Peak power | Highest instantaneous concentric power |
| Average power | Concentric work ÷ concentric time |
| Average velocity | Concentric distance ÷ concentric time |
| Peak velocity | Highest filtered concentric velocity |
| ROM | Largest top-to-bottom range across the set |
| Work | Concentric joules |
| TUT | Concentric time under tension |

---

## Rep detection

Reps are detected from position extrema and filtered velocity:

- movement must exceed the **velocity deadband**;
- a rep must exceed the **minimum rep ROM**;
- the concentric phase ends when the bar pauses or reverses.

Both thresholds are adjustable in **Settings → Rep detection**:

- raise the **deadband** if the take-up motor creates phantom reps;
- raise the **minimum ROM** if setup movements are counted.

The detector works for squats, bench, deadlift, presses, rows, etc., because it only looks at linear string travel. Start at the top or at the bottom; both are handled.

---

## String take-up motor

The motor applies a small PWM duty in the retract direction so slack is removed and the string spools back in. It runs whenever **Keep a light pull on the string** is enabled, not just during a set.

- Strength is adjustable from **0–100 %** in the UI. Internally this maps to a safe duty ceiling of 60/255, so 100 % is still only about 23 % PWM.
- Use the **Direction A / B** buttons or **Auto-detect** to make sure it pulls the string **in**, not out.
- If the string is fully spooled and the motor keeps pulling, it will stall and heat up. Disable take-up when the device is not in use, or keep the strength just high enough to remove slack.
- **Do not run auto-detect with the bar attached.** It deliberately runs the motor for half a second.

---

## Data storage and export

Sessions are stored in **LittleFS** on the ESP32's internal flash:

```text
/sessions/<id>.csv       raw samples
/sessions/<id>.json      session metadata and per-rep stats
/sessions/<id>.sum.csv   one-line summary used by the "Download summary CSV" button
```

`<id>` is derived from the phone's clock at session start (`YYYYMMDD_HHMMSS`), so sessions are dated correctly even though the ESP32 has no RTC.

### Raw CSV

```csv
time_ms,position_m,velocity_mps,power_w,phase,rep
0,0.0000,0.000,0.0,0,0
40,-0.0121,-0.301,0.0,1,0
...
```

`phase`: `0` idle, `1` eccentric, `2` concentric.

### Summary CSV

The History tab's **Download summary CSV** produces one row per completed session:

```csv
id,epoch,exercise,weight_kg,reps,duration_s,avg_power_w,peak_power_w,avg_velocity_mps,peak_velocity_mps,rom_cm,work_j,tut_s
```

### Limits

- A single session stops logging at 1 MB (roughly 15 minutes at 25 Hz).
- A session auto-stops after 30 minutes.
- If free flash drops below 120 KB, the oldest sessions are deleted automatically before a new session starts.
- The default partition table gives LittleFS about 1.4 MB. You can change `board_build.partitions` in `platformio.ini` if you need more history, but the app uses about 975 KB of the 1.28 MB app slot.

---

## Website / API

The page is a single self-contained HTML file served from flash. No internet, CDN, or extra library is required.

The phone talks to these endpoints:

| Endpoint | Purpose |
| --- | --- |
| `GET /api/status` | Live session, settings, calibration and storage status |
| `POST /api/session/start` | Start a set (`weight`, `exercise`, `epoch`) |
| `POST /api/session/stop` | Stop and save the set |
| `POST /api/settings` | Update take-up and rep-detection settings |
| `POST /api/calibrate/start` | Begin the 1 m pull |
| `POST /api/calibrate/finish` | Finish the 1 m pull |
| `POST /api/calibrate/cancel` | Cancel calibration |
| `POST /api/takeup/autodir` | Auto-detect take-up direction |
| `POST /api/zero` | Zero the current string position |
| `POST /api/flip-direction` | Flip which encoder direction is "up" |
| `GET /api/sessions` | List saved sessions (metadata JSON array) |
| `GET /api/sessions/download?id=...` | Download a raw session CSV |
| `GET /api/sessions/metadata?id=...` | Download one session's metadata JSON |
| `POST /api/sessions/delete` | Delete one session |
| `POST /api/sessions/deleteAll` | Delete all sessions |
| `GET /api/export/summary.csv` | Download all session summaries |
| `POST /api/reboot` | Reboot the ESP32 (saves an active set first) |

---

## Hardware assumptions

From the existing project:

| Signal | ESP32 pin |
| --- | --- |
| Encoder A | GPIO 13 |
| Encoder B | GPIO 18 |
| Motor A1 | GPIO 26 |
| Motor A2 | GPIO 25 |

Encoder counts per output revolution are taken from the existing gearing constant (`13 × 30613 / 1500 ≈ 265.3`). The actual **metres per count** comes from the 1 m calibration, so the spool diameter does not need to be known.

The encoder is read on the rising edge of A, with direction from B. `getPosition()` reads the 64-bit counter inside a critical section.

---

## Calibration notes

- Pull the string out **1.00 m**. The calibration uses the absolute count difference, so it does not matter which way the spool is wound.
- The direction of that pull is treated as "lowering the bar". Lifting is the opposite direction.
- If lifting shows as negative velocity, use **Settings → Flip lift direction**, then re-run **Auto-detect take-up direction**.
- String build-up on the spool changes the effective circumference slightly. A 1 m calibration is an average; for long travel, recalibrate if you notice a consistent scale error.
- Calibration and settings are stored in NVS and survive reboots.

---

## Troubleshooting

**Phone says "connected, no internet"**
: That is expected — the ESP32 access point has no internet. Choose "Keep connection" / stay on `BarbellPower`. The captive-portal prompt should open the page automatically.

**Cannot open 192.168.4.1**
: Make sure the phone is still on `BarbellPower`, not cellular. Try `http://barbell.local` or re-enable WiFi.

**Power reads 0 while lifting**
: The encoder direction is probably inverted. Do **Flip lift direction**, then re-run auto-detect for take-up. Also confirm the string actually moves with the bar.

**Watts are much too high/low**
: Re-do the 1 m calibration. Check that the weight is the **total moving weight**, not plates only.

**Phantom reps**
: Increase the velocity deadband and/or minimum rep ROM. Lower the take-up strength.

**String spools the wrong way**
: Use the other direction button, or run auto-detect with the bar detached.

**"Storage unavailable"**
: LittleFS could not mount. Check the partition table / flash the filesystem. The firmware still runs live, but sessions are not saved.

**Recordings stop early**
: The 1 MB per-session cap or the free-space guard was reached. Download and delete old sessions from History.

---

## Project layout

```text
src/main.cpp              setup/loop, WiFi + meter wiring
src/PowerMeter.cpp        sampling, velocity, power, rep detection, calibration
src/SessionRecorder.cpp   LittleFS CSV/JSON storage, listing, deletion
src/WebApp.cpp            AP, captive portal, REST API
src/WebPage.h             single-page mobile UI (HTML/CSS/JS, PROGMEM)
src/Settings.cpp          NVS-backed settings
src/MotorController.cpp   encoder ISR, PWM, position read
include/                  headers + AppConfig.h defaults
platformio.ini            esp32dev build, LittleFS partition
```

---

## Safety

This is a motorized device with a string under tension. Keep fingers, hair, and clothing away from the spool and string. Do not use the auto-detect feature with the bar attached. Use a weak take-up strength and disable it when the rig is unattended. The AP password is compiled into `include/AppConfig.h`; change it if you want to keep other people off the device.
