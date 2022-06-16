#include "tdm.h"

#include <ctype.h>
#include <float.h>
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
  char *buffer = NULL;
  *data = NULL;
  *size = 0;

  FILE *f = fopen(text_file, "r");
  if (!f) {
    result = tdm_result(1, "Could not open text file '%s'.", text_file);
    goto finished;
  }

  // Read the file's contents into memory.
  fseek(f, 0, SEEK_END);
  size_t file_size = (size_t)ftell(f);
  rewind(f);
  buffer = malloc(sizeof(char) * file_size);
  fread(buffer, sizeof(char), file_size, f);

  // The file contains a bunch of numbers separated by whitespace. We read these
  // numbers into an array, dynamically resizing it as needed.

  // Line up the first number.
  size_t offset = 0;
  while (!isdigit(buffer[offset]) && offset < file_size) ++offset;
  if (offset == file_size) {
    result = tdm_result(1, "No numeric data found in '%s'!", text_file);
    goto finished;
  }

  // Allocate (preliminary) storage.
  size_t n = 0, cap = 1024;
  real_t *array = malloc(sizeof(real_t) * cap);

  // Read numbers till the end.
  while (offset < file_size) {
    char *endptr;
    real_t datum = strtod(&buffer[offset], &endptr);
    if (endptr) {
      if (!isspace(endptr[0])) { // whitespace is fine--go to next number
        array[n] = datum;
        ++n;
        if (n >= cap) { // resize if needed
          while (cap <= n) cap *= 2;
          array = realloc(array, sizeof(real_t) * cap);
        }
        offset += (endptr - &buffer[offset]);
        while (isspace(buffer[offset])) ++offset;
      } else {
        result = tdm_result(1, "Invalid numeric data found at byte %zd of '%s'!",
                            offset, text_file);
        free(array);
        goto finished;
      }
    }
  }

  // Hand off the data.
  *data = array;
  *size = n;
finished:
  if (buffer) free(buffer);
  if (f) fclose(f);
  return result;
}

tdm_result_t extract_points(tdm_config_t config,
                            size_t      *num_points,
                            point_t    **points) {
  tdm_result_t result = {};

  // Read point elevation, latitude, longitude data and transform it to 3D
  // cartesian coordinates on a plane.
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

  // Transform the point data to cartesian coordinates above the x-y plane.
  // Here, we use x to indicate displacements between longitudes, and y for
  // displacements between latitudes.

  // First, scan the data to find min/max lat/lon values so we can tell where
  // we are on the earth.
  real_t min_lat = FLT_MAX, max_lat = -FLT_MAX,
         min_lon = FLT_MAX, max_lon = -FLT_MAX;
  for (size_t i = 0; i < n; ++i) {
    if (min_lat > lat_data[i]) min_lat = lat_data[i];
    if (max_lat < lat_data[i]) max_lat = lat_data[i];
    if (min_lon > lat_data[i]) min_lon = lat_data[i];
    if (max_lon < lat_data[i]) max_lon = lat_data[i];
  }

  // Now we assume that the data covers a portion of the earth that is small
  // enough to assume zero curvature, and we use the center-point latitude and
  // longitude to estimate distances using North-East-Up (NEU) coordinates fit
  // to a tangent plane at a "median" latitude and longitude.
  // WARNING: This calculation doesn't work when you're near the poles (but
  // WARNING: then, using lat/lon coordinates near the poles is foolish, no?).
  real_t med_lat = 0.5 * (min_lat + max_lat);
  real_t med_lon = 0.5 * (min_lon + max_lon);

  // Compute differential coordinate spacings dx_dlat (easterly distance between
  // longitudes per degree latitude) and dy_dlon (northerly distance between
  // latitudes) at this point using the WGS84 spheroid approximation
  // (https://en.wikipedia.org/wiki/Geographic_coordinate_system#Length_of_a_degree).
  real_t dx_dlon = 111412.84 * cos(med_lat) - 93.5 * cos(3*med_lat) +
                   0.118 * cos(5*med_lat);
  real_t dy_dlat = 111132.92 - 559.82 * cos(2.0*med_lat) +
                   1.175 * cos(4*med_lat) - 0.0023 * cos(6*med_lat);

  *num_points = n;
  *points = malloc(sizeof(point_t) * n);
  for (size_t i = 0; i < n; ++i) {
    // The origin of our NEU coordinate system is at the median point we
    // computed above. Because we're on a tangent plane, we can compute
    // distances by multiplying displacements in latitude/longitude by the
    // differential coordinate spacings.
    real_t dlon = lon_data[i] - med_lon;
    real_t dlat = lat_data[i] - med_lat;
    point_t p = {
      .x = dx_dlon * dlon,
      .y = dy_dlat * dlat,
      .z = elev_data[i]
    };
    (*points)[i] = p;
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
    jigsaw(&config.jigsaw, &geom, NULL, NULL, &trimesh);

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

