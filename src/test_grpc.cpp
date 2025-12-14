#include "test_grpc.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "godot_cpp/core/property_info.hpp"
#include "godot_cpp/variant/variant.hpp"

#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "gen/helloworld.grpc.pb.h"

using namespace godot;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

String TestGRPC::say_hello_to_server(const String &name)
{
    std::string server_addr = server_address.utf8().get_data();
    std::string client_name = name.utf8().get_data();
    
    try {
        // Create a channel
        auto channel = grpc::CreateChannel(server_addr, grpc::InsecureChannelCredentials());
        
        // Create the stub
        std::unique_ptr<Greeter::Stub> stub = Greeter::NewStub(channel);
        
        // Prepare the request
        HelloRequest request;
        request.set_name(client_name);
        
        // Prepare the response
        HelloReply reply;
        
        // Create client context
        ClientContext context;
        
        // Make the RPC call
        Status status = stub->SayHello(&context, request, &reply);
        
        // Check status
        if (status.ok()) {
            return String(reply.message().c_str());
        } else {
            std::string error_msg = "RPC failed: " + std::to_string(status.error_code()) + 
                                   ": " + status.error_message();
            return String(error_msg.c_str());
        }
    } catch (const std::exception& e) {
        return String("Error: ") + String(e.what());
    }
}

void TestGRPC::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("say_hello_to_server", "name"), &TestGRPC::say_hello_to_server);

    ClassDB::bind_method(D_METHOD("get_server_address"), &TestGRPC::get_server_address);
    ClassDB::bind_method(D_METHOD("set_server_address"), &TestGRPC::set_server_address);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "server_address"), "set_server_address", "get_server_address");
}

String TestGRPC::get_server_address() const
{
    return server_address;
}

void TestGRPC::set_server_address(const String &new_address)
{
    server_address = new_address;
}
