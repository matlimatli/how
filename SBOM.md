# Software Bill of Materials (SBOM)

All third-party dependencies used by **how**.

| Component | Version | Source | License | Usage |
|-----------|---------|--------|---------|-------|
| libcurl | System | [curl.se](https://curl.se/) | MIT/X derivative | HTTP requests to LLM APIs |
| nlohmann/json | 3.12.0 | [GitHub](https://github.com/nlohmann/json) | MIT | JSON parsing for API payloads and history |
| Google Test | 1.17.0 | [GitHub](https://github.com/google/googletest) | BSD-3-Clause | Unit testing (build-time only) |

## Fetched at build time

`nlohmann/json` and `googletest` are fetched via CMake `FetchContent` during the build. They are not vendored in the repository. The exact versions are pinned by git tag in `CMakeLists.txt`.

## System dependencies

`libcurl` is expected to be installed on the build system. It is found via CMake's `find_package(CURL REQUIRED)`.
