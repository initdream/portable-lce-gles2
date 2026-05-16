#pragma once

#include <vector>

namespace JavaKeyInput {


    inline std::vector<int> pressedKeys;
    inline std::vector<wchar_t> typedChars;
    inline bool keysCurrent[512] = {};
    inline bool keysPrev[512] = {};

}  // namespace JavaKeyInput
