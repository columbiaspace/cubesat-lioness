# Camera::camera

Passive component that handles camera specific payload capabilities.

Taking and Saving Images

## Usage Examples
The camera handler can be commanded to take an image. It will take the image and save it to a file. 

### Diagrams
Add diagrams here

### Typical Usage
Prior to taking a picture, the payload power loadswitch must be activated. Then "PING" the camera with the ping command. If the PING command returns successfully, then the camera is ready to take an image.

## Class Diagram
Add a class diagram here

## Port Descriptions
| Name | Description |


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
|TAKE_IMAGE	|Send "snap" command to the payload com component|
|SET_CONTINUOUS|Boolean that enables or disables continuous photo taking|

## Events
| Name | Description |
|SaveSucessful|Confirm that image was sucessfully taken and saved|
|FileWriteError|Error when writing file|

## Telemetry
| Name | Description |
|ImagesSaved|Number of images saved|
|FileErrorCount||

## Unit Tests
Add unit test descriptions in the chart below
| Name | Description | Output | Coverage |
|---|---|---|---|
|---|---|---|---|

## Requirements
Add requirements in the chart below
| Name | Description | Validation |
|Camera-001|The Camera has a command to take an image.|Manual Test|
|Camera-002|The Camera saves image data bytes to a new file.|Manual Test|

## Change Log
| Date | Description |
|---|---|
|---| Initial Draft |