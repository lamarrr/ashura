/// SPDX-License-Identifier: MIT
#include "ashura/engine/animation.h"
#include "ashura/std/types.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

typedef struct ash::Vec2 Position;

class TerminalUI
{
private:
  const std::string CLEAR_SCREEN =
      "\033[2J\033[1;1H";        // Clear screen and move to top-left
  const std::string CURSOR_HOME = "\033[H";           // Move to top-left
  const std::string HIDE_CURSOR = "\033[?25l";        // Hide cursor
  const std::string SHOW_CURSOR = "\033[?25h";        // Show cursor
  const std::string CLEAR_TO_END =
      "\033[0J";        // Clear from cursor to end of screen
  bool is_first_update;

public:
  TerminalUI() : is_first_update(true)
  {
    std::cout << HIDE_CURSOR;
  }

  ~TerminalUI()
  {
    std::cout << SHOW_CURSOR;
  }

  void update(const std::string &data)
  {
    if (is_first_update)
    {
      // Clear screen only on first update
      std::cout << CLEAR_SCREEN;
      is_first_update = false;
    }
    else
    {
      // For subsequent updates, just move cursor to home
      std::cout << CURSOR_HOME;
    }

    // Output data and clear any remaining content
    std::cout << data << CLEAR_TO_END << std::flush;
  }
};

class TerminalTable
{
public:
  struct Column
  {
    std::string header;
    ash::usize  width;

    enum class Alignment
    {
      LEFT,
      RIGHT,
      CENTER
    } alignment;

    Column(const std::string &h, size_t w, Alignment a) :
        header(h), width(w), alignment(a)
    {
    }
  };

private:
  std::vector<Column>                   columns;
  std::vector<std::vector<std::string>> rows;
  std::string                           table_border     = "─";
  std::string                           column_separator = "│";
  bool                                  use_unicode      = true;

  std::string align_text(const std::string &text, size_t width,
                         Column::Alignment alignment) const
  {
    if (text.length() >= width)
    {
      return text.substr(0, width);
    }

    size_t padding = width - text.length();
    switch (alignment)
    {
      case Column::Alignment::RIGHT:
        return std::string(padding, ' ') + text;
      case Column::Alignment::CENTER:
      {
        size_t left_pad  = padding / 2;
        size_t right_pad = padding - left_pad;
        return std::string(left_pad, ' ') + text + std::string(right_pad, ' ');
      }
      default:        // LEFT
        return text + std::string(padding, ' ');
    }
  }

  std::string create_separator(std::string corner = "┼") const
  {
    std::string separator;
    for (size_t i = 0; i < columns.size(); ++i)
    {
      if (i == 0)
      {
        separator += use_unicode ? "├" : "+";
      }
      separator += std::string(columns[i].width, '-');
      separator += (i < columns.size() - 1) ? (use_unicode ? corner : "+") :
                                              (use_unicode ? "┤" : "+");
    }
    return separator;
  }

public:
  TerminalTable(bool use_unicode_chars = true) : use_unicode(use_unicode_chars)
  {
    if (!use_unicode)
    {
      table_border     = "-";
      column_separator = "|";
    }
  }

  // Add a column with specified alignment
  void add_column(const std::string &header, size_t width,
                  Column::Alignment alignment = Column::Alignment::LEFT)
  {
    columns.emplace_back(header, width, alignment);
  }

  // Add a row of data
  void add_row(const std::vector<std::string> &row)
  {
    if (row.size() != columns.size())
    {
      throw std::runtime_error("Row size doesn't match column count");
    }
    rows.push_back(row);
  }

  // Clear all rows but keep columns
  void clear_rows()
  {
    rows.clear();
  }

  // Generate the formatted table
  std::string to_string() const
  {
    std::stringstream ss;

    // Top border
    ss << (use_unicode ? "┌" : "+");
    for (size_t i = 0; i < columns.size(); ++i)
    {
      ss << std::string(columns[i].width, '-')
         << (i < columns.size() - 1 ? (use_unicode ? "┬" : "+") :
                                      (use_unicode ? "┐" : "+"));
    }
    ss << "\n";

    // Headers
    ss << column_separator;
    for (size_t i = 0; i < columns.size(); ++i)
    {
      ss << align_text(columns[i].header, columns[i].width,
                       Column::Alignment::CENTER)
         << column_separator;
    }
    ss << "\n";

    // Header separator
    ss << create_separator("┼") << "\n";

    // Data rows
    for (const auto &row : rows)
    {
      ss << column_separator;
      for (size_t i = 0; i < columns.size(); ++i)
      {
        ss << align_text(row[i], columns[i].width, columns[i].alignment)
           << column_separator;
      }
      ss << "\n";
    }

    // Bottom border
    ss << (use_unicode ? "└" : "+");
    for (size_t i = 0; i < columns.size(); ++i)
    {
      ss << std::string(columns[i].width, '-')
         << (i < columns.size() - 1 ? (use_unicode ? "┴" : "+") :
                                      (use_unicode ? "┘" : "+"));
    }
    ss << "\n";

    return ss.str();
  }
};

class TableManager
{
private:
  struct TableInfo
  {
    std::unique_ptr<TerminalTable> table;
    Position                       position;
    std::string                    title;

    TableInfo(std::unique_ptr<TerminalTable> t, Position pos,
              std::string title = "") :
        table(std::move(t)), position(pos), title(title)
    {
    }

    TableInfo() = delete;
  };

  std::map<std::string, TableInfo> tables;
  int                              terminal_width;
  int                              terminal_height;
  std::vector<std::vector<char>>   screen;

  // ANSI escape sequences for cursor positioning
  std::string move_cursorTo(int x, int y)
  {
    return "\033[" + std::to_string(y) + ";" + std::to_string(x) + "H";
  }

  // Calculate table dimensions including borders
  std::pair<int, int> get_table_dimensions(const TerminalTable &table)
  {
    std::stringstream ss(table.to_string());
    std::string       line;
    int               width = 0, height = 0;

    while (std::getline(ss, line))
    {
      width = std::max(width, static_cast<int>(line.length()));
      height++;
    }
    return {width, height};
  }

  void validate_id(const std::string &id) const
  {
    if (id.empty())
    {
      throw std::invalid_argument("Table ID cannot be empty");
    }
  }

  void validate_position(const Position &pos) const
  {
    if (pos.x < 0 || pos.y < 0 || pos.x >= terminal_width ||
        pos.y >= terminal_height)
    {
      throw std::out_of_range("Position is outside terminal bounds");
    }
  }

  void initialize_screen()
  {
    screen = std::vector<std::vector<char>>(
        terminal_height, std::vector<char>(terminal_width, ' '));
  }

  void write_to_screen(int startY, int startX, const std::string &text)
  {
    if (startY >= 0 && startY < terminal_height)
    {
      size_t xPos = static_cast<size_t>(startX);
      for (char c : text)
      {
        if (xPos < static_cast<size_t>(terminal_width))
        {
          screen[startY][xPos++] = c;
        }
      }
    }
  }

  int get_table_height(const TableInfo &table_info) const
  {
    std::stringstream ss(table_info.table->to_string());
    int               height = 0;
    std::string       line;
    while (std::getline(ss, line))
    {
      height++;
    }
    return height + (table_info.title.empty() ?
                         0 :
                         1);        // Add 1 for title if present
  }

  void clear_screen_area(int startY, int endY)
  {
    for (int y = startY; y <= endY && y < terminal_height; ++y)
    {
      for (int x = 0; x < terminal_width; ++x)
      {
        screen[y][x] = ' ';
      }
    }
  }

public:
  TableManager(int width = 80, int height = 24) :
      terminal_width(width), terminal_height(height)
  {
    if (width <= 0 || height <= 0)
    {
      throw std::invalid_argument("Invalid terminal dimensions");
    }
    initialize_screen();
  }

  // Add a new table with position and identifier
  void add_table(const std::string &id, std::unique_ptr<TerminalTable> table,
                 Position pos, const std::string &title = "")
  {
    validate_id(id);
    validate_position(pos);
    tables.try_emplace(id, TableInfo(std::move(table), pos, title));
  }

  // Get reference to a table for updating
  TerminalTable &get_table(const std::string &id)
  {
    auto it = tables.find(id);
    if (it == tables.end())
    {
      throw std::runtime_error("Table not found: " + id);
    }
    return *(it->second.table);
  }

  void set_table_position(const std::string &id, Position new_pos)
  {
    auto it = tables.find(id);
    if (it != tables.end())
    {
      it->second.position = new_pos;
    }
  }

  // Generate formatted output with all tables
  std::string render()
  {
    // Clear the screen buffer
    initialize_screen();

    int max_bottom = 0;

    // Render each table at its position
    for (const auto &[id, table_info] : tables)
    {
      int currentY = table_info.position.y;

      std::stringstream ss;

      if (!table_info.title.empty())
      {
        write_to_screen(currentY++, table_info.position.x, table_info.title);
      }

      ss << table_info.table->to_string();
      std::string line;
      while (std::getline(ss, line) && currentY < terminal_height)
      {
        write_to_screen(currentY++, table_info.position.x, line);
      }

      // Update maximum bottom position
      max_bottom = std::max(max_bottom, currentY);
    }

    // Combine screen buffer into output string
    std::stringstream output;
    for (int y = 0; y < max_bottom; ++y)
    {
      for (int x = 0; x < terminal_width; ++x)
      {
        output << screen[y][x];
      }
      output << "\n";
    }

    return output.str();
  }
};

std::shared_ptr<ash::Animation<ash::f32>> create_simple_animation(
    const ash::f32       duration = 1.0f,
    const ash::CurveType easing   = ash::CurveType::Linear,
    const bool           loop     = false)
{
  auto anim = std::make_shared<ash::Animation<ash::f32>>(
      0.0f, 10.0f,
      ash::AnimationConfig{
          .duration = duration, .loop = loop, .easing = easing});

  return anim;
}

int main()
{

  TerminalUI            ui;
  auto animator = std::make_shared<ash::AnimationManager>();
  TableManager          manager(80, 24);

  auto simple_animation_table = std::make_unique<TerminalTable>();
  simple_animation_table->add_column("Linear", 10,
                                     TerminalTable::Column::Alignment::CENTER);
  simple_animation_table->add_column("EaseIn", 10,
                                     TerminalTable::Column::Alignment::CENTER);
  simple_animation_table->add_column("EaseOut", 10,
                                     TerminalTable::Column::Alignment::CENTER);
  simple_animation_table->add_column("EaseInOut", 10,
                                     TerminalTable::Column::Alignment::CENTER);
  const ash::f32 duration         = 10.0f;
  const ash::f32 start            = 0.0f;
  const ash::f32 end              = 10.0f;
  auto           linear_animation = animator->create<ash::f32>(
      start, end,
      ash::AnimationConfig{.duration = duration,
                                     .loop     = false,
                                     .easing   = ash::CurveType::Linear});
  auto easein_animation = animator->create<ash::f32>(
      start, end,
      ash::AnimationConfig{.duration = duration,
                           .loop     = false,
                           .easing   = ash::CurveType::EaseIn});

  auto easeout_animation = animator->create<ash::f32>(
      start, end,
      ash::AnimationConfig{.duration = duration,
                           .loop     = false,
                           .easing   = ash::CurveType::EaseOut});

  auto easein_out_animation = animator->create<ash::f32>(
      start, end,
      ash::AnimationConfig{.duration = 10.0f,
                           .loop     = false,
                           .easing   = ash::CurveType::EaseInOut});

  manager.add_table("Simple", std::move(simple_animation_table),
                    {.x = 1, .y = 1}, "Simple Animation");

  animator->play_all();
  

  while (true)
  {
    animator->tick();

    auto &_simple_animation = manager.get_table("Simple");
    _simple_animation.clear_rows();
    _simple_animation.add_row(
        {std::format("{:4.1f}", linear_animation->value()),
         std::format("{:4.1f}", easein_animation->value()),
         std::format("{:4.1f}", easeout_animation->value()),
         std::format("{:4.1f}", easein_out_animation->value())});

    ui.update(manager.render());

    if(easein_out_animation->value() == 10.0f){
      animator->clear();
      break;
    }
  }

  return 0;
}