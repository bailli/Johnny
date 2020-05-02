#include "RobinsonMenu.h"

#include "types.h"

#include <iostream>

SCRANTIC::RobinsonMenu::RobinsonMenu(SDL_Renderer* renderer, int width, int height)
    : renderer(renderer),
      width(width),
      height(height),
      currentPage(""),
      currentItem(0) {
}

SCRANTIC::RobinsonMenu::~RobinsonMenu() {
    for (auto it = menuScreen.begin(); it != menuScreen.end(); ++it) {
        SDL_DestroyTexture(it->second);
    }
}

void SCRANTIC::RobinsonMenu::initMenu(std::map<std::string, std::vector<std::string>> items, TTF_Font *font) {
    std::string page;

    SDL_Color c1 = { 255,   0,   0, 255 };
    SDL_Color c2 = { 255, 255, 255, 255 };
    SDL_Rect rect = { 20, 0, 0, 0 };

    for (auto it = items.begin(); it != items.end(); ++it) {
        page = it->first;

        SDL_Surface *menu = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);

        rect.y = 10;

        TTF_SizeText(font, page.c_str(), &rect.w, &rect.h);
        SDL_Surface *tmpSurface = TTF_RenderText_Blended(font, page.c_str(), c1);

        if (tmpSurface == NULL) {
            std::cerr << "ERROR: Renderer: Could not render text: " << page << std::endl;
        } else {
            SDL_BlitSurface(tmpSurface, NULL, menu, &rect);
            SDL_FreeSurface(tmpSurface);
        }

        for (auto item : it->second) {
            rect.y = rect.y + rect.h + 10;

            TTF_SizeText(font, item.c_str(), &rect.w, &rect.h);
            tmpSurface = TTF_RenderText_Blended(font, item.c_str(), c2);

            if (tmpSurface == NULL) {
                std::cerr << "ERROR: Renderer: Could not render text: " << item << std::endl;
            } else {
                SDL_BlitSurface(tmpSurface, NULL, menu, &rect);
                SDL_FreeSurface(tmpSurface);
                rect.w += 6;
                rect.x -= 3;
                rect.h += 6;
                rect.y -= 3;
                menuRects[page].push_back(rect);
                rect.x += 3;
                rect.y += 3;
                rect.h -= 6;
            }
        }

        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, menu);

        if (tex == NULL) {
            std::cerr << "ERROR: Renderer: Could not convert menu surface to to texture: " << page << std::endl;
        } else {
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(tex, 200);
        }

        SDL_FreeSurface(menu);

        menuScreen[page] = tex;
    }
}

void SCRANTIC::RobinsonMenu::render() {
    auto it = menuScreen.find(currentPage);
    if (it == menuScreen.end()) {
        std::cerr << "Menu screen not found! " << currentPage << std::endl;
        std::cerr << "Resetting menu position!" << std::endl;
        currentPage = menuScreen.begin()->first;
        currentItem = 0;
        return;
    }

    SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
    SDL_RenderFillRect(renderer, &menuRects[currentPage][currentItem]);
    SDL_RenderCopy(renderer, it->second, NULL, NULL);
}

bool SCRANTIC::RobinsonMenu::navigateMenu(SDL_Keycode key, std::string &newPage, size_t &newPosition) {
    std::map<std::string, SDL_Texture*>::iterator it;

    switch (key) {
    case SDLK_LEFT:
    case SDLK_RIGHT:
        it = menuScreen.find(currentPage);
        if (it == menuScreen.end()) {
            std::cerr << "Menu screen not found! " << currentPage << std::endl;
            return false;
        }

        currentItem = 0;

        if (key == SDLK_LEFT) {
            if (it == menuScreen.begin()) {
                it = menuScreen.end();
            }
            --it;
        } else {
            ++it;
            if (it == menuScreen.end()) {
                it = menuScreen.begin();
            }
        }
        currentPage = it->first;
        break;

    case SDLK_UP:
        if (currentItem == 0) {
            currentItem = menuRects[currentPage].size() - 1;
        } else {
            --currentItem;
        }
        break;

    case SDLK_DOWN:
        ++currentItem;
        if (currentItem == menuRects[currentPage].size()) {
            currentItem = 0;
        }
        break;

    case SDLK_RETURN:
        newPage = currentPage;
        newPosition = currentItem;
        return true;
    }

    return false;
}

void SCRANTIC::RobinsonMenu::setMenuPostion(std::string page, size_t pos) {
    if (menuScreen.find(page) != menuScreen.end()) {
        currentPage = page;
    }
    if (pos < menuRects[currentPage].size()) {
        currentItem = pos;
    }
}
