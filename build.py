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

def compile_shader(shader_path, output_file):
    """Compile the shader and create a single header file at output_file."""
    
    # Ensure the shader path exists
    if not os.path.exists(shader_path):
        print(f"Error: Shader file {shader_path} does not exist.")
        sys.exit(1)
    
    # Ensure the output directory exists
    output_dir = os.path.dirname(output_file)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Run glslangValidator to compile shaders into SPIR-V
    print("Compiling shaders into SPIR-V...")
    run_command(f"glslangValidator {shader_path} -V -S vert -o {output_dir}/cube.vert.spv --quiet -DVERTEX")
    run_command(f"glslangValidator {shader_path} -V -S frag -o {output_dir}/cube.frag.spv --quiet")

    # Convert the SPIR-V binaries to header files
    print("Generating header files from SPIR-V binaries...")
    run_command(f"xxd -i {output_dir}/cube.vert.spv | perl -w -p -e 's/\\Aunsigned /const unsigned /;' > {output_dir}/cube.vert.h")
    run_command(f"xxd -i {output_dir}/cube.frag.spv | perl -w -p -e 's/\\Aunsigned /const unsigned /;' > {output_dir}/cube.frag.h")

    # Combine into a single header file
    print("Combining shader headers into one file...")
    with open(output_file, 'w') as f:
        f.write("#ifndef TESTGPU_SPIRV_H\n")
        f.write("#define TESTGPU_SPIRV_H\n\n")
        
        f.write("// Vertex Shader\n")
        with open(f"{output_dir}/cube.vert.h", 'r') as vert_file:
            f.write(vert_file.read())

        f.write("\n// Fragment Shader\n")
        with open(f"{output_dir}/cube.frag.h", 'r') as frag_file:
            f.write(frag_file.read())

        f.write("\n#endif // TESTGPU_SPIRV_H\n")

    # Clean up intermediate files
    print("Cleaning up intermediate files...")
    os.remove(f"{output_dir}/cube.vert.h")
    os.remove(f"{output_dir}/cube.frag.h")
    os.remove(f"{output_dir}/cube.vert.spv")
    os.remove(f"{output_dir}/cube.frag.spv")

    print(f"Shader header file created: {output_file}")

def main():
    # Ensure correct number of arguments
    if len(sys.argv) != 3:
        print("Usage: python compile_shaders.py <shader-path> <output-file>")
        sys.exit(1)

    # Get input arguments
    shader_path = sys.argv[1]
    output_file = sys.argv[2]

    # Compile the shader
    compile_shader(shader_path, output_file)

if __name__ == "__main__":
    main()
