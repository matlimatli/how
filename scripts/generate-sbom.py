#!/usr/bin/env python3
"""Generate an SPDX 2.3 SBOM for the how project."""

import json
import uuid
from datetime import datetime, timezone

sbom = {
    "spdxVersion": "SPDX-2.3",
    "dataLicense": "CC0-1.0",
    "SPDXID": "SPDXRef-DOCUMENT",
    "name": "how",
    "documentNamespace": f"https://github.com/matlimatli/how/spdx/{uuid.uuid4()}",
    "creationInfo": {
        "created": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "creators": ["Tool: scripts/generate-sbom.py"],
        "licenseListVersion": "3.25",
    },
    "packages": [
        {
            "SPDXID": "SPDXRef-Package-how",
            "name": "how",
            "versionInfo": "1.0.0",
            "supplier": "Person: Mattias Lindblad",
            "downloadLocation": "https://github.com/matlimatli/how",
            "filesAnalyzed": False,
            "licenseConcluded": "MIT",
            "licenseDeclared": "MIT",
            "copyrightText": "Copyright (c) 2026 Mattias Lindblad",
            "primaryPackagePurpose": "APPLICATION",
        },
        {
            "SPDXID": "SPDXRef-Package-libcurl",
            "name": "libcurl",
            "versionInfo": "system",
            "supplier": "Organization: curl project",
            "downloadLocation": "https://curl.se/",
            "filesAnalyzed": False,
            "licenseConcluded": "curl",
            "licenseDeclared": "curl",
            "copyrightText": "Copyright (c) Daniel Stenberg",
            "externalRefs": [
                {
                    "referenceCategory": "PACKAGE-MANAGER",
                    "referenceType": "purl",
                    "referenceLocator": "pkg:generic/libcurl",
                }
            ],
        },
        {
            "SPDXID": "SPDXRef-Package-nlohmann-json",
            "name": "nlohmann-json",
            "versionInfo": "3.12.0",
            "supplier": "Person: Niels Lohmann",
            "downloadLocation": "https://github.com/nlohmann/json/archive/refs/tags/v3.12.0.tar.gz",
            "filesAnalyzed": False,
            "licenseConcluded": "MIT",
            "licenseDeclared": "MIT",
            "copyrightText": "Copyright (c) 2013-2025 Niels Lohmann",
            "externalRefs": [
                {
                    "referenceCategory": "PACKAGE-MANAGER",
                    "referenceType": "purl",
                    "referenceLocator": "pkg:github/nlohmann/json@v3.12.0",
                }
            ],
        },
        {
            "SPDXID": "SPDXRef-Package-googletest",
            "name": "googletest",
            "versionInfo": "1.17.0",
            "supplier": "Organization: Google LLC",
            "downloadLocation": "https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz",
            "filesAnalyzed": False,
            "licenseConcluded": "BSD-3-Clause",
            "licenseDeclared": "BSD-3-Clause",
            "copyrightText": "Copyright 2008 Google Inc.",
            "comment": "Build-time only dependency (unit tests).",
            "externalRefs": [
                {
                    "referenceCategory": "PACKAGE-MANAGER",
                    "referenceType": "purl",
                    "referenceLocator": "pkg:github/google/googletest@v1.17.0",
                }
            ],
        },
    ],
    "relationships": [
        {
            "spdxElementId": "SPDXRef-DOCUMENT",
            "relatedSpdxElement": "SPDXRef-Package-how",
            "relationshipType": "DESCRIBES",
        },
        {
            "spdxElementId": "SPDXRef-Package-how",
            "relatedSpdxElement": "SPDXRef-Package-libcurl",
            "relationshipType": "DEPENDS_ON",
        },
        {
            "spdxElementId": "SPDXRef-Package-how",
            "relatedSpdxElement": "SPDXRef-Package-nlohmann-json",
            "relationshipType": "DEPENDS_ON",
        },
        {
            "spdxElementId": "SPDXRef-Package-how",
            "relatedSpdxElement": "SPDXRef-Package-googletest",
            "relationshipType": "DEV_DEPENDENCY_OF",
        },
    ],
}

print(json.dumps(sbom, indent=2))
