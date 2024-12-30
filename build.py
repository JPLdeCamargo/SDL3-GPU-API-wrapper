import os
import subprocess
import sys

def run_command(command):
    """Run a shell command and handle errors."""
    try:
        subprocess.check_call(command, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Error while running command: {e}")
        sys.exit(1)

def compile_shader(shader_path, output_file, shader_type):
    """Compile the shader and create a single header file at output_file."""
    
    # Ensure the shader path exists
    if not os.path.exists(shader_path):
        print(f"Error: Shader file {shader_path} does not exist.")
        sys.exit(1)
    
    # Ensure the output directory exists
    output_dir = os.path.dirname(output_file)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Validate shader type
    if shader_type not in ["vert", "frag"]:
        print(f"Error: Unsupported shader type '{shader_type}'. Use 'vert' or 'frag'.")
        sys.exit(1)

    # Run glslangValidator to compile shaders into SPIR-V
    print(f"Compiling {shader_type} shader into SPIR-V...")
    output_spv = f"{output_dir}/shader.{shader_type}.spv"
    run_command(f"glslangValidator {shader_path} -V -S {shader_type} -o {output_spv} --quiet")

    # Convert the SPIR-V binaries to header files
    print("Generating header files from SPIR-V binaries...")
    output_header = f"{output_dir}/shader.{shader_type}.h"
    run_command(f"xxd -i {output_spv} | perl -w -p -e 's/\\Aunsigned /const unsigned /;' > {output_header}")

    # Combine into a single header file
    print("Combining shader headers into one file...")
    with open(output_file, 'w') as f:
        f.write("#ifndef TESTGPU_SPIRV_H\n")
        f.write("#define TESTGPU_SPIRV_H\n\n")
        
        f.write(f"// {shader_type.capitalize()} Shader\n")
        with open(output_header, 'r') as header_file:
            f.write(header_file.read())

        f.write("\n#endif // TESTGPU_SPIRV_H\n")

    # Clean up intermediate files
    print("Cleaning up intermediate files...")
    os.remove(output_header)
    os.remove(output_spv)

    print(f"Shader header file created: {output_file}")

def main():
    # Ensure correct number of arguments
    if len(sys.argv) != 4:
        print("Usage: python compile_shaders.py <shader-path> <output-file> <shader-type>")
        sys.exit(1)

    # Get input arguments
    shader_path = sys.argv[1]
    output_file = sys.argv[2]
    shader_type = sys.argv[3]

    # Compile the shader
    compile_shader(shader_path, output_file, shader_type)

if __name__ == "__main__":
    main()

