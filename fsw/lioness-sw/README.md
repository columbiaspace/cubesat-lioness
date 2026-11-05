# LIONESS Flight Software
## All of the code that exists on the flight computer goes here

## Flight Processor

All the code that exists on the flight processor (IMX.8X) is written in FPrime for a yocto linux distribution. 
Make sure you have all of the necessary fprime requirements when developing code for this.


### Development Environment

A docker container is provided so that the development for the linux machine is really easy. If you have not used docker before, [check out this link to learn what it is and how to install it on your machine.](https://docs.docker.com/desktop)

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


### Connecting to and Deploying on IMX8X

#### Step 1: Connect Over USB/Debugging Bridge

1. First connect your pc to the Symphony board’s micro-usb debug port.
2. Then determine which port on your laptop the usb connection shows up as. On mac you run the following command. It usually starts with `/dev/tty.usbserial`

    ```bash
    ls /dev/tty*
    ```

3. Connect to the serial debugging bridge:

    ```bash
    screen /dev/tty.usbserial-D20JMUEV 115200
    ```

4. When prompted to login, enter `root`.

#### Step 2: SSH via Raspberry Pi

1. SSH into the Raspberry Pi you want to use as an SSH bridge.

    ```bash
    ssh pi1csi@10.206.242.203
    ```

2. Connect the Pi to the IMX8X over an ethernet cable. (Make sure to use `ETH0` on the IMX8X.)
3. On the Pi, assign a temporary address to the ethernet port.

    ```bash
    sudo ip addr add 192.168.1.1/24 dev eth0
    ```

4. On the IMX8X (via the debugging bridge), assign a local address.

    ```bash
    ip addr add 192.168.1.2/24 dev eth0
    ip link set eth0 up
    ```

5. In the Pi, test the connection.

    ```bash
    ping -c 3 192.168.1.2
    ```

6. You can ssh into the IMX8X from the Pi now.

   ```bash
   ssh root@192.168.1.2
   ```

## Peripheral Code
As of 4/5/26, the MCU-based peripheral code is written for the RP2350. This hasn't been written yet so there's nothing to say here :)
