/**
 * @file test_rpc.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief test executing simple RPCs
 *
 * @copyright
 * Copyright 2021 Deutsche Telekom AG.
 * Copyright 2021 CESNET, z.s.p.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE

#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdarg.h>

#include <cmocka.h>
#include <libyang/libyang.h>
#include <nc_client.h>

#include "np_test.h"

NP_GLOB_SETUP_FUNC

static void
test_lock(void **state)
{
    struct np_test *st = *state;
    struct nc_rpc *rpc;
    NC_MSG_TYPE msgtype;
    uint64_t msgid;
    struct lyd_node *envp, *op;

    /* lock RPC */
    rpc = nc_rpc_lock(NC_DATASTORE_RUNNING);
    assert_non_null(rpc);

    /* send request */
    msgtype = nc_send_rpc(st->nc_sess, rpc, 1000, &msgid);
    assert_int_equal(msgtype, NC_MSG_RPC);

    /* receive reply */
    msgtype = nc_recv_reply(st->nc_sess, rpc, msgid, 2000, &envp, &op);
    assert_int_equal(msgtype, NC_MSG_REPLY);
    assert_null(op);
    assert_string_equal(LYD_NAME(lyd_child(envp)), "ok");

    nc_rpc_free(rpc);
    lyd_free_tree(envp);

    /* unlock RPC */
    rpc = nc_rpc_unlock(NC_DATASTORE_RUNNING);
    assert_non_null(rpc);

    /* send request */
    msgtype = nc_send_rpc(st->nc_sess, rpc, 1000, &msgid);
    assert_int_equal(msgtype, NC_MSG_RPC);

    /* receive reply */
    msgtype = nc_recv_reply(st->nc_sess, rpc, msgid, 2000, &envp, &op);
    assert_int_equal(msgtype, NC_MSG_REPLY);
    assert_null(op);
    assert_string_equal(LYD_NAME(lyd_child(envp)), "ok");

    nc_rpc_free(rpc);
    lyd_free_tree(envp);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_lock),
    };

    nc_verbosity(NC_VERB_ERROR);
    return cmocka_run_group_tests(tests, np_glob_setup, np_glob_teardown);
}