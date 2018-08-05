/**
 * @file
 * Type representing a regular expression
 *
 * @authors
 * Copyright (C) 2017-2018 Richard Russon <rich@flatcap.org>
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @page config-regex Type: Regular expression
 *
 * Type representing a regular expression.
 */

#include "config.h"
#include <stddef.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include "mutt/buffer.h"
#include "mutt/mbyte.h"
#include "mutt/memory.h"
#include "mutt/regex3.h"
#include "mutt/string2.h"
#include "regex2.h"
#include "set.h"
#include "types.h"

/**
 * regex_destroy - Destroy a Regex object - Implements ::cst_destroy
 */
static void regex_destroy(const struct ConfigSet *cs, void *var, const struct ConfigDef *cdef)
{
  if (!cs || !var || !cdef)
    return; /* LCOV_EXCL_LINE */

  struct Regex **r = (struct Regex **) var;
  if (!*r)
    return;

  regex_free(r);
}

/**
 * regex_string_set - Set a Regex by string - Implements ::cst_string_set
 */
static int regex_string_set(const struct ConfigSet *cs, void *var, struct ConfigDef *cdef,
                            const char *value, struct Buffer *err)
{
  if (!cs || !cdef)
    return CSR_ERR_CODE; /* LCOV_EXCL_LINE */

  /* Store empty strings as NULL */
  if (value && (value[0] == '\0'))
    value = NULL;

  struct Regex *r = NULL;

  int rc = CSR_SUCCESS;

  if (var)
  {
    struct Regex *curval = *(struct Regex **) var;
    if (curval && (mutt_str_strcmp(value, curval->pattern) == 0))
      return CSR_SUCCESS | CSR_SUC_NO_CHANGE;

    if (value)
    {
      r = regex_create(value, cdef->type, err);
      if (!r)
        return CSR_ERR_INVALID;
    }

    if (cdef->validator)
    {
      rc = cdef->validator(cs, cdef, (intptr_t) r, err);

      if (CSR_RESULT(rc) != CSR_SUCCESS)
      {
        regex_free(&r);
        return rc | CSR_INV_VALIDATOR;
      }
    }

    regex_destroy(cs, var, cdef);

    *(struct Regex **) var = r;

    if (!r)
      rc |= CSR_SUC_EMPTY;
  }
  else
  {
    if (cdef->type & DT_INITIAL_SET)
      FREE(&cdef->initial);

    cdef->type |= DT_INITIAL_SET;
    cdef->initial = IP mutt_str_strdup(value);
  }

  return rc;
}

/**
 * regex_string_get - Get a Regex as a string - Implements ::cst_string_get
 */
static int regex_string_get(const struct ConfigSet *cs, void *var,
                            const struct ConfigDef *cdef, struct Buffer *result)
{
  if (!cs || !cdef)
    return CSR_ERR_CODE; /* LCOV_EXCL_LINE */

  const char *str = NULL;

  if (var)
  {
    struct Regex *r = *(struct Regex **) var;
    if (r)
      str = r->pattern;
  }
  else
  {
    str = (char *) cdef->initial;
  }

  if (!str)
    return CSR_SUCCESS | CSR_SUC_EMPTY; /* empty string */

  mutt_buffer_addstr(result, str);
  return CSR_SUCCESS;
}

/**
 * regex_native_set - Set a Regex config item by Regex object - Implements ::cst_native_set
 */
static int regex_native_set(const struct ConfigSet *cs, void *var,
                            const struct ConfigDef *cdef, intptr_t value, struct Buffer *err)
{
  if (!cs || !var || !cdef)
    return CSR_ERR_CODE; /* LCOV_EXCL_LINE */

  int rc;

  if (cdef->validator)
  {
    rc = cdef->validator(cs, cdef, value, err);

    if (CSR_RESULT(rc) != CSR_SUCCESS)
      return rc | CSR_INV_VALIDATOR;
  }

  rc = CSR_SUCCESS;
  struct Regex *orig = (struct Regex *) value;
  struct Regex *r = NULL;

  if (orig && orig->pattern)
  {
    const int flags = orig->not? DT_REGEX_ALLOW_NOT : 0;
    r = regex_create(orig->pattern, flags, err);
    if (!r)
      rc = CSR_ERR_INVALID;
  }
  else
  {
    rc |= CSR_SUC_EMPTY;
  }

  if (CSR_RESULT(rc) == CSR_SUCCESS)
  {
    regex_free(var);
    *(struct Regex **) var = r;
  }

  return rc;
}

/**
 * regex_native_get - Get a Regex object from a Regex config item - Implements ::cst_native_get
 */
static intptr_t regex_native_get(const struct ConfigSet *cs, void *var,
                                 const struct ConfigDef *cdef, struct Buffer *err)
{
  if (!cs || !var || !cdef)
    return INT_MIN; /* LCOV_EXCL_LINE */

  struct Regex *r = *(struct Regex **) var;

  return (intptr_t) r;
}

/**
 * regex_reset - Reset a Regex to its initial value - Implements ::cst_regex
 */
static int regex_reset(const struct ConfigSet *cs, void *var,
                       const struct ConfigDef *cdef, struct Buffer *err)
{
  if (!cs || !var || !cdef)
    return CSR_ERR_CODE; /* LCOV_EXCL_LINE */

  struct Regex *r = NULL;
  const char *initial = (const char *) cdef->initial;

  struct Regex *currx = *(struct Regex **) var;
  const char *curval = currx ? currx->pattern : NULL;

  int rc = CSR_SUCCESS;
  if (!currx)
    rc |= CSR_SUC_EMPTY;

  if (mutt_str_strcmp(initial, curval) == 0)
    return rc | CSR_SUC_NO_CHANGE;

  if (initial)
  {
    r = regex_create(initial, cdef->type, err);
    if (!r)
      return CSR_ERR_CODE;
  }

  if (cdef->validator)
  {
    rc = cdef->validator(cs, cdef, (intptr_t) r, err);

    if (CSR_RESULT(rc) != CSR_SUCCESS)
    {
      regex_destroy(cs, &r, cdef);
      return rc | CSR_INV_VALIDATOR;
    }
  }

  if (!r)
    rc |= CSR_SUC_EMPTY;

  regex_destroy(cs, var, cdef);

  *(struct Regex **) var = r;
  return rc;
}

/**
 * regex_init - Register the Regex config type
 * @param cs Config items
 */
void regex_init(struct ConfigSet *cs)
{
  const struct ConfigSetType cst_regex = {
    "regex",          regex_string_set, regex_string_get, regex_native_set,
    regex_native_get, regex_reset,      regex_destroy,
  };
  cs_register_type(cs, DT_REGEX, &cst_regex);
}

/**
 * regex_create - Create an Regex from a string
 * @param str   Regular expression
 * @param flags Type flags, e.g. #DT_REGEX_MATCH_CASE
 * @param err   Buffer for error messages
 * @retval ptr New Regex object
 * @retval NULL Error
 */
struct Regex *regex_create(const char *str, int flags, struct Buffer *err)
{
  if (!str)
    return NULL; /* LCOV_EXCL_LINE */

  int rflags = 0;
  struct Regex *reg = mutt_mem_calloc(1, sizeof(struct Regex));

  reg->regex = mutt_mem_calloc(1, sizeof(regex_t));
  reg->pattern = mutt_str_strdup(str);

  /* Should we use smart case matching? */
  if (((flags & DT_REGEX_MATCH_CASE) == 0) && mutt_mb_is_lower(str))
    rflags |= REG_ICASE;

  if ((flags & DT_REGEX_NOSUB))
    rflags |= REG_NOSUB;

  /* Is a prefix of '!' allowed? */
  if (((flags & DT_REGEX_ALLOW_NOT) != 0) && (str[0] == '!'))
  {
    reg->not = true;
    str++;
  }

  int rc = REGCOMP(reg->regex, str, rflags);
  if ((rc != 0) && err)
  {
    regerror(rc, reg->regex, err->data, err->dsize);
    regex_free(&reg);
    return NULL;
  }

  return reg;
}

/**
 * regex_free - Free a Regex object
 * @param r Regex to free
 */
void regex_free(struct Regex **r)
{
  if (!r || !*r)
    return; /* LCOV_EXCL_LINE */

  FREE(&(*r)->pattern);
  if ((*r)->regex)
    regfree((*r)->regex);
  FREE(&(*r)->regex);
  FREE(r);
}