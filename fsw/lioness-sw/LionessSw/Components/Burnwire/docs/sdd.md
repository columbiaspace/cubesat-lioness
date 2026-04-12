# LionessSw::Burnwire

Ignites grounded Burnwire

# Burnwire Component — SDD (Software Design Description)

---

## Requirements

| ID     | Requirement                                                                                       | Verification     |
| ------ | ------------------------------------------------------------------------------------------------- | ---------------- |
| BW-001 | The burnwire shall turn on and off in response to port calls |     |
| BW-002 | The burnwire shall turn on and off in response to commands             |    |
| BW-003 | The burnwire component shall provide an event when it is turned on and off                        |  |
| BW-004 | The burnwire component shall activate by turning both the GPIO pins that activate the burnwire    |  |
| BW-005 | The burnwire component shall be controlled by a safety timeout attached to a 1Hz rate group       ||
| BW-006 | The safety timeout shall emit an event when it is changed                                         |  |
| BW-007 | The burnwire safety timer shall emit an event when it starts and stops                            | |

---

## Ports

| Name      | Type           | Description                              |
| --------- | -------------- | ---------------------------------------- |
| burnStop  | Fw::Signal     | Receive stop signal to stop the burnwire |
| burnStart | Fw::Signal     | Receive start signal to start burnwire   |
| gpioSet   | Drv::GpioWrite | Control GPIO state to driver             |
| schedIn   | Svc::Sched     | Run scheduled execution (rate group)     |

---

## Commands

| Name           | Description         |
| -------------- | ------------------- |
| START_BURNWIRE | Starts the burnwire |
| STOP_BURNWIRE  | Stops the burnwire  |

---

## Events

| Name              | Description                                                           |
| ----------------- | --------------------------------------------------------------------- |
| SetBurnwireState  | Emits burnwire state when the burnwire turns on or off                |
| SafetyTimerStatus | Emits safety timer state when the safety timer has stopped or started |
| SafetyTimerState  | Emits the amount of time the safety timer will run for when it starts |
| BurnwireEndCount  | Reports how long the burnwire actually burned for                     |

---

## Component State

| Name    | Description                              |
| ------- | ---------------------------------------- |
| m_state | Keeps track if the burnwire is ON or OFF |

---

## Tests

| Name                            | Description                                                                                          | Output      | Coverage |
| ------------------------------- | ---------------------------------------------------------------------------------------------------- | ----------- | -------- |
| test_01_start_and_stop_burnwire | Tests burnwire functionality by enabling it and asserting that satellite power consumption increases | Integration | ---      |

---

## Parameters

| Name         | Description                                                                    |
| ------------ | ------------------------------------------------------------------------------ |
| SAFETY_TIMER | Default (set in FPP, currently 10) maximum time the burnwire is allowed to run |
