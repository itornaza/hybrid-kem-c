# hybrid-kem-c: Sources

Version 0.1 (draft), November 2026. Author: ion-.

External sources used by the project documents. In-text citations use the IDs below and link here. Only this file contains external links.

## Specifications and standards

<a id="s1"></a>

### [S1] X-Wing draft -11

D. Connolly, P. Schwabe, B. E. Westerbaan, *X-Wing: general-purpose hybrid post-quantum KEM*, draft-connolly-cfrg-xwing-kem-11, 23 Sep 2026.

https://datatracker.ietf.org/doc/draft-connolly-cfrg-xwing-kem/11/

Appendix C of this draft is the source of the X-Wing test vectors used for FR-10.

<a id="s2"></a>

### [S2] NIST FIPS 203

NIST FIPS 203, *Module-Lattice-Based Key-Encapsulation Mechanism Standard*, 13 Aug 2024 (Table 3, sections 7.2 and 7.3, Algorithms 13 and 16).

https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.203.pdf

<a id="s3"></a>

### [S3] RFC 7748

A. Langley, M. Hamburg, S. Turner, RFC 7748, *Elliptic Curves for Security*, Jan 2016 (sections 5 and 6.1).

https://www.rfc-editor.org/rfc/rfc7748

<a id="s4"></a>

### [S4] NIST FIPS 202

NIST FIPS 202, *SHA-3 Standard*, Aug 2015.

https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.202.pdf

## Test vectors

<a id="s5"></a>

### [S5] NIST ACVP-Server ML-KEM vectors

NIST Automated Cryptographic Validation Protocol (ACVP) server, ML-KEM test vectors (FIPS 203 keyGen and encapDecap JSON files).

https://github.com/usnistgov/ACVP-Server

X-Wing Appendix C test vectors are in [S1](#s1).

## Libraries

<a id="s6"></a>

### [S6] liboqs 0.16.0

liboqs 0.16.0 (tag commit 5a1a854b, released 9 Jul 2026); ML-KEM backend mlkem-native 0ba906cb. Headers: `kem.h`, `kem_ml_kem.h` (per-algorithm functions `OQS_KEM_ml_kem_768_keypair`, `_keypair_derand`, `_encaps`, `_encaps_derand` and `_decaps`), `src/common/common.h` (`OQS_STATUS`) and `docs/algorithms/kem/ml_kem.yml`; source `src/kem/ml_kem/kem_ml_kem_768.c`.

https://github.com/open-quantum-safe/liboqs/tree/0.16.0

<a id="s7"></a>

### [S7] OpenSSL 3.5.9

OpenSSL 3.5.9 (29 Sep 2026), 3.5 long-term support (LTS) branch. Manual pages EVP_PKEY_new(3), EVP_PKEY_derive(3), EVP_DigestInit(3), EVP_PKEY_encapsulate(3), EVP_PKEY-ML-KEM(7) and EVP_KEM-ML-KEM(7).

https://docs.openssl.org/3.5/

<a id="s8"></a>

### [S8] wolfSSL 5.9.4

wolfSSL 5.9.4 (25 Sep 2026): `wolfssl/wolfcrypt/wc_mlkem.h`.

https://github.com/wolfSSL/wolfssl/tree/v5.9.4-stable

## Coding standards and analysis tools

<a id="s9"></a>

### [S9] MISRA C editions

MISRA (originally the Motor Industry Software Reliability Association), *MISRA C* page: MISRA C:2012 (published 18 Mar 2013; revised Feb 2019, consolidating Amendment 1 and Technical Corrigendum 1), MISRA C:2023 (Apr 2023, consolidating later work and covering C11 and C18) and MISRA C:2025 (Mar 2025, the current edition). The guideline documents are sold by MISRA and are not reproduced in this project.

https://misra.org.uk/misra-c/

<a id="s10"></a>

### [S10] MISRA C:2012 Amendment 2

MISRA forum, *MISRA C:2012 AMD2 and MISRA Compliance:2020 available*, 25 Feb 2020. Amendment 2 brings C11 into scope; free download.

https://forum.misra.org.uk/showthread.php?tid=1529

<a id="s11"></a>

### [S11] MISRA Compliance:2020

MISRA, *MISRA Compliance:2020: Achieving compliance with MISRA Coding Guidelines*, Feb 2020, free (section 4, deviations and deviation records; section 5, guideline categories Mandatory, Required and Advisory; section 6, adopted code).

https://misra.org.uk/app/uploads/2021/06/MISRA-Compliance-2020.pdf

<a id="s12"></a>

### [S12] MISRA C:2012 headlines for cppcheck

MISRA, *MISRA C 2012 Tools* repository (GitLab), file `misra_c_2012__headlines_for_cppcheck - AMD1+AMD2.txt`: free guideline headlines (17 directives, 158 rules) for use with cppcheck, licensed Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0). Downloaded locally; not committed to this repository.

https://gitlab.com/MISRA/MISRA-C/MISRA-C-2012/tools

<a id="s13"></a>

### [S13] cppcheck 2.22.0 and its MISRA add-on

cppcheck 2.22.0 (19 Sep 2026), GNU General Public License v3.0. Manual section on the `misra.py` add-on (rule texts loaded with `--rule-texts` from a `misra.json` add-on file; open-source checkers cover the MISRA rules only partially) and `addons/misra.py` ("MISRA C 2012 checkers (including amendment 1 and 2)").

https://github.com/cppcheck-opensource/cppcheck/blob/2.22.0/man/manual.md

<a id="s14"></a>

### [S14] clang-tidy checks list

LLVM, *Clang-Tidy Checks* list, including the cert-* checks and the checks they alias.

https://clang.llvm.org/extra/clang-tidy/checks/list.html

<a id="s15"></a>

### [S15] SEI CERT C Coding Standard

Software Engineering Institute (SEI), Carnegie Mellon University, *SEI CERT C Coding Standard* (rules and recommendations for secure coding in C).

https://wiki.sei.cmu.edu/confluence/display/c/SEI+CERT+C+Coding+Standard

## Hardware, language and amendment sources

<a id="s16"></a>

### [S16] wolfSSL 5.9.4 `types.h`

wolfSSL 5.9.4, `wolfssl/wolfcrypt/types.h`, lines 1866-1867: `INVALID_DEVID` is defined as `(-2)`.

https://github.com/wolfSSL/wolfssl/blob/v5.9.4-stable/wolfssl/wolfcrypt/types.h

<a id="s17"></a>

### [S17] RP2350 datasheet

Raspberry Pi Ltd, *RP2350 Datasheet* (build of 19 Nov 2026), section 4.2 "SRAM", page 312: 520 kB (520 × 1024 bytes) of on-chip static RAM (SRAM).

https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf

<a id="s18"></a>

### [S18] MISRA C:2012 Amendment 2 document

MISRA, *MISRA C:2012 Amendment 2: Updates for ISO/IEC 9899:2011 Core functionality*, Feb 2020, free download. Adds Rule 1.4 (Required), which covers C11 Annex K (bounds-checking interfaces). Cited by rule number only.

https://misra.org.uk/app/uploads/2021/06/MISRA-C-2012-AMD2.pdf

<a id="s19"></a>

### [S19] C11 committee draft N1570

ISO/IEC JTC1/SC22/WG14 (the C standards committee), N1570, committee draft of ISO/IEC 9899:2011 (C11), 12 Apr 2011: 6.3.1.1 (integer promotions), 6.3.1.3 (signed conversions), 6.5 paragraph 5 (overflow), 6.5.7 (shift operators), 7.24.2.1 (`memcpy`), 7.24.2.2 (`memmove`), K.3.7.4.1 (`memset_s`).

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf
