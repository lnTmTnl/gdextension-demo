#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string.hpp"

#include <grpcpp/grpcpp.h>

class TestGRPC: public godot::Node{
    GDCLASS(TestGRPC, godot::Node)
protected:
    static void _bind_methods();
private:
    godot::String server_address = "localhost:50051";
    
    godot::String get_server_address() const;
    void set_server_address(const godot::String &new_address);
    
    godot::String say_hello_to_server(const godot::String &name);
};
