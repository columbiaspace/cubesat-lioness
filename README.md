# cubesat-lioness

## NOTE:
DO NOT push to main. Please put a PR up for us to review it before or else you will incur the wrath of the git gods.

## Organization

```
.
├── .github     # CI/CD
├── fsw         # FPrime core flight software
│   └── lioness-sw
│       ├── FlatSat     # Deployment of the core flight software for the FlatSat
│       └── LionessSw   # Core flight software FPrime components and implementation code
├── gsw         # Ground software
├── hw          # Peripheral board firmware
├── sim         # Simulators
└── test        # Test and other miscellaneous code
```

## Core Flight Software Installation and Build
These instructions sets up the FPrime project that will run on the main flight computer located in `fsw`.

Open a terminal at the project root.
```shell
cd fsw/lioness-sw
```
The following command will create and activate a virtual environment called `fprime-venv`.
```
python3 -m venv fprime-venv
. fprime-venv/bin/activate
```
Install requirements and build the project.
```
git submodule update --init --recursive
pip3 install -r requirements.txt
fprime-util generate
fprime-util build
```
