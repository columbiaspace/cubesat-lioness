# Camera::camera

Component that handles camera specific payload capabilities.

Taking Images

## Usage Examples
Component recieves call to an image and sends the image back to another component to be saved.

### Diagrams
Add diagrams here

### Typical Usage

## Class Diagram
Add a class diagram here

## Port Descriptions
Port Data Type | Name | Direction | Kind | Usage
-------------- | ---- | --------- | ---- | -----
TakePicturePort | run | input | synchronous | receives a call to take a picture
SendPicturePort | return | output | synchronous | returns the image buffer 


## Component States
| Name | Description |

## Sequence Diagrams
Add sequence diagrams here

## Parameters
| Name | Description |
|---|---|
|---|---|

## Commands
| Name | Description |

## Events
| Name | Description |
|ImageSucessful|Confirm that image was sucessfully taken|

## Telemetry
| Name | Description |
|ImagesTaken |Number of images taken|

## Unit Tests
Add unit test descriptions in the chart below
| Name | Description | Output | Coverage |
|---|---|---|---|
|---|---|---|---|

## Requirements
Add requirements in the chart below
| Name | Description | Validation |
|Camera-001|The Camera recieves call to take photo|
|Camera-002|The Camera returns the image through its output port|

## Change Log
| Date | Description |
|---|---|
| February 27 2026 | Initial Draft |
| March 6 2026 | Initial ommands, events, telemetry, and requirements | 
| March 27 2026 | run port |
| April 10 2026 | change the component functionality | 