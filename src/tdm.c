#include "tdm.h"

tdm_result_t parse_args(int argc, char **argv, tdm_config_t *config) {
  return (tdm_result_t){0};
}

tdm_result_t triangulate_dem(tdm_config_t config,
                             const char  *dem_file,
                             DM          *surface_mesh) {
  return (tdm_result_t){0};
}

tdm_result_t extrude_surface_mesh(tdm_config_t config,
                                  DM           surface_mesh,
                                  DM          *column_mesh) {
  return (tdm_result_t){0};
}

tdm_result_t write_mesh(tdm_config_t config, DM mesh, const char *prefix) {
  return (tdm_result_t){0};
}

