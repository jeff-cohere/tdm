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
    fprintf(stderr, "%s: no input specified!\n", exe_name);
    fprintf(stderr, "%s: usage:\n", exe_name);
    fprintf(stderr, "%s <dem>\n", exe_name);
  }
  exit(1);
}

static void shutdown(void) {
  PetscFinalize();
}

// Extracts and returns a positive integer value from an argument (with a
// given description).
static int parse_int_arg(const char *arg, const char *desc) {
  char *end;
  int value = strtol(arg, &end, 10);
  if (*end != '\0') {
    fprintf(stderr, "Invalid %s: %s\n", desc, arg);
    exit(1);
  } else if (value < 1) {
    fprintf(stderr, "Non-positive %s: %d\n", desc, value);
    exit(1);
  }
  return value;
}

// Extracts and returns a real value from an argument (with a
// given description).
static real_t parse_real_arg(const char *arg, const char *desc) {
  char *end;
  int value = strtod(arg, &end);
  if (*end != '\0') {
    fprintf(stderr, "Invalid %s: %s\n", desc, arg);
    exit(1);
  }
  return value;
}

void parse_args(int argc, char **argv, tdm_config_t *config,
                char *elev_file, char *lat_file, char *lon_file) {
  // Set Jigsaw defaults.
  config->jigsaw_config = (jigsaw_jig_t){
    ._verbosity = 0,
    ._geom_seed = 8,
    ._geom_feat = 0,
    ._geom_eta1 = 45.0,
    ._geom_eta2 = 45.0,
    ._init_near = 1e-8,
    ._hfun_scal = 0, // FIXME
    ._hfun_hmax = 0.02,
    ._hfun_hmin = 0.0,
    ._bnds_kern = 0, // FIXME
    ._mesh_dims = 3,
    ._mesh_kern = 0,
    ._mesh_iter = INT_MAX,
    ._mesh_top1 = 0,
    ._mesh_top2 = 0,
    ._mesh_rad2 = 1.05,
    ._mesh_rad3 = 2.05,
    ._mesh_siz1 = 4.0/3.0 + FLT_MIN,
    ._mesh_siz2 = 4.0/3.0 + FLT_MIN,
    ._mesh_siz3 = 4.0/3.0 + FLT_MIN,
    ._mesh_off2 = 0.90,
    ._mesh_off3 = 1.10,
    ._mesh_snk2 = 0.2,
    ._mesh_snk3 = 0.33,
    ._mesh_eps1 = 0.33,
    ._mesh_eps2 = 0.33,
    ._mesh_vol3 = 0.0,
    ._optm_kern = 0, // FIXME
    ._optm_iter = 16,
    ._optm_qtol = 1e-4,
    ._optm_qlim = 0.9375,
    ._optm_tria = 1,
    ._optm_dual = 0,
    ._optm_zip_ = 1,
    ._optm_div_ = 1
  };

  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, "vfe:n:s:M:m:k:")) != -1) {
    switch (c) {
      case 'v':
        config->jigsaw_config._verbosity = 1;
        break;
      case 'f':
        config->jigsaw_config._geom_feat = 1;
        break;
      case 'e':
        config->jigsaw_config._geom_eta2 =
          parse_int_arg(optarg, "feature angle (eta)");
        break;
      case 'n':
        config->jigsaw_config._init_near =
          parse_real_arg(optarg, "init-near parameter");
        break;
      case 's':
        config->jigsaw_config._hfun_scal =
          parse_int_arg(optarg, "hfunc-scal parameter");
        break;
      case 'M':
        config->jigsaw_config._hfun_hmax =
          parse_real_arg(optarg, "hfunc-hmax parameter");
        break;
      case 'm':
        config->jigsaw_config._hfun_hmin =
          parse_real_arg(optarg, "hfunc-hmin parameter");
        break;
      case 'k':
        config->jigsaw_config._bnds_kern =
          parse_int_arg(optarg, "bnds-kern parameter");
        break;
      case '?':
        if ((optopt != 'v') || (optopt != 'f')) {
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        } else if (isprint(optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        else {
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        }
        exit(1);
      default:
        abort();
    }
    if (opterr) {
      exit(opterr);
    }
  }
  if (optind < argc+2) {
    strncpy(elev_file, argv[optind], FILENAME_MAX);
    strncpy(lat_file,  argv[optind+1], FILENAME_MAX);
    strncpy(lon_file,  argv[optind+2], FILENAME_MAX);
  } else {
    fprintf(stderr, "At least one file was not specified!");
    exit(1);
  }
}

// Reads real-valued data from a text file into an array.
static tdm_result_t read_point_data(const char  *text_file,
                                    real_t     **data,
                                    size_t      *size) {
  tdm_result_t result = {};
  return result;
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
  tdm_config_t config;
  char elev_file[FILENAME_MAX], lat_file[FILENAME_MAX], lon_file[FILENAME_MAX];
  parse_args(argc, argv, &config, elev_file, lat_file, lon_file);

  // Read the DEM point data from the files.
  real_t *point_elev, *point_lat, *point_lon;
  size_t num_points, num_points1;
  tdm_result_t result = read_point_data(elev_file, &point_elev, &num_points);
  if (!result.err_code) {
    result = read_point_data(lat_file, &point_lat, &num_points1);
    if (result.err_code) {
      // FIXME
    } else if (num_points1 != num_points) {
      // FIXME
    } else {
      result = read_point_data(lon_file, &point_lon, &num_points1);
      if (result.err_code) {
        // FIXME
      } else if (num_points1 != num_points) {
        // FIXME
      }
    }
  }

  // Generate a triangulation from the point data and config options.
  DM surface_mesh;
  result = triangulate_dem(config, num_points, point_elev, point_lat,
                           point_lon, &surface_mesh);
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
