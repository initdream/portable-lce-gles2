#include "ScrolledSelectionList.h"

#include <chrono>

#include "Button.h"

#include "minecraft/client/renderer/Tesselator.h"

class Minecraft;

ScrolledSelectionList::ScrolledSelectionList(Minecraft* minecraft, int width,
                                             int height, int y0, int y1,
                                             int itemHeight) {
    this->minecraft = minecraft;
    this->width = width;
    this->height = height;
    this->y0 = y0;
    this->y1 = y1;
    this->itemHeight = itemHeight;
    this->x0 = 0;
    this->x1 = width;

    // 4J Stu - Smoe default initialisers
    this->upId = 0;
    this->downId = 0;

    this->yDrag = -1.0f;
    this->yDragScale = 0.0f;
    this->yo = 0.0f;

    this->lastSelection = -1;
    this->lastSelectionTime = 0;

    this->renderSelection = true;
    this->_renderHeader = false;
    this->headerHeight = 0;
    // End
}

void ScrolledSelectionList::setRenderSelection(bool renderSelection) {
    this->renderSelection = renderSelection;
}

void ScrolledSelectionList::setRenderHeader(bool renderHeader,
                                            int headerHeight) {
    this->_renderHeader = renderHeader;
    this->headerHeight = headerHeight;

    if (!_renderHeader) {
        this->headerHeight = 0;
    }
}

int ScrolledSelectionList::getMaxPosition() {
    return getNumberOfItems() * itemHeight + headerHeight;
}

void ScrolledSelectionList::renderHeader(int x, int y, Tesselator* t) {}

void ScrolledSelectionList::clickedHeader(int headerMouseX, int headerMouseY) {}

void ScrolledSelectionList::renderDecorations(int mouseX, int mouseY) {}

int ScrolledSelectionList::getItemAtPosition(int x, int y) {
    if (y < y0 || y > y1) return -1;

    int listHalfWidth = 150;
    int xLeft = width / 2 - listHalfWidth;
    int xRight = width / 2 + listHalfWidth;

    if (x < xLeft || x > xRight) return -1;

    int relativeY = (y - y0 - headerHeight + (int)yo - 4);
    int slot = relativeY / itemHeight;

    if (slot >= 0 && slot < getNumberOfItems()) {
        return slot;
    }
    return -1;
}

void ScrolledSelectionList::init(std::vector<Button*>* buttons, int upButtonId,
                                 int downButtonId) {
    this->upId = upButtonId;
    this->downId = downButtonId;
}

void ScrolledSelectionList::capYPosition() {
    int max = getMaxPosition() - (y1 - y0 - 4);
    if (max < 0) max = 0;
    if (yo < 0) yo = 0;
    if (yo > max) yo = (float)max;
}

void ScrolledSelectionList::buttonClicked(Button* button) {
    if (!button->active) return;

    if (button->id == upId) {
        yo -= (float)(itemHeight);
        capYPosition();
    } else if (button->id == downId) {
        yo += (float)(itemHeight);
        capYPosition();
    }
}

int64_t ScrolledSelectionList::_currentTimeMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
    .count();
}

void ScrolledSelectionList::render(int xm, int ym, float a) {
    int n = getNumberOfItems();
    if (n <= 0) return;

    int listHalfWidth = 150;
    int listLeft = width / 2 - listHalfWidth;
    int listRight = width / 2 + listHalfWidth;

    for (int i = 0; i < n; ++i) {
        int yTop = y0 + 4 + i * itemHeight + headerHeight - (int)yo;
        int h = itemHeight - 4;
        if (yTop + h < y0 || yTop > y1) continue;

        if (renderSelection && isSelectedItem(i)) {
            fill(listLeft - 2, yTop - 2, listRight + 2, yTop + h + 2,
                 0xFF808080);
            fill(listLeft - 1, yTop - 1, listRight + 1, yTop + h + 1,
                 0xFF000000);
        }
        renderItem(i, listLeft, yTop, h, nullptr);
    }
}

bool ScrolledSelectionList::mouseClicked(int x, int y, int button) {
    if (button != 0) return false;

    int item = getItemAtPosition(x, y);
    if (item < 0) return false;

    int64_t now = _currentTimeMs();
    bool doubleClick =
    (item == lastSelection) && (now - lastSelectionTime < 250);

    selectItem(item, doubleClick);

    lastSelection = item;
    lastSelectionTime = now;
    return true;
}

void ScrolledSelectionList::renderHoleBackground(int y0, int y1, int a0,
                                                 int a1) {
    // 4J Unused
}
