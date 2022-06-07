# TDycore Meshing Workflow

This document outlines an approach to generating a three-dimensional mesh
consisting of triangular prisms aligned with the z axis, given input files
containing

1. `dem.txt`, a digital elevation data (a "DEM"), with heights [m] stored in a
   2D array
2. `lat.txt` and `lon.txt`, (respectively) latitude and longitude data stored in
   files formatted identically to `dem.txt`
3. a "mask" text file in the same format that indicates whether an elevation
   point is incorporated into the mesh (nonzero) or ignored (zero).

## Overview

This workflow uses Darren Engwirda's [JIGSAW](https://github.com/dengwirda/jigsaw/)
meshing library and the DMPlex data structure from [PETSc](https://petsc.org).
Here's a high-level summary of the stages in the process.

1. Data in the text files is processed into a [MSH](https://github.com/dengwirda/jigsaw/wiki/MSH-File-Format)
   file that contains a set of points describing a 2D surface embedded in 3D
   space, expressed within a 2D array (using the `euclidian-grid` identifier in
   the `MSH` file's `MSHID` segment).

2. A set of configuration parameters is specified in a [JIG](https://github.com/dengwirda/jigsaw/wiki/JIG-File-Format)
   file by a user. These parameters determine how JIGSAW tessellates the point
   data in the `MSH` file.

3. The `MSH` and `JIG` files are fed to `JIGSAW`, which produces another `MSH`
   file containing a triangulated surface mesh.

4. Using logic similar to that in [DMPlexCreatePLYFromFile](https://petsc.org/main/src/dm/impls/plex/plexply.c.html#DMPlexCreatePLYFromFile),
   we create a `DMPlex` object representing the triangulated surface mesh.

5. We then extrude the surface mesh "in the z direction" with a call to
   [DMPlexExtrude](https://petsc.org/main/docs/manualpages/DMPLEX/DMPlexExtrude/),
   applying user-ѕpecified parameters as needed.

6. We save the resulting extruded geometry to an Exodus file for use by TDycore.

It may be possible to write a single utility program that performs all this
work, depending on how we want to specify parameters for the various operations.
In what follows, we refer to the 6 stages above as Stage 1, Stage 2, and so on.

**From Gautam:**
```
How about having a JIGSAS-TO-TDycoreMESH tool that outputs
* Surface triangular .exo file for the possibility of using DMPlex to extrude
  the mesh
* 3D mesh that is extruded and outputted in following formats:
    * .exo : For DMPlex
    * .h5: PFLOTRAN ugrid format
I believe DMPlex also supports a .h5 format and we can think about adding
support for that format too a bit later.
```

## Technical Details

### Stage 1

To create a `MSH` file from the various text files, we need to make a few
assumptions:

1. For now, we ignore the curvature of the Earth. This avoids complexity arising
   from the use of spherical coordinates, which probably require the use of
   `JIGSAW(GEO)` anyway.
2. We interpret the elevations in `dem.txt` as heights above mean sea level (or
   some other common measure).
