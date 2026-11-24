# hybrid-kem-c

Hybrid X25519 + ML-KEM-768 key exchange in C, built and verified the systems engineering way.

## What it is

`hybrid-kem-c` is a small C11 library and set of example programs for X-Wing, a hybrid key encapsulation mechanism (KEM) specification (IETF draft, see [S1](docs/SOURCES.md#s1)) that combines classical X25519 with post-quantum ML-KEM-768. X25519 and the hash functions come from OpenSSL, ML-KEM-768 from liboqs, and an educational ML-KEM-768 is written from scratch alongside. Two parties agree a 32-byte shared secret that stays secure as long as either component holds.

X-Wing sizes, in bytes:

| Public key | Secret key (seed) | Ciphertext | Shared secret |
| ---------- | ----------------- | ---------- | ------------- |
| 1216       | 32                | 1120       | 32            |

The project has three parts:

- an X-Wing library built on liboqs and OpenSSL 3.5, with a small, fixed-size C API;
- a from-scratch, educational ML-KEM-768 written to be read alongside FIPS 203 ([S2](docs/SOURCES.md#s2)) (not constant-time, not for production);
- an optional port to an RP2350-class microcontroller board, talking to a host over USB serial.

## Approach

The work is specified and verified first, then implemented. The document set:

- [Concept of Operations](docs/CONOPS.md)
- [System Requirements Specification](docs/SRS.md)
- [Interface Control Document](docs/ICD.md)
- [Verification plan and traceability](docs/VERIFICATION.md)
- [Technical risk register](docs/RISKS.md)
- [Coding standard and deviation log](docs/CODING-STANDARD.md)
- [Sources](docs/SOURCES.md)

Every requirement in [SRS.md](docs/SRS.md) is traced to a verification method in [VERIFICATION.md](docs/VERIFICATION.md).

Coding standard: the project follows MISRA C:2012 (Amendments 1 and 2) as its coding standard. It is checked with the open-source cppcheck MISRA add-on (partial rule coverage, no directives) plus inspection, and deviations are logged in [CODING-STANDARD.md](docs/CODING-STANDARD.md). This is not a certified compliance claim. MISRA's headline file is downloaded locally and is ignored by `.gitignore`; it is never committed ([SRS.md](docs/SRS.md) QUAL-08).

## Threat model

`hybrid-kem-c` is a key encapsulation mechanism (KEM): it gives two parties a fresh 32-byte shared secret. It implements X-Wing ([S1](docs/SOURCES.md#s1)). This section covers what the construction protects against, what it does not, and which side-channel properties come from the underlying libraries.

### What it protects against

- **Harvest-now-decrypt-later against X25519.** Suppose an attacker records traffic today and later gets a quantum computer. They can then break X25519, but they still have to break ML-KEM-768 to recover the shared secret.
- **A break of either component alone.** The draft states that X-Wing is secure if SHA-3 is secure and either X25519 or ML-KEM-768 is secure. More precisely, if SHA3-256, SHA3-512 and SHAKE256 are modelled as random oracles, X-Wing's IND-CCA security is bounded by the IND-CCA security of ML-KEM-768 and the gap-CDH security of Curve25519 (draft section 6, [S1](docs/SOURCES.md#s1)). Both rest on the draft's assumption that SHA3-256, SHA3-512 and SHAKE256 ([S4](docs/SOURCES.md#s4)) can be modelled as random oracles.

### What it does not protect against

- **Lack of authentication.** X-Wing is not an authenticated KEM (draft sections 1.4 and 5.6). It does not prove who holds a public key or who sent a ciphertext. The surrounding protocol must defend against an active man-in-the-middle, for example with signatures or certificates.
- **Side channels in the underlying libraries.** `hybrid-kem-c` relies on liboqs and OpenSSL for every operation on secret data and adds no countermeasures of its own. Timing, power and fault attacks on those libraries are out of scope. Their documented status is in the table below.

### Why the combiner hashes ct_X and pk_X but not ct_M

The shared secret is SHA3-256(ss_M || ss_X || ct_X || pk_X || XWingLabel) ([ICD.md](docs/ICD.md) section 4.5). The combiner hashes the X25519 ciphertext and public key, as the draft specifies. The ML-KEM-768 ciphertext ct_M is left out by the draft's design (sections 1.2 and 6). The draft's security analysis relies on the specifics of ML-KEM-768's Fujisaki-Okamoto (FO) transform, and states that leaving out the post-quantum ciphertext is not known to be safe in general, so the combiner is not assumed to be secure with other KEMs. Leaving ct_M out also saves hashing 1088 bytes. This is not a binding claim: binding is covered below.

### Binding properties (claimed, not proven)

The draft claims that X-Wing satisfies MAL-BIND-K-PK and MAL-BIND-K-CT, and lists further binding properties that follow from them. Its proof reference is marked TODO in -11, so these are the draft's claims, not proven results (draft section 6.1, [S1](docs/SOURCES.md#s1)). The draft also notes two limits:

- ML-KEM on its own does not achieve these properties.
- They do not hold if a regular ML-KEM decapsulation key is transmitted in place of the 32-byte seed (draft section 5.5.1).

`hybrid-kem-c` stores and accepts only the seed as the secret key.

### Constant-time status of the primitives

| Primitive | Provider | Documented constant-time status | Source |
|---|---|---|---|
| ML-KEM-768 keygen, encaps and decaps | liboqs 0.16.0 (mlkem-native backend) | `no-secret-dependent-branching-claimed: true` for the `ref`, `x86_64` and `aarch64` implementations; checked with Valgrind for `ref` and `x86_64` only | [S6](docs/SOURCES.md#s6) (`docs/algorithms/kem/ml_kem.yml`) |
| X25519 | OpenSSL 3.5.9 | Not documented by OpenSSL | [S7](docs/SOURCES.md#s7) |
| SHA3-256 and SHAKE256 | OpenSSL 3.5.9 | Not documented by OpenSSL | [S7](docs/SOURCES.md#s7) |

The library's own code does not branch on secret data or use it to index memory. It wipes secrets with `OQS_MEM_cleanse` or `OPENSSL_cleanse` on every exit path ([SRS.md](docs/SRS.md) SEC-02, SEC-08 and SEC-09).

**The educational ML-KEM-768 (M2) is not constant-time.** It is written to be read alongside FIPS 203 ([S2](docs/SOURCES.md#s2)) and is not for production use ([SRS.md](docs/SRS.md) SEC-07).

### Documented difference from the draft

OpenSSL 3.5 rejects an all-zero X25519 shared secret, which RFC 7748 section 6.1 allows ([S3](docs/SOURCES.md#s3)). As a result, `hkem_encaps` with a small-order X25519 public key, or `hkem_decaps` with a small-order X25519 ciphertext, returns `HKEM_ERR_X25519` instead of a shared secret. This reveals nothing secret, because whether a point has small order is public. A tampered ML-KEM-768 ciphertext still never causes an error (implicit rejection). See [ICD.md](docs/ICD.md) section 4.6.

## Milestones

| Milestone               | Scope                                                                                                                                                              |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| M0                      | An ML-KEM-768 round trip with liboqs, cross-checked against OpenSSL 3.5's native ML-KEM                                                                            |
| M1                      | The X-Wing hybrid KEM, built on OpenSSL 3.5 (X25519, SHA-3) and liboqs (ML-KEM-768), behind a small C API                                                          |
| M2                      | An educational ML-KEM-768 written from scratch in plain C, checked against NIST ACVP test vectors and interoperable both ways with liboqs, OpenSSL 3.5 and wolfSSL |
| M3                      | A tutorial lab                                                                                                                                                     |
| M4                      | A short demonstration program (runs from a clean checkout in under 2 minutes)                                                                                      |
| M5                      | One small upstream contribution                                                                                                                                    |
| M6 (optional, after M2) | The M2 ML-KEM-768 plus X25519 on an RP2350-class board (520 KB SRAM), talking to a host over USB serial                                                            |

Boards with only a few KB of RAM, such as the ATmega328P (2 KB SRAM), cannot hold the 2400-byte ML-KEM-768 decapsulation key and are out of scope.

## Dependencies

Pinned versions, re-checked at each milestone review:

| Dependency | Version | Role |
|---|---|---|
| liboqs | 0.16.0 ([S6](docs/SOURCES.md#s6)) | ML-KEM-768 |
| OpenSSL | 3.5.9 (3.5 LTS) ([S7](docs/SOURCES.md#s7)) | X25519, SHA3-256, SHAKE256, RNG; native ML-KEM for interop |
| wolfSSL | 5.9.4, built with `--enable-mlkem` ([S8](docs/SOURCES.md#s8)) | M2 interop only |
| X-Wing draft | draft-connolly-cfrg-xwing-kem-11 ([S1](docs/SOURCES.md#s1)) | Construction and test vectors |
| Toolchain | clang and CMake, C11 | Exact compiler versions recorded at each milestone review |
| cppcheck | 2.22.0, with the `misra.py` add-on ([S13](docs/SOURCES.md#s13)) | MISRA C:2012 static analysis |
| clang-tidy | Recorded at each milestone review ([S14](docs/SOURCES.md#s14)) | SEI CERT C subset (`cert-*`), separate from MISRA |

Build flags: `-std=c11 -Wall -Wextra -Wpedantic -Werror`. Debug builds add `-fsanitize=address,undefined`.

## Sources

All external sources are listed in [SOURCES.md](docs/SOURCES.md).

## License

MIT. See [LICENSE](LICENSE). Copyright (c) 2026 ion-.
