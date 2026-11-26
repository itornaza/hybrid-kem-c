/*
 * test_mlkem_roundtrip.c
 *
 * ML-KEM-768 (FIPS 203) round trip through liboqs: checks the key,
 * ciphertext and shared-secret sizes, runs keygen, encaps and decaps,
 * and checks that both shared secrets match.
 * Returns 0 on pass, 1 on any failure (read by ctest).
 *
 * Verifies: FR-01 (partial: FR-02, QUAL-03)
 *
 * Copyright (c) 2026 ion-
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <oqs/oqs.h>

int main(void)
{
    int rc = 1; /* Assume failure until every step passes */
    OQS_KEM *kem    = NULL;
    uint8_t *pk     = NULL;
    uint8_t *sk     = NULL;
    uint8_t *ct     = NULL;
    uint8_t *ss_enc = NULL;
    uint8_t *ss_dec = NULL;

    /* Initialize OQS KEM */
    kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_768);
    if (kem == NULL) {
        fprintf(stderr, "OQS_KEM_new failed\n");
        goto cleanup;
    }

    /* Length checks */
    if ((kem->length_public_key    != 1184U) ||
        (kem->length_secret_key    != 2400U) ||
        (kem->length_ciphertext    != 1088U) ||
        (kem->length_shared_secret !=   32U)) {
        fprintf(stderr, "unexpected ML-KEM-768 sizes\n");
        goto cleanup;
    }

    /* Memory allocations */
    pk = malloc(kem->length_public_key);
    if (pk == NULL) {
        fprintf(stderr, "length_public_key malloc failed\n");
        goto cleanup;
    }
    sk = malloc(kem->length_secret_key);
    if (sk == NULL) {
        fprintf(stderr, "length_secret_key malloc failed\n");
        goto cleanup;
    }
    ct = malloc(kem->length_ciphertext);
    if (ct == NULL) {
        fprintf(stderr, "length_ciphertext malloc failed\n");
        goto cleanup;
    }
    ss_enc = malloc(kem->length_shared_secret);
    if (ss_enc == NULL) {
        fprintf(stderr, "length_shared_secret malloc failed\n");
        goto cleanup;
    }
    ss_dec = malloc(kem->length_shared_secret);
    if (ss_dec == NULL) {
        fprintf(stderr, "length_shared_secret malloc failed\n");
        goto cleanup;
    }

    /* KEM round trip */
    if (OQS_KEM_keypair(kem, pk, sk) != OQS_SUCCESS) {
        fprintf(stderr, "keypair failed\n");
        goto cleanup;
    }
    if (OQS_KEM_encaps(kem, ct, ss_enc, pk) != OQS_SUCCESS) {
        fprintf(stderr, "encaps failed\n");
        goto cleanup;
    }
    if (OQS_KEM_decaps(kem, ss_dec, ct, sk) != OQS_SUCCESS) {
        fprintf(stderr, "decaps failed\n");
        goto cleanup;
    }
    if (OQS_MEM_secure_bcmp(ss_enc, ss_dec, kem->length_shared_secret) != 0) {
        fprintf(stderr, "shared secrets differ\n");
        goto cleanup;
    }

    rc = 0; /* Reached only if everything passed */

cleanup:
    /* Free secrets (wiped first) */
    if (kem != NULL) {
        OQS_MEM_secure_free(sk, kem->length_secret_key);
        OQS_MEM_secure_free(ss_enc, kem->length_shared_secret);
        OQS_MEM_secure_free(ss_dec, kem->length_shared_secret);
    }
    /* Free everything else */
    free(pk);
    free(ct);
    OQS_KEM_free(kem);

    return rc;
}
