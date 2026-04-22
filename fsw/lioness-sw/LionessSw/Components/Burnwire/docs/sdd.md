# LionessSw::Burnwire

# Burnwire Software Design Description (SDD)

## Overview
The Burnwire component controls a burnwire actuator via a GPIO driver. It supports command-based activation with a safety timeout enforced through a scheduled rate group. The component ensures the burnwire is automatically turned off after a specified duration.

---

## Requirements

| ID     | Requirement |
|--------|------------|
| BW-001 | The burnwire shall turn on and off in response to commands |
| BW-002 | The burnwire component shall provide an event when it is turned on and off |
| BW-003 | The burnwire component shall activate by driving the GPIO pin(s) controlling the burnwire |
| BW-004 | The burnwire component shall be controlled by a safety timeout driven by a scheduled rate group |
| BW-005 | The safety timeout shall emit an event when it is set |
| BW-006 | The burnwire shall report the total burn duration when it stops |

---

## Ports

| Name    | Type            | Description                              |
|---------|-----------------|------------------------------------------|
| gpioSet | Drv::GpioWrite  | Sends GPIO state to the GPIO driver      |
| schedIn | Svc::Sched      | Scheduled execution input (rate group)   |

---

## Commands

| Name           | Description                                 |
|----------------|---------------------------------------------|
| START_BURNWIRE | Starts the burnwire for a specified duration |
| STOP_BURNWIRE  | Stops the burnwire (optional, if implemented)|

---

## Events

| Name              | Description                                                   |
|-------------------|---------------------------------------------------------------|
| SetBurnwireState  | Emitted when burnwire turns ON or OFF                         |
| SafetyTimerState  | Emits the requested burn duration when burn starts            |
| BurnwireEndCount  | Reports the actual burn duration when burnwire stops          |

---

## Parameters

| Name         | Description                                              |
|--------------|----------------------------------------------------------|
| SAFETY_TIMER | Maximum allowed burn duration (seconds)                  |

---

## Component State

| Name      | Description                                      |
|-----------|--------------------------------------------------|
| m_state   | Tracks whether burnwire is ON or OFF              |
| m_timer   | Counts elapsed time (ticks/seconds)               |
| m_timeout | Maximum allowed burn duration (seconds)           |

---

## Behavior

1. **Command Handling**
   - `START_BURNWIRE` command:
     - Retrieves `SAFETY_TIMER` parameter
     - Clamps requested duration to maximum allowed value
     - Sets `m_timeout`
     - Resets `m_timer`
     - Sets burnwire state to ON
     - Emits `SafetyTimerState` event

2. **Scheduled Execution (`schedIn_handler`)**
   - Runs at a fixed rate (nominally 1 Hz)
   - If burnwire is ON:
     - Increment `m_timer`
     - On first tick:
       - Set GPIO HIGH
       - Emit `SetBurnwireState(ON)`
     - When `m_timer >= m_timeout`:
       - Set GPIO LOW
       - Emit `BurnwireEndCount`
       - Emit `SetBurnwireState(OFF)`
       - Reset state and timer

---

---

## Notes

- The component assumes a 1 Hz scheduling rate for correct timing behavior.
- GPIO control is abstracted through the `Drv::GpioWrite` interface.
- Safety timeout prevents unintended prolonged burnwire activation.