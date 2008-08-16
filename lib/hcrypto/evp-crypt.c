/*
 * Copyright (c) 2008 Kungliga Tekniska H�gskolan
 * (Royal Institute of Technology, Stockholm, Sweden). 
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 *
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 */

/* Windows crypto provider plugin, sample */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

RCSID("$Id$");

#define HC_DEPRECATED

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <evp.h>

#include <crypt.h>

/*
 *
 */

struct generic_key {
    HCRYPTKEY *hKey;
};

static int
crypto_des_ede3_cbc_init(EVP_CIPHER_CTX *ctx,
			 const unsigned char * key,
			 const unsigned char * iv,
			 int encp)
{
    struct generic_key *gk = ctx->cipher_data;
    BYTE *k = key->key->keyvalue.data;
    gk->hKey = importKey3DES_CBC(&k[0], &k[8], &k[16]);
    return 1;
}

static int
generic_cbc_do_cipher(EVP_CIPHER_CTX *ctx,
		       unsigned char *out,
		       const unsigned char *in,
		       unsigned int size)
{
    struct generic_key *gk = ctx->cipher_data;
    BOOL bResult;
    DWORD paramData;

    bResult = CryptSetKeyParam(gk->hKey, KP_IV, ctx->iv, 0);
    _ASSERT(bResult);

    paramData = CRYPT_MODE_CBC;
    bResult = CryptSetKeyParam(gk->hKey, KP_MODE, (BYTE*)&paramData, 0);
    _ASSERT(bResult);

    if (ctx->encrypt)
	bResult = encryptData(in, out, size, gk->hKey);
    else
	bResult = decryptData(in, out, size, gk->hKey);
    _ASSERT(bResult);

    return 1;
}

static int
generic_cleanup(EVP_CIPHER_CTX *ctx)
{
    struct generic_key *gk = ctx->cipher_data;
    CryptDestroyKey(gk->hKey);
    gk->hKey = NULL;
    return 1;
}

/**
 * The tripple DES cipher type
 *
 * @return the DES-EDE3-CBC EVP_CIPHER pointer.
 *
 * @ingroup hcrypto_evp
 */

const EVP_CIPHER *
EVP_wincrypt_des_ede3_cbc(void)
{
    static const EVP_CIPHER des_ede3_cbc = {
	0,
	8,
	24,
	8,
	EVP_CIPH_CBC_MODE,
	crypto_des_ede3_cbc_init,
	generic_cbc_do_cipher,
	generic_cleanup,
	sizeof(struct generic_key),
	NULL,
	NULL,
	NULL,
	NULL
    };
    return &des_ede3_cbc;
}