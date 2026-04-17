# Components::StarTracker

Processes images into star vectors and satellite orientation

## Star Tracker Algorithm Overview

Based on [LOST](https://github.com/UWCubeSat/lost).

After extracting centroids from an image, the star identification algorithm has three phases: pyramid star identification, remaining star identification, and attitude determination.

### Phase 1: Pyramid Star ID

Identifies an initial 4 observed stars by finding a unique matching 4-star pattern in the catalog. Angular distances between stars are rotation-invariant, so a set of pairwise distances acts as a fingerprint that can be matched regardless of the satellite's orientation.

1. Iterate 4-star combinations (i, j, k, r) from the observed stars, in a shuffled order designed to spread selections across the FOV.
2. For each quad, compute the 6 pairwise angular distances (ij, ik, ir, jk, jr, kr) and query the k-vector database for catalog star pairs at matching distances for ij, ik, and ir.
3. Cross-match: for each candidate catalog pair (iC, jC) from the ij results, find catalog stars kC from the ik results and rC from the ir results that share the same iC, then verify the remaining distances (jk, jr, kr) and chirality (orientation). If exactly one consistent 4-star match exists, the pyramid is uniquely identified.

### Phase 2: Identify Remaining Stars

Uses the 4 identified pyramid stars as anchors to triangulate every other observed star against the catalog.

1. For each unidentified star, track which identified stars are nearby and find the pair whose angular separation (as seen from the unidentified star) is closest to 90 degrees. Perpendicular neighbors give two maximally independent distance constraints; collinear neighbors make triangulation degenerate.
2. Maintain two lists: "below" (stars with a good triangulation pair) and "above" (angle from 90 degrees still above threshold). Pop the best candidate from "below", or the best from "above" if "below" is empty.
3. Triangulate the candidate: measure its angular distance to both identified neighbors. Since those neighbors have known catalog positions, query the database for catalog stars at those same distances with matching chirality. Accept if exactly one catalog match exists.
4. Each newly identified star is broadcast to remaining unidentified stars, updating their neighbor lists and potentially promoting them to "below". This creates a cascade where each identification enables more.

### Phase 3: QUEST Attitude Determination

Computes the optimal rotation quaternion from all identified star pairs (observed to catalog) via Davenport's q-method (QUEST). Solves the characteristic polynomial of Wahba's problem by Newton-Raphson iteration to find the maximum eigenvalue, then constructs the corresponding quaternion.

## Usage Examples
Add usage examples here

### Diagrams
Add diagrams here

### Typical Usage
And the typical usage of the component here

## Class Diagram
Add a class diagram here

## Port Descriptions
| Name | Description |
|---|---|
|---|---|

## Component States
Add component states in the chart below
| Name | Description |
|---|---|
|---|---|

## Sequence Diagrams
Add sequence diagrams here

## Parameters
| Name | Description |
|---|---|
|---|---|

## Commands
| Name | Description |
|---|---|
|---|---|

## Events
| Name | Description |
|---|---|
|---|---|

## Telemetry
| Name | Description |
|---|---|
|---|---|

## Unit Tests
Add unit test descriptions in the chart below
| Name | Description | Output | Coverage |
|---|---|---|---|
|---|---|---|---|

## Requirements
Add requirements in the chart below
| Name | Description | Validation |
|---|---|---|
|---|---|---|

## Change Log
| Date | Description |
|---|---|
|---| Initial Draft |