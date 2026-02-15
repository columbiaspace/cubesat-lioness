# LionessSw::WatchdogEmulator

Host-side watchdog timer emulator used to test watchdog-kick behavior before flight hardware is available.

## Purpose
The component models a simple hardware watchdog:
- a countdown timer starts at a fixed timeout
- a watchdog kick resets the timer
- if the timer reaches zero, the component emits a reset signal

This allows mission software integration and fault-path testing without a physical watchdog board.

## Typical Usage
1. Connect watchdog producer output to `kickIn`.
2. Connect a periodic scheduler output to `schedIn`.
3. Connect `resetOut` to a reset handler in the deployment.
4. Run at a known scheduler rate and interpret `TIMEOUT_TICKS` against that rate.

Example: if `schedIn` is called at 10 Hz and timeout is 50 ticks, reset is asserted after 5 seconds without a kick.

## Port Descriptions
| Name | Kind | Description |
|---|---|---|
| `kickIn` | `sync input` (`Svc.WatchDog`) | Watchdog kick/stroke input. Resets the countdown timer. |
| `schedIn` | `sync input` (`Svc.Sched`) | Periodic tick used to decrement countdown. |
| `resetOut` | `output` (`Fw.Signal`) | Emitted once when countdown reaches zero. |
| `timeCaller` | `time get` | Standard framework time port (not required for core countdown logic). |

## Behavioral Design
### Initialization
- `m_countdownTicks` is set to `TIMEOUT_TICKS` (`50`).
- `m_resetAsserted` is set to `false`.

### Kick Handling (`kickIn_handler`)
- Any kick resets `m_countdownTicks` to `TIMEOUT_TICKS`.
- `m_resetAsserted` is cleared to `false`.

### Scheduler Handling (`schedIn_handler`)
- On each scheduler call, `m_countdownTicks` is decremented if greater than zero.
- When countdown reaches zero, `resetOut` is emitted once and `m_resetAsserted` is set to `true`.
- Additional scheduler ticks at zero do not repeatedly emit reset until a new kick occurs.

## Component States
| State | Description |
|---|---|
| `CountingDown` | Countdown is greater than zero; component is waiting for next kick or expiry. |
| `Expired` | Countdown is zero and reset has been emitted once. |

State transitions:
- `CountingDown -> Expired`: no kick arrives before countdown reaches zero.
- `Expired -> CountingDown`: a kick is received.

## Timing Model
`TIMEOUT_TICKS` is compile-time fixed in implementation (`50` ticks).  
Absolute timeout duration depends on how often `schedIn` is called:

`timeout_seconds = TIMEOUT_TICKS / scheduler_rate_hz`

## Commands, Events, Telemetry, Parameters
This initial emulator intentionally defines no commands, events, telemetry channels, or runtime parameters.

## Unit Test Plan
| Test | Description | Expected Result | Coverage |
|---|---|---|---|

## Requirements
| ID | Requirement | Validation |
|---|---|---|
| `WDE-001` | The emulator shall maintain a countdown timer initialized to a predetermined timeout value. | Unit test + code inspection |
| `WDE-002` | Receipt of a watchdog kick shall reset the countdown timer. | Unit test |
| `WDE-003` | If the countdown reaches zero, the emulator shall emit a system reset signal. | Unit test |
| `WDE-004` | Reset signal emission shall be one-shot per expiry until re-kicked. | Unit test |

## Change Log
| Date | Description |
|---|---|
| 2026-02-15 | Initial SDD authored for watchdog emulator component. |
