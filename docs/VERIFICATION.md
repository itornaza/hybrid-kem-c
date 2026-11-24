# hybrid-kem-c: Verification Plan and Traceability

Version 0.1 (draft), November 2026. Author: ion-.

This plan states how each requirement in [SRS.md](SRS.md) (System Requirements Specification) is verified. Together with [SRS.md](SRS.md) it is the project's requirements list; evidence and status are updated at each milestone review (MR).

## 1. Verification methods

| Code | Method | Meaning |
|---|---|---|
| T | Test | Run code with defined inputs and check the outputs automatically. |
| A | Analysis | Measure or calculate (benchmarks, stack and code size, memory budgets) and check the result. |
| I | Inspection | Review code, headers or documents against the requirement. |
| D | Demonstration | Run the system as a user would and observe the result. |

## 2. Test environment

- Build: CMake, `-std=c11 -Wall -Wextra -Wpedantic -Werror`; debug builds add AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan).
- Pinned dependencies: liboqs (Open Quantum Safe) 0.16.0 ([S6](SOURCES.md#s6)), OpenSSL 3.5.9 ([S7](SOURCES.md#s7)), wolfSSL 5.9.4 ([S8](SOURCES.md#s8)), X-Wing draft -11 ([S1](SOURCES.md#s1)) (see [ICD.md](ICD.md) (Interface Control Document) section 6). [README.md](../README.md) records the host central processing unit (CPU), operating system (OS) and compiler versions.
- Static analysis tool versions (cppcheck and clang-tidy) are recorded in [README.md](../README.md) at each milestone review, like the other pins.

### 2.1 Static analysis and coding-standard checks

All tools are free and open source; no paid tool or paid rule text is used. Scope, the inspection checklist and the deviation log are in [CODING-STANDARD.md](CODING-STANDARD.md).

| Check                              | Standard                                                                                                                                                   | Tool and how it runs                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           | Used for                                                                                                 |
| ---------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| MISRA C static analysis            | MISRA C:2012 with Amendments 1 and 2 ([S9](SOURCES.md#s9), [S10](SOURCES.md#s10)) (MISRA = originally the Motor Industry Software Reliability Association) | cppcheck ([S13](SOURCES.md#s13)) with its open-source MISRA add-on `misra.py`. A `misra.json` add-on file passes `--rule-texts=` the local path of MISRA's free headline file `misra_c_2012__headlines_for_cppcheck - AMD1+AMD2.txt` ([S12](SOURCES.md#s12)); run as `cppcheck --std=c11 --addon=misra.json --enable=style <in-scope files>` ([S13](SOURCES.md#s13)) through the CMake `misra` target. `--std=c11` is needed because cppcheck 2.22.0 assumes C23 by default (manual, section "C/C++ standard") | QUAL-06 (A), QUAL-07                                                                                     |
| Inspection of unchecked guidelines | Same                                                                                                                                                       | Checklist review of the guidelines the add-on does not check ([CODING-STANDARD.md](CODING-STANDARD.md) section 3)                                                                                                                                                                                                                                                                                                                                                                                              | QUAL-06 (I)                                                                                              |
| Deviation review                   | Same                                                                                                                                                       | Every deviation-log entry reviewed and approved or rejected at the milestone review ([CODING-STANDARD.md](CODING-STANDARD.md) section 4)                                                                                                                                                                                                                                                                                                                                                                       | QUAL-06 (I)                                                                                              |
| SEI CERT C checks                  | SEI (Software Engineering Institute) CERT C Coding Standard ([S15](SOURCES.md#s15))                                                                        | clang-tidy ([S14](SOURCES.md#s14)) with `--checks='-*,cert-*'` on all project C code, tests included, through the CMake `tidy` target                                                                                                                                                                                                                                                                                                                                                                          | QUAL-07; supporting check; findings fixed or noted in [CODING-STANDARD.md](CODING-STANDARD.md) section 5 |
| Dynamic checks                     | None (run-time checks)                                                                                                                                     | AddressSanitizer (ASan), LeakSanitizer and UndefinedBehaviorSanitizer (UBSan) in debug builds                                                                                                                                                                                                                                                                                                                                                                                                                  | SEC-04, QUAL-01                                                                                          |

Notes:

- MISRA C and SEI CERT C are different standards with different goals. MISRA C is a set of guidelines for the use of C in critical systems ([S9](SOURCES.md#s9)); SEI CERT C is a secure-coding standard of rules and recommendations ([S15](SOURCES.md#s15)). They are not equivalent, and passing one check is not conformance to the other standard.
- clang-tidy's cert-* checks cover only a small subset of CERT C: its check list ([S14](SOURCES.md#s14)) has 22 cert-*-c checks, all listed as aliases of other clang-tidy checks. The project makes no CERT C conformance claim.
- cppcheck 2.22.0's add-on covers MISRA C:2012 with Amendments 1 and 2 but checks no directives; all 17 directives are on the manual checklist. MISRA C:2025 is now the latest edition; the project does not switch to it ([SRS.md](SRS.md) section 4).
- cppcheck's manual states that the open-source MISRA checkers cover the rules only partially ([S13](SOURCES.md#s13)). Method used for the count: the cppcheck 2.22.0 `addons/misra.py` source was read on 24 Nov 2026. It has active (not commented-out) add-on checks for 130 rules, and its `getCppcheckRules()` list maps 25 more rules to cppcheck core checks, so 155 of the 158 rules in the headline file have some check. Rule 1.1, Rule 3.2 (its check call is commented out and there is no check function) and Rule 12.5 have none. A mapped check can still miss violations. The gaps are covered by inspection ([CODING-STANDARD.md](CODING-STANDARD.md) section 3) and tracked as RK-11 in [RISKS.md](RISKS.md).
- The headline file is MISRA copyright under a Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0) licence ([S12](SOURCES.md#s12)). Each developer downloads it; it is not committed to this MIT-licensed repository, and its texts are not copied into these documents. The documents cite rule numbers only (QUAL-08).

## 3. Test-vector sources

- **ACVP**: the National Institute of Standards and Technology (NIST) Automated Cryptographic Validation Protocol (ACVP) ML-KEM (Module-Lattice-Based Key-Encapsulation Mechanism) keyGen and encapDecap JSON (JavaScript Object Notation) files for FIPS 203 (Federal Information Processing Standard 203), from [S5](SOURCES.md#s5). Used for EDU-03 and EDU-04.
- **X-Wing -11 Appendix C**: three vectors (seed, sk, pk, eseed, ct, ss) in draft-connolly-cfrg-xwing-kem-11 ([S1](SOURCES.md#s1)). The draft marks them "TODO: replace" (they are to be replaced by vectors that reuse ML-KEM and X25519 values); they are usable for -11 and are re-checked if the pinned revision changes. Used for FR-10.
- **Cross-implementation checks**: liboqs ([S6](SOURCES.md#s6)), OpenSSL 3.5 native ML-KEM ([S7](SOURCES.md#s7)) and wolfSSL ML-KEM ([S8](SOURCES.md#s8)), used where no published vector applies (FR-04, EDU-05 to EDU-07).

## 4. Traceability matrix

All statuses are Planned until evidence is recorded at the milestone review.

| ID | Short title | Milestone | Method | Planned test or evidence | Status |
|---|---|---|---|---|---|
| FR-01 | liboqs ML-KEM-768 round trip | M0 | T | Round-trip test output | Planned |
| FR-02 | Shared-secret compare, non-zero exit | M0 | T | Test with a forced mismatch gives non-zero exit | Planned |
| FR-03 | Tampered ciphertext, implicit rejection | M0 | T | Tampered-ciphertext test: different secret, no error | Planned |
| FR-04 | liboqs and OpenSSL ML-KEM interop | M0 | T | Interop test output, both directions | Planned |
| FR-05 | X-Wing per draft -11 | M1 | T | Appendix C vectors and round-trip tests | Planned |
| FR-06 | 32 B seed keygen with SHAKE256 expansion | M1 | T | Keygen test against Appendix C seed, sk, pk | Planned |
| FR-07 | Decaps re-derives keys from seed | M1 | T | Decaps test using only the 32 B seed | Planned |
| FR-08 | Derandomized keygen | M1 | T | Same seed gives same pk; sk equals seed | Planned |
| FR-09 | Derandomized encaps with 64 B eseed | M1 | T | Appendix C eseed gives expected ct and ss | Planned |
| FR-10 | Match draft Appendix C vectors | M1 | T | All three -11 vectors pass | Planned |
| FR-11 | (Optional) Board key exchange over Universal Serial Bus (USB) serial | M6 | D | Host-to-board exchange with matching secrets | Planned |
| IF-01 | API functions exposed | M1 | I | Header review against [ICD.md](ICD.md) section 2 | Planned |
| IF-02 | Fixed-size buffers, static asserts | M1 | I | Header and .c review; build fails if a size is wrong | Planned |
| IF-03 | Distinct negative return codes | M1 | T | Fault-injection tests for each error code | Planned |
| IF-04 | Implicit rejection; small-order ct_X rejected | M1 | T | Tampered ct_M test; random ct_X tamper test | Planned |
| IF-05 | Byte layout and order | M1 | T | Vector test and split-offset checks | Planned |
| IF-06 | (Optional) Serial protocol | M6 | T | Protocol test against [ICD.md](ICD.md) section 7 | Planned |
| IF-07 | All-zero ct_X and pk_X give `HKEM_ERR_X25519` | M1 | T | Tests with 32 zero bytes for ct_X and pk_X | Planned |
| IF-08 | `hkem_keygen_derand` with aliased `sk` and `seed` | M1 | T | Tests with `sk == seed` and with partly overlapping buffers; outputs equal the non-aliased call | Planned |
| SEC-01 | M0 wipes and frees | M0 | I | Code review; ASan/LeakSanitizer clean run | Planned |
| SEC-02 | Wipe all secrets on every path | M1 | I | Code review of every exit path; cleanse calls only | Planned |
| SEC-03 | Check every return code | M1 | I | Code review; liboqs results tested as `rc != OQS_SUCCESS` | Planned |
| SEC-04 | Sanitizers clean | M1 | T | Test suite under ASan, LeakSanitizer, UBSan | Planned |
| SEC-05 | Threat model page | M1 | I | [README.md](../README.md) threat model review against draft sections 1.2, 6 and 6.1 | Planned |
| SEC-06 | Binding stated as draft claim | M1 | I | [README.md](../README.md) wording review | Planned |
| SEC-07 | Educational code labelled | M2 | I | Educational README review | Planned |
| SEC-08 | Constant-time status recorded | M1 | I | [README.md](../README.md) table per primitive with sources; glue-code review | Planned |
| SEC-09 | Approved wipe functions and lengths | M1 (M2, M6) | I | Review and search of in-scope code for Annex K functions, `explicit_bzero`, `memset` wipes and `sizeof` on array parameters; the M6 volatile wipe helper reviewed at MR6 | Planned |
| QUAL-01 | Strict C11 build with CMake | M0 | T | Clean build log with all flags | Planned |
| QUAL-02 | Versions pinned | M0 | I | [README.md](../README.md) version table at each review | Planned |
| QUAL-03 | M0 size check | M0 | T | M0 output matches FIPS 203 sizes | Planned |
| PERF-01 | Benchmark median of 1000 or more runs | M1 | A | Benchmark table with method | Planned |
| PERF-02 | Benchmark context stated | M1 | I | Benchmark table review | Planned |
| PERF-03 | Educational stack and code size | M2 | A | Stack high-water and `size` table | Planned |
| QUAL-04 | Demo under 2 minutes | M4 | D | Timed clean-checkout run | Planned |
| QUAL-05 | (Optional) RP2350-class board | M6 | A | Memory budget versus board static RAM (SRAM) | Planned |
| PERF-04 | (Optional) Board timing, stack, code size | M6 | A | Board measurement table | Planned |
| QUAL-06 | MISRA C:2012 (Amendments 1 and 2): findings fixed or deviated | M1 | A, I | `misra` target output and cppcheck checkers report; inspection checklist done; deviation log reviewed. Library at MR1, educational code at MR2, board code at MR6 | Planned |
| QUAL-07 | `misra` and `tidy` targets clean; tool versions pinned | M1 | T | Clean run log of both targets (exit code 0); [README.md](../README.md) pin table | Planned |
| QUAL-08 | No MISRA rule texts in the repository | M1 | I | Search of the tracked files (`git ls-files`) for the headline file name; document review for rule texts | Planned |
| EDU-01 | FIPS 203 ML-KEM-768 with input checks | M2 | T | ACVP runs plus invalid-key tests (sections 7.2, 7.3) | Planned |
| EDU-02 | Seeded entry points | M2 | T | Seeded ACVP runs | Planned |
| EDU-03 | ACVP keyGen vectors | M2 | T | All keyGen vectors pass | Planned |
| EDU-04 | ACVP encapDecap vectors | M2 | T | All encapDecap vectors pass | Planned |
| EDU-05 | Interop with liboqs | M2 | T | Interop test, both directions | Planned |
| EDU-06 | Interop with OpenSSL 3.5 | M2 | T | Interop test, both directions | Planned |
| EDU-07 | Interop with wolfSSL | M2 | T | Interop test, both directions | Planned |
| EDU-08 | No dynamic allocation in lattice code | M2 | I | Code review; no allocator symbols in lattice objects | Planned |

Coverage: 48 of 48 [SRS.md](SRS.md) requirements traced.

## 5. Milestone reviews, MR0 to MR6 (MR6 only if M6 goes ahead)

Each review checks the milestone's requirements against evidence, updates the Status column above and the risk register in [RISKS.md](RISKS.md), and re-records the version pins.

| Review | Entry criteria | Exit criteria |
|---|---|---|
| MR0 | M0 program builds; [SRS.md](SRS.md) and [ICD.md](ICD.md) drafts in the repo | FR-01 to FR-04, SEC-01, QUAL-01 to QUAL-03 verified; ASan/UBSan clean |
| MR1 | X-Wing library and tests complete | All M1 requirements verified, including IF-08, SEC-09 and QUAL-06 to QUAL-08 for the library; Appendix C vectors pass; draft revision re-checked on the datatracker ([S1](SOURCES.md#s1)) |
| MR2 | Educational ML-KEM-768 complete | All EDU requirements, SEC-07, PERF-03, and QUAL-06, QUAL-07 and SEC-09 (educational code) verified; three-way interop passes |
| MR3 | Tutorial lab written | Every lab command re-run from a clean checkout matches the shown output |
| MR4 | Demonstration program ready | QUAL-04 demonstrated from a clean checkout |
| MR5 | Upstream pull request opened | Review comments answered; outcome recorded |
| MR6 | (Optional) Board build runs, after MR2 | FR-11, IF-06, QUAL-05, PERF-04, and QUAL-06 and SEC-09 (board code, including the volatile wipe helper) verified; board measurements in [README.md](../README.md) |
