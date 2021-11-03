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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"
#include "plugins_exts.h"

/**
 * @brief Schema mount.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
schema_mount_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext, struct lysc_ext_instance *c_ext)
{
    return LY_SUCCESS;
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
