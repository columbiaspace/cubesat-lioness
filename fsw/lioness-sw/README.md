# LIONESS Flight Software
## All of the code that exists on the flight computer goes here

### Flight Processor
All the code that exists on the flight processor (IMX.8X) is written in FPrime for a yocto linux distribution. 
Make sure you have all of the necessary fprime requirements when developing code for this.

#### Development Environment
A docker container is provided so that the development for the linux machine is really easy

Build and run the docker container:
```bash
./run-docker.sh
```

Make sure you are in the venv:
```
source venv/bin/activate
```

Once inside, build for the EVK:
```bash
fprime-util generate (imx8x, rpi5, native) 
fprime-util build (imx8x, rpi5, native)
```

### Peripheral Code
As of 4/5/26, the MCU-based peripheral code is written for the RP2350. This hasn't been written yet so there's nothing to say here :)
