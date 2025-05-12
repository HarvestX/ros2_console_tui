// Copyright 2025 HarvestX Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __ROS2_CONSOLE_TUI_NODE_KEY_DEFINITIONS_HPP__
#define __ROS2_CONSOLE_TUI_NODE_KEY_DEFINITIONS_HPP__

#include <ncurses.h>

#include <functional>
#include <string>

namespace ros2_console_tui_node
{

enum class Key {
  // Lowercase letters
  LowercaseA,
  LowercaseB,
  LowercaseC,
  LowercaseD,
  LowercaseE,
  LowercaseF,
  LowercaseG,
  LowercaseH,
  LowercaseI,
  LowercaseJ,
  LowercaseK,
  LowercaseL,
  LowercaseM,
  LowercaseN,
  LowercaseO,
  LowercaseP,
  LowercaseQ,
  LowercaseR,
  LowercaseS,
  LowercaseT,
  LowercaseU,
  LowercaseV,
  LowercaseW,
  LowercaseX,
  LowercaseY,
  LowercaseZ,

  // Uppercase letters
  UppercaseA,
  UppercaseB,
  UppercaseC,
  UppercaseD,
  UppercaseE,
  UppercaseF,
  UppercaseG,
  UppercaseH,
  UppercaseI,
  UppercaseJ,
  UppercaseK,
  UppercaseL,
  UppercaseM,
  UppercaseN,
  UppercaseO,
  UppercaseP,
  UppercaseQ,
  UppercaseR,
  UppercaseS,
  UppercaseT,
  UppercaseU,
  UppercaseV,
  UppercaseW,
  UppercaseX,
  UppercaseY,
  UppercaseZ,

  // Digits
  Digit0,
  Digit1,
  Digit2,
  Digit3,
  Digit4,
  Digit5,
  Digit6,
  Digit7,
  Digit8,
  Digit9,

  // Ctrl + A-Z
  CtrlA,
  CtrlB,
  CtrlC,
  CtrlD,
  CtrlE,
  CtrlF,
  CtrlG,
  CtrlH,
  CtrlI,
  CtrlJ,
  CtrlK,
  CtrlL,
  CtrlM,
  CtrlN,
  CtrlO,
  CtrlP,
  CtrlQ,
  CtrlR,
  CtrlS,
  CtrlT,
  CtrlU,
  CtrlV,
  CtrlW,
  CtrlX,
  CtrlY,
  CtrlZ,

  // Special keys
  Enter,
  Escape,
  Backspace,
  Tab,
  Up,
  Down,
  Left,
  Right,
};

#define CTRL(x) ((x) & 0x1F)

int to_keycode(Key key)
{
  using K = Key;
  switch (key) {
    // Lowercase
    case K::LowercaseA:
      return 'a';
    case K::LowercaseB:
      return 'b';
    case K::LowercaseC:
      return 'c';
    case K::LowercaseD:
      return 'd';
    case K::LowercaseE:
      return 'e';
    case K::LowercaseF:
      return 'f';
    case K::LowercaseG:
      return 'g';
    case K::LowercaseH:
      return 'h';
    case K::LowercaseI:
      return 'i';
    case K::LowercaseJ:
      return 'j';
    case K::LowercaseK:
      return 'k';
    case K::LowercaseL:
      return 'l';
    case K::LowercaseM:
      return 'm';
    case K::LowercaseN:
      return 'n';
    case K::LowercaseO:
      return 'o';
    case K::LowercaseP:
      return 'p';
    case K::LowercaseQ:
      return 'q';
    case K::LowercaseR:
      return 'r';
    case K::LowercaseS:
      return 's';
    case K::LowercaseT:
      return 't';
    case K::LowercaseU:
      return 'u';
    case K::LowercaseV:
      return 'v';
    case K::LowercaseW:
      return 'w';
    case K::LowercaseX:
      return 'x';
    case K::LowercaseY:
      return 'y';
    case K::LowercaseZ:
      return 'z';

    // Uppercase
    case K::UppercaseA:
      return 'A';
    case K::UppercaseB:
      return 'B';
    case K::UppercaseC:
      return 'C';
    case K::UppercaseD:
      return 'D';
    case K::UppercaseE:
      return 'E';
    case K::UppercaseF:
      return 'F';
    case K::UppercaseG:
      return 'G';
    case K::UppercaseH:
      return 'H';
    case K::UppercaseI:
      return 'I';
    case K::UppercaseJ:
      return 'J';
    case K::UppercaseK:
      return 'K';
    case K::UppercaseL:
      return 'L';
    case K::UppercaseM:
      return 'M';
    case K::UppercaseN:
      return 'N';
    case K::UppercaseO:
      return 'O';
    case K::UppercaseP:
      return 'P';
    case K::UppercaseQ:
      return 'Q';
    case K::UppercaseR:
      return 'R';
    case K::UppercaseS:
      return 'S';
    case K::UppercaseT:
      return 'T';
    case K::UppercaseU:
      return 'U';
    case K::UppercaseV:
      return 'V';
    case K::UppercaseW:
      return 'W';
    case K::UppercaseX:
      return 'X';
    case K::UppercaseY:
      return 'Y';
    case K::UppercaseZ:
      return 'Z';

    // Digits
    case K::Digit0:
      return '0';
    case K::Digit1:
      return '1';
    case K::Digit2:
      return '2';
    case K::Digit3:
      return '3';
    case K::Digit4:
      return '4';
    case K::Digit5:
      return '5';
    case K::Digit6:
      return '6';
    case K::Digit7:
      return '7';
    case K::Digit8:
      return '8';
    case K::Digit9:
      return '9';

    // Ctrl
    case K::CtrlA:
      return CTRL('a');
    case K::CtrlB:
      return CTRL('b');
    case K::CtrlC:
      return CTRL('c');
    case K::CtrlD:
      return CTRL('d');
    case K::CtrlE:
      return CTRL('e');
    case K::CtrlF:
      return CTRL('f');
    case K::CtrlG:
      return CTRL('g');
    case K::CtrlH:
      return CTRL('h');
    case K::CtrlI:
      return CTRL('i');
    case K::CtrlJ:
      return CTRL('j');
    case K::CtrlK:
      return CTRL('k');
    case K::CtrlL:
      return CTRL('l');
    case K::CtrlM:
      return CTRL('m');
    case K::CtrlN:
      return CTRL('n');
    case K::CtrlO:
      return CTRL('o');
    case K::CtrlP:
      return CTRL('p');
    case K::CtrlQ:
      return CTRL('q');
    case K::CtrlR:
      return CTRL('r');
    case K::CtrlS:
      return CTRL('s');
    case K::CtrlT:
      return CTRL('t');
    case K::CtrlU:
      return CTRL('u');
    case K::CtrlV:
      return CTRL('v');
    case K::CtrlW:
      return CTRL('w');
    case K::CtrlX:
      return CTRL('x');
    case K::CtrlY:
      return CTRL('y');
    case K::CtrlZ:
      return CTRL('z');

    // Special
    case K::Enter:
      return '\n';
    case K::Escape:
      return 27;
    case K::Backspace:
      return 127;
    case K::Tab:
      return '\t';
    case K::Up:
      return KEY_UP;
    case K::Down:
      return KEY_DOWN;
    case K::Left:
      return KEY_LEFT;
    case K::Right:
      return KEY_RIGHT;

    default:
      return -1;
  }
}

struct KeyBindingEntry
{
  Key key;
  std::string label;
  std::function<void()> action;
  bool showInStatusBar = true;
};

}  // namespace ros2_console_tui_node

#endif  // __ROS2_CONSOLE_TUI_NODE_KEY_DEFINITIONS_HPP__
