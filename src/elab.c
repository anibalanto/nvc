//
//  Copyright (C) 2011  Nick Gasson
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "phase.h"
#include "util.h"

#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

static ident_t hpathf(ident_t path, char sep, const char *fmt, ...)
{
   va_list ap;
   char buf[256];
   va_start(ap, fmt);
   vsnprintf(buf, sizeof(buf), fmt, ap);
   va_end(ap);

   // LRM specifies instance path is lowercase
   char *p = buf;
   while (*p != '\0') {
      *p = tolower(*p);
      ++p;
   }

   return ident_prefix(path, ident_new(buf), sep);
}

static const char *simple_name(const char *full)
{
   // Strip off any library prefix from the parameter
   const char *start = full;
   for (const char *p = full; *p != '\0'; p++) {
      if (*p == '.')
         start = p + 1;
   }

   return start;
}

static void elab_arch(tree_t t, tree_t out, ident_t path)
{
   for (unsigned i = 0; i < tree_stmts(t); i++) {
      tree_t s = tree_stmt(t, i);
      assert(tree_kind(s) == T_PROCESS);  // TODO: elab_stmt

      tree_set_ident(s, hpathf(path, ':', "%s", istr(tree_ident(s))));

      tree_add_stmt(out, s);
   }
}

struct arch_search_params {
   ident_t name;
   tree_t  *arch;
};

static void find_arch(tree_t t, void *context)
{
   struct arch_search_params *params = context;

   if (tree_kind(t) == T_ARCH && tree_ident2(t) == params->name)
      *(params->arch) = t;
}

static void elab_entity(tree_t t, tree_t out, ident_t path)
{
   // XXX: LRM rules for selecting architecture?

   tree_t arch = NULL;
   struct arch_search_params params = { tree_ident(t), &arch };
   lib_foreach(lib_work(), find_arch, &params);

   if (arch == NULL)
      fatal("no suitable architecture for entity %s", istr(tree_ident(t)));

   printf("selected architecture %s of %s\n", istr(tree_ident(t)),
          istr(tree_ident(arch)));

   ident_t new_path = hpathf(path, '@', "%s(%s)",
                             istr(tree_ident(t)),
                             simple_name(istr(tree_ident(arch))));

   elab_arch(arch, out, new_path);
}

tree_t elab(tree_t top)
{
   lib_load_all(lib_work());

   tree_t e = tree_new(T_ELAB);
   tree_set_ident(e, ident_prefix(tree_ident(top),
                                  ident_new("elab"), '.'));

   switch (tree_kind(top)) {
   case T_ENTITY:
      elab_entity(top, e, NULL);
      break;
   default:
      fatal("%s is not a suitable top-level unit", istr(tree_ident(top)));
   }

   lib_put(lib_work(), e);
   return e;
}
