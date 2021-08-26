




# Gesture Detector
We don't want this to be a separate widget.
the user should be able to add it as a member of the widget tree and then perform desired actions.

doesn't make sense to map all features of a platform to another, especially high-level ones

# Detectors in general

```cpp
using namespace vlk::sched;

struct IOSWidget: Widget{

IOSGestureDetector detector;

void tick(nanoseconds, Events const& events,  System const& system){
IOSGesture gesture = detector.route(events);
if(gesture.panned() || gesture.zoomed()) { flip_button(); }

TaskScheduler * scheduler = system.get("Builtin.Scheduler").unwrap()->as<TaskScheduler>().unwrap();

HttpClient * http = system.get("Builtin.Http").unwrap()->as<HttpClient>().unwrap();

// system.http
// the subsystems will outlive the widgets

// returns the passed in future, so no data race
Future processed = await(*scheduler, http->get("...", "..."),  [](Future<Response> && response){
    return process(response.copy().unwrap());
});




}



};

```


# we need good event routing to the widgets, raw events


events.keyboard
events.mouse
events.close_requested ? should still close??

