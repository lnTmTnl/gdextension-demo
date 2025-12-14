#!/usr/bin/env python
import os
import sys
import glob

from methods import print_error

# Function to recursively find .cpp files in the given directories
def find_sources(dirs, exts):
    """
    Recursively searches the specified directories for .cpp files.
    
    Args:
        dirs (list): List of directory paths to search.
        exts (list): List of file extensions that are acceptable and should contain C++ code.
    Returns:
        list: List of full paths to .cpp files found.
    """
    sources = []
    for dir in dirs:
        for root, _, files in os.walk(dir):
            for file in files:
                if any(file.endswith(ext) for ext in exts):
                    sources.append(os.path.join(root, file))
    return sources

# Configuration
libname = "demoplugin"
projectdir = "test_project"

# Set up the environment
env = Environment(tools=["default"], PLATFORM="")

# Custom configuration file
customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

# Define GDExtension-specific options
opts = Variables(customs, ARGUMENTS)
opts.Add('source_dirs', 'List of source directories (comma-separated)', 'src') # Directory for source files
opts.Add('source_exts', 'List of source file extensions (comma-separated)', '.cpp,.c,.cc,.cxx') 
opts.Add('include_dirs', 'List of include directories (comma-separated)', 'src') # Directory for headers - some might want to create a separate include directory
opts.Add('doc_output_dir', 'Directory for documentation output', 'gen')
opts.Add('precision', 'Floating-point precision (single or double)', 'single')  # Default to single
opts.Add('bundle_id_prefix', 'Bundle identifier prefix (reverse-DNS format)', 'com.gdextension')  # Default prefix
opts.Add(EnumVariable(
    'threads',
    'Enable threads for web builds',
    'no',  # default
    allowed_values=('yes', 'no', 'true', 'false')
))

# Build profiles can be used to decrease compile times.
# You can either specify "disabled_classes", OR
# explicitly specify "enabled_classes" which disables all other classes.

is_2d_profile_used = True
is_3d_profile_used = False
is_custom_profile_used = False
if is_2d_profile_used:
    env["build_profile"] = "2d_build_profile.json"
elif is_3d_profile_used:
    env["build_profile"] = "3d_build_profile.json"
elif is_custom_profile_used:
    env["build_profile"] = "build_profile.json"

# Update the environment with the options
opts.Update(env)

# Generate help text for the options
Help(opts.GenerateHelpText(env))

# Check for godot-cpp submodule
if not (os.path.isdir("godot-cpp") and os.listdir("godot-cpp")):
    print_error("""godot-cpp is not available within this folder, as Git submodules haven't been initialized.
Run the following command to download godot-cpp:

    git submodule update --init --recursive""")
    sys.exit(1)

# Include godot-cpp SConstruct, passing all command-line arguments
env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

# Add gRPC configuration
env.Append(CPPPATH=[
    '/home/lnTmTnl/.local/include',
    '/usr/include',
    '/home/lnTmTnl/gRPC/grpc/third_party/protobuf/src'  # gRPC's protobuf headers
])
# 使用pkg-config获取gRPC库依赖
import subprocess

# 设置PKG_CONFIG_PATH环境变量
pkg_config_path = '/home/lnTmTnl/.local/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig'
env['ENV']['PKG_CONFIG_PATH'] = pkg_config_path

# 获取编译标志
try:
    cflags = subprocess.check_output(
        ['pkg-config', '--cflags', 'grpc++', 'absl_flags', 'absl_flags_parse', 'absl_log_initialize'],
        env={'PKG_CONFIG_PATH': pkg_config_path}
    ).decode().strip().split()
    env.Append(CCFLAGS=cflags)
except Exception as e:
    print(f"Warning: Failed to get pkg-config cflags: {e}")

# 获取链接标志（静态链接）
try:
    # 定义PROTOBUF_ABSL_DEPS（来自gRPC示例）
    protobuf_absl_deps = [
        'absl_absl_check', 'absl_absl_log', 'absl_algorithm', 'absl_base',
        'absl_bind_front', 'absl_bits', 'absl_btree', 'absl_cleanup',
        'absl_cord', 'absl_core_headers', 'absl_debugging', 'absl_die_if_null',
        'absl_dynamic_annotations', 'absl_flags', 'absl_flat_hash_map',
        'absl_flat_hash_set', 'absl_function_ref', 'absl_hash', 'absl_layout',
        'absl_log_initialize', 'absl_log_severity', 'absl_memory',
        'absl_node_hash_map', 'absl_node_hash_set', 'absl_optional',
        'absl_span', 'absl_status', 'absl_statusor', 'absl_strings',
        'absl_synchronization', 'absl_time', 'absl_type_traits',
        'absl_utility', 'absl_variant'
    ]
    
    # 构建pkg-config参数
    pkg_config_args = ['pkg-config', '--libs', '--static', 'protobuf', 'grpc++']
    pkg_config_args.extend(['absl_flags', 'absl_flags_parse', 'absl_log_initialize'])
    pkg_config_args.extend(protobuf_absl_deps)
    
    ldflags = subprocess.check_output(
        pkg_config_args,
        env={'PKG_CONFIG_PATH': pkg_config_path}
    ).decode().strip().split()
    
    # 添加必要的链接标志
    ldflags.extend(['-pthread', '-Wl,--no-as-needed', '-ldl', '-lrt'])
    
    # 在pkg-config的静态库周围添加-Wl,--whole-archive包装器
    # 包裹absl库和re2库，同时去重
    new_ldflags = []
    in_static_section = False
    seen_libs = set()
    
    for flag in ldflags:
        # 去重：跳过已经见过的库标志
        if flag.startswith('-l') and flag in seen_libs:
            continue
        
        if flag.startswith('-l'):
            seen_libs.add(flag)
        
        # 检查是否是absl、re2或address_sorting静态库
        if flag.startswith('-labsl_') or flag == '-lre2' or flag == '-laddress_sorting':
            if not in_static_section:
                new_ldflags.append('-Wl,--whole-archive')
                in_static_section = True
        elif in_static_section:
            new_ldflags.append('-Wl,--no-whole-archive')
            in_static_section = False
        
        new_ldflags.append(flag)
    
    if in_static_section:
        new_ldflags.append('-Wl,--no-whole-archive')
    
    # 检查是否有libupb.a，如果有则使用-lupb替换所有-lupb_*子库
    if os.path.exists('/home/lnTmTnl/.local/lib/libupb.a'):
        # 过滤掉所有-lupb_*子库，添加-lupb
        filtered_ldflags = []
        upb_added = False
        
        for flag in new_ldflags:
            if flag.startswith('-lupb_'):
                # 跳过upb子库
                continue
            elif flag == '-Wl,--whole-archive' or flag == '-Wl,--no-whole-archive':
                # 保留包装器标志
                filtered_ldflags.append(flag)
            elif flag.startswith('-labsl_') or flag == '-lre2' or flag == '-laddress_sorting':
                # 保留absl库、re2库和address_sorting库
                filtered_ldflags.append(flag)
            else:
                # 其他标志
                filtered_ldflags.append(flag)
        
        # 在absl部分后添加-lupb
        for i, flag in enumerate(filtered_ldflags):
            if flag == '-Wl,--no-whole-archive':
                # 在absl部分结束后添加-lupb
                filtered_ldflags.insert(i, '-lupb')
                upb_added = True
                break
        
        if not upb_added:
            # 如果没有找到-Wl,--no-whole-archive，在末尾添加
            filtered_ldflags.append('-lupb')
        
        new_ldflags = filtered_ldflags
    
    # 添加到LINKFLAGS
    env.Append(LINKFLAGS=new_ldflags)
    
    # 同时添加到LIBS，确保链接器能找到库
    env.Append(LIBS=['grpc++', 'grpc', 'gpr', 'protobuf', 'cares', 'ssl', 'crypto', 'z', 'pthread', 'dl', 'rt'])
    
except Exception as e:
    print(f"Warning: Failed to get pkg-config libs: {e}")
    # 回退到手动配置
    env.Append(LIBPATH=['/home/lnTmTnl/.local/lib', '/usr/lib/x86_64-linux-gnu'])
    env.Append(LIBS=['grpc++', 'grpc', 'gpr', 'protobuf', 'cares', 'ssl', 'crypto', 'z', 'pthread', 'dl', 'rt'])

# 添加必要的C++标志
env.Append(CCFLAGS=['-std=c++17', '-pthread', '-D_REENTRANT', '-fexceptions'])

# Process GDExtension-specific options
source_dirs = env['source_dirs'].split(',')   # Convert comma-separated string to list
source_exts = env['source_exts'].split(',')   # Convert comma-separated string to list
include_dirs = env['include_dirs'].split(',') # Convert comma-separated string to list
include_dirs.append('src/gen')  # Add generated protobuf headers directory
doc_output_dir = env['doc_output_dir']        # Directory for documentation output
precision = env.get('precision', 'single')     # Ensure precision defaults to single
bundle_id_prefix = env.get('bundle_id_prefix', 'com.gdextension')  # Ensure prefix defaults to com.gdextension

# Append include directories to CPPPATH
env.Append(CPPPATH=include_dirs)

# Find all .cpp files recursively in the specified source directories
sources = find_sources(source_dirs, source_exts)

# Remove generated protobuf files from sources list (they will be added explicitly)
gen_files_to_exclude = ['src/gen/helloworld.pb.cc', 'src/gen/helloworld.grpc.pb.cc']
sources = [s for s in sources if s not in gen_files_to_exclude]

# Add generated protobuf source files explicitly
gen_sources = ['src/gen/helloworld.pb.cc', 'src/gen/helloworld.grpc.pb.cc']
sources.extend([s for s in gen_sources if os.path.exists(s)])

# Handle documentation generation if applicable
if env.get("target") in ["editor", "template_debug"]:
    try:
        doc_output_file = os.path.join(doc_output_dir, 'doc_data.gen.cpp')
        doc_data = env.GodotCPPDocData(doc_output_file, source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

# Determine suffixes based on env (align with godot-cpp conventions)
arch_suffix = f".{env['arch']}" if env['arch'] and env['arch'] != 'universal' else ''
# Normalize 'threads' to a lowercase string, defaulting to 'no'
threads_val = str(env.get('threads', 'no')).strip().lower()

# Determine if '.threads' suffix should be added
threads_suffix = '.threads' if env['platform'] == 'web' and threads_val in ('yes', 'true') else ''

suffix = f".{env['platform']}.{env['target']}{arch_suffix}{threads_suffix}.{precision}"
lib_filename = f"{env.subst('$SHLIBPREFIX')}{libname}{suffix}{env.subst('$SHLIBSUFFIX')}"

# Generate Info.plist content for macOS and iOS
def generate_info_plist(platform, target, precision):
    framework_name = f"lib{libname}.{platform}.{target}.{precision}"
    bundle_id = f"{bundle_id_prefix}.{libname}"  # Use configurable prefix
    if platform == 'macos':
        return f"""<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>{framework_name}</string>
    <key>CFBundleIdentifier</key>
    <string>{bundle_id}</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>{framework_name}</string>
    <key>CFBundlePackageType</key>
    <string>FMWK</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0.0</string>
    <key>CFBundleSupportedPlatforms</key>
    <array>
        <string>MacOSX</string>
    </array>
    <key>CFBundleVersion</key>
    <string>1.0.0</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.12</string>
</dict>
</plist>"""
    else:  # ios
        return f"""<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>{framework_name}</string>
    <key>CFBundleIdentifier</key>
    <string>{bundle_id}</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>{framework_name}</string>
    <key>CFBundlePackageType</key>
    <string>FMWK</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0.0</string>
    <key>CFBundleSupportedPlatforms</key>
    <array>
        <string>iPhoneOS</string>
    </array>
    <key>CFBundleVersion</key>
    <string>1.0.0</string>
    <key>LSMinimumSystemVersion</key>
    <string>12.0</string>
</dict>
</plist>"""

# Function to write Info.plist content to a file
def write_info_plist(target, source, env, plist_content):
    with open(target[0].abspath, 'w') as f:
        f.write(plist_content)

# Build the shared library and create frameworks
library = None
install_source = None
if env['platform'] in ['macos', 'ios']:
    # Build the shared library first in bin/{platform}
    temp_lib = env.SharedLibrary(
        f"bin/{env['platform']}/{lib_filename}",
        source=sources
    )
    if env['platform'] == 'macos':
        # Ensure universal if specified
        if env.get('arch') != 'universal':
            env['arch'] = 'universal'  # Fallback to universal for macOS
        framework_name = f"lib{libname}.macos.{env['target']}.{precision}.framework"
        framework_binary = f"lib{libname}.macos.{env['target']}.{precision}"
        framework_dir = f"bin/{env['platform']}/{framework_name}"
        # Create Info.plist file
        plist_file = f"{framework_dir}/Info.plist"
        env.Command(
            plist_file,
            [],
            lambda target, source, env: write_info_plist(target, source, env, generate_info_plist('macos', env['target'], precision))
        )
        # Create the .framework structure in bin/macos
        library = env.Command(
            f"{framework_dir}/{framework_binary}",
            temp_lib,
            [
                f"mkdir -p {framework_dir}",
                f"cp $SOURCE $TARGET",
                f"rm -f bin/{env['platform']}/{lib_filename}"  # Clean up temporary .dylib
            ]
        )
        env.Depends(library, plist_file)  # Ensure Info.plist is created before the framework binary
        install_source = framework_dir
    else:  # iOS
        # Single arm64 build
        if not env.get('arch'):
            env['arch'] = 'arm64'
        temp_framework_name = f"lib{libname}.ios.{env['target']}.{precision}.framework"
        framework_binary = f"lib{libname}.ios.{env['target']}.{precision}"
        framework_name = f"lib{libname}.ios.{env['target']}.{precision}.xcframework"
        temp_framework_dir = f"bin/{env['platform']}/{temp_framework_name}"
        # Create Info.plist file
        plist_file = f"{temp_framework_dir}/Info.plist"
        env.Command(
            plist_file,
            [],
            lambda target, source, env: write_info_plist(target, source, env, generate_info_plist('ios', env['target'], precision))
        )
        # Create temporary .framework in bin/ios
        temp_framework = env.Command(
            f"{temp_framework_dir}/{framework_binary}",
            temp_lib,
            [
                f"mkdir -p {temp_framework_dir}",
                f"cp $SOURCE $TARGET"
            ]
        )
        env.Depends(temp_framework, plist_file)  # Ensure Info.plist is created before the framework binary
        # Create .xcframework in bin/ios
        library = env.Command(
            f"bin/{env['platform']}/{framework_name}",
            temp_framework,
            [
                f"xcodebuild -create-xcframework -framework {temp_framework_dir} -output $TARGET",
                f"rm -rf {temp_framework_dir}",  # Clean up temporary .framework
                f"rm -f bin/{env['platform']}/{lib_filename}"  # Clean up temporary .dylib
            ]
        )
        install_source = f"bin/{env['platform']}/{framework_name}"
else:
    # For other platforms, build a single shared library
    library = env.SharedLibrary(
        f"bin/{env['platform']}/{lib_filename}",
        source=sources
    )
    install_source = library

# Install the library to test_project
install_dir = f"{projectdir}/{libname}/bin/{env['platform']}/"
copy = env.Install(install_dir, source=install_source)

# Set default targets
default_args = [library, copy]
Default(*default_args)