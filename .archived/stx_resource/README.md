# Resource Management Library



//
// I often find myself needing to group resources and avoiding needless
// allocations when I'm certain the resource will both be valid.
//
//
// I need something like: shared<Resource>
//
// i.e. a curl http client:
//
// struct Resource{
//  Parent * parent; // parent handle
//  Child * child; // child handle
// };
//
// Requirement: child must be deleted before parent.
//
// I'd still have an allocation for the `Resource` struct or the individual
// objects no matter what approach I take with `shared_ptr`. (aliasing
// constructors, custom deleters, etc ).
//
//
// Approach one: two shared_ptrs
//
// struct Resource{
//  shared_ptr<Parent> parent{create_parent(), make_parent_deleter()};
//  shared_ptr<Child> child{parent->create_child(), make_child_deleter()};
// };
//
// Problem with approach: two control block allocations, and we have to make
// sure the child is deleted before the parent, therefore we need to enforce
// that the child is placed after parent. and we'd still need to make sure the
// refcount of the child does not exceed that of the parent.
//
// Approach two: aliasing constructors? No way!
//
// struct Resource{
// shared_ptr<Parent> parent{create_parent(),  make_parent_deleter() };
// shared_ptr<Child> child{parent, create_child(), child_deleter() ???????? };
// };
//
// Problem with approach: two control blocks will still be needed due to
// the new deleter, shared_ptr does not support this pattern
//
//
//
//
//
//
//
//
//
//
//
//
//
// // one control block, one
// allocates a control block struct
//
//
// CurlEasyHandle{
//  CURL * easy;
//  shared_ptr<CURLM> multi;
// };
//
// shared_ptr<CurlEasyHandle> easy { new CurlEasyHandle{curl_easy_init(),
//              multi}, CurlEasyHandle::Deleter{}  };  // allocates memory for
//              the handle
// struct and allocates another control block + intentional ref-count
//
//
//
//
//
//
//
//
//
// I can't reasonably do this using `shared_ptr` since I'd have two allocations
// for two control blocks plus one ref-count plus one handle struct allocation
// .i.e.
//
// Intuitively, all I really need here is just one control block and one deleter
// along with the `ClientHandle` struct. that's not really possible with
// shared_ptr as it assumes the resource will always be a pointer just as its
// name implies.
//
// A number of APIs also use integers or structs to represent resource handles.
// i.e. Vulkan's Queue Families, Vulkan's Descriptor sets, OpenGL's horrible
// integer handles and many others.
//
//
//
//
//
//
//
//
//
//
// child can be created from parent at any point in time and it is not created
// along with the parent. and the child needs to be deleted before the parent is
// deleted. we'd somehow need to refcount the parent along with the child.
//
//
//
//
//
//
//
//
//
