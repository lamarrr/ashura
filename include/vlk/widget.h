



struct Canvas;
struct Context{};

struct StatefulWidget{


virtual bool is_dirty() { return dirty_; };


virtual void draw(Canvas& canvas, Context const& context){
	mark_clean_();
}

protected:

void mark_dirty_() {
	dirty_ = true;
}

void mark_clean_(){
	dirty_ = false;
}

private:
	bool dirty_;

};

namespace desc{
struct Image{
enum Format{
rgba,
argb,
rgb
};

uint64_t width;
uint64_t height;

// depends on how skia receives it
stx::Span<u32 const> data;

};
};


struct Coordinates{
f32 x, y;
};

struct BackgroundImage{
Image image;
Coordinates coordinates;
};

namespace box{
using px = u32;

struct Dimensions{
stx::Option<px> width = 0; // auto-fit if None
stx::Option<px> height = 0; // auto-fit if None
};

struct Color{
u32 rgba;
static rgba(u8, u8, u8, u8);
static rgb(u8, u8, u8);

constexpr auto Color with_opacity(u8) const noexcept;
constexpr auto Color with_red(u8) const noexcept;
constexpr auto Color with_green(u8) const noexcept;
constexpr auto Color with_blue(u8) const noexcept;

static constexpr auto Red = Color::rgba(0xFF, 0x00, 0x00, 0xFF);
static constexpr auto White = Color::rgba(0xFF, 0xFF, 0xFF, 0xFF);
static constexpr auto Black = Color::rgba(0x00, 0x00, 0x00, 0xFF);
static constexpr auto Blue = Color::rgba(0x00, 0x00, 0xFF, 0xFF);
static constexpr auto Green = Color::rgba(0x00, 0xFF, 0x00, 0xFF);
};

struct Tbrl{
px top = 0, bottom = 0, right = 0, left = 0;
static uniform(px);
static xy(px, px);
static tbrl(px, px, px, px);
};

struct Border: public Tbrl{
	using Tbrl::Tbrl;
};

struct Padding: public Tbrl{
	using Tbrl::Tbrl;
};

struct Properties{
std::variant<Color, BackgroundImage> background;
Border border;
Dimensions dimensions;
px padding [4];
px margin [4];
};

};

struct DynamicButton: StatefulWidget{

bool is_dirty() override {
	return content_changed_;
};

void draw(Canvas& canvas, Context const& context){
	auto subcanvas = canvas.subcanvas();
	subcanvas.draw_text("", ...,...,...);
	subcanvas.draw_rect(...,...,...,...);
	StatefulWidget::draw(canvas, context);
	// use recorder actually
}


Properties properties_;

};


struct StatelessWidget{

virtual void draw(Canvas& canvas, Context const& context) = 0;
};



template<typename ClockType = std::chrono::steady_clock>
struct Ticker{

void start(){
last_tick_time_point_ = ClockType::now();
}

std::chrono::nanoseconds tick(){
auto previous_tick_time_point = last_tick_time_point_;
last_tick_timepoint_ = ClockType::now();
return last_tick_time_point_ - previous_tick_time_point;
};

typename ClockType::timepoint last_tick_time_point_;
};



struct Body{
Color color;
};



// dynamic library to get ui description and doesn't contain the actual contents of the engine itself.
// The engine is always loaded but gets ui description from DLL.
// It's okay to use virtual inheritance here since it is not in a real-time loop and it only gets the UI desription

// the whole 2d ui performs only one flushandsubmit call to skia

namespace ui{
inline namespace desc{

struct Screen{
	size_t width;
	size_t height;
	bool portrait_mode;

	auto aspect_ratio();
};

struct Context{
	Screen screen;
};


struct InputsHandler{
enum class Keys{
Up,
Down,
Left,
Right,
...
} keys;
struct Cursor {};

};

struct Widget{

virtual void draw(Context const & context) {
struct CanvasBackend; // in exported header file
// in internal impl => reinterpret_cast to SkCanvas. with this the user won't have to interact with the canvas backend,
// only we get to do that and we don't have to duplicate the canvas implementation
	
using CanvasBackend =  CanvasBackendT*;
	
	VLK_ENSURE(vlk::canvas_backend() == CanvasBackendType::Skia);

	CanvasBackend canvas_backend = context.canvas_backend();
	SkCanvas& canvas = *reinterpret_cast<SkiaCanvas*>(canvas_backend);
	auto& rect = canvas.rect(...);
	rect.text("Empty Widget", 0, 0);
};

private:
	Position position_;
};

}; // namespace desc
}; // namespace ui


Same goes for 3D?

extern "C" void* vlk_get_func(char const* name){
// get widgets description
// get whole 2d context information required for the engine
};


virtual struct StatefulWidget{

bool is_dirty();

};

virtual struct StatelessWidget{
};


