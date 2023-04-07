#pragma once

namespace ash
{

struct ClipBoard
{
  void get_text();

  void set_text();

  bool has_text();

  // TODO(lamarrr)
  // virtual void on_copy();
  // virtual void on_cut();
  // virtual void on_paste();
};

}        // namespace ash
