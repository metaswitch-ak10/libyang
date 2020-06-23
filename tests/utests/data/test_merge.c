/**
 * @file test_merge.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief tests for complex data merges.
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>

#include "libyang.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *source;
    struct lyd_node *target;
    struct lyd_node *result;
};

static int
setup_dflt(void **state)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error.\n");
        return -1;
    }

    /* libyang context */
    if (ly_ctx_new(NULL, 0, &st->ctx)) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
teardown_dflt(void **state)
{
    struct state *st = (*state);

    lyd_free_siblings(st->target);
    lyd_free_siblings(st->source);
    lyd_free_siblings(st->result);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_batch(void **state)
{
    struct state *st = (*state);
    LY_ERR ret;
    uint32_t i;
    char *str;

    const char *start =
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>yang</name>"
            "<revision>2016-02-11</revision>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>";
    const char *data[] = {
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-yang-library</name>"
            "<revision>2016-02-01</revision>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf-acm</name>"
            "<revision>2012-02-22</revision>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf</name>"
            "<revision>2011-06-01</revision>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf-monitoring</name>"
            "<revision>2010-10-04</revision>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf-with-defaults</name>"
            "<revision>2011-06-01</revision>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>yang</name>"
            "<revision>2016-02-11</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:1</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-yang-library</name>"
            "<revision>2016-02-01</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf-acm</name>"
            "<revision>2012-02-22</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf</name>"
            "<revision>2011-06-01</revision>"
            "<namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>"
            "<feature>writable-running</feature>"
            "<feature>candidate</feature>"
            "<feature>rollback-on-error</feature>"
            "<feature>validate</feature>"
            "<feature>startup</feature>"
            "<feature>xpath</feature>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf-monitoring</name>"
            "<revision>2010-10-04</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    ,
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>ietf-netconf-with-defaults</name>"
            "<revision>2011-06-01</revision>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults</namespace>"
            "<conformance-type>implement</conformance-type>"
        "</module>"
    "</modules-state>"
    };
    const char *output_template =
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
        "<module>"
            "<name>yang</name>"
            "<revision>2016-02-11</revision>"
            "<conformance-type>implement</conformance-type>"
            "<namespace>urn:ietf:params:xml:ns:yang:1</namespace>"
        "</module>"
        "<module>"
            "<name>ietf-yang-library</name>"
            "<revision>2016-02-01</revision>"
            "<conformance-type>implement</conformance-type>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>"
        "</module>"
        "<module>"
            "<name>ietf-netconf-acm</name>"
            "<revision>2012-02-22</revision>"
            "<conformance-type>implement</conformance-type>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-acm</namespace>"
        "</module>"
        "<module>"
            "<name>ietf-netconf</name>"
            "<revision>2011-06-01</revision>"
            "<conformance-type>implement</conformance-type>"
            "<namespace>urn:ietf:params:xml:ns:netconf:base:1.0</namespace>"
            "<feature>writable-running</feature>"
            "<feature>candidate</feature>"
            "<feature>rollback-on-error</feature>"
            "<feature>validate</feature>"
            "<feature>startup</feature>"
            "<feature>xpath</feature>"
        "</module>"
        "<module>"
            "<name>ietf-netconf-monitoring</name>"
            "<revision>2010-10-04</revision>"
            "<conformance-type>implement</conformance-type>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring</namespace>"
        "</module>"
        "<module>"
            "<name>ietf-netconf-with-defaults</name>"
            "<revision>2011-06-01</revision>"
            "<conformance-type>implement</conformance-type>"
            "<namespace>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults</namespace>"
        "</module>"
    "</modules-state>";

    st->target = lyd_parse_mem(st->ctx, start, LYD_XML, LYD_OPT_PARSE_ONLY);
    assert_non_null(st->target);

    for (i = 0; i < 11; ++i) {
        st->source = lyd_parse_mem(st->ctx, data[i], LYD_XML, LYD_OPT_PARSE_ONLY);
        assert_non_null(st->source);

        ret = lyd_merge(&st->target, st->source, LYD_MERGE_DESTRUCT);
        assert_int_equal(ret, LY_SUCCESS);
        st->source = NULL;
    }

    lyd_print_mem(&str, st->target, LYD_XML, 0);
    assert_string_equal(str, output_template);
    free(str);
}

static void
test_leaf(void **state)
{
    struct state *st = (*state);
    const char *sch = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "    container A {"
                    "      leaf f1 {type string;}"
                    "      container B {"
                    "        leaf f2 {type string;}"
                    "      }"
                    "    }"
                    "  }";
    const char *trg = "<A xmlns=\"urn:x\"> <f1>block</f1> </A>";
    const char *src = "<A xmlns=\"urn:x\"> <f1>aa</f1> <B> <f2>bb</f2> </B> </A>";
    const char *result = "<A xmlns=\"urn:x\"><f1>aa</f1><B><f2>bb</f2></B></A>";
    char *printed = NULL;

    assert_non_null(lys_parse_mem(st->ctx, sch, LYS_IN_YANG));

    st->source = lyd_parse_mem(st->ctx, src, LYD_XML, LYD_VALOPT_DATA_ONLY);
    assert_non_null(st->source);

    st->target = lyd_parse_mem(st->ctx, trg, LYD_XML, LYD_VALOPT_DATA_ONLY);
    assert_non_null(st->target);

    /* merge them */
    assert_int_equal(lyd_merge(&st->target, st->source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate(&st->target, NULL, LYD_VALOPT_DATA_ONLY), LY_SUCCESS);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYDP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_container(void **state)
{
    struct state *st = (*state);
    const char *sch =
        "module A {"
            "namespace \"aa:A\";"
            "prefix A;"
            "container A {"
                "leaf f1 {type string;}"
                "container B {"
                    "leaf f2 {type string;}"
                "}"
                "container C {"
                    "leaf f3 {type string;}"
                "}"
            "}"
        "}";

    const char *trg = "<A xmlns=\"aa:A\"> <B> <f2>aaa</f2> </B> </A>";
    const char *src = "<A xmlns=\"aa:A\"> <C> <f3>bbb</f3> </C> </A>";
    const char *result = "<A xmlns=\"aa:A\"><B><f2>aaa</f2></B><C><f3>bbb</f3></C></A>";
    char *printed = NULL;

    assert_non_null(lys_parse_mem(st->ctx, sch, LYS_IN_YANG));

    st->source = lyd_parse_mem(st->ctx, src, LYD_XML, LYD_VALOPT_DATA_ONLY);
    assert_non_null(st->source);

    st->target = lyd_parse_mem(st->ctx, trg, LYD_XML, LYD_VALOPT_DATA_ONLY);
    assert_non_null(st->target);

    /* merge them */
    assert_int_equal(lyd_merge(&st->target, st->source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate(&st->target, NULL, LYD_VALOPT_DATA_ONLY), LY_SUCCESS);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYDP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_list(void **state)
{
    struct state *st = (*state);
    const char *sch =
    "module merge {"
        "namespace \"http://test/merge\";"
        "prefix merge;"

        "container inner1 {"
            "list b-list1 {"
                "key p1;"
                "leaf p1 {"
                    "type uint8;"
                "}"
                "leaf p2 {"
                    "type string;"
                "}"
                "leaf p3 {"
                    "type boolean;"
                    "default false;"
                "}"
            "}"
        "}"
    "}";


    const char *trg =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>a</p2>"
            "<p3>true</p3>"
        "</b-list1>"
    "</inner1>";
    const char *src =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>b</p2>"
        "</b-list1>"
    "</inner1>";
    const char *result =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>b</p2>"
            "<p3>true</p3>"
        "</b-list1>"
    "</inner1>";
    char *printed = NULL;

    assert_non_null(lys_parse_mem(st->ctx, sch, LYS_IN_YANG));

    st->source = lyd_parse_mem(st->ctx, src, LYD_XML, LYD_VALOPT_DATA_ONLY);
    assert_non_null(st->source);

    st->target = lyd_parse_mem(st->ctx, trg, LYD_XML, LYD_VALOPT_DATA_ONLY);
    assert_non_null(st->target);

    /* merge them */
    assert_int_equal(lyd_merge(&st->target, st->source, LYD_MERGE_EXPLICIT), LY_SUCCESS);
    assert_int_equal(lyd_validate(&st->target, NULL, LYD_VALOPT_DATA_ONLY), LY_SUCCESS);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYDP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_list2(void **state)
{
    struct state *st = (*state);
    const char *sch =
    "module merge {"
        "namespace \"http://test/merge\";"
        "prefix merge;"

        "container inner1 {"
            "list b-list1 {"
                "key p1;"
                "leaf p1 {"
                    "type uint8;"
                "}"
                "leaf p2 {"
                    "type string;"
                "}"
                "container inner2 {"
                    "leaf p3 {"
                        "type boolean;"
                        "default false;"
                    "}"
                    "leaf p4 {"
                        "type string;"
                    "}"
                "}"
            "}"
        "}"
    "}";


    const char *trg =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>a</p2>"
            "<inner2>"
                "<p4>val</p4>"
            "</inner2>"
        "</b-list1>"
    "</inner1>";
    const char *src =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>b</p2>"
        "</b-list1>"
    "</inner1>";
    const char *result =
    "<inner1 xmlns=\"http://test/merge\">"
        "<b-list1>"
            "<p1>1</p1>"
            "<p2>b</p2>"
            "<inner2>"
                "<p4>val</p4>"
            "</inner2>"
        "</b-list1>"
    "</inner1>";
    char *printed = NULL;

    assert_non_null(lys_parse_mem(st->ctx, sch, LYS_IN_YANG));

    st->source = lyd_parse_mem(st->ctx, src, LYD_XML, LYD_VALOPT_DATA_ONLY);
    assert_non_null(st->source);

    st->target = lyd_parse_mem(st->ctx, trg, LYD_XML, LYD_VALOPT_DATA_ONLY);
    assert_non_null(st->target);

    /* merge them */
    assert_int_equal(lyd_merge(&st->target, st->source, LYD_MERGE_EXPLICIT), LY_SUCCESS);
    assert_int_equal(lyd_validate(&st->target, NULL, LYD_VALOPT_DATA_ONLY), LY_SUCCESS);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYDP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_case(void **state)
{
    struct state *st = (*state);
    const char *sch =
    "module merge {"
        "namespace \"http://test/merge\";"
        "prefix merge;"
        "container cont {"
            "choice ch {"
                "container inner {"
                    "leaf p1 {"
                        "type string;"
                    "}"
                "}"
                "case c2 {"
                    "leaf p1 {"
                        "type string;"
                    "}"
                "}"
            "}"
        "}"
    "}";

    const char *trg =
    "<cont xmlns=\"http://test/merge\">"
        "<inner>"
            "<p1>1</p1>"
        "</inner>"
    "</cont>";
    const char *src =
    "<cont xmlns=\"http://test/merge\">"
        "<p1>1</p1>"
    "</cont>";
    const char *result =
    "<cont xmlns=\"http://test/merge\">"
        "<p1>1</p1>"
    "</cont>";
    char *printed = NULL;

    assert_non_null(lys_parse_mem(st->ctx, sch, LYS_IN_YANG));

    st->source = lyd_parse_mem(st->ctx, src, LYD_XML, LYD_VALOPT_DATA_ONLY);
    assert_non_null(st->source);

    st->target = lyd_parse_mem(st->ctx, trg, LYD_XML, LYD_VALOPT_DATA_ONLY);
    assert_non_null(st->target);

    /* merge them */
    assert_int_equal(lyd_merge(&st->target, st->source, 0), LY_SUCCESS);
    assert_int_equal(lyd_validate(&st->target, NULL, LYD_VALOPT_DATA_ONLY), LY_SUCCESS);

    /* check the result */
    lyd_print_mem(&printed, st->target, LYD_XML, LYDP_WITHSIBLINGS);
    assert_string_equal(printed, result);
    free(printed);
}

static void
test_dflt(void **state)
{
    struct state *st = (*state);
    struct lyd_node *tmp;
    const char *sch =
    "module merge-dflt {"
        "namespace \"urn:merge-dflt\";"
        "prefix md;"
        "container top {"
            "leaf a {"
                "type string;"
            "}"
            "leaf b {"
                "type string;"
            "}"
            "leaf c {"
                "type string;"
                "default \"c_dflt\";"
            "}"
        "}"
    "}";

    assert_non_null(lys_parse_mem(st->ctx, sch, LYS_IN_YANG));

    st->target = lyd_new_path(NULL, st->ctx, "/merge-dflt:top/c", "c_dflt", 0);
    assert_non_null(st->target);
    assert_int_equal(lyd_validate(&(st->target), NULL, LYD_VALOPT_DATA_ONLY), LY_SUCCESS);

    st->source = lyd_new_path(NULL, st->ctx, "/merge-dflt:top/a", "a_val", 0);
    assert_non_null(st->source);
    tmp = lyd_new_path(st->source, st->ctx, "/merge-dflt:top/b", "b_val", 0);
    assert_non_null(tmp);
    assert_int_equal(lyd_validate(&(st->source), NULL, LYD_VALOPT_DATA_ONLY), LY_SUCCESS);

    assert_int_equal(lyd_merge(&st->target, st->source, LYD_MERGE_DESTRUCT), LY_SUCCESS);
    st->source = NULL;

    /* c should be replaced and now be default */
    assert_true(lyd_node_children(st->target, 0)->flags & LYD_DEFAULT);
}

static void
test_dflt2(void **state)
{
    struct state *st = (*state);
    struct lyd_node *tmp;
    const char *sch =
    "module merge-dflt {"
        "namespace \"urn:merge-dflt\";"
        "prefix md;"
        "container top {"
            "leaf a {"
                "type string;"
            "}"
            "leaf b {"
                "type string;"
            "}"
            "leaf c {"
                "type string;"
                "default \"c_dflt\";"
            "}"
        "}"
    "}";

    assert_non_null(lys_parse_mem(st->ctx, sch, LYS_IN_YANG));

    st->target = lyd_new_path(NULL, st->ctx, "/merge-dflt:top/c", "c_dflt", 0);
    assert_non_null(st->target);
    assert_int_equal(lyd_validate(&(st->target), NULL, LYD_VALOPT_DATA_ONLY), LY_SUCCESS);

    st->source = lyd_new_path(NULL, st->ctx, "/merge-dflt:top/a", "a_val", 0);
    assert_non_null(st->source);
    tmp = lyd_new_path(st->source, st->ctx, "/merge-dflt:top/b", "b_val", 0);
    assert_non_null(tmp);
    assert_int_equal(lyd_validate(&(st->source), NULL, LYD_VALOPT_DATA_ONLY), LY_SUCCESS);

    assert_int_equal(lyd_merge(&st->target, st->source, LYD_MERGE_EXPLICIT), LY_SUCCESS);

    /* c should not be replaced, so c remains not default */
    assert_false(lyd_node_children(st->target, 0)->flags & LYD_DEFAULT);
}

static void
test_leafrefs(void **state)
{
    struct state *st = (*state);
    const char *sch = "module x {"
                      "  namespace urn:x;"
                      "  prefix x;"
                      "  list l {"
                      "    key n;"
                      "    leaf n { type string; }"
                      "    leaf t { type string; }"
                      "    leaf r { type leafref { path '/l/n'; } }}}";
    const char *trg = "<l xmlns=\"urn:x\"><n>a</n></l>"
                      "<l xmlns=\"urn:x\"><n>b</n><r>a</r></l>";
    const char *src = "<l xmlns=\"urn:x\"><n>c</n><r>a</r></l>"
                      "<l xmlns=\"urn:x\"><n>a</n><t>*</t></l>";
    const char *res = "<l xmlns=\"urn:x\"><n>a</n><t>*</t></l>"
                      "<l xmlns=\"urn:x\"><n>b</n><r>a</r></l>"
                      "<l xmlns=\"urn:x\"><n>c</n><r>a</r></l>";
    char *prt = NULL;

    assert_non_null(lys_parse_mem(st->ctx, sch, LYS_IN_YANG));

    st->target = lyd_parse_mem(st->ctx, trg, LYD_XML, LYD_VALOPT_DATA_ONLY);
    assert_non_null(st->target);

    st->source = lyd_parse_mem(st->ctx, src, LYD_XML, LYD_VALOPT_DATA_ONLY);
    assert_non_null(st->source);

    assert_int_equal(lyd_merge(&st->target, st->source, LYD_MERGE_DESTRUCT), LY_SUCCESS);
    st->source = NULL;

    lyd_print_mem(&prt, st->target, LYD_XML, LYDP_WITHSIBLINGS);
    assert_string_equal(prt, res);
    free(prt);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_batch, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_leaf, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_container, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_list, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_list2, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_case, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_dflt, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_dflt2, setup_dflt, teardown_dflt),
        cmocka_unit_test_setup_teardown(test_leafrefs, setup_dflt, teardown_dflt),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}