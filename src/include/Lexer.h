enum Token {
    tokEof = -1,

    tokDef = -2,
    tokExtern = -3,

    tokIdentifier = -4,
    tokNumber = 5
};

static std::string identifierStr;
static double numVal;

static int getTok() {
    static int lastChar = ' ';

    while (isspace(lastChar)) {
        lastChar = getchar();
    }

    if (isalpha(lastChar)) {
        identifierStr = lastChar;
        while (isalnum(lastChar = getchar())) {
            identifierStr += lastChar;
        }

        if (identifierStr == "def") {
            return tokDef;
        }
        if (identifierStr == "extern") {
            return tokExtern;
        }
        return tokIdentifier;
    }

    if (isdigit(lastChar) || lastChar == '.') {
        std::string numStr;
        do {
            numStr += lastChar;
            lastChar = getchar();
        } while (isdigit(lastChar) || lastChar == '.');
        numVal = strtod(numStr.c_str(), nullptr);
        return tokNumber;
    }

    if (lastChar == '#') {
        do {
            lastChar = getchar();
        } while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

        if (lastChar != EOF) {
            return getTok();
        }
    }

    if (lastChar == EOF) {
        return tokEof;
    }

    int thisChar = lastChar;
    lastChar = getchar();
    return thisChar;
}