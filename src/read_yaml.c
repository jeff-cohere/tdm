#include "read_yaml.h"

#include <petsc/private/khash/khash.h> // for checking duplicate param names
#include <yaml.h>

#include <stdbool.h>
#include <stdint.h>

// A hash table representing a set of C strings. Used to guarantee that input
// parameter/setting names are unique.
KHASH_SET_INIT_STR(yaml_name_set);

// This type keeps track of the state of the YAML parser.
typedef struct parser_state_t {
  bool parsing_data;
  khash_t(yaml_name_set) *data_param_names;

  bool parsing_jigsaw;
  khash_t(yaml_name_set) *jigsaw_param_names;

  bool parsing_extrusion;
  bool parsing_thicknesses;
  khash_t(yaml_name_set) *extrusion_param_names;

  bool parsing_output;
  bool parsing_surface_mesh_output;
  bool parsing_column_mesh_output;
  khash_t(yaml_name_set) *output_param_names;

  char current_param[128];
} parser_state_t;

// Parses a parameter name, checking to see if it's a duplicate.
static tdm_result_t check_param_name(const char             *block_name,
                                     khash_t(yaml_name_set) *prior_names,
                                     const char            **valid_names,
                                     const char             *param_name) {
  // Have we seen this parameter name before?
  khiter_t iter = kh_get(yaml_name_set, prior_names, param_name);
  if (iter != kh_end(prior_names)) { // yes
    return tdm_result(1, "Parameter %s in %s block appears more than once!",
                      param_name, block_name);
  }

  // Is the name valid?
  const char **which = valid_names;
  while (*which) {
    if (!strcmp(param_name, *which)) break;
  }
  if (!*which) {
    return tdm_result(1, "Invalid parameter name in %s block: '%s'",
                      param_name, block_name);
  }

  // Add this parameter name to our set of tracked names.
  int ret;
  iter = kh_put(yaml_name_set, prior_names, strdup(param_name), &ret);
  return (tdm_result_t){0};
}

// Parses a parameter in the data block.
static tdm_result_t parse_data_param(parser_state_t *state,
                                     const char     *param,
                                     tdm_config_t   *config) {
  if (!strcmp(state->current_param, "dem")) {
    config->dem_file = strdup(param);
  } else if (!strcmp(state->current_param, "lat")) {
    config->lat_file = strdup(param);
  } else if (!strcmp(state->current_param, "lon")) {
    config->lon_file = strdup(param);
  } else if (!strcmp(state->current_param, "mask")) {
    config->mask_file = strdup(param);
  }
  state->current_param[0] = 0;
  return (tdm_result_t){0};
}

// Parses a (32-bit) integer from a string.
static tdm_result_t parse_int32(const char *str, int32_t *value) {
  char *endptr;
  long v = strtol(str, &endptr, 10);
  if (endptr) {
    return tdm_result(1, "Invalid integer value: %s", str);
  }
  *value = v;
  return (tdm_result_t){0};
}

// Parses a real number from a string.
static tdm_result_t parse_real(const char *str, real_t *value) {
  char *endptr;
  double v = strtod(str, &endptr);
  if (endptr) {
    return tdm_result(1, "Invalid real value: %s", str);
  }
  *value = (real_t)v;
  return (tdm_result_t){0};
}

// Parses a parameter in the jigsaw block.
static tdm_result_t parse_jigsaw_param(parser_state_t *state,
                                       const char     *param,
                                       tdm_config_t   *config) {
  tdm_result_t result = {};
  if (!strcmp(state->current_param, "verbosity")) {
    result = parse_int32(param, &(config->jigsaw._verbosity));
  } else if (!strcmp(state->current_param, "geom_seed")) {
    result = parse_int32(param, &(config->jigsaw._geom_seed));
  } else if (!strcmp(state->current_param, "geom_feat")) {
    result = parse_int32(param, &(config->jigsaw._geom_feat));
  } else if (!strcmp(state->current_param, "geom_eta1")) {
    result = parse_real(param, &(config->jigsaw._geom_eta1));
  } else if (!strcmp(state->current_param, "geom_eta2")) {
    result = parse_real(param, &(config->jigsaw._geom_eta2));
  } else if (!strcmp(state->current_param, "init_near")) {
    result = parse_real(param, &(config->jigsaw._init_near));
  } else if (!strcmp(state->current_param, "hfun_scal")) {
    result = parse_int32(param, &(config->jigsaw._hfun_scal));
  } else if (!strcmp(state->current_param, "hfun_hmax")) {
    result = parse_real(param, &(config->jigsaw._hfun_hmax));
  } else if (!strcmp(state->current_param, "hfun_hmin")) {
    result = parse_real(param, &(config->jigsaw._hfun_hmin));
  } else if (!strcmp(state->current_param, "bnds_kern")) {
    result = parse_int32(param, &(config->jigsaw._bnds_kern));
  } else if (!strcmp(state->current_param, "mesh_dims")) { // Should be 3
    result = parse_int32(param, &(config->jigsaw._mesh_dims));
  } else if (!strcmp(state->current_param, "mesh_kern")) {
    result = parse_int32(param, &(config->jigsaw._mesh_kern));
  } else if (!strcmp(state->current_param, "mesh_iter")) {
    result = parse_int32(param, &(config->jigsaw._mesh_iter));
  } else if (!strcmp(state->current_param, "mesh_top1")) {
    result = parse_int32(param, &(config->jigsaw._mesh_top1));
  } else if (!strcmp(state->current_param, "mesh_top2")) {
    result = parse_int32(param, &(config->jigsaw._mesh_top2));
  } else if (!strcmp(state->current_param, "mesh_rad2")) {
    result = parse_real(param, &(config->jigsaw._mesh_rad2));
  } else if (!strcmp(state->current_param, "mesh_rad3")) {
    result = parse_real(param, &(config->jigsaw._mesh_rad3));
  } else if (!strcmp(state->current_param, "mesh_siz1")) {
    result = parse_real(param, &(config->jigsaw._mesh_siz1));
  } else if (!strcmp(state->current_param, "mesh_siz2")) {
    result = parse_real(param, &(config->jigsaw._mesh_siz2));
  } else if (!strcmp(state->current_param, "mesh_siz3")) {
    result = parse_real(param, &(config->jigsaw._mesh_siz3));
  } else if (!strcmp(state->current_param, "mesh_off2")) {
    result = parse_real(param, &(config->jigsaw._mesh_off2));
  } else if (!strcmp(state->current_param, "mesh_off3")) {
    result = parse_real(param, &(config->jigsaw._mesh_off3));
  } else if (!strcmp(state->current_param, "mesh_snk2")) {
    result = parse_real(param, &(config->jigsaw._mesh_snk2));
  } else if (!strcmp(state->current_param, "mesh_snk3")) {
    result = parse_real(param, &(config->jigsaw._mesh_snk3));
  } else if (!strcmp(state->current_param, "mesh_eps1")) {
    result = parse_real(param, &(config->jigsaw._mesh_eps1));
  } else if (!strcmp(state->current_param, "mesh_eps2")) {
    result = parse_real(param, &(config->jigsaw._mesh_eps2));
  } else if (!strcmp(state->current_param, "mesh_vol3")) {
    result = parse_real(param, &(config->jigsaw._mesh_vol3));
  } else if (!strcmp(state->current_param, "optm_kern")) {
    result = parse_int32(param, &(config->jigsaw._optm_kern));
  } else if (!strcmp(state->current_param, "optm_iter")) {
    result = parse_int32(param, &(config->jigsaw._optm_iter));
  } else if (!strcmp(state->current_param, "optm_qtol")) {
    result = parse_real(param, &(config->jigsaw._optm_qtol));
  } else if (!strcmp(state->current_param, "optm_qlim")) {
    result = parse_real(param, &(config->jigsaw._optm_qlim));
  } else if (!strcmp(state->current_param, "optm_tria")) {
    result = parse_int32(param, &(config->jigsaw._optm_tria));
  } else if (!strcmp(state->current_param, "optm_dual")) {
    result = parse_int32(param, &(config->jigsaw._optm_dual));
  } else if (!strcmp(state->current_param, "optm_zip")) {
    result = parse_int32(param, &(config->jigsaw._optm_zip_));
  } else if (!strcmp(state->current_param, "optm_div")) {
    result = parse_int32(param, &(config->jigsaw._optm_div_));
  }
  state->current_param[0] = 0;
  return result;
}

// Parses a parameter in the extrusion block.
static tdm_result_t parse_extrusion_param(parser_state_t *state,
                                          const char     *param,
                                          tdm_config_t   *config) {
  if (!strcmp(state->current_param, "layers")) {
    result = parse_int32(param, &(config->num_layers));
  } else if (!strcmp(state->current_param, "thickness")) {
    result = parse_real(param, &(config->total_layer_thickness));
  } else if (!state->parsing_thicknesses &&
             !strcmp(state->current_param, "thicknesses")) {
    state->parsing_thicknesses = true;
  }
  state->current_param[0] = 0;
  return (tdm_result_t){0};
}

// Parses a parameter in the output block.
static tdm_result_t parse_output_param(parser_state_t *state,
                                       const char     *param,
                                       tdm_config_t   *config) {
  if (!state->parsing_surface_mesh_output &&
      !strcmp(state->current_param, "surface_mesh")) {
    state->parsing_surface_mesh_output = true;
  } else if (!state->parsing_column_mesh_output &&
             !strcmp(state->current_param, "column_mesh")) {
    state->parsing_column_mesh_output = true;
  } else if (state->parsing_surface_mesh_output) {
    if (!strcmp(state->current_param, "format")) {
      if (!strcmp(param, "exodus")) {
        config->surface_mesh_format = TDM_EXODUS;
      } else if (!strcmp(param, "hdf5")) {
        config->surface_mesh_format = TDM_HDF5;
      } else {
        // FIXME
      }
    } else if (!strcmp(state->current_param, "filename")) {
      config->surface_mesh_file = strdup(param);
    }
  } else if (state->parsing_column_mesh_output) {
    if (!strcmp(state->current_param, "format")) {
      if (!strcmp(param, "exodus")) {
        config->column_mesh_format = TDM_EXODUS;
      } else if (!strcmp(param, "hdf5")) {
        config->column_mesh_format = TDM_HDF5;
      } else {
        // FIXME
      }
    } else if (!strcmp(state->current_param, "filename")) {
      config->column_mesh_file = strdup(param);
    }
  }
  return (tdm_result_t){0};
}

// Handles a YAML event, populating our config.
static tdm_result_t handle_yaml_event(yaml_event_t   *event,
                                      parser_state_t *state,
                                      tdm_config_t   *config)
{
  tdm_result_t result = {};
  if (event->type == YAML_SCALAR_EVENT) {
    const char *value = (const char*)(event->data.scalar.value);
    // data block
    if (!state->parsing_data && !strcmp(value, "data")) {
      state->parsing_data = true;
    } else if (state->parsing_data) {
      if (!state->current_param[0]) { // check the parameter name
        const char *valid_names[] = {"dem", "lat", "lon", "mask"};
        result = check_param_name("data", state->data_param_names,
                                  valid_names, value);
        strncpy(state->current_param, value, 128);
      } else { // parse the value
        result = parse_data_param(state, value, config);
      }
    } else if (!state->parsing_jigsaw && !strcmp(value, "jigsaw")) {
      state->parsing_jigsaw = true;
    } else if (state->parsing_jigsaw) {
      if (!state->current_param[0]) { // check the parameter name
        const char *valid_names[] = {"verbosity", "geom_seed", "geom_feat"}; // FIXME
        result = check_param_name("jigsaw", state->jigsaw_param_names,
                                  valid_names, value);
        strncpy(state->current_param, value, 128);
      } else { // parse the value
        result = parse_jigsaw_param(state, value, config);
      }
    } else if (!state->parsing_extrusion && !strcmp(value, "extrusion")) {
      state->parsing_extrusion = true;
    } else if (state->parsing_extrusion) {
      if (!state->current_param[0]) { // check the parameter name
        const char *valid_names[] = {"layers", "thickness", "thicknesses"};
        result = check_param_name("extrusion", state->extrusion_param_names,
                                  valid_names, value);
        strncpy(state->current_param, value, 128);
      } else { // parse the value
        result = parse_extrusion_param(state, value, config);
      }
    } else if (!state->parsing_output && !strcmp(value, "output")) {
      state->parsing_output = true;
    } else if (state->parsing_output) {
      if (!state->current_param[0]) { // check the parameter name
        const char *valid_names[] = {"surface_mesh", "column_mesh"};
        result = check_param_name("output", state->output_param_names,
                                  valid_names, value);
        strncpy(state->current_param, value, 128);
      } else { // parse the value
        result = parse_output_param(state, value, config);
      }
    }
  } else if (event->type == YAML_MAPPING_START_EVENT) {
    if (state->current_param[0]) { // we're already parsing a parameter
      return tdm_result(1, "Illegal mapping encountered in parameter %s",
        state->current_param);
    }
  } else if (event->type == YAML_MAPPING_END_EVENT) {
    state->parsing_data = false;
    state->parsing_jigsaw = false;
    state->parsing_extrusion = false;
    state->parsing_output = false;
    state->current_param[0] = 0;
  } else if (event->type == YAML_SEQUENCE_START_EVENT) {
    if (state->parsing_data) {
      return tdm_result(1, "Encountered illegal array value in data block.");
    } else if (state->parsing_jigsaw) {
      return tdm_result(1, "Encountered illegal array value in jigsaw block.");
    } else if (state->parsing_extrusion && !state->parsing_thicknesses &&
               !strcmp(state->current_param, "thicknesses")) {
      state->parsing_thicknesses = true;
    } else if (state->parsing_output) {
      return tdm_result(1, "Encountered illegal array value in output block.");
    }
  } else if (event->type == YAML_SEQUENCE_END_EVENT) {
    if (state->parsing_extrusion && state->parsing_thicknesses) {
      state->parsing_thicknesses = false;
    }
    state->current_param[0] = 0;
  }
  return result;
}

static void destroy_name_set(khash_t(yaml_name_set) *name_set) {
  for (khiter_t iter = kh_begin(name_set); iter != kh_end(name_set); ++iter) {
    if (kh_exist(name_set, iter)) {
      free((char*)kh_key(name_set, iter)); // free strdup'd parameter name
    }
  }
  kh_destroy(yaml_name_set, name_set);
}

static void destroy_state(parser_state_t state) {
  destroy_name_set(state.data_param_names);
  destroy_name_set(state.jigsaw_param_names);
  destroy_name_set(state.extrusion_param_names);
  destroy_name_set(state.output_param_names);
}

tdm_result_t read_yaml(const char *yaml_file, tdm_config_t *config) {
  tdm_result_t result = {};

  FILE *file = fopen(yaml_file, "r");
  if (!file) {
    return tdm_result(1, "The file '%s' could not be opened.", yaml_file);
  }
  yaml_parser_t parser;
  yaml_parser_initialize(&parser);
  yaml_parser_set_input_file(&parser, file);

  parser_state_t state = {
    .data_param_names      = kh_init(yaml_name_set),
    .jigsaw_param_names    = kh_init(yaml_name_set),
    .extrusion_param_names = kh_init(yaml_name_set),
    .output_param_names    = kh_init(yaml_name_set)
  };
  yaml_event_type_t event_type;
  do {
    yaml_event_t event;

    // Parse the next YAML "event" and handle any errors encountered.
    yaml_parser_parse(&parser, &event);
    if (parser.error != YAML_NO_ERROR) {
      result = tdm_result(1, parser.problem);
      goto finished;
    }

    // Process the event, using it to populate our YAML data, and handle
    // any errors resulting from properly-formed YAML that doesn't conform
    // to Skywalker's spec.
    result = handle_yaml_event(&event, &state, config);
    if (result.err_code) goto finished;
    event_type = event.type;
    yaml_event_delete(&event);
  } while (event_type != YAML_STREAM_END_EVENT);

finished:
  yaml_parser_delete(&parser);
  destroy_state(state);
  fclose(file);

  return result;
}

