# Star Tracker Prototype

This algorithm is based on https://github.com/UWCubeSat/lost.


## Building

```shell
cd test/star-tracker
cmake -B build
cmake --build build
```


## Usage

Usage, see:
```shell
./star_tracker_test -h
```

Default values for the all args are at the beginning of `main` in `star_tracker_test.cpp`.
By default, the program expects a `image.png` or `image.raw` (in YUV420 format) and a `database.dat` in the same directory as the executable.

Example usage:
```shell
./star_tracker_test image.png database.dat 5 49.0 22.2 0.05 1000 0.0001 1000 1920 1080
```


### Database

To generate the database, see https://github.com/UWCubeSat/lost. We need at least the star catalog and kvector, such as the following (default) command:
```shell
./lost database \
  --max-stars 5000 \
  --kvector \
  --kvector-min-distance 0.2 \
  --kvector-max-distance 15 \
  --kvector-distance-bins 10000 \
  --output my-database.dat
```
