# hybrid-kem-c: System Requirements Specification (SRS)

Version 0.1 (draft), November 2026. Author: ion-.

## 1. Purpose

This System Requirements Specification (SRS) states what `hybrid-kem-c` shall do. `hybrid-kem-c` is a small C11 library with example programs that implements X-Wing, a hybrid key encapsulation mechanism (KEM) that combines X25519 and ML-KEM-768 (Module-Lattice-Based Key-Encapsulation Mechanism, 768 parameter set), as defined in an Internet Engineering Task Force (IETF) Internet-Draft (see [S1](SOURCES.md#s1)). The project also contains a from-scratch ML-KEM-768 written for learning, not for production use.

Each requirement has a unique ID, a milestone (M0 to M6) and a planned verification method. Interfaces, sizes and byte formats are in [ICD.md](ICD.md) (Interface Control Document). Verification methods, evidence and the traceability matrix are in [VERIFICATION.md](VERIFICATION.md). Together, SRS.md and [VERIFICATION.md](VERIFICATION.md) are the project's requirements list. Operational context is in [CONOPS.md](CONOPS.md), technical risks in [RISKS.md](RISKS.md), the coding standard and deviation log in [CODING-STANDARD.md](CODING-STANDARD.md), and sources in [SOURCES.md](SOURCES.md).

## 2. Scope

In scope:

- **M0**: an ML-KEM-768 round trip with liboqs (Open Quantum Safe), cross-checked against OpenSSL 3.5's native ML-KEM.
- **M1**: the X-Wing hybrid KEM, built on OpenSSL 3.5 (X25519, SHA-3 (Secure Hash Algorithm 3)) and liboqs (ML-KEM-768), behind a small C application programming interface (API).
- **M2**: an educational ML-KEM-768 written from scratch in plain C (ML-KEM-768 only; SHA-3 and SHAKE (the SHA-3 extendable-output functions) may come from OpenSSL), checked against the National Institute of Standards and Technology (NIST) Automated Cryptographic Validation Protocol (ACVP) test vectors ([S5](SOURCES.md#s5)).
- **M3 to M5**: a tutorial lab, a short demonstration and one small upstream contribution. Only their product-facing requirements appear here.
- **M6 (optional, after M2)**: running the M2 ML-KEM-768 plus X25519 on a microcontroller board, talking to a host computer over USB (Universal Serial Bus) serial. M6 has its own milestone review, MR6, held only if M6 goes ahead.

Out of scope: authentication (this is a KEM only, with no signatures), side-channel resistance of the underlying libraries, constant-time behaviour of the educational ML-KEM-768, a constant-time test harness (timing tests or Valgrind-based checks), other ML-KEM parameter sets, ML-DSA (Module-Lattice-Based Digital Signature Algorithm), performance optimisation, and coding-standard conformance of third-party libraries.

### 2.1 Stakeholders

| ID | Stakeholder | Need |
|---|---|---|
| SN1 | Developers evaluating hybrid KEMs | A small, readable, tested X-Wing implementation with documented sizes, formats and measured performance |
| SN2 | Learners | A from-scratch ML-KEM-768 that maps clearly onto FIPS 203 ([S2](SOURCES.md#s2)) and is honestly labelled educational |
| SN3 | Interoperability (interop) testers | Reproducible, derandomized outputs and cross-checks against other ML-KEM and X-Wing implementations |

### 2.2 Conventions

- "shall" marks a requirement. Each requirement is atomic and verifiable.
- Verification methods: **T** = Test, **A** = Analysis, **I** = Inspection, **D** = Demonstration.
- FIPS = Federal Information Processing Standard. RFC = Request for Comments. MR = milestone review. B = bytes, KB = kilobytes.
- MISRA C = the C coding guidelines published by MISRA (originally the Motor Industry Software Reliability Association). SEI = Software Engineering Institute (Carnegie Mellon University), publisher of the SEI CERT C Coding Standard.

## 3. Requirements

### 3.1 Functional (FR)

| ID | Requirement | Milestone | Method |
|---|---|---|---|
| FR-01 | The M0 program shall generate an ML-KEM-768 key pair, encapsulate and decapsulate using liboqs (`OQS_KEM_new(OQS_KEM_alg_ml_kem_768)`, `OQS_KEM_keypair`, `OQS_KEM_encaps`, `OQS_KEM_decaps`). | M0 | T |
| FR-02 | The M0 program shall compare the encapsulated and decapsulated shared secrets and exit with a non-zero status if they differ. | M0 | T |
| FR-03 | The M0 program shall show that a tampered ciphertext decapsulates to a different shared secret without an error being returned (implicit rejection). | M0 | T |
| FR-04 | The M0 program shall show that an ML-KEM-768 ciphertext produced by liboqs decapsulates to the same shared secret with OpenSSL 3.5's native ML-KEM, and the reverse. | M0 | T |
| FR-05 | The library shall implement X-Wing key generation, encapsulation and decapsulation as specified in draft-connolly-cfrg-xwing-kem-11 ([S1](SOURCES.md#s1)) (sizes, key expansion, combiner and label). | M1 | T |
| FR-06 | X-Wing key generation shall output a 32 B seed as the secret (decapsulation) key and derive the ML-KEM-768 and X25519 keys from it with SHAKE256, as in the draft's `expandDecapsulationKey`. | M1 | T |
| FR-07 | X-Wing decapsulation shall re-derive the ML-KEM-768 and X25519 keys from the 32 B seed. | M1 | T |
| FR-08 | The library shall provide a derandomized key generation that takes a caller-supplied 32 B seed and returns that seed as the secret key. | M1 | T |
| FR-09 | The library shall provide a derandomized encapsulation that takes a caller-supplied 64 B encapsulation seed (eseed), using eseed[0:32] for ML-KEM-768 and eseed[32:64] as the X25519 ephemeral key. | M1 | T |
| FR-10 | The derandomized outputs shall match the three test vectors in Appendix C of the pinned draft revision (-11, [S1](SOURCES.md#s1)). The draft marks these vectors "TODO: replace"; they are usable for -11, and the test is re-run if the pinned revision changes. If a later pinned revision has no usable vectors, the README shall say so and round-trip and cross-implementation tests apply instead. | M1 | T |
| FR-11 | (Optional) A host program shall complete an X-Wing key exchange with the M6 board over USB serial, with both sides deriving the same 32 B shared secret. | M6 | D |

### 3.2 Interface (IF)

| ID | Requirement | Milestone | Method |
|---|---|---|---|
| IF-01 | The library shall expose the KEM operations `hkem_keygen`, `hkem_encaps` and `hkem_decaps`, plus the derandomized variants `hkem_keygen_derand` and `hkem_encaps_derand`, as defined in [ICD.md](ICD.md) section 2. | M1 | I |
| IF-02 | All API buffers shall be fixed-size, caller-allocated byte arrays whose sizes are named constants in the public header, equal to the sizes in [ICD.md](ICD.md) section 3, and checked at compile time with C11 `_Static_assert`. Checks that use liboqs constants shall be in the library's .c file, not the public header. | M1 | I |
| IF-03 | Each API function shall return 0 on success and a distinct negative code per failure cause ([ICD.md](ICD.md) section 2.1). | M1 | T |
| IF-04 | `hkem_decaps` shall not return an error for a tampered ML-KEM-768 ciphertext part; it shall return the combiner output (implicit rejection). An X25519 part that is a small-order point is rejected by OpenSSL and returns `HKEM_ERR_X25519` ([ICD.md](ICD.md) section 4.6). | M1 | T |
| IF-05 | Public keys and ciphertexts shall use the byte layout and concatenation order in [ICD.md](ICD.md) section 4 (ML-KEM-768 part first, then the X25519 part). | M1 | T |
| IF-06 | (Optional) The board and the host shall exchange messages using the serial protocol in [ICD.md](ICD.md) section 7. | M6 | T |
| IF-07 | The test suite shall show that `hkem_decaps` with ct_X set to 32 zero bytes, and `hkem_encaps` with pk_X set to 32 zero bytes, each return `HKEM_ERR_X25519`. | M1 | T |
| IF-08 | `hkem_keygen_derand` shall give the correct result when `sk` and `seed` are the same buffer or overlap ([ICD.md](ICD.md) section 2.2). | M1 | T |

### 3.3 Security (SEC)

| ID | Requirement | Milestone | Method |
|---|---|---|---|
| SEC-01 | The M0 program shall wipe every secret buffer with `OQS_MEM_cleanse` before freeing it, and shall free every allocation. | M0 | I |
| SEC-02 | The library shall wipe all secret material (the seed, the 96 B expanded seed, the ML-KEM-768 decapsulation key, the X25519 secret and ephemeral keys, ss_M, ss_X and intermediate buffers) on every exit path, including error paths, using `OQS_MEM_cleanse` or `OPENSSL_cleanse` rather than plain `memset`. | M1 | I |
| SEC-03 | Every return code from liboqs and OpenSSL shall be checked and handled; any liboqs status other than `OQS_SUCCESS` shall be treated as failure, tested as `rc != OQS_SUCCESS`. | M1 | I |
| SEC-04 | The test suite shall run under AddressSanitizer (ASan), LeakSanitizer and UndefinedBehaviorSanitizer (UBSan) with no findings. | M1 | T |
| SEC-05 | [README.md](../README.md) shall contain a one-page threat model stating: what the hybrid protects against (harvest-now-decrypt-later against X25519; a break of either X25519 or ML-KEM-768 alone); the assumption this rests on (the draft's security argument models SHA3-256, SHA3-512 and SHAKE256 as random oracles, draft section 6); what it does not protect against (authentication, side channels in the underlying libraries); and why the combiner hashes the X25519 ciphertext and public key but not the ML-KEM-768 ciphertext (ct_M is left out by the draft's design, relying on the properties of ML-KEM-768 and its Fujisaki-Okamoto (FO) transform as analysed in the draft, sections 1.2 and 6; the draft does not claim this is safe for other KEMs). The threat model shall not claim that any binding property is proven (SEC-06). | M1 | I |
| SEC-06 | The threat model shall describe X-Wing's binding properties (MAL-BIND-K-PK and MAL-BIND-K-CT) as claimed by the draft (its proof reference is marked TODO in -11), not as proven. | M1 | I |
| SEC-07 | The educational ML-KEM-768 shall be labelled "educational, not constant-time, not for production" in its README, with a list of where it differs from a hardened implementation. | M2 | I |
| SEC-08 | M1 shall do every operation on secret data with library primitives only, and the README shall record each primitive's documented constant-time status, with its source (liboqs 0.16.0 ([S6](SOURCES.md#s6)) ML-KEM-768: `no-secret-dependent-branching-claimed: true`; OpenSSL ([S7](SOURCES.md#s7)) X25519 and SHA-3: not documented by OpenSSL). The project's own code shall not branch on, or index memory by, secret data. The educational M2 code is explicitly not constant-time (SEC-07). | M1 | I |
| SEC-09 | No in-scope code ([CODING-STANDARD.md](CODING-STANDARD.md) section 2) shall use C11 Annex K (bounds-checking interface) functions, `explicit_bzero` or a plain `memset` to wipe secrets. Host builds shall use `OQS_MEM_cleanse` or `OPENSSL_cleanse`, and the M6 board code shall use one reviewed volatile wipe function. Wipe lengths shall use named size constants, never `sizeof` of an array parameter. | M1 (M2, M6) | I |

### 3.4 Quality, build and portability (PERF/QUAL)

| ID      | Requirement                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      | Milestone | Method |
| ------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | --------- | ------ |
| QUAL-01 | All project code shall build with CMake using `-std=c11 -Wall -Wextra -Wpedantic -Werror`, and debug builds shall add `-fsanitize=address,undefined`.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            | M0        | T      |
| QUAL-02 | [README.md](../README.md) shall pin the versions of liboqs, OpenSSL, wolfSSL and the compiler, and the X-Wing draft revision ([ICD.md](ICD.md) section 6), re-recorded at each milestone review.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 | M0        | I      |
| QUAL-03 | The M0 program shall print the ML-KEM-768 encapsulation key, decapsulation key, ciphertext and shared secret sizes and check them against FIPS 203 ([S2](SOURCES.md#s2)) (1184 B, 2400 B, 1088 B, 32 B).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         | M0        | T      |
| PERF-01 | The benchmark shall report the median of at least 1000 runs each for keygen, encaps and decaps.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  | M1        | A      |
| PERF-02 | Each published benchmark result shall state the central processing unit (CPU), operating system (OS), compiler and library versions used.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        | M1        | I      |
| PERF-03 | The educational README shall report the stack high-water mark and the code size (`size` on the object files) for keygen, encaps and decaps.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      | M2        | A      |
| QUAL-04 | The demonstration program shall run from a clean checkout in under 2 minutes.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    | M4        | D      |
| QUAL-05 | (Optional) The M6 target shall be an RP2350-class board (520 KB static RAM (SRAM)). Boards with only a few KB of RAM, such as the ATmega328P (2 KB SRAM), are excluded because they cannot hold the 2400 B ML-KEM-768 decapsulation key.                                                                                                                                                                                                                                                                                                                                                                                                                                                         | M6        | A      |
| PERF-04 | (Optional) The M6 README shall report cycle counts or timings, the stack high-water mark and the code size on the board for keygen, encaps and decaps.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           | M6        | A      |
| QUAL-06 | The in-scope code (library `src/` and `hkem.h` from M1, educational ML-KEM-768 from M2, optional M6 board code from M6; [CODING-STANDARD.md](CODING-STANDARD.md) section 2) shall follow MISRA C:2012 as amended by Amendments 1 and 2 ([S9](SOURCES.md#s9), [S10](SOURCES.md#s10)). Every finding of the cppcheck MISRA add-on and of the [CODING-STANDARD.md](CODING-STANDARD.md) section 3 inspection shall be fixed or covered by a deviation record ([CODING-STANDARD.md](CODING-STANDARD.md) section 4). Mandatory guidelines shall have no findings. Third-party code and headers (liboqs, OpenSSL, wolfSSL, board support libraries) and the test, benchmark, demonstration and M0 programs are out of scope. | M1 | A, I |
| QUAL-07 | The `misra` and `tidy` build targets shall finish with exit code 0 on the in-scope code, with each deviated site suppressed inline and tagged with its deviation ID. The cppcheck and clang-tidy versions shall be pinned in [README.md](../README.md). | M1 | T |
| QUAL-08 | The repository shall not contain MISRA rule texts or headline files. Documents and code comments shall cite guideline numbers only. | M1 | I |

### 3.5 Educational ML-KEM-768 (EDU)

| ID | Requirement | Milestone | Method |
|---|---|---|---|
| EDU-01 | The educational code shall implement FIPS 203 ([S2](SOURCES.md#s2)) ML-KEM-768 key generation, encapsulation and decapsulation with implicit rejection, including K-PKE (the internal public-key encryption scheme), the number-theoretic transform (NTT) and its inverse, SampleNTT, SamplePolyCBD, encoding and compression, and the FIPS 203 section 7.2 encapsulation-key check (type and modulus) and section 7.3 decapsulation input checks. | M2 | T |
| EDU-02 | The educational code shall provide deterministic (seeded) key generation and encapsulation entry points. | M2 | T |
| EDU-03 | The educational code shall match the NIST ACVP ML-KEM keyGen test vectors ([S5](SOURCES.md#s5), FIPS 203 JSON (JavaScript Object Notation) files). | M2 | T |
| EDU-04 | The educational code shall match the NIST ACVP ML-KEM encapDecap test vectors. | M2 | T |
| EDU-05 | The educational code shall interoperate both ways with liboqs ML-KEM-768. | M2 | T |
| EDU-06 | The educational code shall interoperate both ways with OpenSSL 3.5's native ML-KEM-768. | M2 | T |
| EDU-07 | The educational code shall interoperate both ways with wolfSSL's ML-KEM-768. | M2 | T |
| EDU-08 | The lattice code shall use no dynamic memory allocation. | M2 | I |

### 3.6 Requirement count

| Group | Count |
|---|---|
| FR | 11 |
| IF | 8 |
| SEC | 9 |
| PERF/QUAL | 12 |
| EDU | 8 |
| **Total** | **48** |

## 4. Decisions and open items

- FR-10: draft -11 (23 Sep 2026) is the latest revision on the IETF datatracker ([S1](SOURCES.md#s1)) and is pinned. It is re-checked at MR1; the project moves to a later revision only if it replaces the Appendix C vectors or changes the construction, and records any move under QUAL-02.
- X25519 all-zero shared secret: the project adds no check of its own. OpenSSL 3.5 already rejects an all-zero X25519 output (RFC 7748 section 6.1 ([S3](SOURCES.md#s3)) allows this), which is a documented deviation from the draft (IF-04, IF-07, [ICD.md](ICD.md) section 4.6).
- Draft erratum: -11 section 5.2.1 ([S1](SOURCES.md#s1)) swaps sk and pk in its description of `GenerateKeyPairDerand()`. This project follows sections 5.1 and 5.2 (sk is the 32 B decapsulation key, pk the 1216 B encapsulation key).
- M6 serial protocol: TBD (see [ICD.md](ICD.md) section 7).
- QUAL-06 edition: MISRA C:2012 with Amendments 1 and 2. Amendment 2 brings C11 into scope ([S10](SOURCES.md#s10)), and this is the edition that the free, open-source cppcheck MISRA add-on checks and for which MISRA publishes free rule headlines ([S12](SOURCES.md#s12), [S13](SOURCES.md#s13)). Later editions exist: MISRA C:2023, which consolidates the later amendments, and MISRA C:2025, the current edition ([S9](SOURCES.md#s9)). A move to a later edition is considered at a milestone review only if a free checker supports it. MISRA C:2025 is now the latest edition; the project does not switch to it. cppcheck 2.22.0's add-on covers MISRA C:2012 with Amendments 1 and 2 but checks no directives, so the directives are on the manual checklist ([CODING-STANDARD.md](CODING-STANDARD.md) section 3).
- QUAL-06 is a code-quality requirement, not a security claim. MISRA C and the SEI CERT C Coding Standard are different standards with different goals and are not equivalent; the project runs a partial CERT C check as a separate, supporting analysis ([VERIFICATION.md](VERIFICATION.md) section 2.1).
- Dynamic memory (MISRA Dir 4.12): the M1 library calls liboqs's per-algorithm ML-KEM-768 functions, which take caller buffers and need no `OQS_KEM` object ([ICD.md](ICD.md) section 6). This is an internal dependency change; the public API is unchanged. OpenSSL's EVP objects are still allocated inside OpenSSL, and one project-level deviation covers this ([CODING-STANDARD.md](CODING-STANDARD.md) section 4, DV-04). The project's own code never calls `malloc`, `calloc`, `realloc` or `free`, so no Rule 21.3 deviation is needed. The M0 program (FR-01) keeps the generic `OQS_KEM_*` API; it is outside the MISRA scope.
- IF-08: `hkem_keygen_derand` copies the seed into `sk` with `memmove`, so the same or overlapping buffers are allowed ([ICD.md](ICD.md) section 2.2).
- SEC-05: the draft leaves ct_M out of the combiner by design (sections 1.2 and 6, [S1](SOURCES.md#s1)) and notes that ML-KEM on its own does not achieve MAL-BIND-K-CT (section 6.1). The requirement therefore does not say that the FO transform binds ct_M.

## 5. Sources

All external sources are listed in [SOURCES.md](SOURCES.md).
