/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2019 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_CRMFERR_H
# define HEADER_CRMFERR_H

# include <openssl/opensslconf.h>

# ifndef OPENSSL_NO_CRMF

#  ifdef  __cplusplus
extern "C"
#  endif
int ERR_load_CRMF_strings(void);

/*
 * CRMF function codes.
 */
#  define CRMF_F_CRMF_POPOSIGNINGKEY_INIT                  100
#  define CRMF_F_OSSL_CRMF_CERTID_GEN                      101
#  define CRMF_F_OSSL_CRMF_CERTTEMPLATE_FILL               102
#  define CRMF_F_OSSL_CRMF_ENCRYPTEDVALUE_GET1_ENCCERT     103
#  define CRMF_F_OSSL_CRMF_MSGS_VERIFY_POPO                104
#  define CRMF_F_OSSL_CRMF_MSG_CREATE_POPO                 105
#  define CRMF_F_OSSL_CRMF_MSG_GET0_TMPL                   106
#  define CRMF_F_OSSL_CRMF_MSG_GET_CERTREQID               107
#  define CRMF_F_OSSL_CRMF_MSG_PKIPUBLICATIONINFO_PUSH0_SINGLEPUBINFO 108
#  define CRMF_F_OSSL_CRMF_MSG_PUSH0_EXTENSION             109
#  define CRMF_F_OSSL_CRMF_MSG_PUSH0_REGCTRL               110
#  define CRMF_F_OSSL_CRMF_MSG_PUSH0_REGINFO               111
#  define CRMF_F_OSSL_CRMF_MSG_SET0_EXTENSIONS             112
#  define CRMF_F_OSSL_CRMF_MSG_SET0_SINGLEPUBINFO          113
#  define CRMF_F_OSSL_CRMF_MSG_SET_CERTREQID               114
#  define CRMF_F_OSSL_CRMF_MSG_SET_PKIPUBLICATIONINFO_ACTION 115
#  define CRMF_F_OSSL_CRMF_MSG_SET_VALIDITY                116
#  define CRMF_F_OSSL_CRMF_PBMP_NEW                        117
#  define CRMF_F_OSSL_CRMF_PBM_NEW                         118

/*
 * CRMF reason codes.
 */
#  define CRMF_R_BAD_PBM_ITERATIONCOUNT                    100
#  define CRMF_R_CRMFERROR                                 102
#  define CRMF_R_ERROR                                     103
#  define CRMF_R_ERROR_DECODING_CERTIFICATE                104
#  define CRMF_R_ERROR_DECRYPTING_CERTIFICATE              105
#  define CRMF_R_ERROR_DECRYPTING_SYMMETRIC_KEY            106
#  define CRMF_R_FAILURE_OBTAINING_RANDOM                  107
#  define CRMF_R_ITERATIONCOUNT_BELOW_100                  108
#  define CRMF_R_MALFORMED_IV                              101
#  define CRMF_R_NULL_ARGUMENT                             109
#  define CRMF_R_SETTING_MAC_ALGOR_FAILURE                 110
#  define CRMF_R_SETTING_OWF_ALGOR_FAILURE                 111
#  define CRMF_R_UNSUPPORTED_ALGORITHM                     112
#  define CRMF_R_UNSUPPORTED_ALG_FOR_POPSIGNINGKEY         113
#  define CRMF_R_UNSUPPORTED_CIPHER                        114
#  define CRMF_R_UNSUPPORTED_METHOD_FOR_CREATING_POPO      115
#  define CRMF_R_UNSUPPORTED_POPO_METHOD                   116
#  define CRMF_R_UNSUPPORTED_POPO_NOT_ACCEPTED             117

# endif
#endif
