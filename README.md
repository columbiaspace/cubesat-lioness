# cubesat-lioness

## NOTE:
DO NOT push to main. Please put a PR up for us to review it before or else you will incur the wrath of the git gods.

## Installation and Build
Start at the project root.

```shell
cd fsw/lioness-sw
python3 -m venv fprime-venv
. fprime-venv/bin/activate
git submodule update --init --recursive
pip3 install -r requirements.txt
fprime-util generate
fprime-util build
```
