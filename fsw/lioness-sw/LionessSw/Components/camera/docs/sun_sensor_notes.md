

# Sun Sensor Notes


# (Step 1) Determining the Physical Characteristics:




In general, there are three main categories of sun sensors [^1]:

* **Coarse Analogue Sun Sensors**: analog sun sensors that measure the current output, which is proportional to the cosine of the angle between the sun and the normal to a photocell.
    * **Analogue**: the output is a continuous electrical signal proportional to the intensity of the light hitting it.
* **Fine Analogue Sun Sensors**: fine sun sensors that use an aperture to create a sunspot on either a four-quadrant photodiode or a position-sensitive device.
* **Digital Sun Sensors**: sensors that operate based on integrating a 2-dimensional light sensor and signal processing, so as to discriminate between direct sunlight and reflected sunlight.


<br>

For a brief explanation of each design type, please visit Bolshakov et. al[^2].


## Sun Sensor Design Types
| Design        | Cost     | Accuracy    | FOV  | Power Consumption (mW) |Weight |
|---            |---       |---          |---   |---                     |---    |
| Photodiode    | $8       | 3-17°       | 360° | 0                      |  $    | 
| Solar Panel   | N/A      | ±3°         | 360° | $                      |  $    |  
| Quadrant      | $385     | 0.06-0.5°   | ±60° | $                      |  $    |
| Matrix        | $5       | 0.006-0.02° | ±60° | $                      |  $    |
| Array         | $4-N/A   | <0.1°       | ±60° | $                      |  $    |
| Digital Array | $4-N/A   | 0.33°       | 103° | $                      |  $    |

 * Note: A 360° FOV assumes placement of the respective sensor on each face of the cube.

 * Note: The Quadrant, Matrix, and Array-based designs are limited by the "window" of their aperture mask. 

 * Note: The Digital Array-based design has a total viewing range of 103° (represented as a ±51.5° sensor cone).

 * Note: The following reference provides a general breakdown of each design presented[^2].



<br>

## Cost

**Photodiode**: $1.29 per photodiode and 6 are required ($7.74)
([DigiKey](https://www.digikey.com/en/products/detail/vishay-semiconductor-opto-division/BPW34/1681149)) ([BPW34, BPW34S DataSheet](https://www.vishay.com/docs/81521/bpw34.pdf))



<br>

**Quadrant**: $384.76
([DigiKey](https://www.digikey.com/en/product-highlight/o/osi/qd7-0-sd-qd50-0-sd-position-sensing-detectors)) ([QD7-0-SD DataSheet](https://www.digikey.com/en/product-highlight/o/osi/qd7-0-sd-qd50-0-sd-position-sensing-detectors))

<br>
**Solar Panel**: If Solar Panel already exists, then no new sensors are possibly required for purchase. 

* "An alternative to dedicated photodiodes for light intensity detection
is to use solar panels, that are commonly found on even the smallest
satellites. The geometry of this design is very similar to the photodiode
based one. Each face of the satellite has one or more solar cells mounted.
Using a trigonometric relation, the relationship between the current
outputs and the incident angle of light can be established" [^2]. 

<br>

**Matrix**: $5.10
([JSumo](https://www.jsumo.com/ov7670-arduino-camera-module)) ([OV7670 DataSheet](https://www.openimpulse.com/blog/wp-content/uploads/wpsc/downloadables/OV7670-CMOS-VGA-Camera-Datasheet.pdf))


<br>

**Array & Digital Array**: The digital array was proposed in [^2]. Both operate similarly, but the Digital Array utilizes "a chirped pattern of slits, while using a digital readout of the photodiode arrays."

<br>

The MLX75306 Linear Optical Array used in the paper is now discontinued, but maybe the following is a possible alternative?

* ([Rochester](https://www.rocelec.com/part/tsl1401cl/01tRl00000FE30cIAD)) $4, ([TSL1401CL](https://www.mouser.com/datasheet/2/588/TSL1401CL-1214741.pdf) DataSheet)


<br>

## Power Consumption (Work in Progress)

**Photodiodes**: operating in (zero-bias) photovoltaic mode generate their own current and consume 0 mW of bus power[^3].

**Quadrant**:

**Solar Panel**: Near 0(?)

**Matrix**:

**Array**:

**Digital Array**:


<br>

## Building:
**Photodiodes**: Requires 6 separate sensors, one mounted onto each face of the CubeSat. 

**Quadrant**: 1-2 Sensors mounted on the primary pointing faces (Requires an aperture mask per sensor).

**Solar Panel**: 0 additional parts if our the solar cells are mounted on each face of the CubeSat. 

**Matrix**: 1-2 Sensors mounted on the primary pointing faces (CMOS/CCD Camera Module per sensor).

**Array**: 1-2 Sensors mounted on the primary pointing faces (For each sensor, two perpendicular linear photodiode arrays and an aperture mask).

**Digital Array**: 1-2 Sensors mounted on the primary pointing faces (Requirements in [^2]).





<br>

### Aside:
Depending on the design type, data is retrived through either analogue conversion (Photodiode, Quadrant, Solar Panel) or different conversion protocols for digital (Matrix, Array, Digital Array).

* For Analogue, the hardware generates an electrical current that must be passes through an ADC (Analog-to-Digital Converter) to be read as binary.

* Matrix sensors transmit images over I2C or Parallel (needs processing to determine the sun's position).

* Array sensors shift each pixel sequentially to ADC, returning an array of multi-bit numbers representing the intensity of the light across the sensors view (in grayscale).

* Digital Array sensors, as shown in [^2], return a bit-array over SPI(Serial Peripheral Interface). Less power consumption than Array.

<br><br>

# (Step 2) Determining the hardware interface
Once we know what hardware for the sun sensor we will need, we need to figure out how we are going to retreive its data. Determine how we can do this.



# References
[^1]: SatSearch, [_Sun sensors on the global marketplace for space_, Hywel](https://blog.satsearch.co/2020-02-12-sun-sensors-an-overview-of-systems-available-on-the-global-marketplace-for-space)

[^2]: [_Array-based digital sun-sensor design for CubeSat application_](https://resolver.ebsco.com/c/hvnjcg/result?sid=EBSCO:edselp&genre=article&issn=00945765&ISBN=&volume=178&issue=&date=20210101&spage=81&pages=81-88&title=Acta%20Astronautica&atitle=Array-based%20digital%20sun-sensor%20design%20for%20CubeSat%20application&aulast=Bolshakov%2C%20Konstantin&id=DOI:10.1016/j.actaastro.2020.08.005), Bolshakov et. al


[^3] RP-Photonics, [_Operation Modes of Photodiodes_, Dr. Paschotta](https://www.rp-photonics.com/photodiodes.html#:~:text=Photovoltaic%20mode:%20Here%2C%20one%20does,quite%20linearly%20with%20optical%20power.)




how are my notes?