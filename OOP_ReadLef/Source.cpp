#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

enum class SiteClass {
    undefined = 0,
    core,
    pad,
};

enum class MacroClass {
    undefined = 0,
    core,
    pad,
};

enum class PinDirection {
    undefined = 0,
    input,
    output,
    tristate,
    inout,
    feedthru,
};

enum class LayerType {
    undefined = 0,
    cut, 
    routing,
    implant,
    masterslice,
};
 
enum class LayerDirection {
    undefined = 0,
    horizontal,
    vertical,
};

struct Size {
    float x,
        y;
};

struct Site {
    std::string name;
    bool        symmetryX,
        symmetryY;
    SiteClass   siteClass;
    Size        size;
public:
    Site(std::string name) : name(name), symmetryX(false), symmetryY(false), siteClass(SiteClass::undefined), size({ 0.0f, 0.0f }) {}
};

class Polygon {
public:
    std::vector<double>  position;
//public:
//    bool ReadPosition(std::ifstream& lefFile, std::string& name);
};

//class Position {
//public:
//    float poss;
//};





class Layer {
public:
    std::string             name;
    std::vector<Polygon*>   polygons;
public: 
    Layer(std::string name);
    ~Layer();
public: 
    bool ReadPolygon(std::ifstream& lefFile, std::string& name);
};

Layer::Layer(std::string name) : name(name) {}
Layer::~Layer() {
    for (size_t i = 0; i < polygons.size(); ++i)
        delete polygons[i];
    polygons.clear();
}


class Pin {
public:
    std::string             name;
    PinDirection            direction;
    std::vector<Layer*>     layers;
public:
    Pin(std::string name);
    ~Pin();
public:
    bool ReadLayer(std::ifstream& lefFile, std::string& name);    
};

Pin::Pin(std::string name) : name(name), direction(PinDirection::undefined) {}
Pin::~Pin() {
    for (size_t i = 0; i < layers.size(); ++i)
        delete layers[i];
    layers.clear();
}

class OBS {
public:
    std::vector<Layer*>     layers;
public:
    ~OBS();
public:
    bool ReadLayer(std::ifstream& lefFile, std::string& name);
};

OBS::~OBS() {
    for (size_t i = 0; i < layers.size(); ++i)
        delete layers[i];
    layers.clear();
}

class Macro {
public:
    std::string         name;
    std::vector<Pin*>   pins;
    std::vector<OBS*>   obss;
    SiteClass           site_class;
    float size_x, size_y;
    bool        symmetryX,
        symmetryY;
    MacroClass           macro_class;
public:
    Macro(std::string name);
    ~Macro();
public:
    bool ReadPin(std::ifstream& lefFile, std::string& name);
    bool ReadObs(std::ifstream& lefFile, std::string& name);
};

Macro::Macro(std::string name) : name(name) {}
Macro::~Macro() {
    for (size_t i = 0; i < pins.size(); ++i)
        delete pins[i];
    pins.clear();
}


class LEFFile {
    std::string           fileName;
    // LEF data starts here
    float                 version;
    int                   microns;  // units
    float                 manufacturingGrid;
    std::vector<Site*>   sites;
    std::vector<Macro*>  macroes;
public:
    LEFFile();
    ~LEFFile();
public:
    bool Read(std::string filename);
private:
    bool ReadUnits(std::ifstream& lefFile);
    bool ReadSite(std::ifstream& lefFile, std::string& name);
    bool ReadMacro(std::ifstream& lefFile, std::string& name);
};

LEFFile::LEFFile() {}
LEFFile::~LEFFile() {
    for (size_t i = 0; i < sites.size(); ++i)
        sites[i];
    sites.clear();
    for (size_t i = 0; i < macroes.size(); ++i)
        delete macroes[i];
    macroes.clear();
}

void trim_left(std::string& text) {
    size_t pos = text.find_first_not_of(" \t");
    if (!pos)
        return;
    if (pos == std::string::npos)
        return;
    text.erase(0, pos);
}

bool LEFFile::Read(std::string filename) {
    std::ifstream lefFile(filename);
    if (!lefFile.is_open())
        return false;

    std::string line,
        token;
    while (std::getline(lefFile, line)) {
        // 1. Trim the line to be sure that first symbol is nos a space char
        trim_left(line);
        // 2. Ignore empty and commented lines
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;

        std::istringstream iss(line);
        iss >> token;
        // 3. Ignore unimportans statements
        if (token == "DIVIDERCHAR")
            continue;
        if (token == "DIVIDERCHAR")
            continue;
        // 4. Read importans statements
        if (token == "VERSION") {
            iss >> version;
            continue;
        }
        if (token == "UNITS") {
            if (!ReadUnits(lefFile)) {
                lefFile.close();
                return false;
            }
            continue;
        }
        if (token == "MANUFACTURINGGRID") {
            iss >> manufacturingGrid;
            continue;
        }
        if (token == "SITE") {
            iss >> token;
            if (!ReadSite(lefFile, token)) {
                lefFile.close();
                return false;
            }
            continue;
        }
        if (token == "MACRO") {
            iss >> token;
            if (!ReadMacro(lefFile, token)) {
                lefFile.close();
                return false;
            }
            continue;
        }
    }

    lefFile.close();
    fileName = filename;
    return true;
}

bool LEFFile::ReadUnits(std::ifstream& lefFile) {
    std::string line,
        token;
    while (std::getline(lefFile, line)) {
        trim_left(line);
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;

        std::istringstream iss(line);
        iss >> token;
        if (token == "END") {
            iss >> token;
            if (token == "UNITS")
                return true;
            std::cerr << "_err_ : [Reading UNITS] 'END' statement met with unsupported token '" << token << "'. UNITS read failed." << std::endl;
            return false;
        }

        if (token == "DATABASE") {
            iss >> token;
            if (token == "MICRONS") {
                iss >> microns;
                continue;
            }
            std::cerr << "_wrn_ : [Reading UNITS] Supported 'DATABASE' statement is 'MICRONS' only! Line ignored." << std::endl;
            continue;
        }
        std::cerr << "_wrn_ : [Reading UNITS] Unsupported token '" << token << "'. Line ignored." << std::endl;
        continue;
    }
    return true;
}

bool LEFFile::ReadSite(std::ifstream& lefFile, std::string& name) {
    Site* p_site = new Site(name);
    sites.push_back(p_site);

    std::string line,
        token;


    while (std::getline(lefFile, line)) {
        trim_left(line);
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;

        std::istringstream iss(line);
        iss >> token;

        if (token == "END") {
            iss >> token;
            if (token == name)
                return true;
            std::cerr << "_err_ : [Reading SITE] 'END' statement met with name different from '" << name << "' (which was used to define site). UNITS read failed." << std::endl;
            return false;
        }

        if (token == "SYMMETRY") {
            iss >> token;
            if (token == "Y")
                p_site->symmetryY = true;
            if (token == "X") {
                p_site->symmetryX = true;
                iss >> token;
                if (token == "Y")
                    p_site->symmetryY = true;
            }
            continue;
        }

        if (token == "CLASS") {
            iss >> token;
            if (token == "CORE") {
                p_site->siteClass = SiteClass::core;
                continue;
            }
            if (token == "PAD") {
                p_site->siteClass = SiteClass::pad;
                continue;
            }
            std::cerr << "_wrn_ : [Reading SITE] Unsupported SITE CLASS type '" << token << "'. Line ignored." << std::endl;
            continue;
        }

        if (token == "SIZE") {
            iss >> p_site->size.x >> token >> p_site->size.y;
            continue;
        }

        std::cerr << "_wrn_ : [Reading SITE] Unsupported token '" << token << "'. Line ignored." << std::endl;
        continue;
    }
    return true;
}


bool Layer::ReadPolygon(std::ifstream& lefFile, std::string& name) {
    Polygon* p_pol = new Polygon;
    polygons.push_back(p_pol);

    std::string line,
        token;
    while (std::getline(lefFile, line)) {
        trim_left(line);
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;

        std::istringstream iss(line);
        iss >> token;

        if (token == ";") {
            return true;
            std::cerr << "_wrn_ : [Reading POLYGON] Unsupported symbol" << token << "'. Line ignored." << std::endl;
            return false;
        }
    }
    return true;
}

bool Pin::ReadLayer(std::ifstream& lefFile, std::string& name) {
    Layer* p_lay = new Layer(name);
    layers.push_back(p_lay);

    std::string line,
        token;
    while (std::getline(lefFile, line)) {
        trim_left(line);
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;

        std::istringstream iss(line);
        iss >> token;

        if (token == "END") {
            return true;
            std::cerr << "_wrn_ : [Reading LAYER] Unsupported symbol" << token << "'. Line ignored." << std::endl;
            return false;
        }
      
        if (token == "POLYGON") {
            if (!p_lay->ReadPolygon(lefFile, token)) {
                lefFile.close();
                return false;
            }
            continue;
        }
        if (token == "RECT") {
            if (!p_lay->ReadPolygon(lefFile, token)) {
                lefFile.close();
                return false;
            }
            continue;
        }
    }
    return true;
}

bool Macro::ReadPin(std::ifstream& lefFile, std::string& name) {
    Pin* p_pin = new Pin(name);
    pins.push_back(p_pin);

    std::string line,
        token;
    while (std::getline(lefFile, line)) {
        trim_left(line);
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;

        std::istringstream iss(line);
        iss >> token;

        if (token == "END") {
            iss >> token;
            if (token == name)
                return true;
            std::cerr << "_err_ : [Reading PIN] 'END' statement met with name different from '" << name << "' (which was used to define site). UNITS read failed." << std::endl;
            return false;
        }

        if(token =="DIRECTION") {
            iss >> token;
            if (token == "INPUT") {
                p_pin->direction = PinDirection::input;
                continue;
            }
            if (token == "OUTPUT") {
                p_pin->direction = PinDirection::output;
                continue;
            }
            if (token == "INOUT") {
                p_pin->direction = PinDirection::inout;
                continue;
            }
            if (token == "FEEDTHRU") {
                p_pin->direction = PinDirection::feedthru;
                continue;
            }
            if (token == "TRISTATE") {
                p_pin->direction = PinDirection::tristate;
                continue;
            }
            std::cerr << "_wrn_ : [Reading PIN] Unsupported DIRECTION'" << token << "'. Line ignored." << std::endl;
            continue;
        }
        if (token == "LAYER") {
            iss >> token;
            if (!p_pin->ReadLayer(lefFile, token)) {
                lefFile.close();
                return false;
            }
            continue;
        }
    }
    return true;
}

bool Macro::ReadObs(std::ifstream& lefFile, std::string& name) {
    OBS* p_obs = new OBS;
    obss.push_back(p_obs);
    std::string line,
        token;
    while (std::getline(lefFile, line)) {
        trim_left(line);
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;

        std::istringstream iss(line);
        iss >> token;
        if (token == "END") {
            return true;
            std::cerr << "_wrn_ : [Reading LAYER] Unsupported" << token << "'. Line ignored." << std::endl;
            return false;
        }
        if (token == "LAYER") {
            iss >> token;
            if (!p_obs->ReadLayer(lefFile, token)) {
                lefFile.close();
                return false;
            }
            continue;
        }
    }
    return true;
}

bool OBS::ReadLayer(std::ifstream& lefFile, std::string& name) {
    Layer* p_lay = new Layer(name);
    layers.push_back(p_lay);

    std::string line,
        token;
    while (std::getline(lefFile, line)) {
        trim_left(line);
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;

        std::istringstream iss(line);
        iss >> token;

        if (token == ";") {
            iss >> token;
            if (token == "END")
                return true;
            std::cerr << "_wrn_ : [Reading LAYER] Unsupported" << token << "'. Line ignored." << std::endl;
            return false;
        }
        if (token == "POLYGON") {
            if (!p_lay->ReadPolygon(lefFile, token)) {
                lefFile.close();
                return false;
            }
            continue;
        }
        if (token == "RECT") {
            if (!p_lay->ReadPolygon(lefFile, token)) {
                lefFile.close();
                return false;
            }
            continue;
        }

    }
    return true;
}


bool LEFFile::ReadMacro(std::ifstream& lefFile, std::string& name) {
    Macro* p_mac = new Macro(name);
    macroes.push_back(p_mac);

    std::string line,
        token;

    while (std::getline(lefFile, line)) {
        trim_left(line);
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;

        std::istringstream iss(line);
        iss >> token;

        if (token == "END") {
            iss >> token;
            if (token == name)
                return true;
            std::cerr << "_err_ : [Reading MACRO] 'END' statement met with name different from '" << name << "' (which was used to define site). UNITS read failed." << std::endl;
            return false;
        }
        
        if (token == "CLASS") {
            iss >> token;
            if (token == "CORE") {
                p_mac->macro_class = MacroClass::core;
                continue;
            }
            if (token == "PAD") {
                p_mac->macro_class = MacroClass::pad;
                continue;
            }
            std::cerr << "_wrn_ : [Reading MACRO] Unsupported SITE CLASS type '" << token << "'. Line ignored." << std::endl;
            continue;
        }

        if (token == "SYMMETRY") {
            iss >> token;
            if (token == "Y")
                p_mac->symmetryY = true;
            if (token == "X") {
                p_mac->symmetryX = true;
                iss >> token;
                if (token == "Y")
                    p_mac->symmetryY = true;
            }
            continue;
        }

        if (token == "SIZE") {
            iss >> p_mac->size_x >> token >> p_mac->size_y;
            continue;
        }

        if (token == "PIN") {
            iss >> token;
            if (!p_mac->ReadPin(lefFile, token)) {
                lefFile.close();
                return false;
            }
            continue;
        }
        if (token == "OBS") {
            iss >> token;
            if (!p_mac->ReadObs(lefFile, token)) {
                lefFile.close();
                return false;
            }
            continue;
        }

        std::cerr << "_wrn_ : [Reading MACRO] Unsupported token '" << token << "'. Line ignored." << std::endl;
        continue;
    }
    return true;
}



int main() {
    std::string lefFileName = "ilin.lef";

    LEFFile lef;
    if (!lef.Read(lefFileName)) {
        std::cerr << "_err_ : Can't read input file '" << lefFileName << "'." << std::endl << "\tAbort." << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


//переход из LAYER в POLYGON
//как считать коор-ты