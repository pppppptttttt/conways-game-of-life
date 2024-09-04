#include "raylib.h"
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <raylib-cpp.hpp>
#include <string_view>

namespace {

constexpr int CELL_SIZE = 25;
constexpr int DEFAULT_CELLS_IN_ROW = 40;
constexpr int DEFAULT_CELLS_IN_COL = 40;
constexpr std::string_view WINDOW_TITLE = "Conway's Game of Life";
constexpr int PLAY_FPS = 5;
constexpr int IDLE_FPS = 60;
enum class Cell { DEAD, ALIVE = 0x00AA00FF };
using row_t = std::vector<Cell>;
using field_t = std::vector<row_t>;

} // namespace

field_t read_field_from_file(std::string_view filename) {
  std::ifstream pattern_file{filename.data()};

  if (!pattern_file.is_open()) {
    TraceLog(LOG_ERROR, "Couldn't open pattern file!");
    std::exit(EXIT_FAILURE);
  }
  pattern_file.exceptions(std::ios_base::badbit);

  field_t field;
  std::size_t cells_in_row = 0;

  try {
    bool first_line = true;
    std::string line;
    while (std::getline(pattern_file, line)) {
      // make sure if field is rectangle <=> constant row length
      if (!first_line && cells_in_row != line.length()) {
        TraceLog(LOG_ERROR, "Error while parsing pattern file!");
        std::exit(EXIT_FAILURE);
      } else {
        cells_in_row = line.length();
        first_line = false;
      }

      row_t row{cells_in_row};
      for (std::size_t i = 0; i < cells_in_row; ++i) {
        row[i] = line[i] != '.' ? Cell::ALIVE : Cell::DEAD;
      }

      field.push_back(std::move(row));
    }

    if (first_line) {
      TraceLog(LOG_ERROR, "Pattern file must not be empty!");
      std::exit(EXIT_FAILURE);
    }

  } catch (const std::exception &e) {
    TraceLog(LOG_ERROR, "%s\n", e.what());
    TraceLog(LOG_ERROR, "Error while reading pattern file!");
    std::exit(EXIT_FAILURE);
  }

  return field;
}

field_t next_step(const field_t &field) {
  auto in_field = [&field](int x, int y) -> bool {
    return x >= 0 && x < field.back().size() && y >= 0 && y < field.size();
  };
  constexpr int shifts[][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, 1},
                               {1, 1},   {1, 0},  {1, -1}, {0, -1}};

  static field_t next_field{field.size(), row_t{field.back().size()}};
  for (int y = 0; y < field.size(); ++y) {
    for (int x = 0; x < field[y].size(); ++x) {
      int neighbours_alive = 0;
      for (const auto [dx, dy] : shifts) {
        const int nx = x + dx;
        const int ny = y + dy;
        if (in_field(nx, ny) && field[ny][nx] != Cell::DEAD) {
          ++neighbours_alive;
        }
      }

      switch (field[y][x]) {
      case Cell::ALIVE:
        if (neighbours_alive < 2 || neighbours_alive > 3) {
          next_field[y][x] = Cell::DEAD;
        } else {
          next_field[y][x] = Cell::ALIVE;
        }
        break;

      case Cell::DEAD:
        if (neighbours_alive == 3) {
          next_field[y][x] = Cell::ALIVE;
        }
        break;

      default:
        std::unreachable();
      }
    }
  }

  return next_field;
}

void draw_field(const field_t &field) {
  for (int y = 0; y < field.size(); ++y) {
    for (int x = 0; x < field[y].size(); ++x) {
      raylib::Rectangle rect{static_cast<float>(x * CELL_SIZE),
                             static_cast<float>(y * CELL_SIZE), CELL_SIZE,
                             CELL_SIZE};
      rect.Draw(raylib::Color(static_cast<int>(field[y][x])));
      rect.DrawLines(raylib::Color::DarkGray());
    }
  }
}

bool is_field_empty(const field_t &field) {
  return !std::ranges::any_of(field, [](const std::vector<Cell> &row) -> bool {
    return std::ranges::any_of(
        row, [](Cell cell) -> bool { return cell == Cell::ALIVE; });
  });
}

int main(int argc, char **argv) {
  field_t field{DEFAULT_CELLS_IN_COL, row_t{DEFAULT_CELLS_IN_ROW}};

  if (argc == 1) {
  } else if (argc == 2) {
    field = read_field_from_file(argv[1]);
  } else {
    TraceLog(LOG_ERROR, "Usage: %s or %s <pattern-file>", argv[0], argv[0]);
    return 1;
  }

  const int cells_in_col = field.size();
  const int cells_in_row = field.back().size();

  raylib::Window window(cells_in_row * CELL_SIZE, cells_in_col * CELL_SIZE,
                        WINDOW_TITLE.data());

  SetTargetFPS(IDLE_FPS);
  bool play = false;
  while (!window.ShouldClose()) {
    window.BeginDrawing();
    window.ClearBackground(raylib::Color::Black());

    draw_field(field);

    if (!play || is_field_empty(field)) {
      SetTargetFPS(IDLE_FPS);
      play = false;
      if (raylib::Mouse::IsButtonPressed(MOUSE_LEFT_BUTTON)) {
        const auto [row, column] = std::pair<int, int>{
            static_cast<int>(raylib::Mouse::GetPosition().x / CELL_SIZE),
            static_cast<int>(raylib::Mouse::GetPosition().y / CELL_SIZE)};

        if (field[column][row] == Cell::DEAD)
          field[column][row] = Cell::ALIVE;
        else
          field[column][row] = Cell::DEAD;
      }
    } else {
      SetTargetFPS(PLAY_FPS);
    }

    if (IsKeyPressed(KEY_ENTER)) {
      play = !play;
    }

    if (play) {
      field = next_step(field);
    }

    window.EndDrawing();
  }
}