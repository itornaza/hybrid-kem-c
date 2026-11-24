# hybrid-kem-c: Interface Control Document (ICD)

Version 0.1 (draft), November 2026. Author: ion-.

## 1. Purpose and scope

This Interface Control Document (ICD) defines the interfaces of `hybrid-kem-c`: the public C application programming interface (API) for milestone M1, the byte sizes and layouts of keys and ciphertexts, the dependency libraries, and (later) the serial protocol for the optional board milestone M6. Requirements that refer to this document are in [SRS.md](SRS.md) (System Requirements Specification).

The KEM (key encapsulation mechanism) is X-Wing, as defined in the IETF Internet-Draft [S1](SOURCES.md#s1), which combines X25519 (Request for Comments (RFC) 7748, [S3](SOURCES.md#s3)) and ML-KEM-768 (Module-Lattice-Based Key-Encapsulation Mechanism, National Institute of Standards and Technology (NIST) Federal Information Processing Standard (FIPS) 203, [S2](SOURCES.md#s2)). Revision -11 is pinned and re-checked at milestone review MR1; drafts can change.

## 2. Public C API (M1)

The project defines three operations, plus two derandomized variants for test vectors:

| Function | Inputs | Outputs | Errors |
|---|---|---|---|
| `hkem_keygen` | none | 1216 B public key, 32 B seed secret key | negative on random number generator (RNG) or library failure |
| `hkem_keygen_derand` | 32 B seed | 1216 B public key, 32 B secret key (a copy of the seed) | negative on library failure |
| `hkem_encaps` | public key | 1120 B ciphertext, 32 B shared secret | negative on RNG or library failure, if pk_M fails the FIPS 203 section 7.2 check (`HKEM_ERR_MLKEM`), or if pk_X is a small-order point (`HKEM_ERR_X25519`) |
| `hkem_encaps_derand` | public key, 64 B eseed | as `hkem_encaps` | as `hkem_encaps`, without RNG failure |
| `hkem_decaps` | seed secret key, ciphertext (keys re-derived from the seed) | 32 B shared secret | negative only on library failure, or `HKEM_ERR_X25519` if ct_X is a small-order point; never on a bad ct_M (implicit rejection) |

Public header `hkem.h`. Outputs come first, then inputs, as in liboqs.

```c
#include <stdint.h>

#define HKEM_PUBLICKEYBYTES     1216U
#define HKEM_SECRETKEYBYTES       32U
#define HKEM_CIPHERTEXTBYTES    1120U
#define HKEM_SHAREDSECRETBYTES    32U
#define HKEM_KEYGEN_SEEDBYTES     32U
#define HKEM_ENCAPS_SEEDBYTES     64U

#define HKEM_OK           0
#define HKEM_ERR_RNG     (-1)
#define HKEM_ERR_MLKEM   (-2)
#define HKEM_ERR_X25519  (-3)
#define HKEM_ERR_HASH    (-4)
#define HKEM_ERR_ARG     (-5)

/* All functions return HKEM_OK (0) on success or a negative HKEM_ERR_* code. */
int hkem_keygen(uint8_t pk[HKEM_PUBLICKEYBYTES],
                uint8_t sk[HKEM_SECRETKEYBYTES]);

/* sk is returned as a copy of seed (draft section 5.2.1).
   sk and seed may be the same buffer or overlap (copied with memmove; IF-08). */
int hkem_keygen_derand(uint8_t pk[HKEM_PUBLICKEYBYTES],
                       uint8_t sk[HKEM_SECRETKEYBYTES],
                       const uint8_t seed[HKEM_KEYGEN_SEEDBYTES]);

int hkem_encaps(uint8_t ct[HKEM_CIPHERTEXTBYTES],
                uint8_t ss[HKEM_SHAREDSECRETBYTES],
                const uint8_t pk[HKEM_PUBLICKEYBYTES]);

int hkem_encaps_derand(uint8_t ct[HKEM_CIPHERTEXTBYTES],
                       uint8_t ss[HKEM_SHAREDSECRETBYTES],
                       const uint8_t pk[HKEM_PUBLICKEYBYTES],
                       const uint8_t eseed[HKEM_ENCAPS_SEEDBYTES]);

int hkem_decaps(uint8_t ss[HKEM_SHAREDSECRETBYTES],
                const uint8_t ct[HKEM_CIPHERTEXTBYTES],
                const uint8_t sk[HKEM_SECRETKEYBYTES]);

_Static_assert(HKEM_PUBLICKEYBYTES == 1184U + 32U, "X-Wing pk size");
_Static_assert(HKEM_CIPHERTEXTBYTES == 1088U + 32U, "X-Wing ct size");
_Static_assert(HKEM_SECRETKEYBYTES == 32U, "X-Wing sk size");
_Static_assert(HKEM_SHAREDSECRETBYTES == 32U, "X-Wing ss size");
```

Compile-time checks that use liboqs constants go in the library's .c file, so the public header does not depend on liboqs:

```c
_Static_assert(OQS_KEM_ml_kem_768_length_public_key + 32 == HKEM_PUBLICKEYBYTES, "pk");
_Static_assert(OQS_KEM_ml_kem_768_length_ciphertext + 32 == HKEM_CIPHERTEXTBYTES, "ct");
_Static_assert(OQS_KEM_ml_kem_768_length_keypair_seed == 64, "keypair seed");
_Static_assert(OQS_KEM_ml_kem_768_length_encaps_seed + 32 == HKEM_ENCAPS_SEEDBYTES, "eseed");
```

### 2.1 Return codes

| Value | Name | Meaning |
|---|---|---|
| `0` | `HKEM_OK` | Success |
| `-1` | `HKEM_ERR_RNG` | The random source failed |
| `-2` | `HKEM_ERR_MLKEM` | A liboqs call returned non-zero; for encaps this includes a failed FIPS 203 section 7.2 encapsulation-key check |
| `-3` | `HKEM_ERR_X25519` | An OpenSSL X25519 call failed, including the all-zero output rejection (section 4.6) |
| `-4` | `HKEM_ERR_HASH` | An OpenSSL SHA3-256 or SHAKE256 call failed |
| `-5` | `HKEM_ERR_ARG` | Bad argument (for example a NULL pointer) |

On any error, outputs must not be used, and secret outputs are wiped.

liboqs 0.16.0 performs the FIPS 203 section 7.2 encapsulation-key check (type and modulus) in `OQS_KEM_ml_kem_768_encaps` and `OQS_KEM_ml_kem_768_encaps_derand` (mlkem-native `mlk_kem_check_pk`; these are the functions the generic `OQS_KEM_encaps` dispatches to) and returns `OQS_ERROR` if it fails. The type (length) check is also covered by the fixed 1184 B buffer. liboqs cannot tell a key-check failure from other failures, so the library treats any liboqs status other than `OQS_SUCCESS` as failure (not only `== OQS_ERROR`). It tests `rc != OQS_SUCCESS`, not `rc != 0`: `OQS_STATUS` is an enumeration ([S6](SOURCES.md#s6), `common.h`), and comparing it with a plain integer mixes essential type categories (MISRA C:2012 Rule 10.4).

### 2.2 Buffer rules

- All buffers are caller-allocated and fixed-size.
- The library wipes all internal secret material on every exit path ([SRS.md](SRS.md) SEC-02), with `OQS_MEM_cleanse` or `OPENSSL_cleanse` (SEC-09). `memset_s` and the other C11 Annex K (bounds-checking interface) functions are not used, because MISRA C:2012 Rule 1.4, added by Amendment 2, prohibits them ([S18](SOURCES.md#s18)); `__STDC_WANT_LIB_EXT1__` stays undefined.
- Wipes and copies use the `HKEM_*` (or liboqs length) constants, never `sizeof` of an array parameter. A parameter declared as `uint8_t sk[HKEM_SECRETKEYBYTES]` is a pointer, so `sizeof sk` gives the pointer size, not 32 (MISRA Rule 12.5, Mandatory; cppcheck does not check it).
- `hkem_keygen_derand` copies `seed` into `sk` with `memmove`, so `sk` and `seed` may be the same buffer or overlap. `memcpy` between overlapping objects is undefined behaviour (C11 7.24.2.1, [S19](SOURCES.md#s19); MISRA Rule 19.1, Mandatory). Tested by IF-08.

## 3. Sizes

| Item | Bytes | Source |
|---|---|---|
| ML-KEM-768 encapsulation key (public) | 1184 | FIPS 203, Table 3 ([S2](SOURCES.md#s2)) |
| ML-KEM-768 decapsulation key (secret) | 2400 | FIPS 203, Table 3 ([S2](SOURCES.md#s2)) |
| ML-KEM-768 ciphertext | 1088 | FIPS 203, Table 3 ([S2](SOURCES.md#s2)) |
| ML-KEM-768 shared secret | 32 | FIPS 203, Table 3 ([S2](SOURCES.md#s2)) |
| ML-KEM-768 derandomized keygen seed (d, z) in liboqs | 64 | liboqs 0.16.0 `kem_ml_kem.h` (`OQS_KEM_ml_kem_768_length_keypair_seed`) ([S6](SOURCES.md#s6)) |
| ML-KEM-768 derandomized encaps seed (m) in liboqs | 32 | liboqs 0.16.0 `kem_ml_kem.h` (`OQS_KEM_ml_kem_768_length_encaps_seed`) ([S6](SOURCES.md#s6)) |
| X25519 public key (u-coordinate) | 32 | RFC 7748, section 5 ([S3](SOURCES.md#s3)) |
| X25519 secret scalar | 32 | RFC 7748, section 5 ([S3](SOURCES.md#s3)) |
| X25519 shared secret | 32 | RFC 7748, section 6.1 ([S3](SOURCES.md#s3)) |
| X-Wing encapsulation key (public) | 1216 | draft-connolly-cfrg-xwing-kem-11, section 5.1 ([S1](SOURCES.md#s1)) |
| X-Wing decapsulation key (secret seed) | 32 | draft-connolly-cfrg-xwing-kem-11, section 5.1 ([S1](SOURCES.md#s1)) |
| X-Wing ciphertext | 1120 | draft-connolly-cfrg-xwing-kem-11, section 5.1 ([S1](SOURCES.md#s1)) |
| X-Wing shared secret | 32 | draft-connolly-cfrg-xwing-kem-11, section 5.1 ([S1](SOURCES.md#s1)) |
| X-Wing derandomized encapsulation seed (eseed) | 64 | draft-connolly-cfrg-xwing-kem-11, section 5.4.1 ([S1](SOURCES.md#s1)) |
| X-Wing expanded seed (SHAKE256 output) | 96 | draft-connolly-cfrg-xwing-kem-11, section 5.2 ([S1](SOURCES.md#s1)) |

## 4. Byte layouts

All values are fixed-length byte strings; X-Wing serialisation is plain concatenation (`||`), with no length prefixes or headers.

### 4.1 Public key (1216 B)

| Offset | Length | Field |
|---|---|---|
| 0 | 1184 | pk_M: ML-KEM-768 encapsulation key |
| 1184 | 32 | pk_X: X25519 public key |

### 4.2 Secret key (32 B)

The secret key is the 32 B seed `sk` only. It is expanded on use:

```
expanded = SHAKE256(sk, 96 bytes)
(pk_M, sk_M) = ML-KEM-768.KeyGen_internal(d = expanded[0:32], z = expanded[32:64])
sk_X = expanded[64:96]
pk_X = X25519(sk_X, X25519_BASE)
```

- FIPS 203 `KeyGen_internal` returns (encapsulation key, decapsulation key); the draft notes this is the reverse of its own `GenerateKeyPair()` order.
- liboqs 0.16.0 orders the 64 B `keypair_derand` seed as d then z (mlkem-native `kem.c` and `indcpa.c`), so `expanded[0:64]` is passed directly. OpenSSL 3.5 and wolfSSL use the same (d, z) order.
- Expanded keys may be cached in memory (draft section 5.5.1) but must not be transmitted between implementations. This library does not cache them; it re-derives them on every decaps.
- Erratum note: draft -11 section 5.2.1 says `GenerateKeyPairDerand()` returns "the 32 byte secret encapsulation key sk and the 1216 byte decapsulation key pk", which swaps the two names. This project follows sections 5.1 and 5.2: sk is the 32 B decapsulation key and pk is the 1216 B encapsulation key.

### 4.3 Ciphertext (1120 B)

| Offset | Length | Field |
|---|---|---|
| 0 | 1088 | ct_M: ML-KEM-768 ciphertext |
| 1088 | 32 | ct_X: X25519 ephemeral public key |

### 4.4 Derandomized encapsulation seed (64 B)

| Offset | Length | Use |
|---|---|---|
| 0 | 32 | m for ML-KEM-768 `Encaps_internal` (liboqs `encaps_derand` seed) |
| 32 | 32 | ek_X, the X25519 ephemeral secret scalar |

In `hkem_encaps`, the 64 bytes come from `RAND_priv_bytes`; in `hkem_keygen`, the 32 B seed does too.

### 4.5 Combiner

```
ss = SHA3-256( ss_M || ss_X || ct_X || pk_X || XWingLabel )
XWingLabel = "\./" || "/^\"   (6 ASCII bytes, hex 5c2e2f2f5e5c)
```

| Order | Input | Bytes |
|---|---|---|
| 1 | ss_M, ML-KEM-768 shared secret | 32 |
| 2 | ss_X, X25519 shared secret | 32 |
| 3 | ct_X, X25519 ephemeral public key | 32 |
| 4 | pk_X, X25519 public key | 32 |
| 5 | XWingLabel | 6 |

Total hash input: 134 B. ASCII = American Standard Code for Information Interchange. The combiner does not hash ct_M or pk_M; the draft explains this is safe only because of ML-KEM-768's Fujisaki-Okamoto (FO) transform (draft sections 1 and 6), and its security also assumes SHA3-256 and SHAKE256 are secure. The draft claims the binding properties MAL-BIND-K-PK and MAL-BIND-K-CT for X-Wing; its proof reference is marked TODO in -11, so this is a claim, not a proven result.

### 4.6 Encoding and endianness

- X25519 scalars and u-coordinates are 32 B little-endian strings. Scalars are clamped as in RFC 7748 ([S3](SOURCES.md#s3)) `decodeScalar25519`; the most significant bit of a received u-coordinate is masked (RFC 7748 section 5). X25519_BASE is the u-coordinate 9 (hex `09` followed by 31 zero bytes).
- ML-KEM-768 keys and ciphertexts are byte strings produced by FIPS 203 `ByteEncode`/`Compress`; bits are packed little-endian within bytes (FIPS 203 section 4.2.1 ([S2](SOURCES.md#s2))). No further encoding is applied.
- SHAKE256 output length is given in bytes here; the draft writes it in bits (96*8).
- All-zero X25519 output: OpenSSL 3.5's `EVP_PKEY_derive` always rejects an all-zero X25519 output (RFC 7748 section 6.1 allows this). The draft doesn't require it, so this is a documented deviation: a small-order pk_X (encaps) or ct_X (decaps) gives `HKEM_ERR_X25519`. The project adds no check of its own. The deviation leaks nothing secret, because whether a point has small order is public. Random tampering of ct_X lands on a small-order point with negligible probability.

## 5. Decapsulation behaviour

`hkem_decaps` re-derives (sk_M, sk_X, pk_M, pk_X) from the seed, splits the ciphertext as in section 4.3, computes ss_M = ML-KEM-768.Decaps(ct_M, sk_M) and ss_X = X25519(sk_X, ct_X), then returns the combiner output. ML-KEM-768 uses implicit rejection: a bad ct_M gives a pseudorandom ss_M, not an error. If ct_X is a small-order point, OpenSSL's derive fails and decaps returns `HKEM_ERR_X25519` (section 4.6).

## 6. Dependencies

| Library | Version (pinned) | APIs used |
|---|---|---|
| liboqs (Open Quantum Safe) | 0.16.0 (tag commit 5a1a854b, released 9 Jul 2026; [S6](SOURCES.md#s6)); ML-KEM backend mlkem-native 0ba906cb | M1 library: the per-algorithm functions in `kem_ml_kem.h`: `OQS_KEM_ml_kem_768_keypair`, `OQS_KEM_ml_kem_768_keypair_derand`, `OQS_KEM_ml_kem_768_encaps`, `OQS_KEM_ml_kem_768_encaps_derand` and `OQS_KEM_ml_kem_768_decaps`, plus `OQS_MEM_cleanse`. They take caller buffers and need no `OQS_KEM` object, so the library makes no liboqs allocation (in `kem_ml_kem_768.c`, only `OQS_KEM_ml_kem_768_new` allocates). This is an internal dependency change; the public API in section 2 is unchanged. M0 program only (FR-01): `OQS_KEM_new(OQS_KEM_alg_ml_kem_768)`, `OQS_KEM_keypair`, `OQS_KEM_encaps`, `OQS_KEM_decaps`, `OQS_KEM_free`. Status codes (`OQS_STATUS` enumeration): `OQS_SUCCESS` (0), `OQS_ERROR` (-1) and `OQS_EXTERNAL_LIB_ERROR_OPENSSL` (50); test `rc != OQS_SUCCESS`. |
| OpenSSL | 3.5.9 (29 Sep 2026, [S7](SOURCES.md#s7)), 3.5 long-term support (LTS) branch | X25519: `EVP_PKEY_new_raw_private_key_ex`, `EVP_PKEY_new_raw_public_key_ex`, `EVP_PKEY_get_raw_public_key`, `EVP_PKEY_CTX_new_from_pkey`, `EVP_PKEY_derive_init`, `EVP_PKEY_derive_set_peer`, `EVP_PKEY_derive` (EVP = OpenSSL's high-level "envelope" API). Hashes: SHA3-256 (Secure Hash Algorithm 3, [S4](SOURCES.md#s4)) via `EVP_DigestInit_ex`/`EVP_DigestUpdate`/`EVP_DigestFinal_ex`; SHAKE256 via `EVP_DigestFinalXOF(ctx, out, 96)` (XOF = extendable-output function). RNG: `RAND_priv_bytes`. Wiping: `OPENSSL_cleanse`. M0/M2 interop: native ML-KEM-768 via `EVP_PKEY_encapsulate` and `EVP_PKEY_decapsulate` (API since 3.0; ML-KEM support since 3.5). |
| wolfSSL | 5.9.4 (25 Sep 2026, [S8](SOURCES.md#s8)), configured with `--enable-mlkem` (M2 interop only) | `wc_MlKemKey_Init(key, WC_ML_KEM_768, NULL, INVALID_DEVID)` (`INVALID_DEVID` is defined as `(-2)` in `wolfssl/wolfcrypt/types.h`, lines 1866-1867, [S16](SOURCES.md#s16)), `wc_MlKemKey_MakeKeyWithRandom`, `wc_MlKemKey_EncapsulateWithRandom`, `wc_MlKemKey_Decapsulate`, `wc_MlKemKey_EncodePublicKey`/`DecodePublicKey`, `wc_MlKemKey_EncodePrivateKey`/`DecodePrivateKey`, `wc_MlKemKey_Free`. The older `wc_KyberKey_*` names are compatibility aliases only and are not used (`wc_KyberKey_Init` has a different argument order). |
| Toolchain | clang and CMake; exact versions recorded in [README.md](../README.md) at each milestone review | C11 |

## 7. Serial protocol (M6), TBD

To be defined if the optional M6 milestone goes ahead. Target: an RP2350-class board (520 KB static RAM (SRAM), RP2350 datasheet section 4.2, [S17](SOURCES.md#s17)) over USB (Universal Serial Bus) serial. Framing, message types, byte order and error handling: TBD.

## 8. Sources

All external sources are listed in [SOURCES.md](SOURCES.md).
