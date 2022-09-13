enum Token {
    tokEof = -1,

    tokDef = -2,
    tokExtern = -3,

    tokIdentifier = -4,
    tokNumber = -5,

    tokIf = -6,
    tokThen = -7,
    tokElse = -8,

    tokFor = -9,
    tokIn = -10,

    tokBinary = -11,
    tokUnary = -12,

    tokVar = -14,

    tokWhile = -15
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
        if (identifierStr == "if") {
            return tokIf;
        }
        if (identifierStr == "then") {
            return tokThen;
        }
        if (identifierStr == "else") {
            return tokElse;
        }
        if (identifierStr == "for") {
            return tokFor;
        }
        if (identifierStr == "in") {
            return tokIn;
        }
        if (identifierStr == "binary") {
            return tokBinary;
        }
        if (identifierStr == "unary") {
            return tokUnary;
        }
        if (identifierStr == "var") {
            return tokVar;
        }
        if (identifierStr == "while") {
            return tokWhile;
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