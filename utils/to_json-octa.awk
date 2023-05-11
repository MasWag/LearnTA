#!/usr/bin/awk -f

BEGIN {
    if (ARGC == 1) {
        print "Usage: ./to_json-octa.awk [json file] ([json file]...)"
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
    name = $NF
    sub(".*/", "", name)
    sub(".log", "", name)
    print "\""name"\":{"
}
/Total number of membership query:/ {
    print "\"membership_queries\":"$NF","
}
/Total number of equivalence query:/ {
    print "\"equivalence_queries\":"$NF","
}
/Total time of learning:/ {
    # We use ms rather than sec
    print "\"execution_time\":"$NF * 1000.0
}
/transitions/ {
    started = 1
    M = 0
}
started && NF == 5 {
    sub("\\[", "", $4)
    sub("\\(", "", $4)
    split($4, arr, ",")
    if (M < arr[1]) {
        M = arr[1]
    }
    arr[2] *= 1
    if (M < arr[2]) {
        M = arr[2]
    }
}
/init state:/ {
    started=0
    print "\"max_constant\":"M","
}
END {
    if (!ERROR) {
        print "}"
        print "}"
    }
}
