#ifndef ROBINSONMENU_H
#define ROBINSONMENU_H

#include <string>
#include <map>
#include <vector>

#ifdef WIN32
#include <SDL_ttf.h>
#else
#include <SDL2/SDL_ttf.h>
#endif

namespace SCRANTIC {

class RobinsonMenu
{
private:
    std::map<std::string, SDL_Texture *> menuScreen;
    std::map<std::string, std::vector<SDL_Rect>> menuRects;

    SDL_Renderer *renderer;

    int width;
    int height;

    std::string currentPage;
    size_t currentItem;

    RobinsonMenu(const RobinsonMenu& C);

public:
    RobinsonMenu(SDL_Renderer *renderer, int width, int height);
    ~RobinsonMenu();

    void render();
    void initMenu(std::map<std::string, std::vector<std::string>> items, TTF_Font *font);
    void setMenuPostion(std::string page, size_t pos);
    bool navigateMenu(SDL_Keycode key, std::string &newPage, size_t &newPosition);
};

}
#endif // ROBINSONMENU_H
