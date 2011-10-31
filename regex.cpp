#include <vector>
#include <string>
#include <iostream>
#include <pcrecpp.h>

#include "lgram.h"
#include "error.h"
#include "output.h"
#include "options.h"

struct regl {
    pcrecpp::RE *regex;
    pcrecpp::StringPiece *replace;
};

std::vector<struct regl> regexps;

void mand_regex_init() {
    char b[BUFSIZ];
    for(int i = 0; opts.boundary[i] != '\0'; i++) {
        struct regl rl;
        snprintf(b, BUFSIZ, "\\s+\\%c", opts.boundary[i]);
        rl.regex = new pcrecpp::RE(b);
        snprintf(b, BUFSIZ, "\\%c", opts.boundary[i]);
        rl.replace = new pcrecpp::StringPiece(strdup(b));
        regexps.push_back(rl);
    }
}

/* Each each line from the regex file and store regl structures
 * int regexps for regex_replace */
int regex_init(char *fname) {
    char buf[BUFSIZ];
    FILE *fp = NULL;
    size_t sz;
    unsigned long l = 0;
    char *parsed = NULL;

    if((fp = fopen(fname, "r")) == NULL) {
        setlocation(LOCATION, __func__);
        return RERROR;
    }
 

    while((fgets(buf, BUFSIZ, fp)) != NULL)  {
        sz = strlen(buf);
        if(buf[sz-1] != '\n') {
            lgprintf(LG_WARN, "regex input might be too large "
                              "or incorrectly formatted\n");
            lgprintf(LG_WARN, "%s", buf);
        } else {
            buf[sz-1] = '\0';
        }

        // ignore comments
        if(buf[0] == '#')
            continue;
        l++;
        struct regl rl;

        // parse out regular expression
        parsed = strtok(buf, "\t");
        if(parsed == NULL) {
            lgprintf(LG_WARN, "%s is not tab seperated\n",fname);
            return RERROR;
        }

        rl.regex = new pcrecpp::RE(parsed);

        // parse out replacement string
        parsed = strtok(NULL, "\t");
        if(parsed != NULL) 
            rl.replace = new pcrecpp::StringPiece(strdup(parsed));
        else
            // assume replacement is blank
            rl.replace = new pcrecpp::StringPiece("");

        regexps.push_back(rl);
    }

    return fclose(fp);
}

void regex_replace(std::string *s) {
    std::vector<struct regl>::const_iterator it;
    for(it = regexps.begin(); it != regexps.end(); it++) 
        it->regex->GlobalReplace(*it->replace,s);
}

char *regex_creplace(char *s) {
    std::string cpp_s(s);
    regex_replace(&cpp_s);
    return strdup(cpp_s.c_str());
}

void regex_deinit() {
    std::vector<struct regl>::const_iterator it;
    for(it = regexps.begin(); it != regexps.end(); it++) {
        delete it->regex;
        delete it->replace;
    }     
}

