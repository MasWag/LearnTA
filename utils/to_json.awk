#!/usr/bin/awk -f

BEGIN {
    if (ARGC == 1) {
        print "Usage: ./to_json.awk [json file] ([json file]...)"
        ERROR = 1
        exit
    } else {
        for (i = 1; i <= ARGC; i++) {
            for (j = i + 1; j <= ARGC; j++) {
                if (ARGV[j] == ARGV[i]) {
                    print "Error: duplicated inputs"
                    ERROR = 1
                    exit
                }
            }
        }
    }
}
BEGIN {
    print "{"
}
# A hack. In GNU AWK, we can do a similar thing with ENDFILE.
FNR == 1 && FILENAME != ARGV[1] {
    print "},"
}
FNR == 1 {
    name = FILENAME
    sub(".*/", "", name)
    sub(".log", "", name)
    print "\""name"\":{"
}
/Number of symbolic membership queries \(with cache\):/ {
    print "\"symbolic_membership_queries\":"$NF","
}
/Number of membership queries \(with cache\):/ {
    print "\"membership_queries\":"$NF","
}
/Number of equivalence queries \(with cache\):/ {
    print "\"equivalence_queries\":"$NF","
}
/Execution Time:/ {
    print "\"execution_time\":"$(NF - 1)
}
END {
    if (!ERROR) {
        print "}"
        print "}"
    }
}

