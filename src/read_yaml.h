#ifndef TDM_READ_YAML_H
#define TDM_READ_YAML_H

#include "tdm.h"

// This function constructs a tdm_config from the contents of a YAML file,
// reporting any errors encountered in parsing.
tdm_result_t read_yaml(const char *yaml_file, tdm_config_t *config);


#endif
