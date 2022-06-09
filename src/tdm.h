#ifndef TDM_H
#define TDM_H

#include <lib_jigsaw.h>
#include <petsc.h>

// This struct defines the configuration for our Jigsaw-based mesh generation.
typedef struct tdm_config_t {
  jigsaw_jig_t jigsaw_config;
} tdm_config_t;

// This is the maximum length of an error string.
#define TDM_MAX_ERR_LEN 1024

// This struct holds the result of an operation, including an error code and a
// related error message.
typedef struct tdm_result_t {
  int err_code;       // 0 indicates success, nonzero -> failure
  char err_msg[TDM_MAX_ERR_LEN]; // error string
} tdm_result_t;

// Use this one-liner to create a result type with an error code and
// a string.
tdm_result_t tdm_result(int err_code, const char *fmt, ...);

// Generates a triangulated surface mesh from the given DEM file, storing the
// surface mesh in the given DM.
tdm_result_t triangulate_dem(tdm_config_t config,
                             size_t       num_points,
                             real_t       point_elev[num_points],
                             real_t       point_lat[num_points],
                             real_t       point_lon[num_points],
                             DM          *surface_mesh);

// Given a surface mesh, this function extrudes each 2D cell to a column of
// prisms, producing a 3D column mesh.
tdm_result_t extrude_surface_mesh(tdm_config_t config,
                                  DM           surface_mesh,
                                  DM          *column_mesh);

// Writes the given mesh to a format indicated by the given configuration.
tdm_result_t write_mesh(tdm_config_t config, DM mesh, const char *prefix);


#endif
