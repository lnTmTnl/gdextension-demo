extends Control

var grpc: TestGRPC

@export var server_address_input: LineEdit
@export var name_to_send_input: LineEdit
@export var reply_msg_display: Label

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	grpc = TestGRPC.new()
	grpc.server_address = server_address_input.text

func start_grpc_server(): 
	pass

func send_grpc_msg():
	reply_msg_display.text = "Reply: " + grpc.say_hello_to_server(name_to_send_input.text)
