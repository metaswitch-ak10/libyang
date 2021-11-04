/**
 * @file schema_mount.c
 * @author Tadeas Vintrlik <xvintr04@stud.fit.vutbr.cz>
 * @brief libyang extension plugin - Schema Mount (RFC 8528)
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"
#include "libyang.h"
#include "plugins_exts.h"
#include "tree_schema.h"

/**
 * @brief Check if given mount point is unique among its' siblings
 *
 * @param cctx Compilation context.
 * @param p_ext Extension instance of the mount-point
 *
 * @return LY_SUCCESS if is unique. LY_EINVAL otherwise.
 */
static LY_ERR
schema_mount_unique_mount_point(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext)
{
    struct lysp_module *pmod;
    struct lysp_ext_instance *exts;
    LY_ARRAY_COUNT_TYPE u, v;
    struct lysp_node *parent;
    struct lysp_import *module;
    char *ext_prefix, *ext_name;

    /* Check if it is the only instace of the mount-point among its' siblings */
    parent = (struct lysp_node *) p_ext->parent;
    exts = parent->exts;
    pmod = lysc_ctx_get_pmod(cctx);
    LY_ARRAY_FOR(exts, u) {
        /* Extract prefix and name of the extension */
        ext_prefix = strdup(exts[u].name);
        ext_name = strstr(exts[u].name, ":");
        ext_name++;
        ext_prefix[strstr(ext_prefix, ":") - ext_prefix] = '\0';

        module = NULL;
        LY_ARRAY_FOR(pmod->imports, v) {
            if (!strcmp(pmod->imports[v].prefix, ext_prefix)) {
                /* Found the matching module */
                module = &pmod->imports[v];
                break;
            }
        }
        free(ext_prefix);
        if ((&exts[u] != p_ext) && module && (!strcmp(module->name, "ietf-yang-schema-mount")) && (!strcmp(exts[u].name, "mount-point"))) {
            /* Found another instance of mount-point only one allowed per node */
            return LY_EINVAL;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Schema mount compile.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
schema_mount_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext, struct lysc_ext_instance *c_ext)
{
    const struct lys_module *cur_mod;
    const char *label;
    LY_ERR ret;

    /* Check if processing right callback */
    assert(!strcmp(p_ext->name, "mount-point"));

    /* Check if mount point was found in YANG version 1.1 module */
    cur_mod = lysc_ctx_get_cur_mod(cctx);
    if (cur_mod->parsed->version != LYS_VERSION_1_1) {
        return LY_EINVAL;
    }

    /* Check if its' parent is a container or a list */
    if ((p_ext->parent_stmt != LY_STMT_CONTAINER) && (p_ext->parent_stmt != LY_STMT_LIST)) {
        return LY_EINVAL;
    }

    /* Check if the only mount-point among siblings */
    if (schema_mount_unique_mount_point(cctx, p_ext)) {
        return LY_EINVAL;
    }

    /* Insert the label of the mount-point into the dictionary */
    ret = lydict_insert(lysc_ctx_get_ctx(cctx), p_ext->argument, strlen(p_ext->argument), &label);
    if (ret) {
        return ret;
    } else {
        /* Insert label into extension instace */
        c_ext->data = (void *) label;
        return ret;
    }
}

static LY_ERR
schema_mount_validate(struct lysc_ext_instance *ext, struct lyd_node *node)
{
    return LY_SUCCESS;
}

/**
 * @brief Plugin descriptions for the Yang Schema Mount extension.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_schema_mount[] = {
    {
        .module = "ietf-yang-schema-mount",
        .revision = "2019-01-14",
        .name = "mount-point",

        .plugin.id = "libyang 2 - Schema Mount, version 1",
        .plugin.compile = &schema_mount_compile,
        .plugin.validate = &schema_mount_validate,
        .plugin.sprinter = NULL,
        .plugin.free = NULL
    },
    {0} /* terminating zeroed item */
};
