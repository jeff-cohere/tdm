#include "read_yaml.h"
#include "tdm.h"

#include <petsc.h>

#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(const char *exe_name) {
  int rank;
  MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
  if (rank == 0) {
    fprintf(stderr, "%s: no input file specified!\n", exe_name);
    fprintf(stderr, "%s: usage:\n", exe_name);
    fprintf(stderr, "%s <input.yaml>\n", exe_name);
  }
  exit(1);
}

static void shutdown(void) {
  PetscFinalize();
}

#define CHECK_ERROR(result) \
  if (result.err_code) { \
    fprintf(stderr, "%s: %s\n", argv[0], result.err_msg); \
    exit(result.err_code); \
  }


int main(int argc, char **argv) {
  // Fire up PETSc.
  PetscInitializeNoArguments();
  atexit(shutdown);

  // Parse command line args.
  if (argc < 2) {
    usage(argv[0]);
  }

  const char *yaml_file = argv[1];
  tdm_config_t config;
  tdm_result_t result = read_yaml(yaml_file, &config);
  CHECK_ERROR(result);

  // Extract point information from the specified configuration.
  point_t *points;
  size_t num_points;
  result = extract_points(config, &num_points, &points);

  // Generate a triangulation from the point data and config options.
  DM surface_mesh;
  result = triangulate_dem(config, num_points, points, &surface_mesh);
  CHECK_ERROR(result);

  // Write the triangle (surface) mesh to an appropriate format.
  result = write_mesh(config, surface_mesh, "surface_mesh");
  CHECK_ERROR(result);

  // Extrude the triangulated surface mesh into 3D columns of prisms.
  DM column_mesh;
  result = extrude_surface_mesh(config, surface_mesh, &column_mesh);
  CHECK_ERROR(result);

  // Write the column mesh to an appropriate format
  result = write_mesh(config, column_mesh, "column_mesh");
  CHECK_ERROR(result);

  // Clean up.
  DMDestroy(&surface_mesh);
  DMDestroy(&column_mesh);

  return 0;
}
