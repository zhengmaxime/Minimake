#pragma once

#include "vars_rules.h"

char *substitute_vars(struct vars_rules *vr, char *line, int *new);
