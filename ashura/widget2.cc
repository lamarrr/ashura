
struct EventSystem{
// message passing
};


struct Box;
struct Flex {
    // properties for layout
};

// widgets and systems shouldn't be adding passes, they should be using from
// what is available
//
// hit-testing
//
//
struct FlexSystem {
    // react to events, reject and propagate to parent or whatever
    // process_spatial events?
    void process_events();
    // layout widgets, multi-pass resolution
    // allocate for children then fit around children
    //
    //
    // repeatedly call layout on widgets until layout is solved?
    // this is more complicated than the node one
    //
    //
    void layout();
    // add or remove rendering commands from the passes
    void tick();
    void serialize();
    void deserialize();
};

struct WidgetSystem {
    char const name[256];
    void (*layout)();
    void (*tick)();
    void (*serialize)();
    void (*deserialize)();
};

// multi-pass widget layout. go down, come back up
struct Widget {
    int level;
    int parent;
    int next_sibling;
    int first_child;
    int system;
    float allocation_size;
    float size;
    float allocation_offset;
    float offset;
};

struct System {
    Widget* widgets;
    Box* bounding_boxes;
    int num_widgets;

    int add_child();
    void* get_child(int system, int child);
    void update_child(void*);
    void remove_child(int system, int child);
};

int main() {}