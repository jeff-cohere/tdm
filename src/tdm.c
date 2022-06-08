#include "tdm.h"

tdm_result_t triangulate_dem(tdm_config_t config,
                             size_t       num_points,
                             real_t       point_elev[num_points],
                             real_t       point_lat[num_points],
                             real_t       point_lon[num_points],
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

