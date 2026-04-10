# Components::EPS_Sim

Passive component that emulates satellite power over time.

## Usage Examples
The EPS Sim (Energy and Power Systems Simulator) is designed to internally store and update the simulated power level of the satellite.

### Diagrams
Add diagrams here

### Typical Usage
1. The component is declared and instantiated with a `delta` parameter. It begins with full charge and its operating mode set to `OFF`.
2. The component has its operating mode set to `DRAINING` or `CHARGING`.
3. The scheduler calls the `run` port at regular intervals.
4. On each run call, the component increases or decreases its internal power variable by `delta`, dependent on its operating mode.
5. The flight computer may query the EPS Sim to fetch the current power level using a pointer.

## Class Diagram
Add a class diagram here

## Port Descriptions
| Name       | Port Datatype   | Direction | Kind | Usage                                          |
|------------|-----------------|-----------|------|------------------------------------------------|
| run        | Scheduler       | input     | sync | Scheduler port that triggers power updating    |
| getBattery | BatteryLevelGet | input     | sync | Port for sending power level to command caller |

## Component States
Add component states in the chart below
| Name     | Description                             |
|----------|-----------------------------------------|
| OFF      | The EPS Sim is disabled                 |
| DRAINING | The EPS Sim is draining the power level |
| CHARGING | The EPS Sim is charging the power level |

## Sequence Diagrams
Add sequence diagrams here

## Parameters
| Name  | Description                                                 |
|-------|-------------------------------------------------------------|
| delta | The rate at which the power level changes per call of `run` |

## Commands
| Name                      | Description                                                                     |
|---------------------------|---------------------------------------------------------------------------------|
| SET_EPS_SIM_OPMODE        | Sets the operating mode of the simulation, requires an operating mode parameter |
| SET_EPS_SIM_BATTERY_LEVEL | Sets the power level of the simulation directly to the provided value           |

## Events
| Name             | Severity     | Description                       |
|------------------|--------------|-----------------------------------|
| EPS_SimSetOpMode | WARNING_HIGH | Logs the change in operating mode |

## Telemetry
| Name | Description |
|---|---|
|---|---|

## Unit Tests
Add unit test descriptions in the chart below
| Name | Description | Output | Coverage |
|---|---|---|---|
|---|---|---|---|

## Requirements
Add requirements in the chart below
| Name | Description | Validation |
|---|---|---|
|---|---|---|

## Change Log
| Date    | Description               |
|---------|---------------------------|
|2026-4-10| Initial EPS Sim Component |