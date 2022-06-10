#include "tdm.h"

#include <stdarg.h>

// This function returns a newly created result with the given error code and
// formatted message.
tdm_result_t tdm_result(int err_code, const char *fmt, ...) {
  tdm_result_t result = {.err_code = err_code};
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(result.err_msg, TDM_MAX_ERR_LEN, fmt, ap);
  va_end(ap);
  return result;
}

// Reads real-valued data from a text file into an array.
static tdm_result_t read_point_data(const char  *text_file,
                                    real_t     **data,
                                    size_t      *size) {
  tdm_result_t result = {};
  FILE *f = fopen(text_file, "r");
  if (!f) {
    result = tdm_result(1, "Could not open text file '%s'.", text_file);
    goto finished;
  }
finished:
  fclose(f);
  return result;
}

tdm_result_t extract_points(tdm_config_t config,
                            size_t      *num_points,
                            point_t    **points) {
  tdm_result_t result = {};

  // Read the DEM point data from the specified files.

  // Read point elevation, latitude, longitude data and transform it to 3D
  // cartesian coordinates.
  real_t *elev_data = NULL, *lat_data = NULL, *lon_data = NULL,
         *mask_data = NULL;
  size_t n, n1;
  result = read_point_data(config.dem_file, &elev_data, &n);
  if (result.err_code) goto finished;

  result = read_point_data(config.lat_file, &lat_data, &n1);
  if (!result.err_code && (n1 != n)) {
    result = tdm_result(1,
      "Number of latitude coordinates (%zd) != number of elevations (%zd).",
      n1, n);
  }
  if (result.err_code) goto finished;

  result = read_point_data(config.lon_file, &lon_data, &n1);
  if (!result.err_code && (n1 != n)) {
    result = tdm_result(1,
      "Number of longitude coordinates (%zd) != number of elevations (%zd).",
      n1, n);
  }
  if (result.err_code) goto finished;

  result = read_point_data(config.mask_file, &mask_data, &n1);
  if (!result.err_code && (n1 != n)) {
    result = tdm_result(1,
      "Number of mask values (%zd) != number of elevations (%zd).",
      n1, n);
  }
  if (result.err_code) goto finished;

  // Transform the point data to cartesian coordinates.
  *num_points = n;
  *points = malloc(sizeof(point_t) * n);
  for (size_t i = 0; i < n; ++i) {
    // FIXME: What projection do we want to use here?
  }

finished:
  if (elev_data) free(elev_data);
  if (lat_data) free(lat_data);
  if (lon_data) free(lon_data);
  if (mask_data) free(mask_data);
  if (result.err_code) {
    *num_points = 0;
    free(*points);
    *points = NULL;
  }
  return result;
}

tdm_result_t triangulate_dem(tdm_config_t config,
                             size_t       num_points,
                             point_t      points[num_points],
                             DM          *surface_mesh) {

  // Create a structured mesh from the DEM files.
  tdm_result_t result = {};
  jigsaw_msh_t geom;
  // FIXME

  if (!result.err_code) {
    // Run jigsaw to generate a triangulated mesh.
    jigsaw_msh_t trimesh;
    // FIXME: Can we check errors?
    jigsaw(&config.jigsaw_config,
           &geom,
           NULL,
           NULL,
           &trimesh);

    if (!result.err_code) {
      // Create a 2D DMPlex from the triangulated mesh.
    }
  }
  return result;
}

tdm_result_t extrude_surface_mesh(tdm_config_t config,
                                  DM           surface_mesh,
                                  DM          *column_mesh) {
  return (tdm_result_t){0};
}

tdm_result_t write_mesh(tdm_config_t config, DM mesh, const char *prefix) {
  return (tdm_result_t){0};
}

