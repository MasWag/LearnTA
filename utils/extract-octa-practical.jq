#!/usr/bin/env jq -f

[
    to_entries[] |
        select(.key | contains("/") | not) # We assume that the practical examples do not contain /
] |
    from_entries
