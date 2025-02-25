/**
 * @file
 * Test code for the Quad object
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

#define TEST_NO_MAIN
#include "acutest.h"
#include "config.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "mutt/mutt.h"
#include "config/common.h"
#include "config/lib.h"
#include "account.h"

static char VarApple;
static char VarBanana;
static char VarCherry;
static char VarDamson;
static char VarElderberry;
static char VarFig;
static char VarGuava;
static char VarHawthorn;
static char VarIlama;
static char VarJackfruit;
static char VarKumquat;
static char VarLemon;
static char VarMango;
static char VarNectarine;
static char VarOlive;

// clang-format off
static struct ConfigDef Vars[] = {
  { "Apple",      DT_QUAD, &VarApple,      0, 0, NULL              }, /* test_initial_values */
  { "Banana",     DT_QUAD, &VarBanana,     3, 0, NULL              },
  { "Cherry",     DT_QUAD, &VarCherry,     0, 0, NULL              },
  { "Damson",     DT_QUAD, &VarDamson,     0, 0, NULL              }, /* test_string_set */
  { "Elderberry", DT_QUAD, &VarElderberry, 0, 0, NULL              }, /* test_string_get */
  { "Fig",        DT_QUAD, &VarFig,        0, 0, NULL              }, /* test_native_set */
  { "Guava",      DT_QUAD, &VarGuava,      0, 0, NULL              }, /* test_native_get */
  { "Hawthorn",   DT_QUAD, &VarHawthorn,   0, 0, NULL              }, /* test_reset */
  { "Ilama",      DT_QUAD, &VarIlama,      0, 0, validator_fail    },
  { "Jackfruit",  DT_QUAD, &VarJackfruit,  0, 0, validator_succeed }, /* test_validator */
  { "Kumquat",    DT_QUAD, &VarKumquat,    0, 0, validator_warn    },
  { "Lemon",      DT_QUAD, &VarLemon,      0, 0, validator_fail    },
  { "Mango",      DT_QUAD, &VarMango,      0, 0, NULL              }, /* test_inherit */
  { "Nectarine",  DT_QUAD, &VarNectarine,  0, 0, NULL              }, /* test_toggle */
  { "Olive",      DT_BOOL, &VarOlive,      0, 0, NULL              },
  { NULL },
};
// clang-format on

static bool test_initial_values(struct ConfigSet *cs, struct Buffer *err)
{
  log_line(__func__);
  TEST_MSG("Apple = %d\n", VarApple);
  TEST_MSG("Banana = %d\n", VarBanana);

  if (!TEST_CHECK(VarApple == 0))
  {
    TEST_MSG("Expected: %d\n", 0);
    TEST_MSG("Actual  : %d\n", VarApple);
  }

  if (!TEST_CHECK(VarBanana == 3))
  {
    TEST_MSG("Expected: %d\n", 3);
    TEST_MSG("Actual  : %d\n", VarBanana);
  }

  cs_str_string_set(cs, "Apple", "ask-yes", err);
  cs_str_string_set(cs, "Banana", "ask-no", err);

  struct Buffer value;
  mutt_buffer_init(&value);
  value.dsize = 256;
  value.data = mutt_mem_calloc(1, value.dsize);

  int rc;

  mutt_buffer_reset(&value);
  rc = cs_str_initial_get(cs, "Apple", &value);
  if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("%s\n", value.data);
    FREE(&value.data);
    return false;
  }

  if (!TEST_CHECK(mutt_str_strcmp(value.data, "no") == 0))
  {
    TEST_MSG("Apple's initial value is wrong: '%s'\n", value.data);
    FREE(&value.data);
    return false;
  }
  TEST_MSG("Apple = %d\n", VarApple);
  TEST_MSG("Apple's initial value is '%s'\n", value.data);

  mutt_buffer_reset(&value);
  rc = cs_str_initial_get(cs, "Banana", &value);
  if (CSR_RESULT(rc) != CSR_SUCCESS)
  {
    TEST_MSG("%s\n", value.data);
    FREE(&value.data);
    return false;
  }

  if (!TEST_CHECK(mutt_str_strcmp(value.data, "ask-yes") == 0))
  {
    TEST_MSG("Banana's initial value is wrong: '%s'\n", value.data);
    FREE(&value.data);
    return false;
  }
  TEST_MSG("Banana = %d\n", VarBanana);
  TEST_MSG("Banana's initial value is '%s'\n", NONULL(value.data));

  mutt_buffer_reset(&value);
  rc = cs_str_initial_set(cs, "Cherry", "ask-yes", &value);
  if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("%s\n", value.data);
    FREE(&value.data);
    return false;
  }

  mutt_buffer_reset(&value);
  rc = cs_str_initial_get(cs, "Cherry", &value);
  if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("%s\n", value.data);
    FREE(&value.data);
    return false;
  }

  TEST_MSG("Cherry = '%s'\n", VarCherry ? "yes" : "no");
  TEST_MSG("Cherry's initial value is '%s'\n", NONULL(value.data));

  FREE(&value.data);
  log_line(__func__);
  return true;
}

static bool test_string_set(struct ConfigSet *cs, struct Buffer *err)
{
  log_line(__func__);

  const char *valid[] = { "no", "yes", "ask-no", "ask-yes" };
  const char *invalid[] = {
    "nope",
    "ye",
    "",
    NULL,
  };
  const char *name = "Damson";

  int rc;
  for (unsigned int i = 0; i < mutt_array_size(valid); i++)
  {
    VarDamson = ((i + 1) % 4);

    TEST_MSG("Setting %s to %s\n", name, valid[i]);
    mutt_buffer_reset(err);
    rc = cs_str_string_set(cs, name, valid[i], err);
    if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
    {
      TEST_MSG("%s\n", err->data);
      return false;
    }

    if (VarDamson == ((i + 1) % 4))
    {
      TEST_MSG("Value of %s wasn't changed\n", name);
      return false;
    }
    TEST_MSG("%s = %d, set by '%s'\n", name, VarDamson, valid[i]);

    if (i == 2)
    {
      mutt_buffer_reset(err);
      rc = cs_str_string_set(cs, name, valid[i], err);
      if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
      {
        TEST_MSG("%s\n", err->data);
        return false;
      }

      if (rc & CSR_SUC_NO_CHANGE)
      {
        TEST_MSG("Value of %s wasn't changed\n", name);
        continue;
      }
    }
    short_line();
  }

  for (unsigned int i = 0; i < mutt_array_size(invalid); i++)
  {
    mutt_buffer_reset(err);
    rc = cs_str_string_set(cs, name, invalid[i], err);
    if (TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
    {
      TEST_MSG("Expected error: %s\n", err->data);
    }
    else
    {
      TEST_MSG("%s = %d, set by '%s'\n", name, VarDamson, invalid[i]);
      TEST_MSG("This test should have failed\n");
      return false;
    }
    short_line();
  }

  log_line(__func__);
  return true;
}

static bool test_string_get(struct ConfigSet *cs, struct Buffer *err)
{
  log_line(__func__);
  const char *name = "Elderberry";

  int valid[] = { MUTT_NO, MUTT_YES, MUTT_ASKNO, MUTT_ASKYES };

  int rc;
  for (unsigned int i = 0; i < mutt_array_size(valid); i++)
  {
    VarElderberry = valid[i];
    mutt_buffer_reset(err);
    rc = cs_str_string_get(cs, name, err);
    if (CSR_RESULT(rc) != CSR_SUCCESS)
    {
      TEST_MSG("Get failed: %s\n", err->data);
      return false;
    }
    TEST_MSG("%s = %d, %s\n", name, VarElderberry, err->data);
  }

  VarElderberry = 4;
  mutt_buffer_reset(err);
  TEST_MSG("Expect error for next test\n");
  rc = cs_str_string_get(cs, name, err);
  if (!TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
  {
    TEST_MSG("%s\n", err->data);
    return false;
  }

  log_line(__func__);
  return true;
}

static bool test_native_set(struct ConfigSet *cs, struct Buffer *err)
{
  log_line(__func__);
  const char *name = "Fig";
  char value = MUTT_YES;

  TEST_MSG("Setting %s to %d\n", name, value);
  VarFig = MUTT_NO;
  mutt_buffer_reset(err);
  int rc = cs_str_native_set(cs, name, value, err);
  if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("%s\n", err->data);
    return false;
  }

  if (!TEST_CHECK(VarFig == value))
  {
    TEST_MSG("Value of %s wasn't changed\n", name);
    return false;
  }

  TEST_MSG("%s = %d, set to '%d'\n", name, VarFig, value);

  short_line();
  mutt_buffer_reset(err);
  TEST_MSG("Setting %s to %d\n", name, value);
  rc = cs_str_native_set(cs, name, value, err);
  if (!TEST_CHECK((rc & CSR_SUC_NO_CHANGE) != 0))
  {
    TEST_MSG("%s\n", err->data);
    return false;
  }

  if (rc & CSR_SUC_NO_CHANGE)
  {
    TEST_MSG("Value of %s wasn't changed\n", name);
  }

  int invalid[] = { -1, 4 };
  for (unsigned int i = 0; i < mutt_array_size(invalid); i++)
  {
    short_line();
    VarFig = MUTT_NO;
    TEST_MSG("Setting %s to %d\n", name, invalid[i]);
    mutt_buffer_reset(err);
    rc = cs_str_native_set(cs, name, invalid[i], err);
    if (TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
    {
      TEST_MSG("Expected error: %s\n", err->data);
    }
    else
    {
      TEST_MSG("%s = %d, set by '%d'\n", name, VarFig, invalid[i]);
      TEST_MSG("This test should have failed\n");
      return false;
    }
  }

  log_line(__func__);
  return true;
}

static bool test_native_get(struct ConfigSet *cs, struct Buffer *err)
{
  log_line(__func__);
  const char *name = "Guava";

  VarGuava = MUTT_YES;
  mutt_buffer_reset(err);
  intptr_t value = cs_str_native_get(cs, name, err);
  if (!TEST_CHECK(value != INT_MIN))
  {
    TEST_MSG("Get failed: %s\n", err->data);
    return false;
  }
  TEST_MSG("%s = %ld\n", name, value);

  log_line(__func__);
  return true;
}

static bool test_reset(struct ConfigSet *cs, struct Buffer *err)
{
  log_line(__func__);

  const char *name = "Hawthorn";
  VarHawthorn = MUTT_YES;
  mutt_buffer_reset(err);

  TEST_MSG("%s = %d\n", name, VarJackfruit);
  int rc = cs_str_reset(cs, name, err);
  if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("%s\n", err->data);
    return false;
  }

  if (!TEST_CHECK(VarHawthorn != true))
  {
    TEST_MSG("Value of %s wasn't changed\n", name);
    return false;
  }

  TEST_MSG("Reset: %s = %d\n", name, VarHawthorn);

  short_line();
  name = "Ilama";
  mutt_buffer_reset(err);

  TEST_MSG("Initial: %s = %d\n", name, VarIlama);
  dont_fail = true;
  rc = cs_str_string_set(cs, name, "ask-yes", err);
  if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
    return false;
  TEST_MSG("Set: %s = %d\n", name, VarIlama);
  dont_fail = false;

  rc = cs_str_reset(cs, name, err);
  if (TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
  {
    TEST_MSG("Expected error: %s\n", err->data);
  }
  else
  {
    TEST_MSG("%s\n", err->data);
    return false;
  }

  if (!TEST_CHECK(VarIlama == MUTT_ASKYES))
  {
    TEST_MSG("Value of %s changed\n", name);
    return false;
  }

  TEST_MSG("Reset: %s = %d\n", name, VarIlama);

  log_line(__func__);
  return true;
}

static bool test_validator(struct ConfigSet *cs, struct Buffer *err)
{
  log_line(__func__);

  const char *name = "Jackfruit";
  VarJackfruit = MUTT_NO;
  mutt_buffer_reset(err);
  int rc = cs_str_string_set(cs, name, "yes", err);
  if (TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("%s\n", err->data);
  }
  else
  {
    TEST_MSG("%s\n", err->data);
    return false;
  }
  TEST_MSG("String: %s = %d\n", name, VarJackfruit);
  short_line();

  VarJackfruit = MUTT_NO;
  mutt_buffer_reset(err);
  rc = cs_str_native_set(cs, name, 1, err);
  if (TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("%s\n", err->data);
  }
  else
  {
    TEST_MSG("%s\n", err->data);
    return false;
  }
  TEST_MSG("Native: %s = %d\n", name, VarJackfruit);
  short_line();

  name = "Kumquat";
  VarKumquat = MUTT_NO;
  mutt_buffer_reset(err);
  rc = cs_str_string_set(cs, name, "yes", err);
  if (TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("%s\n", err->data);
  }
  else
  {
    TEST_MSG("%s\n", err->data);
    return false;
  }
  TEST_MSG("String: %s = %d\n", name, VarKumquat);
  short_line();

  VarKumquat = MUTT_NO;
  mutt_buffer_reset(err);
  rc = cs_str_native_set(cs, name, 1, err);
  if (TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("%s\n", err->data);
  }
  else
  {
    TEST_MSG("%s\n", err->data);
    return false;
  }
  TEST_MSG("Native: %s = %d\n", name, VarKumquat);
  short_line();

  name = "Lemon";
  VarLemon = MUTT_NO;
  mutt_buffer_reset(err);
  rc = cs_str_string_set(cs, name, "yes", err);
  if (TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
  {
    TEST_MSG("Expected error: %s\n", err->data);
  }
  else
  {
    TEST_MSG("%s\n", err->data);
    return false;
  }
  TEST_MSG("String: %s = %d\n", name, VarLemon);
  short_line();

  VarLemon = MUTT_NO;
  mutt_buffer_reset(err);
  rc = cs_str_native_set(cs, name, 1, err);
  if (TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
  {
    TEST_MSG("Expected error: %s\n", err->data);
  }
  else
  {
    TEST_MSG("%s\n", err->data);
    return false;
  }
  TEST_MSG("Native: %s = %d\n", name, VarLemon);

  log_line(__func__);
  return true;
}

static void dump_native(struct ConfigSet *cs, const char *parent, const char *child)
{
  intptr_t pval = cs_str_native_get(cs, parent, NULL);
  intptr_t cval = cs_str_native_get(cs, child, NULL);

  TEST_MSG("%15s = %ld\n", parent, pval);
  TEST_MSG("%15s = %ld\n", child, cval);
}

static bool test_inherit(struct ConfigSet *cs, struct Buffer *err)
{
  log_line(__func__);
  bool result = false;

  const char *account = "fruit";
  const char *parent = "Mango";
  char child[128];
  snprintf(child, sizeof(child), "%s:%s", account, parent);

  const char *AccountVarStr[] = {
    parent,
    NULL,
  };

  struct Account *a = account_new();
  account_add_config(a, cs, account, AccountVarStr);

  // set parent
  mutt_buffer_reset(err);
  int rc = cs_str_string_set(cs, parent, "yes", err);
  if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("Error: %s\n", err->data);
    goto ti_out;
  }
  dump_native(cs, parent, child);
  short_line();

  // set child
  mutt_buffer_reset(err);
  rc = cs_str_string_set(cs, child, "no", err);
  if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("Error: %s\n", err->data);
    goto ti_out;
  }
  if (rc & CSR_SUC_NO_CHANGE)
  {
    TEST_MSG("Value of %s wasn't changed\n", parent);
  }
  dump_native(cs, parent, child);
  short_line();

  // reset child
  mutt_buffer_reset(err);
  rc = cs_str_reset(cs, child, err);
  if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("Error: %s\n", err->data);
    goto ti_out;
  }
  dump_native(cs, parent, child);
  short_line();

  // reset parent
  mutt_buffer_reset(err);
  rc = cs_str_reset(cs, parent, err);
  if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
  {
    TEST_MSG("Error: %s\n", err->data);
    goto ti_out;
  }
  dump_native(cs, parent, child);

  log_line(__func__);
  result = true;
ti_out:
  account_free(&a);
  return result;
}

static bool test_toggle(struct ConfigSet *cs, struct Buffer *err)
{
  log_line(__func__);

  struct ToggleTest
  {
    char before;
    char after;
  };

  struct ToggleTest tests[] = {
    { MUTT_NO, MUTT_YES },
    { MUTT_YES, MUTT_NO },
    { MUTT_ASKNO, MUTT_ASKYES },
    { MUTT_ASKYES, MUTT_ASKNO },
  };

  const char *name = "Nectarine";
  int rc;

  struct HashElem *he = cs_get_elem(cs, name);
  if (!he)
    return false;

  rc = quad_he_toggle(NULL, he, err);
  if (!TEST_CHECK(CSR_RESULT(rc) == CSR_ERR_CODE))
  {
    TEST_MSG("Toggle succeeded when is shouldn't have\n");
    return false;
  }

  rc = quad_he_toggle(cs, NULL, err);
  if (!TEST_CHECK(CSR_RESULT(rc) == CSR_ERR_CODE))
  {
    TEST_MSG("Toggle succeeded when is shouldn't have\n");
    return false;
  }

  for (size_t i = 0; i < mutt_array_size(tests); i++)
  {
    char before = tests[i].before;
    char after = tests[i].after;
    TEST_MSG("test %zu\n", i);

    VarNectarine = before;
    mutt_buffer_reset(err);
    intptr_t value = cs_he_native_get(cs, he, err);
    if (!TEST_CHECK(value != INT_MIN))
    {
      TEST_MSG("Get failed: %s\n", err->data);
      return false;
    }

    char copy = value;
    if (!TEST_CHECK(copy == before))
    {
      TEST_MSG("Initial value is wrong: %s\n", err->data);
      return false;
    }

    rc = quad_he_toggle(cs, he, err);
    if (!TEST_CHECK(CSR_RESULT(rc) == CSR_SUCCESS))
    {
      TEST_MSG("Toggle failed: %s\n", err->data);
      return false;
    }

    if (!TEST_CHECK(VarNectarine == after))
    {
      TEST_MSG("Toggle value is wrong: %s\n", err->data);
      return false;
    }
  }

  VarNectarine = 8;
  mutt_buffer_reset(err);
  rc = quad_he_toggle(cs, he, err);
  if (TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
  {
    TEST_MSG("Expected error: %s\n", err->data);
  }

  name = "Olive";
  he = cs_get_elem(cs, name);
  if (!he)
    return false;

  mutt_buffer_reset(err);
  rc = quad_he_toggle(cs, he, err);
  if (!TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
  {
    TEST_MSG("Expected error: %s\n", err->data);
  }

  log_line(__func__);
  return true;
}

void config_quad(void)
{
  struct Buffer err;
  mutt_buffer_init(&err);
  err.dsize = 256;
  err.data = mutt_mem_calloc(1, err.dsize);
  mutt_buffer_reset(&err);

  struct ConfigSet *cs = cs_new(30);

  bool_init(cs);
  quad_init(cs);
  dont_fail = true;
  if (!cs_register_variables(cs, Vars, 0))
    return;
  dont_fail = false;

  notify_observer_add(cs->notify, NT_CONFIG, 0, log_observer, 0);

  set_list(cs);

  TEST_CHECK(test_initial_values(cs, &err));
  TEST_CHECK(test_string_set(cs, &err));
  TEST_CHECK(test_string_get(cs, &err));
  TEST_CHECK(test_native_set(cs, &err));
  TEST_CHECK(test_native_get(cs, &err));
  TEST_CHECK(test_reset(cs, &err));
  TEST_CHECK(test_validator(cs, &err));
  TEST_CHECK(test_inherit(cs, &err));
  TEST_CHECK(test_toggle(cs, &err));

  cs_free(&cs);
  FREE(&err.data);
}
