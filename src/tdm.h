#ifndef TDM_H
#define TDM_H

#include <lib_jigsaw.h>
#include <petsc.h>

// TDM can output meshes in the Exodus or HDF5 formats.
typedef enum {
  TDM_EXODUS,
  TDM_HDF5
} tdm_mesh_format_t;

// This struct defines the configuration for our Jigsaw-based mesh generation.
typedef struct tdm_config_t {
  // input data
  const char *dem_file;
  const char *lat_file;
  const char *lon_file;
  const char *mask_file;

  // jigsaw surface triangulation settings
  jigsaw_jig_t jigsaw;

  // extrusion parameters
  int     num_layers;
  real_t  total_layer_thickness;
  real_t *layer_thicknesses;

  // mesh output settings
  tdm_mesh_format_t surface_mesh_format;
  const char       *surface_mesh_file;
  tdm_mesh_format_t column_mesh_format;
  const char       *column_mesh_file;

} tdm_config_t;

// This is the maximum length of an error string stored in tdm_result_t.
#define TDM_MAX_ERR_LEN 1024

// This struct holds the result of an operation, including an error code and a
// related error message.
typedef struct tdm_result_t {
  int err_code;       // 0 indicates success, nonzero -> failure
  char err_msg[TDM_MAX_ERR_LEN]; // error string
} tdm_result_t;

// This is a point in 3D space with a mask value of 1 or 0.
typedef struct point_t {
  real_t x, y, z;
  int mask;
} point_t;

// Use this one-liner to create a result type with an error code and
// a string.
tdm_result_t tdm_result(int err_code, const char *fmt, ...);

// Extracts point data from the files identified in the given configuration,
// computing coordinates by assuming no planetary curvature.
tdm_result_t extract_points(tdm_config_t config,
                            size_t      *num_points,
                            point_t    **points);

// Generates a triangulated surface mesh from the given DEM file, storing the
// surface mesh in the given DM.
tdm_result_t triangulate_dem(tdm_config_t config,
                             size_t       num_points,
                             point_t      points[num_points],
                             DM          *surface_mesh);

// Given a surface mesh, this function extrudes each 2D cell to a column of
// prisms, producing a 3D column mesh.
tdm_result_t extrude_surface_mesh(tdm_config_t config,
                                  DM           surface_mesh,
                                  DM          *column_mesh);

// Writes the given mesh to a format indicated by the given configuration.
tdm_result_t write_mesh(tdm_config_t config, DM mesh, const char *prefix);


#endif
