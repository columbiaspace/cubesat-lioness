# LIONESS Flight Software
## All of the code that exists on the flight computer goes here

### Flight Processor
All the code that exists on the flight processor (IMX.8X) is written in FPrime for a yocto linux distribution. 
Make sure you have all of the necessary fprime requirements when developing code for this.

#### Development Environment
A docker container is provided so that the development for the linux machine is really easy. If you have not used docker before, (check out this link to learn what it is and how to install it on your machine.)[https://docs.docker.com/desktop/?_gl=1*1twthdc*_gcl_au*NDQ2NjU2MzE4LjE3NzU0OTY0Nzg.*_ga*OTE0NDc3Nzc3LjE3NzU0OTY0Nzg.*_ga_XJWPQMJYHQ*czE3NzU0OTY0NzckbzEkZzEkdDE3NzU0OTY0NzgkajU5JGwwJGgw]

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
