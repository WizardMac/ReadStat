//
//  readstat_por.h
//  Wizard
//
//  Created by Evan Miller on 4/17/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "readstat.h"

int parse_por(const char *filename, void *user_ctx,
              readstat_handle_info_callback info_cb, readstat_handle_variable_callback variable_cb,
              readstat_handle_value_callback value_cb, readstat_handle_value_label_callback value_label_cb);