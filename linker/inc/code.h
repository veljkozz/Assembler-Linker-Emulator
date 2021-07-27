
#include <vector>
#include <string>
#include <map>
using namespace std;
struct Place{
    string secName;
    int addr;
};

enum Type{ NO_TYPE=0, TOBJECT, TSECTION};

struct Symbol{
    string name;
    short val;
    string section;
    bool isGlobal;
    int size;
    Type tip;
    string file;
};

struct Section{
    string name;
    vector<char> code;
    int flags;      // not used anywhere yet
    string file;
    int size;
    int start;
};

struct Realloc{
    string symbol;
    int offs;
    int type;
};

enum LinkOpt{ NO_OPTION, HEX, LINKABLE};

class Code{
private:
    vector<string> infiles;
    const char* fout;
    LinkOpt option;
    vector<Place> places;
    map<string, Symbol> symTable;
    map<string, Section> sectionTable;
    map<string, map<string,Symbol>> fileSymTab;
    map<string, map<string, vector<Realloc>>> relsPerFile;
    //    sectionname    filename
    map<string, vector<Section>> sectionsPerFile;
    map<string, vector<Realloc>> rels;
    vector<char> code;
public:
    Code(vector<string> infiles,const char* fout, LinkOpt option, vector<Place> places);

    void loadFiles();
    void checkErrors();
    void merge();
    void relloc();
    void rellocHex();
    void rellocLink();
    void printSymTab();
    void printRel();
    void showCode(vector<char> code);
    void output();
};