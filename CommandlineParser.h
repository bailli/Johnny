#ifndef INPUTPARSER_H
#define INPUTPARSER_H

#include <string>
#include <iostream>
#include <map>

class CommandlineParser {
private:
    std::map<std::string, std::string> parameters;

public:
    CommandlineParser (char **begin, char **end) {
        std::string lastArg = "";
        std::string currentArg = "";

        for (char **it = ++begin; it != end; ++it) {
            currentArg = *it;

            if (currentArg.substr(0, 2) == "--") {
                if (lastArg != "") {
                    parameters.insert({lastArg, ""});
                    lastArg = currentArg;
                    continue;
                }
                lastArg = currentArg;
                continue;
            }

            if (lastArg == "") {
                std::cout << "Invalid parameter: " << currentArg << std::endl;
                continue;
            }

            parameters.insert({lastArg, currentArg});
            lastArg = "";
        }

        if (lastArg != "") {
            parameters.insert({lastArg, ""});
        }
    }

    std::string getCmdOption(const std::string &option) {
        auto it = parameters.find(option);
        if (it == parameters.end()) {
            return "";
        }
        return it->second;
    }

    bool cmdOptionExists(const std::string &option) {
        auto it = parameters.find(option);
        return it != parameters.end();
    }
};

#endif // INPUTPARSER_H
