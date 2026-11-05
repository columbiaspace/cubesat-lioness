# CubeSat LIONESS

Welcome to the main repository for Columbia Space Initiative LIONESS CubeSat's flight software.

## Organization

```
.
├── .github     # CI/CD
├── fsw         # FPrime core flight software
│   └── lioness-sw
│       ├── LionessSw       # Core flight software FPrime components and implementation code
│       │   └── FlatSat     # Deployment of the core flight software for the FlatSat
│       └── README.md       # Core flight software documentation
├── gsw         # Ground software
├── hw          # Peripheral board firmware
├── sim         # Simulators
├── test        # Test and other miscellaneous code
└── README.md
```

## Core Flight Software Installation
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

## Core Flight Software Build
To build the core flight software, see [Flight Software README](fsw/lioness-sw/README.md)
