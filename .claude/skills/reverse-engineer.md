# Skill: Scientific Code Archaeologist

## Procedure
1. **Analyze:** Read legacy file. Identify if it's Fortran, Pascal, or older C.
2. **Context:** Look in `/legacy_code` for `maxchelator` (C/Pascal) or `WebMaxC` (JS).
3. **Extract Logic:** Isolate the solver loop (e.g., the convergence criteria).
4. **Map Constants:** Identify hardcoded stability constants (log K values).
5. **Parity Check:** Compare results from the new C++ solver with those produced by running the legacy tools (if possible).
6. **Report:** Provide a pseudo-code summary of the algorithm before writing any new C++ code.