# hybrid-kem-c: Coding Standard and Deviation Log

Version 0.1 (draft), November 2026. Author: ion-.

This document supports requirements QUAL-06 to QUAL-08 and SEC-09 in [SRS.md](SRS.md) (System Requirements Specification). It sets out the coding standard, its scope, the checks the tools do not cover, and the deviation log. How the checks are run is in [VERIFICATION.md](VERIFICATION.md) section 2.1; sources are in [SOURCES.md](SOURCES.md).

## 1. Standard

- **MISRA C:2012 as amended by Amendments 1 and 2** ([S9](SOURCES.md#s9), [S10](SOURCES.md#s10)). MISRA was originally the Motor Industry Software Reliability Association. Amendment 2 brings C11 into scope, which matches the project's C11 build (QUAL-01).
- **Tool coverage**: cppcheck 2.22.0's open-source add-on covers MISRA C:2012 with Amendments 1 and 2, but it checks no directives, so all directives are on the manual checklist (section 3). MISRA C:2025 is now the latest edition ([S9](SOURCES.md#s9)); the project does not switch to it.
- **Compliance framework**: MISRA Compliance:2020 ([S11](SOURCES.md#s11)). Its guideline categories are used as published:
  - **Mandatory**: never violated. A Mandatory guideline cannot be deviated; any finding is fixed.
  - **Required**: violated only with an approved deviation record.
  - **Advisory**: followed as far as practical. MISRA does not require a deviation for an Advisory violation; this project still logs each one as a short entry so that the log is complete.
- No guideline is re-categorized at present.
- **Rule texts** (QUAL-08): the documents and code comments cite rule and directive numbers only. The free headline file ([S12](SOURCES.md#s12)) is downloaded by each developer for cppcheck and is not committed to the repository; the paid guideline texts are not used or reproduced.
- This is a coding standard the project follows, not a certified compliance claim. MISRA C conformance is a code-quality property, not a security or constant-time claim. It is not equivalent to the SEI (Software Engineering Institute) CERT C Coding Standard ([S15](SOURCES.md#s15)), which is checked separately and only in part (section 5).

## 2. Scope

| In scope | From |
|---|---|
| The library source files and the public header `hkem.h` (M1) | M1 |
| The educational ML-KEM-768 (Module-Lattice-Based Key-Encapsulation Mechanism) source files | M2 |
| (Optional) Code written for the M6 board port, excluding the board vendor's libraries | M6 |

Out of scope:

- liboqs (Open Quantum Safe), OpenSSL and wolfSSL source and headers. They are third-party ("adopted") code in the sense of MISRA Compliance:2020 section 6 ([S11](SOURCES.md#s11)); they are not analysed, and findings located in their headers are not counted.
- Board vendor libraries and headers (M6).
- Test, benchmark, demonstration and interop programs, and the M0 program. They use standard input/output and the libraries' allocating APIs for reporting. They are covered by the strict compiler flags, the sanitizers and the clang-tidy cert-* checks instead.

The educational ML-KEM-768 is in scope because learners read it as example C. Conformance does not make it constant-time or production-ready; it stays labelled "educational, not constant-time, not for production" (SEC-07).

## 3. Inspection checklist

### 3.1 Guidelines the tool does not check

The open-source cppcheck MISRA add-on covers the rules only partially ([S13](SOURCES.md#s13); risk RK-11 in [RISKS.md](RISKS.md)). In the cppcheck 2.22.0 source, 155 of the 158 rules have some check (130 in the add-on, 25 more mapped to cppcheck core checks); Rule 1.1, Rule 3.2 and Rule 12.5 have none, and none of the 17 directives is checked ([VERIFICATION.md](VERIFICATION.md) section 2.1 gives the method). At each milestone review (MR) that verifies QUAL-06, the in-scope code is also checked by:

| Item | Method |
|---|---|
| Directives Dir 1.1, Dir 2.1, Dir 3.1 and Dir 4.1 to Dir 4.14 (no add-on checks) | Inspection, one line per directive: met, not applicable, or deviation ID. Dir 1.1 is met by the record in section 4.3, Dir 2.1 is supported by the `-Werror` build (QUAL-01), Dir 3.1 by the traceability matrix and Dir 4.12 by DV-04 |
| Rule 1.1 (no check) | Strict C11 build with `-std=c11 -Wall -Wextra -Wpedantic -Werror` (QUAL-01) |
| Rule 3.2 (no check; its add-on call is commented out) | Inspection of `//` comments that end in a backslash |
| Rule 12.5, Mandatory (no check) | Inspection of every `sizeof` on a function parameter. An array parameter such as `uint8_t sk[32]` is a pointer, so wipes and copies use the `HKEM_*` constants ([ICD.md](ICD.md) section 2.2; SEC-09) |
| Spot checks where a mapped check may miss cases: rule groups 10 (essential types), 11 (pointer conversions), 17 (functions, including use of return values) and 21 and 22 (standard library and resources) | Inspection of the changed code |

### 3.2 Project rules from the crypto review

| Item | Rule |
|---|---|
| Overlapping copies (Rule 19.1, Mandatory) | `hkem_keygen_derand` copies with `memmove`, so `sk` and `seed` may alias (IF-08) |
| Annex K (Rule 1.4, Required, from Amendment 2 ([S18](SOURCES.md#s18))) | No `memset_s` or other C11 Annex K functions; `__STDC_WANT_LIB_EXT1__` stays undefined (SEC-09) |
| Secret wiping | `OQS_MEM_cleanse` or `OPENSSL_cleanse` on hosts; one reviewed volatile wipe helper only in M6 board code; never plain `memset` or `explicit_bzero` (SEC-09) |
| liboqs results | Tested as `rc != OQS_SUCCESS`, never `rc != 0` (SEC-03) |
| Integer promotion in M2 | `uint16_t * uint16_t` promotes both operands to `int` and can overflow, which is undefined behaviour (C11 6.3.1.1 and 6.5 paragraph 5, [S19](SOURCES.md#s19)). Cast the operands to `uint32_t` first |
| Shifts | Never left-shift a negative value: undefined behaviour (C11 6.5.7 paragraph 4, [S19](SOURCES.md#s19)); never deviated. Shift unsigned values |
| Rules 7.3, 12.1 and 12.2 | Always fixed, never deviated |
| Masks | `0U - x` masks are compliant as written; no deviation needed |
| Size constants | `HKEM_*` size constants carry a `U` suffix ([ICD.md](ICD.md) section 2) |

## 4. Deviations

These are expected findings drafted before any code exists, not tool output.

### 4.1 Deviation record format

| Field | Content |
|---|---|
| ID | DV-nn, also written as a `/* DV-nn */` comment at each location |
| Rule | MISRA C:2012 rule or directive number (number only, no rule text) |
| Category | Required or Advisory. Mandatory guidelines cannot be deviated |
| Location | File and function, or the list of tagged locations; one record may cover several locations of the same use case ([S11](SOURCES.md#s11) section 4.2) |
| Rationale | Why compliance is not practical here, or would make the code worse |
| Risk and mitigation | What could go wrong, and the precautions and checks that control it |
| Approval / reviewer | Reviewer role and the milestone review at which it was approved |
| Status | Proposed, Contingent (pre-written, not expected), Approved, Rejected or Withdrawn |

Required deviations need every field and approval at a milestone review. Advisory entries may be one line (ID, rule, location, reason, status).

### 4.2 Deviation log

| ID | Rule | Category | Location | Rationale | Risk and mitigation | Approval / reviewer | Status |
|---|---|---|---|---|---|---|---|
| DV-01 | Rule 2.2 | Required | Secret wiping on every exit path (SEC-02, SEC-09) | Not expected to be needed. A call to `OQS_MEM_cleanse` or `OPENSSL_cleanse` has a side effect, so it is not dead code. The record is kept ready only in case a cppcheck mapped check (for example an unread variable or redundant assignment ([S13](SOURCES.md#s13))) reports a wipe. | Risk: the compiler removes a wipe (RK-06). Mitigation: plain `memset` wipes are banned (SEC-09); a volatile wipe loop is allowed only in M6 board code, as one reviewed helper; every exit path checked in review | Crypto reviewer | Contingent, not expected |
| DV-02 | Rules 10.1, 10.3, 10.4, 10.6, 10.7 and 10.8, with a Dir 1.1 record (section 4.3) | Required | M2 educational ML-KEM-768 only: signed modular-reduction functions, as tagged sites. The library does no secret arithmetic (SEC-08) | Needed only if signed Montgomery or Barrett reduction is kept for closeness to FIPS 203 (Federal Information Processing Standard 203): signed narrowing, right shifts of signed values and mixed widths clash with the essential-type rules. With unsigned coefficients, DV-02 may not be needed at all. Rules 7.3, 12.1 and 12.2 are always fixed, a left shift of a negative value is never deviated, and `0U - x` masks are compliant as written (section 3.2). | Risk: implementation-defined behaviour or silent narrowing. Mitigation: Dir 1.1 record; exhaustive unit test of each reduction function over all `int16_t` inputs; NIST (National Institute of Standards and Technology) ACVP (Automated Cryptographic Validation Protocol) vectors and interop tests (EDU-03 to EDU-07); UBSan (UndefinedBehaviorSanitizer) runs (SEC-04). The educational code stays "not constant-time" (SEC-07) | Crypto reviewer, MR2 | Proposed; raised at MR2 only if signed reduction is kept |
| DV-03 | (Rules 11.3 and 11.8) | (Required) | Library boundary | Withdrawn: no casts are needed. liboqs takes `uint8_t` buffers and OpenSSL 3.5 takes `unsigned char` buffers, which `uint8_t` buffers pass directly (Dir 1.1 record, section 4.3). If a cast ever appears, Rules 11.3 and 11.8 apply; an 11.8 finding is fixed, not deviated. | n/a | Crypto reviewer | Withdrawn |
| DV-04 | Dir 4.12 | Required | Project level: the M1 library source files that create OpenSSL objects (`EVP_PKEY_new_raw_private_key_ex`, `EVP_PKEY_new_raw_public_key_ex`, `EVP_PKEY_CTX_new_from_pkey`, `EVP_MD_CTX_new`) | OpenSSL's EVP API allocates these objects inside OpenSSL ([S7](SOURCES.md#s7), EVP_PKEY_new(3)); X25519 and SHA-3 are used only through it. The ML-KEM-768 calls use liboqs's allocation-free per-algorithm functions ([ICD.md](ICD.md) section 6), so OpenSSL is the only source of dynamic memory. Our code never calls `malloc`, `calloc`, `realloc` or `free`, so no Rule 21.3 deviation is needed. | Risk: a leak or use after free. Mitigation: every `*_new` freed on every path in one cleanup block; AddressSanitizer (ASan) and LeakSanitizer runs (SEC-04); no allocation in M6 board code, and none in the educational lattice code (EDU-08) | Reviewers, MR1 | Proposed, to approve at MR1 |

### 4.3 Dir 1.1 record (implementation-defined behaviour relied on)

- `uint8_t` is `unsigned char`, so `uint8_t` buffers pass to OpenSSL's `unsigned char *` parameters without a cast.
- Only if DV-02 is raised (M2): right shift of a negative signed value (C11 6.5.7 paragraph 5) and conversion of an out-of-range value to a signed type (C11 6.3.1.3 paragraph 3) are implementation-defined ([S19](SOURCES.md#s19)). The assumed behaviour (arithmetic right shift; two's-complement wrap-around) is recorded together with the compiler versions at MR2.

## 5. SEI CERT C notes (clang-tidy cert-*)

The clang-tidy cert-* checks ([S14](SOURCES.md#s14)) cover a small subset of SEI CERT C and are run as a supporting check, separate from the MISRA log. Each finding is fixed, or listed here with a one-line decision. Listing a finding here is not a MISRA deviation, and running these checks is not a CERT C conformance claim.

| Check      | Location | Decision | Milestone review |
| ---------- | -------- | -------- | ---------------- |
| (none yet) |          |          |                  |
