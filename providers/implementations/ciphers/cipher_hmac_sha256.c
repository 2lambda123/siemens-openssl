/*
 * Copyright 2019-2023 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/* Dispatch functions for hmac sha256 cipher */

/*
 * HMAC low level APIs are deprecated for public use, but still ok for internal
 * use.
 */
#include "internal/deprecated.h"

#include <openssl/proverr.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include "cipher_hmac_sha256.h"
#include "prov/implementations.h"
#include "prov/providercommon.h"

static OSSL_FUNC_cipher_newctx_fn hmac_sha256_newctx;
static OSSL_FUNC_cipher_freectx_fn hmac_sha256_freectx;
static OSSL_FUNC_cipher_dupctx_fn hmac_sha256_dupctx;
static OSSL_FUNC_cipher_encrypt_init_fn hmac_sha256_einit;
static OSSL_FUNC_cipher_decrypt_init_fn hmac_sha256_dinit;
static OSSL_FUNC_cipher_get_params_fn hmac_sha256_get_params;
static OSSL_FUNC_cipher_get_ctx_params_fn hmac_sha256_get_ctx_params;
static OSSL_FUNC_cipher_set_ctx_params_fn hmac_sha256_set_ctx_params;
static OSSL_FUNC_cipher_cipher_fn hmac_sha256_cipher;
static OSSL_FUNC_cipher_final_fn hmac_sha256_final;
static OSSL_FUNC_cipher_gettable_ctx_params_fn hmac_sha256_gettable_ctx_params;
static OSSL_FUNC_cipher_settable_ctx_params_fn hmac_sha256_settable_ctx_params;
#define hmac_sha256_update hmac_sha256_cipher
#define hmac_sha256_gettable_params ossl_cipher_generic_gettable_params

static void *hmac_sha256_newctx(void *provctx)
{
    PROV_HMAC_SHA256_CTX *ctx;

    if (!ossl_prov_is_running())
        return NULL;

    if ((ctx = OPENSSL_zalloc(sizeof(*ctx))) == NULL)
        return NULL;

    if ((ctx->hmac = HMAC_CTX_new()) == NULL) {
        OPENSSL_free(ctx);
        return NULL;
    }
    if ((ctx->evp_md = (EVP_MD *)EVP_get_digestbyname("SHA256")) == NULL) {
        hmac_sha256_freectx(ctx);
        return NULL;
    }
    ossl_cipher_generic_initkey(ctx, HMAC_SHA256_KEYLEN * 8,
                                HMAC_SHA256_BLKLEN * 8,
                                HMAC_SHA256_IVLEN * 8,
                                HMAC_SHA256_MODE, HMAC_SHA256_FLAGS,
                                ossl_prov_cipher_hw_hmac_sha256(
                                    HMAC_SHA256_KEYLEN * 8),
                                NULL);
    return ctx;
}

static void hmac_sha256_freectx(void *vctx)
{
    PROV_HMAC_SHA256_CTX *ctx = (PROV_HMAC_SHA256_CTX *)vctx;

    if (ctx != NULL) {
        ossl_cipher_generic_reset_ctx((PROV_CIPHER_CTX *)vctx);
        HMAC_CTX_free(ctx->hmac);
        ctx->evp_md = NULL;
        OPENSSL_clear_free(ctx, sizeof(*ctx));
    }
}

static void *hmac_sha256_dupctx(void *vctx)
{
    PROV_HMAC_SHA256_CTX *ctx = (PROV_HMAC_SHA256_CTX *)vctx;
    PROV_HMAC_SHA256_CTX *dupctx = NULL;

    if (ctx != NULL)
        return NULL;

    dupctx = OPENSSL_memdup(ctx, sizeof(*dupctx));
    if (dupctx == NULL)
        return NULL;

    if (dupctx->base.tlsmac != NULL && dupctx->base.alloced) {
        dupctx->base.tlsmac = OPENSSL_memdup(dupctx->base.tlsmac,
                                             dupctx->base.tlsmacsize);
        if (dupctx->base.tlsmac == NULL)
            goto err;
    }
    if ((dupctx->hmac = HMAC_CTX_new()) == NULL)
        goto err;

    if (!HMAC_CTX_copy(dupctx->hmac, ctx->hmac))
        goto err;

    return dupctx;
 err:
    OPENSSL_free(dupctx->base.tlsmac);
    HMAC_CTX_free(dupctx->hmac);
    OPENSSL_clear_free(dupctx, sizeof(*dupctx));
    return NULL;
}

static int hmac_sha256_get_params(OSSL_PARAM params[])
{
    return ossl_cipher_generic_get_params(params, 0, HMAC_SHA256_FLAGS,
                                          HMAC_SHA256_KEYLEN * 8,
                                          1 * 8,
                                          HMAC_SHA256_IVLEN * 8);
}

static int hmac_sha256_get_ctx_params(void *vctx, OSSL_PARAM params[])
{
    OSSL_PARAM *p;
    PROV_HMAC_SHA256_CTX *ctx = (PROV_HMAC_SHA256_CTX *)vctx;

    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_IVLEN);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, HMAC_SHA256_IVLEN)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_KEYLEN);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, HMAC_SHA256_KEYLEN)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_AEAD_TAGLEN);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, ctx->tag_len)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_AEAD_TAG);
    if (p != NULL) {
        if (p->data_type != OSSL_PARAM_OCTET_STRING) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
            return 0;
        }
        if (!ctx->base.enc) {
            ERR_raise(ERR_LIB_PROV, PROV_R_TAG_NOT_SET);
            return 0;
        }
        if (p->data_size == 0 || p->data_size > HMAC_SHA256_TAGLEN) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_TAG_LENGTH);
            return 0;
        }
        memcpy(p->data, ctx->tag, p->data_size);
    }

    return 1;
}

static const OSSL_PARAM hmac_sha256_known_gettable_ctx_params[] = {
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_KEYLEN, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_IVLEN, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_AEAD_TAGLEN, NULL),
    OSSL_PARAM_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, NULL, 0),
    OSSL_PARAM_END
};
const OSSL_PARAM *hmac_sha256_gettable_ctx_params(ossl_unused void *cctx,
                                                  ossl_unused void *provctx)
{
    return hmac_sha256_known_gettable_ctx_params;
}

static int hmac_sha256_set_ctx_params(void *vctx, const OSSL_PARAM params[])
{
    const OSSL_PARAM *p;
    size_t len;
    PROV_HMAC_SHA256_CTX *ctx = (PROV_HMAC_SHA256_CTX *)vctx;

    if (params == NULL)
        return 1;

    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_KEYLEN);
    if (p != NULL) {
        if (!OSSL_PARAM_get_size_t(p, &len)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
        if (len != HMAC_SHA256_KEYLEN) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_KEY_LENGTH);
            return 0;
        }
    }
    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_IVLEN);
    if (p != NULL) {
        if (!OSSL_PARAM_get_size_t(p, &len)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
        if (len != HMAC_SHA256_IVLEN) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_IV_LENGTH);
            return 0;
        }
    }
    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_AEAD_TAG);
    if (p != NULL) {
        if (p->data_type != OSSL_PARAM_OCTET_STRING) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
        if (p->data_size == 0 || p->data_size != HMAC_SHA256_TAGLEN) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_TAG_LENGTH);
            return 0;
        }
        if (p->data != NULL) {
            if (ctx->base.enc) {
                ERR_raise(ERR_LIB_PROV, PROV_R_TAG_NOT_NEEDED);
                return 0;
            }
            memcpy(ctx->tag, p->data, p->data_size);
        }
        ctx->tag_len = p->data_size;
    }
    return 1;
}

static const OSSL_PARAM hmac_sha256_known_settable_ctx_params[] = {
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_KEYLEN, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_IVLEN, NULL),
    OSSL_PARAM_END
};
const OSSL_PARAM *hmac_sha256_settable_ctx_params(ossl_unused void *cctx,
                                                  ossl_unused void *provctx)
{
    return hmac_sha256_known_settable_ctx_params;
}

static int hmac_sha256_einit(void *vctx,
                             const unsigned char *key, size_t keylen,
                             const unsigned char *iv, size_t ivlen,
                             const OSSL_PARAM params[])
{
    int ret;

    /* The generic function checks for ossl_prov_is_running() */
    ret = ossl_cipher_generic_einit(vctx, key, keylen, iv, ivlen, NULL);
    if (ret && iv != NULL) {
        PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;
        PROV_CIPHER_HW_HMAC_SHA256 *hw = (PROV_CIPHER_HW_HMAC_SHA256 *)ctx->hw;

        hw->initiv(ctx, iv, ivlen);
    }
    if (ret && !hmac_sha256_set_ctx_params(vctx, params))
        ret = 0;
    return ret;
}

static int hmac_sha256_dinit(void *vctx,
                             const unsigned char *key, size_t keylen,
                             const unsigned char *iv, size_t ivlen,
                             const OSSL_PARAM params[])
{
    int ret;

    /* The generic function checks for ossl_prov_is_running() */
    ret = ossl_cipher_generic_dinit(vctx, key, keylen, iv, ivlen, NULL);
    if (ret && iv != NULL) {
        PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;
        PROV_CIPHER_HW_HMAC_SHA256 *hw = (PROV_CIPHER_HW_HMAC_SHA256 *)ctx->hw;

        hw->initiv(ctx, iv, ivlen);
    }
    if (ret && !hmac_sha256_set_ctx_params(vctx, params))
        ret = 0;
    return ret;
}

static int hmac_sha256_cipher(void *vctx, unsigned char *out,
                              size_t *outl, size_t outsize,
                              const unsigned char *in, size_t inl)
{
    PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;
    PROV_CIPHER_HW_HMAC_SHA256 *hw = (PROV_CIPHER_HW_HMAC_SHA256 *)ctx->hw;

    if (!ossl_prov_is_running())
        return 0;

    if (inl == 0) {
        *outl = 0;
        return 1;
    }

    if (outsize < inl) {
        ERR_raise(ERR_LIB_PROV, PROV_R_OUTPUT_BUFFER_TOO_SMALL);
        return 0;
    }

    if (!hw->base.cipher(ctx, out, in, inl))
        return 0;
    *outl = inl;
    return 1;
}

static int hmac_sha256_final(void *vctx, unsigned char *out, size_t *outl,
                             size_t outsize)
{
    PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;
    PROV_CIPHER_HW_HMAC_SHA256 *hw = (PROV_CIPHER_HW_HMAC_SHA256 *)ctx->hw;

    if (!ossl_prov_is_running())
        return 0;

    if (!hw->base.cipher(ctx, out, NULL, 0))
        return 0;

    *outl = 0;
    return 1;
}

/* ossl_enull_hmac_sha256_functions */
const OSSL_DISPATCH ossl_enull_hmac_sha256_functions[] = {
    { OSSL_FUNC_CIPHER_NEWCTX, (void (*)(void))hmac_sha256_newctx },
    { OSSL_FUNC_CIPHER_FREECTX, (void (*)(void))hmac_sha256_freectx },
    { OSSL_FUNC_CIPHER_DUPCTX, (void (*)(void))hmac_sha256_dupctx },
    { OSSL_FUNC_CIPHER_ENCRYPT_INIT, (void (*)(void))hmac_sha256_einit },
    { OSSL_FUNC_CIPHER_DECRYPT_INIT, (void (*)(void))hmac_sha256_dinit },
    { OSSL_FUNC_CIPHER_UPDATE, (void (*)(void))hmac_sha256_update },
    { OSSL_FUNC_CIPHER_FINAL, (void (*)(void))hmac_sha256_final },
    { OSSL_FUNC_CIPHER_CIPHER, (void (*)(void))hmac_sha256_cipher},
    { OSSL_FUNC_CIPHER_GET_PARAMS, (void (*)(void))hmac_sha256_get_params },
    { OSSL_FUNC_CIPHER_GETTABLE_PARAMS,
      (void (*)(void))hmac_sha256_gettable_params },
    { OSSL_FUNC_CIPHER_GET_CTX_PARAMS,
      (void (*)(void))hmac_sha256_get_ctx_params },
    { OSSL_FUNC_CIPHER_GETTABLE_CTX_PARAMS,
      (void (*)(void))hmac_sha256_gettable_ctx_params },
    { OSSL_FUNC_CIPHER_SET_CTX_PARAMS,
      (void (*)(void))hmac_sha256_set_ctx_params },
    { OSSL_FUNC_CIPHER_SETTABLE_CTX_PARAMS,
      (void (*)(void))hmac_sha256_settable_ctx_params },
    OSSL_DISPATCH_END
};
