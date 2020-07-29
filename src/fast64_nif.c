/*
 * Copyright (c) 2020 Bancolombia, SA. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * 4. The names "Apache Server" and "Apache Group" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache"
 *    nor may "Apache" appear in their names without prior written
 *    permission of the Apache Group.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * THIS SOFTWARE IS PROVIDED BY THE APACHE GROUP ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE APACHE GROUP OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Group and was originally based
 * on public domain software written at the National Center for
 * Supercomputing Applications, University of Illinois, Urbana-Champaign.
 * For more information on the Apache Group and the Apache HTTP server
 * project, please see <http://www.apache.org/>.
 ====================================================================
 */
#include <stdio.h>
#include "erl_nif.h"
#include "base64.h"
#include <string.h>
#include <sys/time.h>
#include "naive.h"

ERL_NIF_TERM encode64(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);

static ERL_NIF_TERM fast_decode64(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    ErlNifBinary input;
    if (argc < 1 || enif_inspect_binary(env, argv[0], &input) == 0) {
        return enif_make_badarg(env);
    }
    if (input.size < 1) {
        return argv[0];
    }

    unsigned char* input_string = malloc(input.size+1);
    memcpy(input_string, input.data, input.size);
    input_string[input.size] = '\0';

    size_t result_len = Base64decode_len(input_string);

    unsigned char* result_ref = malloc(result_len);
    size_t real_size = Base64decode(result_ref, input_string);

    ERL_NIF_TERM result_term;
    unsigned char* result_ref_final = enif_make_new_binary(env, real_size, &result_term);
    memcpy(result_ref_final, result_ref, real_size);

    enif_release_binary(&input);
    free(input_string);
    free(result_ref);
    return result_term;
}

static ERL_NIF_TERM fast_encode64(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    ErlNifBinary input;
    if (argc < 1 || enif_inspect_binary(env, argv[0], &input) == 0) {
        return enif_make_badarg(env);
    }
    if (input.size < 1) {
        return argv[0];
    }

    if(input.size > 300000){
        return encode64(env, argc, argv);
    }

    size_t result_len = Base64encode_len(input.size);

    ERL_NIF_TERM result_term;
    unsigned char* result_ref_final = enif_make_new_binary(env, result_len, &result_term);

    Base64encode((char*)result_ref_final, (const char *) input.data, input.size);
    enif_release_binary(&input);
    return result_term;
}

static ERL_NIF_TERM
encode64_chunk(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifResourceType* res_type = (ErlNifResourceType*)enif_priv_data(env);
    unsigned long offset, i, end, max_per_slice, res_size;
    struct timeval start, stop, slice;
    int pct, total = 0;
    ERL_NIF_TERM newargv[5];
    ErlNifBinary bin;
    void* res;

    if (argc != 5 || !enif_inspect_binary(env, argv[0], &bin) ||
        !enif_get_ulong(env, argv[1], &max_per_slice) ||
        !enif_get_ulong(env, argv[2], &offset) ||
        !enif_get_resource(env, argv[3], res_type, &res) ||
        !enif_get_ulong(env, argv[4], &res_size))
        return enif_make_badarg(env);
    end = offset + max_per_slice;
    if (end > bin.size) end = bin.size;
    i = offset;
    while (i < bin.size) {
        gettimeofday(&start, NULL);
        base64(bin.data+i, end-i, res+i*4/3, res_size-i*4/3);
        i = end;
        if (i == bin.size) break;
        gettimeofday(&stop, NULL);
        /* determine how much of the timeslice was used */
        timersub(&stop, &start, &slice);
        pct = (int)((slice.tv_sec*1000000+slice.tv_usec)/10);
        total += pct;
        if (pct > 100) pct = 100;
        else if (pct == 0) pct = 1;
        if (enif_consume_timeslice(env, pct)) {
            /* the timeslice has been used up, so adjust our max_per_slice byte count based on
             * the processing we've done, then reschedule to run again */
            max_per_slice = i - offset;
            if (total > 100) {
                int m = (int)(total/100);
                if (m == 1)
                    max_per_slice -= (unsigned long)(max_per_slice*(total-100)/100);
                else
                    max_per_slice = (unsigned long)(max_per_slice/m);
            }
            max_per_slice = max_per_slice / 3;
            max_per_slice = max_per_slice * 3;
            newargv[0] = argv[0];
            newargv[1] = enif_make_ulong(env, max_per_slice);
            newargv[2] = enif_make_ulong(env, i);
            newargv[3] = argv[3];
            newargv[4] = argv[4];
            return enif_schedule_nif(env, "encode64_chunk", 0, encode64_chunk, argc, newargv);
        }
        end += max_per_slice;
        if (end > bin.size) end = bin.size;
    }
    return enif_make_resource_binary(env, res, res, res_size);
}

/*
 * encode64 just schedules encode64_chunk for execution, providing an initial
 * guess of 30KB for the max number of bytes to process before yielding the
 * scheduler thread.
 */
ERL_NIF_TERM encode64(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifResourceType* res_type = (ErlNifResourceType*)enif_priv_data(env);
    ERL_NIF_TERM newargv[5];
    ErlNifBinary bin;
    unsigned res_size;
    void* res;

    if (argc != 1 || !enif_inspect_binary(env, argv[0], &bin))
        return enif_make_badarg(env);
    if (bin.size == 0)
        return argv[0];
    res_size = base64_size(bin.size);
    newargv[0] = argv[0];
    newargv[1] = enif_make_ulong(env, 30720); // MOD3
    newargv[2] = enif_make_ulong(env, 0);
    res = enif_alloc_resource(res_type, res_size);
    newargv[3] = enif_make_resource(env, res);
    newargv[4] = enif_make_ulong(env, res_size);
    enif_release_resource(res);
    return enif_schedule_nif(env, "encode64_chunk", 0, encode64_chunk, 5, newargv);
}

static ERL_NIF_TERM
decode64_chunk(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifResourceType* res_type = (ErlNifResourceType*)enif_priv_data(env);
    unsigned long offset, i, end, max_per_slice, res_size;
    struct timeval start, stop, slice;
    int pct, total = 0;
    ERL_NIF_TERM newargv[5];
    ErlNifBinary bin;
    void* res;

    if (argc != 5 || !enif_inspect_binary(env, argv[0], &bin) ||
        !enif_get_ulong(env, argv[1], &max_per_slice) ||
        !enif_get_ulong(env, argv[2], &offset) ||
        !enif_get_resource(env, argv[3], res_type, &res) ||
        !enif_get_ulong(env, argv[4], &res_size))
        return enif_make_badarg(env);
    end = offset + max_per_slice;
    if (end > bin.size) end = bin.size;
    i = offset;
    while (i < bin.size) {
        gettimeofday(&start, NULL);
        unbase64(bin.data+i, end-i, res+i*3/4, res_size-i*3/4);
        i = end;
        if (i == bin.size) break;
        gettimeofday(&stop, NULL);
        /* determine how much of the timeslice was used */
        timersub(&stop, &start, &slice);
        pct = (int)((slice.tv_sec*1000000+slice.tv_usec)/10);
        total += pct;
        if (pct > 100) pct = 100;
        else if (pct == 0) pct = 1;
        if (enif_consume_timeslice(env, pct)) {
            /* the timeslice has been used up, so adjust our max_per_slice byte count based on
             * the processing we've done, then reschedule to run again */
            max_per_slice = i - offset;
            if (total > 100) {
                int m = (int)(total/100);
                if (m == 1)
                    max_per_slice -= (unsigned long)(max_per_slice*(total-100)/100);
                else
                    max_per_slice = (unsigned long)(max_per_slice/m);
            }
            max_per_slice = max_per_slice / 4;
            max_per_slice = max_per_slice * 4;
            newargv[0] = argv[0];
            newargv[1] = enif_make_ulong(env, max_per_slice);
            newargv[2] = enif_make_ulong(env, i);
            newargv[3] = argv[3];
            newargv[4] = argv[4];
            return enif_schedule_nif(env, "decode64_chunk", 0, decode64_chunk, argc, newargv);
        }
        end += max_per_slice;
        if (end > bin.size) end = bin.size;
    }
    return enif_make_resource_binary(env, res, res, res_size);
}

/*
 * decode64 just schedules decode64_chunk for execution, providing an initial
 * guess of 30KB for the max number of bytes to process before yielding the
 * scheduler thread.
 */
static ERL_NIF_TERM
decode64(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifResourceType* res_type = (ErlNifResourceType*)enif_priv_data(env);
    ERL_NIF_TERM newargv[5];
    ErlNifBinary bin;
    unsigned res_size;
    void* res;

    if (argc != 1 || !enif_inspect_binary(env, argv[0], &bin))
        return enif_make_badarg(env);
    if (bin.size == 0)
        return argv[0];
    res_size = unbase64_size(bin.data, bin.size);
    newargv[0] = argv[0];
    newargv[1] = enif_make_ulong(env, 30720); // MOD4
    newargv[2] = enif_make_ulong(env, 0);
    res = enif_alloc_resource(res_type, res_size);
    newargv[3] = enif_make_resource(env, res);
    newargv[4] = enif_make_ulong(env, res_size);
    enif_release_resource(res);
    return enif_schedule_nif(env, "decode64_chunk", 0, decode64_chunk, 5, newargv);
}

static int
nifload(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    *priv_data = enif_open_resource_type(env,
                                         NULL,
                                         "b64fast",
                                         NULL,
                                         ERL_NIF_RT_CREATE|ERL_NIF_RT_TAKEOVER,
                                         NULL);
    return *priv_data == NULL;
}

static int
nifupgrade(ErlNifEnv* env, void** priv_data, void** old_priv_data, ERL_NIF_TERM load_info)
{
    *priv_data = enif_open_resource_type(env,
                                         NULL,
                                         "b64fast",
                                         NULL,
                                         ERL_NIF_RT_TAKEOVER,
                                         NULL);
    return *priv_data == NULL;
}

static ErlNifFunc nif_funcs[] = {
        {"decode64", 1, decode64},
        {"encode64", 1, fast_encode64},
};


ERL_NIF_INIT(Elixir.Fast64, nif_funcs, nifload, NULL, nifupgrade, NULL)