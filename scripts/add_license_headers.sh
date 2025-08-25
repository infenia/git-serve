#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# Copyright 2025 Infenia Private Limited

# Add Apache 2.0 license header to all C++ source files

LICENSE_HEADER="/*
 * Copyright 2025 Infenia Private Limited
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an \"AS IS\" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
"

echo "Adding license headers to C++ source files..."

# Find all C++ files and add the license header if it's not already there
find src -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) -print0 | while IFS= read -r -d $'\0' file; do
    if ! grep -q "Copyright .* Infenia Private Limited" "$file"; then
        echo "Adding license header to $file"
        # Use a temporary file to store the new content
        (echo -e "$LICENSE_HEADER"; cat "$file") > "${file}.new"
        mv "${file}.new" "$file"
    fi
done

echo "Done adding license headers."
